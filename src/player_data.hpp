#pragma once

// Per-player anti-cheat state, attached to each IPlayer as an extension (queryExtension<PlayerACData>).

#include <array>
#include <cstdint>

#include <sdk.hpp>

#include "ac_types.hpp"
#include "detection/signatures.hpp"

class PlayerACData final : public IExtension
{
public:
	// Unique per-extension id used by queryExtension.
	PROVIDE_EXT_UID(0xAC12CEA7DA7A0001);

	PlayerACData() { resetState(); }

	void reset() override { resetState(); }
	void freeExtension() override { delete this; }

	// ---- shared flags ----
	bool mobilePlayer = false; // verified legit mobile client
	bool responded = false; // answered at least one 0x5 check
	bool suspicious = false; // poison module tripped
	bool spawnChecked = false; // spawn wave already scheduled
	bool evaluated = false; // evaluate wave already run
	uint16_t joinChecksum = 0; // trailing checksum from the join RPC (mobile)

	// ---- memory module (0x5) ----
	// Base address sent for each signature (offset randomised per check). The
	// matching cheat, once found, is recorded in `pending` below.
	std::array<int, signatures::kMemorySignatureCount> memBase {};

	// ---- version module (0x45) ----
	int sampCheckAddr = 0;
	std::array<int, 4> clientAddrSent {};
	std::array<int, 4> lastClientRetn {};
	int addCheckConnectAddr = 0;
	int addCheckSpawnAddr = 0;
	bool clientAnomaly = false;

	// ---- reporting bookkeeping ----
	// Modules record hits here during the checks; the component reports every
	// pending cheat once, at the evaluate deadline.
	std::array<bool, Cheat_Max> pending {}; // detected, awaiting report
	std::array<bool, Cheat_Max> exception {}; // suppressed via AC_AddException
	std::array<bool, Cheat_Max> reported {}; // already acted on (dedup)
	CheatId lastCheat = Cheat_None;

private:
	void resetState()
	{
		mobilePlayer = responded = suspicious = false;
		spawnChecked = evaluated = clientAnomaly = false;
		joinChecksum = 0;
		sampCheckAddr = addCheckConnectAddr = addCheckSpawnAddr = 0;
		lastCheat = Cheat_None;
		memBase.fill(0);
		clientAddrSent.fill(0);
		lastClientRetn.fill(0);
		pending.fill(false);
		exception.fill(false);
		reported.fill(false);
	}
};
