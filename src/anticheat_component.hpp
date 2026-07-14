#pragma once

/*
 *  The anti-cheat component. Owns the config, scheduler, enforcement and the
 *  list of detection modules; receives player/network/tick events from the
 *  server and fans them out to the modules; and is the single place that turns
 *  a module's detection into an action (report()).
 */

#include <memory>
#include <random>
#include <vector>

#include <sdk.hpp>

#ifdef AC_ENABLE_PAWN
#include <Server/Components/Pawn/pawn.hpp>
#endif

#include "config.hpp"
#include "detection/module.hpp"
#include "enforcement.hpp"
#include "scheduler.hpp"

class PlayerACData;
class MobileCheck;
class RaknetCheck;

class AntiCheatComponent final
	: public IComponent
	, public PlayerConnectEventHandler
	, public PlayerSpawnEventHandler
	, public PlayerCheckEventHandler
	, public CoreEventHandler
	, public IACContext
#ifdef AC_ENABLE_PAWN
	, public PawnEventHandler
#endif
{
public:
	PROVIDE_UID(0xAC12CEA7C0DE0001);

	static AntiCheatComponent* getInstance();

	// ---- IComponent ----
	StringView componentName() const override { return "Anti-Cheat"; }
	SemanticVersion componentVersion() const override { return SemanticVersion(1, 0, 0, 0); }
	void onLoad(ICore* c) override;
	void onInit(IComponentList* components) override;
	void onReady() override;
	void onFree(IComponent* component) override;
	void free() override;
	void reset() override;

	// ---- player events ----
	void onPlayerConnect(IPlayer& player) override;
	void onPlayerDisconnect(IPlayer& player, PeerDisconnectReason reason) override;
	void onPlayerSpawn(IPlayer& player) override;
	void onClientCheckResponse(IPlayer& player, int actionType, int address, int results) override;

	// ---- core tick ----
	void onTick(Microseconds elapsed, TimePoint now) override;

	// ---- IACContext ----
	ICore* core() override { return core_; }
	const Config& config() const override { return config_; }
	Scheduler& scheduler() override { return scheduler_; }
	int nextMemOffset() override;
	void report(IPlayer& player, PlayerACData& data, CheatId cheat) override;

#ifdef AC_ENABLE_PAWN
	// ---- pawn script (dis)loads ----
	void onAmxLoad(IPawnScript& script) override;
	void onAmxUnload(IPawnScript& script) override;
#endif

	// ---- helpers used by the Pawn natives ----
	bool addException(int playerid, int cheatid);
	bool removeException(int playerid, int cheatid);
	bool isMobile(int playerid);
	int lastCheat(int playerid);
	void setEnabled(bool on) { runtimeEnabled_ = on; }
	bool enabled() const { return runtimeEnabled_ && config_.enabled(); }

	~AntiCheatComponent();

private:
	void buildModules();
	void evaluatePlayer(IPlayer& player, PlayerACData& data);
	void rejectVersion(IPlayer& player, StringView version);
	PlayerACData* dataOf(IPlayer& player);
	PlayerACData* dataOf(int playerid);
	void log(const char* fmt, ...);

	ICore* core_ = nullptr;
	Config config_;
	Scheduler scheduler_;
	std::unique_ptr<Enforcement> enforcement_;

	std::vector<std::unique_ptr<IDetectionModule>> modules_;
	// Non-owning views of the two raw-RakNet modules (also held in modules_);
	// needed to register their per-RPC handlers.
	MobileCheck* mobileModule_ = nullptr;
	RaknetCheck* raknetModule_ = nullptr;

	std::mt19937 rng_ { std::random_device {}() };
	int offLo_ = 0;
	int offHi_ = 4;

	bool runtimeEnabled_ = true;
	bool rpcRegistered_ = false;

#ifdef AC_ENABLE_PAWN
	IPawnComponent* pawn_ = nullptr;
	// Fire OnPlayerCheatDetected in every script. Returns 0 if a script asked us
	// to suppress the built-in action (by returning 0), 1 otherwise.
	int firePawnDetected(int playerid, int cheatid, int action);
#endif

	inline static AntiCheatComponent* instance_ = nullptr;
};
