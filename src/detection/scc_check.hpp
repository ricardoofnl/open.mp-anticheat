#pragma once

// module 7: counters scc_ac_bypass-style static clientcheck-response spoofers.
// the spoofer hooks rpc 103 and, for signature addresses it recognises (a narrow
// [base-0x10, base) window), replays one hardcoded clean byte - ignoring the read's
// offset and count. we bait those bases from inside the window, then read the same
// wire address with different offset/count: a genuine client answers a checksum that
// moves with the read, the spoofer replays the same constant every time. no variation
// (plus a live canary proving the client answers real reads) is the detection.

#include "module.hpp"

class SccCheck final : public IDetectionModule
{
public:
	explicit SccCheck(IACContext& ctx)
		: ctx_(ctx)
	{
	}

	const char* name() const override { return "scc"; }
	bool enabled(const Config& cfg) const override;

	void onConnect(IPlayer& player, PlayerACData& data) override;
	void onCheckResponse(IPlayer& player, PlayerACData& data, int actionType, int address, int results) override;
	void onEvaluate(IPlayer& player, PlayerACData& data) override;

private:
	IACContext& ctx_;
};