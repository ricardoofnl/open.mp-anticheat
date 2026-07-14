#pragma once

// module 3: anti-tamper poison check (0x47/0x48); a clean client clears suspicion, one that echoes the poisoned rpc keeps it (reference 0xcecece/0xdedede trick).

#include "module.hpp"

class PoisonCheck final : public IDetectionModule
{
public:
	explicit PoisonCheck(IACContext& ctx)
		: ctx_(ctx)
	{
	}

	const char* name() const override { return "poison"; }
	bool enabled(const Config& cfg) const override;

	void onConnect(IPlayer& player, PlayerACData& data) override;
	void onCheckResponse(IPlayer& player, PlayerACData& data, int actionType, int address, int results) override;
	void onEvaluate(IPlayer& player, PlayerACData& data) override;

private:
	IACContext& ctx_;
};
