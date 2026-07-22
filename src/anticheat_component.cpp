#include "anticheat_component.hpp"

#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <utility>

#include "detection/faker5_check.hpp"
#include "detection/memory_check.hpp"
#include "detection/mobile_check.hpp"
#include "detection/poison_check.hpp"
#include "detection/raknet_check.hpp"
#include "detection/signatures.hpp"
#include "detection/version_check.hpp"
#include "player_data.hpp"

#ifdef AC_ENABLE_PAWN
#include "natives.hpp"
#endif

using std::chrono::milliseconds;

AntiCheatComponent* AntiCheatComponent::getInstance()
{
	if (instance_ == nullptr)
		instance_ = new AntiCheatComponent();
	return instance_;
}

// lifecycle

void AntiCheatComponent::onLoad(ICore* c)
{
	core_ = c;

	config_.load("components/anticheat.cfg");

	// per-session random offset range for the memory checks (reference goffset): two bytes >= 32 apart, aligned to 4.
	{
		std::uniform_int_distribution<int> d(0, 255);
		int a, b;
		do
		{
			a = d(rng_);
			b = d(rng_);
			if (a > b)
				std::swap(a, b);
		} while (b - a < 32);
		offLo_ = a & 0xFC;
		offHi_ = b & 0xFC;
		if (offHi_ <= offLo_)
			offHi_ = offLo_ + 4;
	}

	enforcement_.reset(new Enforcement(core_, config_, scheduler_));
	buildModules();

	IPlayerPool& players = core_->getPlayers();
	players.getPlayerConnectDispatcher().addEventHandler(this);
	players.getPlayerSpawnDispatcher().addEventHandler(this);
	players.getPlayerCheckDispatcher().addEventHandler(this);
	core_->getEventDispatcher().addEventHandler(this); // ontick

	log("Anti-Cheat v1.0.0 loaded. modules: memory=%d version=%d poison=%d mobile=%d raknet=%d faker5=%d | log_only=%d",
		config_.moduleMemory(), config_.moduleVersion(), config_.modulePoison(),
		config_.moduleMobile(), config_.moduleRaknet(), config_.moduleFaker5(), config_.logOnly());
}

void AntiCheatComponent::buildModules()
{
	modules_.clear();

	// mobile first: it sets the mobile-whitelist flag during onconnect so the other modules skip verified mobile clients.
	MobileCheck* mobile = new MobileCheck(*this);
	mobileModule_ = mobile;
	modules_.push_back(std::unique_ptr<IDetectionModule>(mobile));

	modules_.push_back(std::unique_ptr<IDetectionModule>(new MemoryCheck(*this)));
	modules_.push_back(std::unique_ptr<IDetectionModule>(new VersionCheck(*this)));
	modules_.push_back(std::unique_ptr<IDetectionModule>(new PoisonCheck(*this)));
	modules_.push_back(std::unique_ptr<IDetectionModule>(new FakeR5Check(*this)));

	RaknetCheck* raknet = new RaknetCheck(*this);
	raknetModule_ = raknet;
	modules_.push_back(std::unique_ptr<IDetectionModule>(raknet));
}

void AntiCheatComponent::onInit(IComponentList* components)
{
#ifdef AC_ENABLE_PAWN
	pawn_ = components->queryComponent<IPawnComponent>();
	if (pawn_)
	{
		ac_pawn::onInit(components, pawn_);
		pawn_->getEventDispatcher().addEventHandler(this);
	}
#else
	(void)components;
#endif
}

void AntiCheatComponent::onReady()
{
	if (rpcRegistered_)
		return;
	// register the raw-raknet handlers only if enabled: rpc 25 (join checksum) and rpc 103 (check-response anomaly) on every network.
	if (config_.moduleMobile() && mobileModule_)
		core_->addPerRPCInEventHandler<25>(mobileModule_);
	if (config_.moduleRaknet() && raknetModule_)
		core_->addPerRPCInEventHandler<103>(raknetModule_);
	rpcRegistered_ = true;
}

void AntiCheatComponent::onFree(IComponent* component)
{
#ifdef AC_ENABLE_PAWN
	if (component == pawn_)
		pawn_ = nullptr;
#else
	(void)component;
#endif
}

void AntiCheatComponent::free()
{
	delete this;
}

void AntiCheatComponent::reset()
{
	scheduler_.clear();
}

