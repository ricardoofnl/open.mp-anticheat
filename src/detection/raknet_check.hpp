#pragma once

// module 5: raknet serialization anomaly (raw raknet) - reads the samp.dll check off rpc 103 (bypassing the s0beit-hooked path); off by default, needs the version module's sampcheckaddr.

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

	// singlenetworkineventhandler: incoming client-check response rpc (id 103).
	bool onReceive(IPlayer& peer, NetworkBitStream& bs) override;

private:
	IACContext& ctx_;
};
