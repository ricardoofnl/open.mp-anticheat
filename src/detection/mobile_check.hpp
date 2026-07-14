#pragma once

// module 4: mobile / fakemobile (raw raknet) - hooks join rpc 25 for the checksum, whitelists legit mobile and flags fakemobile. off by default.

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

	// singlenetworkineventhandler: incoming join rpc (id 25).
	bool onReceive(IPlayer& peer, NetworkBitStream& bs) override;

private:
	IACContext& ctx_;
	// checksum from the join rpc, keyed by player, consumed on connect (the extension doesn't exist yet when the join rpc arrives).
	std::unordered_map<const IPlayer*, uint16_t> pendingChecksum_;
};
