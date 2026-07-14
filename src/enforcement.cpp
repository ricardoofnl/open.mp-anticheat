#include "enforcement.hpp"

#include <chrono>
#include <string>

#include "config.hpp"
#include "scheduler.hpp"

namespace
{
// colours from the reference filterscript.
const Colour kColError = Colour::FromRGBA(0xA01616FF);
const Colour kColOk = Colour::FromRGBA(0x20DD6AFF);
}

void Enforcement::warn(IPlayer& player, const char* cheatName)
{
	std::string msg = std::string("[Anti-Cheat] Notice: ") + cheatName + " was detected on your client.";
	player.sendClientMessage(kColOk, msg);
}

void Enforcement::kick(IPlayer& player, const char* cheatName)
{
	std::string msg = std::string("[Anti-Cheat] Detected: ") + cheatName + ". Remove it and reconnect.";
	player.sendClientMessage(kColError, msg);
	scheduleKick(player);
}

void Enforcement::ban(IPlayer& player, const char* cheatName)
{
	std::string msg = std::string("[Anti-Cheat] You have been banned for: ") + cheatName + ".";
	player.sendClientMessage(kColError, msg);

	const PeerNetworkData& nd = player.getNetworkData();
	if (nd.network)
	{
		PeerAddress::AddressString ip;
		if (PeerAddress::ToString(nd.networkID.address, ip))
		{
			std::string reason = std::string("Anti-Cheat: ") + cheatName;
			BanEntry entry(ip, player.getName(), StringView(reason.c_str(), reason.size()));
			const int minutes = cfg_.banDurationMinutes();
			nd.network->ban(entry, minutes > 0 ? Milliseconds(minutes * 60000) : Milliseconds(0));
		}
	}

	scheduleKick(player);
}

void Enforcement::scheduleKick(IPlayer& player)
{
	const int id = player.getID();
	// look the player back up by id when the timer fires - the iplayer& may be gone by then.
	sched_.after(id, std::chrono::milliseconds(cfg_.kickDelayMs()), [this, id]()
		{
			if (IPlayer* p = core_->getPlayers().get(id))
				p->kick();
		});
}
