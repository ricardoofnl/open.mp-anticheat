#pragma once

// per-player anti-cheat state, attached to each iplayer as an extension (queryextension<playeracdata>).

#include <array>
#include <cstdint>

#include <sdk.hpp>

#include "ac_types.hpp"
#include "detection/signatures.hpp"

class PlayerACData final : public IExtension
{
public:
	// unique per-extension id used by queryextension.
	PROVIDE_EXT_UID(0xAC12CEA7DA7A0001);

	PlayerACData() { resetState(); }

	void reset() override { resetState(); }
	void freeExtension() override { delete this; }

	// shared flags
	bool mobilePlayer = false; // verified legit mobile client
	bool responded = false; // answered at least one 0x5 check
	bool suspicious = false; // poison module tripped
	bool spawnChecked = false; // spawn wave already scheduled
	bool evaluated = false; // evaluate wave already run
	uint16_t joinChecksum = 0; // trailing checksum from the join rpc (mobile)

	// memory module (0x5)
	// base address sent for each signature (offset randomised per check); a match is recorded in `pending` below.
	std::array<int, signatures::kMemorySignatureCount> memBase {};

	// version module (0x45)
	int sampCheckAddr = 0;
	std::array<int, 4> clientAddrSent {};
	std::array<int, 4> lastClientRetn {};
	int addCheckConnectAddr = 0;
	int addCheckSpawnAddr = 0;
	bool clientAnomaly = false;

	// faker5 module (0x45 samp / 0x5 gta pe-header reads)
	// header probe addresses sent; a spoofer answers them 0 (file-offset resolves to 0).
	std::array<int, signatures::kHeaderProbeCount> faker5SampHeader {};
	std::array<int, signatures::kHeaderProbeCount> faker5GtaHeader {};
	int faker5SampCanaryAddr = 0;
	int faker5GtaCanaryAddr = 0;
	int faker5SampZero = 0, faker5SampNonZero = 0;
	int faker5GtaZero = 0, faker5GtaNonZero = 0;
	bool faker5SampCanaryLive = false; // an in-section samp read returned real data
	bool faker5GtaCanaryLive = false; // an in-section gta read returned real data

	// scc module (static clientcheck-response spoofer)
	// per bait base: wire address probed, first answer seen (-1 = none), answer count,
	// and whether any answer differed. a spoofer replays one constant, so varied stays false.
	std::array<int, signatures::kSccBaseCount> sccProbeAddr {};
	std::array<int, signatures::kSccBaseCount> sccFirst {};
	std::array<int, signatures::kSccBaseCount> sccCount {};
	std::array<bool, signatures::kSccBaseCount> sccVaried {};
	int sccCanaryAddr = 0;
	bool sccCanaryLive = false; // a live gta read returned real data (the spoofer leaves it alone)

	// reporting bookkeeping
	// modules record hits here; the component reports every pending cheat once, at the evaluate deadline.
	std::array<bool, Cheat_Max> pending {}; // detected, awaiting report
	std::array<bool, Cheat_Max> exception {}; // suppressed via ac_addexception
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
		faker5SampCanaryAddr = faker5GtaCanaryAddr = 0;
		faker5SampZero = faker5SampNonZero = faker5GtaZero = faker5GtaNonZero = 0;
		faker5SampCanaryLive = faker5GtaCanaryLive = false;
		sccCanaryAddr = 0;
		sccCanaryLive = false;
		memBase.fill(0);
		faker5SampHeader.fill(0);
		faker5GtaHeader.fill(0);
		sccProbeAddr.fill(0);
		sccFirst.fill(-1);
		sccCount.fill(0);
		sccVaried.fill(false);
		clientAddrSent.fill(0);
		lastClientRetn.fill(0);
		pending.fill(false);
		exception.fill(false);
		reported.fill(false);
	}
};