AntiCheatComponent::~AntiCheatComponent()
{
	if (!core_)
		return;

	if (rpcRegistered_)
	{
		if (mobileModule_)
			core_->removePerRPCInEventHandler<25>(mobileModule_);
		if (raknetModule_)
			core_->removePerRPCInEventHandler<103>(raknetModule_);
	}

	IPlayerPool& players = core_->getPlayers();
	players.getPlayerConnectDispatcher().removeEventHandler(this);
	players.getPlayerSpawnDispatcher().removeEventHandler(this);
	players.getPlayerCheckDispatcher().removeEventHandler(this);
	core_->getEventDispatcher().removeEventHandler(this);
#ifdef AC_ENABLE_PAWN
	if (pawn_)
		pawn_->getEventDispatcher().removeEventHandler(this);
#endif
}

// player events

void AntiCheatComponent::onPlayerConnect(IPlayer& player)
{
	if (!enabled() || player.isBot())
		return;

	PlayerACData* data = dataOf(player);
	if (!data)
	{
		data = new PlayerACData();
		if (!player.addExtension(data, true))
		{
			delete data;
			data = dataOf(player);
		}
	}
	else
	{
		data->reset();
	}
	if (!data)
		return;

	for (auto& m : modules_)
	{
		if (m->enabled(config_))
			m->onConnect(player, *data);
	}

	// schedule the "evaluate everything collected" pass, like the reference's checkplayer timer.
	const int id = player.getID();
	scheduler_.after(id, milliseconds(config_.evaluateDelayMs()), [this, id]()
		{
			IPlayer* p = core_->getPlayers().get(id);
			if (!p)
				return;
			PlayerACData* d = dataOf(*p);
			if (d && !d->evaluated)
			{
				d->evaluated = true;
				evaluatePlayer(*p, *d);
			}
		});
}

void AntiCheatComponent::onPlayerDisconnect(IPlayer& player, PeerDisconnectReason reason)
{
	(void)reason;
	scheduler_.cancelOwner(player.getID());
	// the extension is auto-freed by the player pool (addextension autodelete).
}

void AntiCheatComponent::onPlayerSpawn(IPlayer& player)
{
	if (!enabled())
		return;
	PlayerACData* data = dataOf(player);
	if (!data || data->spawnChecked)
		return;
	data->spawnChecked = true;

	const int id = player.getID();
	std::uniform_int_distribution<int> jitter(0, 400);
	const int delay = config_.spawnRecheckDelayMs() + jitter(rng_);
	scheduler_.after(id, milliseconds(delay), [this, id]()
		{
			IPlayer* p = core_->getPlayers().get(id);
			if (!p)
				return;
			PlayerACData* d = dataOf(*p);
			if (!d)
				return;
			for (auto& m : modules_)
			{
				if (m->enabled(config_))
					m->onSpawn(*p, *d);
			}
		});
}

void AntiCheatComponent::onClientCheckResponse(IPlayer& player, int actionType, int address, int results)
{
	if (!enabled())
		return;
	PlayerACData* data = dataOf(player);
	if (!data)
		return;
	for (auto& m : modules_)
	{
		if (m->enabled(config_))
			m->onCheckResponse(player, *data, actionType, address, results);
	}
}

void AntiCheatComponent::onTick(Microseconds elapsed, TimePoint now)
{
	(void)elapsed;
	(void)now;
	scheduler_.process();
}

// evaluation

void AntiCheatComponent::evaluatePlayer(IPlayer& player, PlayerACData& data)
{
	const StringView version = player.getClientVersionName();
	const std::string versionStr(version.data(), version.size());
	const bool allowed = data.mobilePlayer || config_.isAllowedVersion(versionStr.c_str());
	if (!allowed)
	{
		rejectVersion(player, version);
		return;
	}
	for (auto& m : modules_)
	{
		if (m->enabled(config_))
			m->onEvaluate(player, data);
	}

	// everything the modules flagged gets reported once, here.
	for (int c = Cheat_None + 1; c < Cheat_Max; ++c)
	{
		if (data.pending[c])
			report(player, data, static_cast<CheatId>(c));
	}
}

