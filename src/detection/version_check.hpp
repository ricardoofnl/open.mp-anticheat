#pragma once

// module 2: client-version + samp.dll checks (0x45) - per-version probes, repeated-read anomaly, and connect/spawn extra checks for non-omp clients.

#include "module.hpp"

class VersionCheck final : public IDetectionModule
{
public:
	explicit VersionCheck(IACContext& ctx)
		: ctx_(ctx)
	{
	}

	const char* name() const override { return "version"; }
	bool enabled(const Config& cfg) const override;

	void onConnect(IPlayer& player, PlayerACData& data) override;
	void onSpawn(IPlayer& player, PlayerACData& data) override;
	void onCheckResponse(IPlayer& player, PlayerACData& data, int actionType, int address, int results) override;
	void onEvaluate(IPlayer& player, PlayerACData& data) override;

private:
	IACContext& ctx_;
};
