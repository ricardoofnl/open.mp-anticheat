#pragma once

// shared types used across the anti-cheat modules.

#include <sdk.hpp>

// what to do when a module reports a detection; mapped per cheat in the config.
enum class CheatAction
{
	Ignore = 0, // do nothing (tolerated/benign mods like silentpatch)
	Warn = 1, // log + message the player, let them keep playing
	Kick = 2, // log + message + kick
	Ban = 3, // log + message + timed ban (config: ban_duration_minutes)
};

// stable id per detectable cheat; values match the reference filterscript's cheatvalues.
enum CheatId
{
	Cheat_None = 0,
	Cheat_S0beit = 1,
	Cheat_CLEO_2 = 2,
	Cheat_CLEO_3 = 3,
	Cheat_CLEO_4 = 4,
	Cheat_CLEO_MoonLoader_5 = 5,
	Cheat_CLEO_MoonLoader_6 = 6,
	Cheat_CLEO_7 = 7,
	Cheat_SilentPatch = 8,
	Cheat_SampFuncs = 9,
	Cheat_SampFuncs_2 = 10,
	Cheat_S0beit_2 = 11,
	Cheat_ModVorbisFile = 12,
	Cheat_UltraWH = 13,
	Cheat_SilentAim = 14,
	Cheat_ImprovedDeagle = 15,
	Cheat_StealthRemastered = 16,
	Cheat_Sensfix = 17,
	Cheat_S0beit_RakNet = 18,
	Cheat_FakeMobile = 19, // mobile module (not in the reference's numeric table)
	Cheat_ModdedClient = 20, // generic modified-client flag (samp.dll mismatch)

	Cheat_Max
};

// one memory signature: read a byte at `address` (obfuscated) and compare to `expected`; mismatch flags `cheat`.
struct MemorySignature
{
	int address;
	int expected;
	CheatId cheat;
};
