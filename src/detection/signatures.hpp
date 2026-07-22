#pragma once

// the detection database: memory addresses, expected bytes, per-version samp.dll addresses and cheat<->name mapping (from antycheat.pwn).

#include <cstdint>
#include "../ac_types.hpp"

namespace signatures
{

// rraddress() in the reference: memory[] addresses have byte 0 and byte 2 swapped (obfuscation); swap them back to get the real address.
inline int unscramble(int input)
{
	return ((input & 0xFF) << 16) + (input & 0xFF00) + ((input & 0xFF0000) >> 16);
}

// 17 single-byte signatures; addresses are stored scrambled (call unscramble() before sending), a read != `expected` flags `cheat`.
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

// read repeatedly; a value that changes between reads => raknet/s0beit anomaly.
static const int kClientAddr[4] = { 0x3A9EB, 0x3AEB9, 0x3AD8D, 0x3A7F2 };

// samp.dll base checks. index: 0=0.3.dl-r1, 1=0.3.7-r5, 2=0.3.7-r4, 3=0.3.7-r3.
static const int kSampAddr[4] = { 0x41B04, 0x42044, 0x41FF4, 0x41904 };

// faker5 counter: pe-header reads. a real client answers these from the live
// module image (dos-stub bytes, non-zero); faker5 maps the module from disk and
// resolves a header rva to file-offset 0, then answers 0 - that is the tell.
static const int kSampHeaderProbe[] = { 0x40, 0x54, 0x68 }; // samp.dll dos-stub rvas (0x45)
static const int kGtaHeaderProbe[] = { 0x400040, 0x400054, 0x400068 }; // gta_sa.exe dos-stub addrs (0x5)
static constexpr int kHeaderProbeCount = int(sizeof(kSampHeaderProbe) / sizeof(kSampHeaderProbe[0]));

// in-section canaries: real data for both a genuine client and faker5; used only
// to confirm the client actually answers checks (guards against clients that reply 0 to everything).
static const int kSampCanaryAddr = 0x1000; // samp.dll .text start (0x45)
static const int kGtaCanaryAddr = 0x401000; // gta_sa.exe .text (0x5)

// scc counter: static-clientcheck-response spoofers (scc_ac_bypass) recognise a
// signature only inside a [base-0x10, base) window and replay one baked constant,
// blind to the read's offset/count. bait a few known bases inside that window, then
// read the same wire address with different offset/count: a genuine client's checksum
// moves with the read, the spoofer's answer never does. bases must be ones the spoofer
// knows, i.e. the standard cheat-signature addresses.
static const int kSccBases[] = { 0x5E8606, 0x4667DB, 0x6FC5B0, 0x5E85F9 };
static constexpr int kSccBaseCount = int(sizeof(kSccBases) / sizeof(kSccBases[0]));
static constexpr int kSccWindowOffset = 0x08; // wire = base - 0x08, inside the spoofer's [base-0x10, base) window
static const int kSccCanaryAddr = 0x401000; // gta_sa.exe .text; the spoofer ignores it, so a real (non-zero) answer proves live reads

// extra per-version samp.dll checks (0x45); each address is two addends (obfuscation) summed then offset-subtracted at send time. same order as ksampaddr.
static const int kAddCheckConnect[4][2] = {
	{ 0x6B4D, 0x33D53 }, // 0.3.dl-r1
	{ 0x247A, 0x38966 }, // 0.3.7-r5
	{ 0x2B414, 0xF97C }, // 0.3.7-r4
	{ 0x62EB, 0x343B5 }, // 0.3.7-r3
};
static const int kAddCheckSpawn[4][2] = {
	{ 0x75EA2, 0x2782E }, // 0.3.dl-r1
	{ 0x3A65F, 0x63231 }, // 0.3.7-r5
	{ 0x74674, 0x2924C }, // 0.3.7-r4
	{ 0x3AB43, 0x6263D }, // 0.3.7-r3
};

// client versions allowed to play (anything else is kicked unless mobile).
static const char* const kAllowedClients[] = {
	"0.3.7-R3", "0.3.7-R4", "0.3.7-R5", "0.3.DL-R1"
};
static constexpr int kAllowedClientCount = int(sizeof(kAllowedClients) / sizeof(kAllowedClients[0]));

// a legit mobile client has this exact gpci (serial) and sends this checksum in the join rpc; same gpci without it => fakemobile.
static const char kMobileGpci[] = "ED40ED0E8089CC44C08EE9580F4C8C44EE8EE990";
static const uint16_t kMobileChecksum = 0xBEEF;

// case-insensitive C-string compare (self-contained; keeps this header sdk-free).
inline bool ieq(const char* a, const char* b)
{
	while (*a && *b)
	{
		char ca = *a, cb = *b;
		if (ca >= 'a' && ca <= 'z')
			ca -= 32;
		if (cb >= 'a' && cb <= 'z')
			cb -= 32;
		if (ca != cb)
			return false;
		++a;
		++b;
	}
	return *a == *b;
}

// map a client version name to an index into ksampaddr / kaddcheck*, or -1.
inline int versionIndex(const char* v)
{
	if (ieq(v, "0.3.DL-R1"))
		return 0;
	if (ieq(v, "0.3.7-R5"))
		return 1;
	if (ieq(v, "0.3.7-R4"))
		return 2;
	if (ieq(v, "0.3.7-R3"))
		return 3;
	return -1;
}

inline bool isAllowedClient(const char* v)
{
	for (int i = 0; i < kAllowedClientCount; ++i)
	{
		if (ieq(v, kAllowedClients[i]))
			return true;
	}
	return false;
}

// name + config key + default action per cheat id; ids for the same logical cheat share a config key (one action per real cheat).
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
	{ Cheat_FakeR5, "FakeR5 / ClientCheck spoofer", "faker5", CheatAction::Kick },
	{ Cheat_SccBypass, "SCC ClientCheck spoofer", "scc", CheatAction::Kick },
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