void AntiCheatComponent::rejectVersion(IPlayer& player, StringView version)
{
	const StringView name = player.getName();
	log("player %.*s uses disallowed client version '%.*s' - rejecting", PRINT_VIEW(name), PRINT_VIEW(version));
	if (config_.logOnly())
		return;

	std::string list;
	for (const std::string& v : config_.allowedVersions())
	{
		if (!list.empty())
			list += ", ";
		list += v;
	}
	std::string msg = "[Anti-Cheat] Your client version is not allowed. Allowed: " + list;
	player.sendClientMessage(Colour::FromRGBA(0xFF0000AA), msg);

	const int id = player.getID();
	scheduler_.after(id, milliseconds(config_.kickDelayMs()), [this, id]()
		{
			if (IPlayer* p = core_->getPlayers().get(id))
				p->kick();
		});
}

// report policy

void AntiCheatComponent::report(IPlayer& player, PlayerACData& data, CheatId cheat)
{
	if (cheat <= Cheat_None || cheat >= Cheat_Max)
		return;
	if (data.exception[cheat] || data.reported[cheat])
		return;
	data.reported[cheat] = true;
	data.lastCheat = cheat;

	const signatures::CheatInfo& info = signatures::cheatInfo(cheat);
	const CheatAction action = config_.actionFor(cheat);
	const StringView name = player.getName();
	const int pid = player.getID();

#ifdef AC_ENABLE_PAWN
	// let the gamemode see it first and optionally suppress the built-in action.
	if (firePawnDetected(pid, static_cast<int>(cheat), static_cast<int>(action)) == 0)
	{
		log("detected %s on %.*s (id %d) - handled by script", info.name, PRINT_VIEW(name), pid);
		return;
	}
#endif

	log("detected %s on %.*s (id %d)", info.name, PRINT_VIEW(name), pid);

	if (config_.logOnly())
		return;

	switch (action)
	{
	case CheatAction::Ignore:
		break;
	case CheatAction::Warn:
		enforcement_->warn(player, info.name);
		break;
	case CheatAction::Kick:
		enforcement_->kick(player, info.name);
		break;
	case CheatAction::Ban:
		enforcement_->ban(player, info.name);
		break;
	}
}

// helpers

int AntiCheatComponent::nextMemOffset()
{
	const int span = (offHi_ - offLo_) / 4 + 1;
	std::uniform_int_distribution<int> d(0, span > 0 ? span - 1 : 0);
	return d(rng_) * 4 + offLo_;
}

PlayerACData* AntiCheatComponent::dataOf(IPlayer& player)
{
	return queryExtension<PlayerACData>(player);
}

PlayerACData* AntiCheatComponent::dataOf(int playerid)
{
	IPlayer* p = core_ ? core_->getPlayers().get(playerid) : nullptr;
	return p ? dataOf(*p) : nullptr;
}

void AntiCheatComponent::log(const char* fmt, ...)
{
	if (!core_)
		return;
	char buf[512];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	core_->printLn("[AC] %s", buf);
}

// native helpers

bool AntiCheatComponent::addException(int playerid, int cheatid)
{
	if (cheatid <= Cheat_None || cheatid >= Cheat_Max)
		return false;
	PlayerACData* d = dataOf(playerid);
	if (!d)
		return false;
	d->exception[cheatid] = true;
	return true;
}

bool AntiCheatComponent::removeException(int playerid, int cheatid)
{
	if (cheatid <= Cheat_None || cheatid >= Cheat_Max)
		return false;
	PlayerACData* d = dataOf(playerid);
	if (!d)
		return false;
	d->exception[cheatid] = false;
	return true;
}

bool AntiCheatComponent::isMobile(int playerid)
{
	PlayerACData* d = dataOf(playerid);
	return d && d->mobilePlayer;
}

int AntiCheatComponent::lastCheat(int playerid)
{
	PlayerACData* d = dataOf(playerid);
	return d ? static_cast<int>(d->lastCheat) : 0;
}

#ifdef AC_ENABLE_PAWN
void AntiCheatComponent::onAmxLoad(IPawnScript& script)
{
	ac_pawn::onAmxLoad(script);
}

void AntiCheatComponent::onAmxUnload(IPawnScript& script)
{
	(void)script;
}

int AntiCheatComponent::firePawnDetected(int playerid, int cheatid, int action)
{
	return pawn_ ? ac_pawn::fireDetected(pawn_, playerid, cheatid, action) : 1;
}
#endif
