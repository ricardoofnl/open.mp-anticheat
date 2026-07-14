#pragma once

// module 1: byte-signature memory scan (0x5); on connect reads every kmemorysignatures address, a mismatch flags the cheat.

#include "module.hpp"

class MemoryCheck final : public IDetectionModule
{
public:
	explicit MemoryCheck(IACContext& ctx)
		: ctx_(ctx)
	{
	}

	const char* name() const override { return "memory"; }
	bool enabled(const Config& cfg) const override;

	void onConnect(IPlayer& player, PlayerACData& data) override;
	void onCheckResponse(IPlayer& player, PlayerACData& data, int actionType, int address, int results) override;
	void onEvaluate(IPlayer& player, PlayerACData& data) override;

private:
	IACContext& ctx_;
};
