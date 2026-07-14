#pragma once

/*
 *  Module 5: RakNet serialization anomaly (raw RakNet). Hooks the incoming
 *  client-check response RPC (id 103) and reads the samp.dll check result
 *  straight off the wire - bypassing the high-level onClientCheckResponse path
 *  that S0beit hooks. If that raw value disagrees with the clean value, the
 *  client's RakNet is tampered. Off by default (config module.raknet); relies on
 *  the version module having set sampCheckAddr. Registered on RPC 103 by the
 *  component in onReady.
 *
 *  Note: this is a conservative take on the reference's re-serialization trick -
 *  it reads and verifies rather than re-injecting packets, which keeps it safe.
 */

#include "module.hpp"

class RaknetCheck final : public IDetectionModule, public SingleNetworkInEventHandler
{
public:
	explicit RaknetCheck(IACContext& ctx)
		: ctx_(ctx)
	{
	}

	const char* name() const override { return "raknet"; }
	bool enabled(const Config& cfg) const override;

	// SingleNetworkInEventHandler: incoming client-check response RPC (id 103).
	bool onReceive(IPlayer& peer, NetworkBitStream& bs) override;

private:
	IACContext& ctx_;
};
