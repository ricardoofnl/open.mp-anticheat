#pragma once

/*
 *  Module 2: client-version + samp.dll checks (action 0x45). Sends the
 *  per-version samp.dll probes, repeatedly reads the client addresses to catch
 *  a value that changes between reads (RakNet/S0beit anomaly), and does the
 *  extra connect/spawn wave checks for non-omp clients. The version whitelist
 *  itself is enforced by the component at evaluate time.
 */

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
