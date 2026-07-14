#include "raknet_check.hpp"

#include <cstdint>

#include <bitstream.hpp>

#include "../config.hpp"
#include "../player_data.hpp"

bool RaknetCheck::enabled(const Config& cfg) const
{
	return cfg.moduleRaknet();
}

bool RaknetCheck::onReceive(IPlayer& peer, NetworkBitStream& bs)
{
	PlayerACData* data = queryExtension<PlayerACData>(peer);
	if (!data)
		return true;

	// Peek actionid / memaddr / retndata, then restore the read pointer for the
	// server's own handler. Never block.
	const int startOffset = bs.GetReadOffset();

	uint8_t actionid = 0;
	int32_t memaddr = 0;
	uint8_t retndata = 0;
	const bool ok = bs.readUINT8(actionid) && bs.readINT32(memaddr) && bs.readUINT8(retndata);

	bs.SetReadOffset(startOffset);

	// The clean samp.dll check value is 192; a raw response that differs means
	// the high-level path was tampered with (S0beit).
	if (ok && actionid == 0x45 && data->sampCheckAddr != 0 && memaddr == data->sampCheckAddr && (retndata & 0xFF) != 192)
		data->pending[Cheat_S0beit_RakNet] = true;

	return true;
}
