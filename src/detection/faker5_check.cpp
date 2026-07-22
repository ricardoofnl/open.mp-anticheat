#include "faker5_check.hpp"

#include "../config.hpp"
#include "../player_data.hpp"
#include "signatures.hpp"

bool FakeR5Check::enabled(const Config& cfg) const
{
	return cfg.moduleFaker5();
}

void FakeR5Check::onConnect(IPlayer& player, PlayerACData& data)
{
	// faker5 only runs inside a classic samp.dll client; the omp client and verified
	// mobile clients lay memory out differently and would false-positive on header reads.
	if (data.mobilePlayer || player.isUsingOmp())
		return;

	// read the pe-header of samp.dll (0x45) and gta_sa.exe (0x5): a real client answers
	// from the live image (non-zero), faker5 resolves the header rva to file-offset 0.
	for (int i = 0; i < signatures::kHeaderProbeCount; ++i)
	{
		data.faker5SampHeader[i] = signatures::kSampHeaderProbe[i];
		player.sendClientCheck(0x45, signatures::kSampHeaderProbe[i], 0x0, 0x4);

		data.faker5GtaHeader[i] = signatures::kGtaHeaderProbe[i];
		player.sendClientCheck(0x5, signatures::kGtaHeaderProbe[i], 0x0, 0x4);
	}

	// in-section canaries: real data for both a genuine client and faker5; confirm the
	// client answers checks with real bytes at all before we trust an all-zero header.
	data.faker5SampCanaryAddr = signatures::kSampCanaryAddr;
	player.sendClientCheck(0x45, signatures::kSampCanaryAddr, 0x0, 0x4);

	data.faker5GtaCanaryAddr = signatures::kGtaCanaryAddr;
	player.sendClientCheck(0x5, signatures::kGtaCanaryAddr, 0x0, 0x4);
}

void FakeR5Check::onCheckResponse(IPlayer& player, PlayerACData& data, int actionType, int address, int results)
{
	(void)player;
	if (actionType != 0x45 && actionType != 0x5)
		return;

	const int value = results & 0xFF;

	// canaries: any non-zero answer proves the client reads real bytes for this module.
	if (actionType == 0x45 && address == data.faker5SampCanaryAddr)
	{
		if (value != 0)
			data.faker5SampCanaryLive = true;
		return;
	}
	if (actionType == 0x5 && address == data.faker5GtaCanaryAddr)
	{
		if (value != 0)
			data.faker5GtaCanaryLive = true;
		return;
	}

	for (int i = 0; i < signatures::kHeaderProbeCount; ++i)
	{
		if (actionType == 0x45 && address == data.faker5SampHeader[i])
		{
			value == 0 ? ++data.faker5SampZero : ++data.faker5SampNonZero;
			return;
		}
		if (actionType == 0x5 && address == data.faker5GtaHeader[i])
		{
			value == 0 ? ++data.faker5GtaZero : ++data.faker5GtaNonZero;
			return;
		}
	}
}

void FakeR5Check::onEvaluate(IPlayer& player, PlayerACData& data)
{
	(void)player;
	if (data.mobilePlayer)
		return;

	// a live canary but only-zero header answers means the module was read from a clean
	// disk image, not live memory - the faker5 signature. evaluate samp and gta apart so
	// a spoofer that only ships one clean file (e.g. no r5.dll) is still caught on the other.
	const bool sampSpoof = data.faker5SampCanaryLive && data.faker5SampNonZero == 0 && data.faker5SampZero >= 2;
	const bool gtaSpoof = data.faker5GtaCanaryLive && data.faker5GtaNonZero == 0 && data.faker5GtaZero >= 2;
	if (sampSpoof || gtaSpoof)
		data.pending[Cheat_FakeR5] = true;
}
