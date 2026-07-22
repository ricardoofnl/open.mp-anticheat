#pragma once

// module 6: counters faker5-style clientcheck spoofers. faker5 hooks the memory
// query (0x5/0x45) and answers from a clean gta_sa.exe / r5.dll on disk instead
// of live memory, hiding every in-memory cheat. it reads via file offset, so a
// pe-header address resolves to file-offset 0 and comes back 0 - where a real
// client returns live header bytes. that mismatch is the detection.

#include "module.hpp"

class FakeR5Check final : public IDetectionModule
{
public:
	explicit FakeR5Check(IACContext& ctx)
		: ctx_(ctx)
	{
	}

	const char* name() const override { return "faker5"; }
	bool enabled(const Config& cfg) const override;

	void onConnect(IPlayer& player, PlayerACData& data) override;
	void onCheckResponse(IPlayer& player, PlayerACData& data, int actionType, int address, int results) override;
	void onEvaluate(IPlayer& player, PlayerACData& data) override;

private:
	IACContext& ctx_;
};
