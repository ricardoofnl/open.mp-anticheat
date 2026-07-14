#pragma once

/*
 *  Module 4: mobile / FakeMobile (raw RakNet). Hooks the incoming join RPC
 *  (id 25) to read the trailing checksum, then on connect compares the player's
 *  serial (gpci) and that checksum against the known mobile values. A legit
 *  mobile client is whitelisted; the mobile serial without the right checksum is
 *  FakeMobile. Off by default (config module.mobile) because it peeks raw
 *  packets. Registered on RPC 25 by the component in onReady.
 */

#include <cstdint>
#include <unordered_map>

#include "module.hpp"

class MobileCheck final : public IDetectionModule, public SingleNetworkInEventHandler
{
public:
	explicit MobileCheck(IACContext& ctx)
		: ctx_(ctx)
	{
	}

	const char* name() const override { return "mobile"; }
	bool enabled(const Config& cfg) const override;

	void onConnect(IPlayer& player, PlayerACData& data) override;

	// SingleNetworkInEventHandler: incoming join RPC (id 25).
	bool onReceive(IPlayer& peer, NetworkBitStream& bs) override;

private:
	IACContext& ctx_;
	// Checksum captured from the join RPC, keyed by player, consumed on connect
	// (the player extension doesn't exist yet when the join RPC arrives).
	std::unordered_map<const IPlayer*, uint16_t> pendingChecksum_;
};
