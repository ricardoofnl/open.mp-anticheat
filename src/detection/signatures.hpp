#pragma once

/*
 *  The detection "database": every memory address, expected byte, per-version
 *  samp.dll address and cheat<->name mapping, ported verbatim from the
 *  reference filterscript (antycheat.pwn). Kept in one place so tuning the
 *  detections never touches the module logic.
 */

#include <cstdint>
#include "../ac_types.hpp"

namespace signatures
{

// rrAddress() in the reference: the memory[] addresses are stored with byte 0
// and byte 2 swapped (cheap source-level obfuscation). Swap them back to get
// the real address to send to the client.
inline int unscramble(int input)
{
	return ((input & 0xFF) << 16) + (input & 0xFF00) + ((input & 0xFF0000) >> 16);
}

// 17 single-byte signatures. Addresses are stored SCRAMBLED (as in the
// reference); call unscramble() before sending. A read that doesn't match
// `expected` flags `cheat`.
static const MemorySignature kMemorySignatures[] = {
	{ 0x06865E, 192, Cheat_S0beit },
	{ 0xA88774, 72, Cheat_CLEO_2 },
	{ 0xDB6746, 192, Cheat_CLEO_3 },
	{ 0xFDB957, 68, Cheat_CLEO_4 },
	{ 0x52D558, 196, Cheat_CLEO_MoonLoader_5 },
	{ 0xE4FC58, 64, Cheat_CLEO_MoonLoader_6 },
	{ 0x1BA246, 8, Cheat_CLEO_7 },
	{ 0xB0C56F, 200, Cheat_SilentPatch },
	{ 0xF9855E, 200, Cheat_SampFuncs },
	{ 0x910152, 204, Cheat_SampFuncs_2 },
	{ 0xC7FB6E, 196, Cheat_S0beit_2 },
	{ 0xF4C853, 132, Cheat_ModVorbisFile },
	{ 0xB47E74, 132, Cheat_UltraWH },
	{ 0x242C52, 192, Cheat_SilentAim },
	{ 0x603C74, 200, Cheat_ImprovedDeagle },
	{ 0x004D58, 132, Cheat_StealthRemastered },
	{ 0x682252, 132, Cheat_Sensfix },
};
static constexpr int kMemorySignatureCount = int(sizeof(kMemorySignatures) / sizeof(kMemorySignatures[0]));

// Read repeatedly; a value that changes between reads => RakNet/S0beit anomaly.
static const int kClientAddr[4] = { 0x3A9EB, 0x3AEB9, 0x3AD8D, 0x3A7F2 };

// samp.dll base checks. Index: 0=0.3.DL-R1, 1=0.3.7-R5, 2=0.3.7-R4, 3=0.3.7-R3.
static const int kSampAddr[4] = { 0x41B04, 0x42044, 0x41FF4, 0x41904 };

// Extra per-version samp.dll checks (action 0x45). The reference stores each
// address as two addends (obfuscation); we sum them then subtract the random
// offset at send time. Same index order as kSampAddr.
static const int kAddCheckConnect[4][2] = {
	{ 0x6B4D, 0x33D53 }, // 0.3.DL-R1
	{ 0x247A, 0x38966 }, // 0.3.7-R5
	{ 0x2B414, 0xF97C }, // 0.3.7-R4
	{ 0x62EB, 0x343B5 }, // 0.3.7-R3
};
static const int kAddCheckSpawn[4][2] = {
	{ 0x75EA2, 0x2782E }, // 0.3.DL-R1
	{ 0x3A65F, 0x63231 }, // 0.3.7-R5
	{ 0x74674, 0x2924C }, // 0.3.7-R4
	{ 0x3AB43, 0x6263D }, // 0.3.7-R3
};

// Client versions allowed to play (anything else is kicked unless mobile).
static const char* const kAllowedClients[] = {
	"0.3.7-R3", "0.3.7-R4", "0.3.7-R5", "0.3.DL-R1"
};
static constexpr int kAllowedClientCount = int(sizeof(kAllowedClients) / sizeof(kAllowedClients[0]));

// A legitimate mobile client identifies with this exact gpci (serial) AND sends
// this checksum in the join RPC. Same gpci without the checksum => FakeMobile.
static const char kMobileGpci[] = "ED40ED0E8089CC44C08EE9580F4C8C44EE8EE990";
static const uint16_t kMobileChecksum = 0xBEEF;

// Map a client version name to an index into kSampAddr / kAddCheck*, or -1.
inline int versionIndex(StringView v)
{
	if (v == "0.3.DL-R1")
		return 0;
	if (v == "0.3.7-R5")
		return 1;
	if (v == "0.3.7-R4")
		return 2;
	if (v == "0.3.7-R3")
		return 3;
	return -1;
}

inline bool isAllowedClient(StringView v)
{
	for (int i = 0; i < kAllowedClientCount; ++i)
	{
		if (v == kAllowedClients[i])
			return true;
	}
	return false;
}

// Human name + config key + default action for every cheat id. Several ids can
// map to the same logical cheat (e.g. multiple CLEO/S0beit signatures) and then
// share a config key, so the server owner sets one action per real cheat.
struct CheatInfo
{
	CheatId id;
	const char* name;
	const char* configKey;
	CheatAction defaultAction;
};

static const CheatInfo kCheatInfo[] = {
	{ Cheat_S0beit, "S0beit", "s0beit", CheatAction::Kick },
	{ Cheat_CLEO_2, "CLEO", "cleo", CheatAction::Kick },
	{ Cheat_CLEO_3, "CLEO", "cleo", CheatAction::Kick },
	{ Cheat_CLEO_4, "CLEO", "cleo", CheatAction::Kick },
	{ Cheat_CLEO_MoonLoader_5, "CLEO / MoonLoader", "cleo_moonloader", CheatAction::Kick },
	{ Cheat_CLEO_MoonLoader_6, "CLEO / MoonLoader", "cleo_moonloader", CheatAction::Kick },
	{ Cheat_CLEO_7, "CLEO", "cleo", CheatAction::Kick },
	{ Cheat_SilentPatch, "SilentPatch", "silentpatch", CheatAction::Warn },
	{ Cheat_SampFuncs, "SampFuncs", "sampfuncs", CheatAction::Kick },
	{ Cheat_SampFuncs_2, "SampFuncs", "sampfuncs", CheatAction::Kick },
	{ Cheat_S0beit_2, "S0beit", "s0beit", CheatAction::Kick },
	{ Cheat_ModVorbisFile, "Modified VorbisFile.dll", "vorbisfile", CheatAction::Warn },
	{ Cheat_UltraWH, "UltraWH", "ultrawh", CheatAction::Kick },
	{ Cheat_SilentAim, "Silent Aim", "silentaim", CheatAction::Kick },
	{ Cheat_ImprovedDeagle, "Improved Deagle", "improved_deagle", CheatAction::Kick },
	{ Cheat_StealthRemastered, "StealthRemastered", "stealth_remastered", CheatAction::Kick },
	{ Cheat_Sensfix, "Sensfix.asi", "sensfix", CheatAction::Kick },
	{ Cheat_S0beit_RakNet, "S0beit / RakNet anomaly", "s0beit_raknet", CheatAction::Kick },
	{ Cheat_FakeMobile, "FakeMobile", "fakemobile", CheatAction::Ban },
	{ Cheat_ModdedClient, "Modded client", "modded_client", CheatAction::Kick },
};
static constexpr int kCheatInfoCount = int(sizeof(kCheatInfo) / sizeof(kCheatInfo[0]));

inline const CheatInfo& cheatInfo(CheatId id)
{
	for (int i = 0; i < kCheatInfoCount; ++i)
	{
		if (kCheatInfo[i].id == id)
			return kCheatInfo[i];
	}
	static const CheatInfo unknown { Cheat_None, "Unknown", "unknown", CheatAction::Kick };
	return unknown;
}

} // namespace signatures
