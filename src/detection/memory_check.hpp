#pragma once

/*
 *  Module 1: the byte-signature memory scan. On connect it asks the client to
 *  read every address in signatures::kMemorySignatures (action 0x5, address
 *  obfuscated with a random offset); a byte that doesn't match flags the cheat.
 */

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
