#include "scc_check.hpp"

#include "../config.hpp"
#include "../player_data.hpp"
#include "signatures.hpp"

bool SccCheck::enabled(const Config& cfg) const
{
	return cfg.moduleScc();
}

void SccCheck::onConnect(IPlayer& player, PlayerACData& data)
{
	// the spoofer is a classic-client samp.dll asi; the omp client and verified mobile
	// clients answer 0x5 reads differently and are out of its reach anyway.
	if (data.mobilePlayer || player.isUsingOmp())
		return;

	// canary: a read the spoofer does not recognise. a real (non-zero) answer proves the
	// client answers live reads, so all-constant bait answers mean spoofing, not a dead client.
	data.sccCanaryAddr = signatures::kSccCanaryAddr;
	player.sendClientCheck(0x5, signatures::kSccCanaryAddr, 0x0, 0x4);

	for (int i = 0; i < signatures::kSccBaseCount; ++i)
	{
		// wire address sits inside the spoofer's [base-0x10, base) window, so it rewrites
		// every one of these; the three reads differ only in offset/count, which it ignores.
		const int wire = signatures::kSccBases[i] - signatures::kSccWindowOffset;
		data.sccProbeAddr[i] = wire;
		player.sendClientCheck(0x5, wire, signatures::kSccWindowOffset, 0x4); // effective read = base
		player.sendClientCheck(0x5, wire, 0x0, 0x4);                          // effective read = base - 0x08
		player.sendClientCheck(0x5, wire, signatures::kSccWindowOffset, 0x1); // effective read = base, 1 byte
	}
}

void SccCheck::onCheckResponse(IPlayer& player, PlayerACData& data, int actionType, int address, int results)
{
	(void)player;
	if (actionType != 0x5)
		return;

	const int value = results & 0xFF;

	if (address == data.sccCanaryAddr)
	{
		if (value != 0)
			data.sccCanaryLive = true;
		return;
	}

	for (int i = 0; i < signatures::kSccBaseCount; ++i)
	{
		if (address == data.sccProbeAddr[i])
		{
			++data.sccCount[i];
			if (data.sccFirst[i] < 0)
				data.sccFirst[i] = value;
			else if (value != data.sccFirst[i])
				data.sccVaried[i] = true;
			return;
		}
	}
}

void SccCheck::onEvaluate(IPlayer& player, PlayerACData& data)
{
	(void)player;
	if (data.mobilePlayer)
		return;

	// a base is "static" if it answered but never varied as we moved the read offset/count.
	int evaluated = 0, staticBases = 0;
	for (int i = 0; i < signatures::kSccBaseCount; ++i)
	{
		if (data.sccCount[i] >= 2)
		{
			++evaluated;
			if (!data.sccVaried[i])
				++staticBases;
		}
	}

	// enough bases answered, every one of them replayed a constant, and the client
	// otherwise answers live reads (canary): a static clientcheck-response spoofer.
	if (data.sccCanaryLive && evaluated >= 3 && staticBases == evaluated)
		data.pending[Cheat_SccBypass] = true;
}