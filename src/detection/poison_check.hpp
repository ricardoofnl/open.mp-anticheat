#pragma once

/*
 *  Module 3: anti-tamper "poison" check (actions 0x47 / 0x48). We seed suspicion
 *  and ask the client to read address 0; a clean client answers with address 0
 *  and clears suspicion, while a client that mirrors/echoes the check RPC keeps
 *  it set. Mirrors the reference's 0xCECECE / 0xDEDEDE trick.
 */

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
