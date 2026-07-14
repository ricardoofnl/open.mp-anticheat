#include "memory_check.hpp"

#include "../config.hpp"
#include "../player_data.hpp"
#include "signatures.hpp"

bool MemoryCheck::enabled(const Config& cfg) const
{
	return cfg.moduleMemory();
}

void MemoryCheck::onConnect(IPlayer& player, PlayerACData& data)
{
	for (int i = 0; i < signatures::kMemorySignatureCount; ++i)
	{
		const int base = signatures::unscramble(signatures::kMemorySignatures[i].address);
		const int off = ctx_.nextMemOffset();
		// Send base-off with the offset added back client-side, so the effective
		// read address is `base` but a cheat can't key off a fixed value.
		data.memBase[i] = base - off;
		player.sendClientCheck(0x5, base - off, off, 0x4);
	}
	// One extra 0x5 probe, as in the reference, purely to confirm the client
	// answers 0x5 checks at all (see onEvaluate).
	player.sendClientCheck(0x5, 0x53EA05, 0x0, 0x4);
}

void MemoryCheck::onCheckResponse(IPlayer& player, PlayerACData& data, int actionType, int address, int results)
{
	(void)player;
	if (actionType != 0x5)
		return;

	if (!data.mobilePlayer)
		data.responded = true;

	for (int i = 0; i < signatures::kMemorySignatureCount; ++i)
	{
		if (address == data.memBase[i])
		{
			if ((results & 0xFF) != (signatures::kMemorySignatures[i].expected & 0xFF))
				data.pending[signatures::kMemorySignatures[i].cheat] = true;
			break;
		}
	}
}

void MemoryCheck::onEvaluate(IPlayer& player, PlayerACData& data)
{
	(void)player;
	// A client that never answered a single 0x5 check (and isn't a verified
	// mobile client) is blocking the memory query - treat it as modded.
	if (!data.responded && !data.mobilePlayer)
		data.pending[Cheat_ModdedClient] = true;
}
