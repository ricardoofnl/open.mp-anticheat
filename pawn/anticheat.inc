// anticheat.inc - pawn api for the open.mp anti-cheat component (#include <anticheat>; needs the component built with ac_enable_pawn).

#if defined _INC_anticheat
	#endinput
#endif
#define _INC_anticheat

// cheat ids (mirror the component's cheatid enum)
#define AC_CHEAT_NONE                (0)
#define AC_CHEAT_S0BEIT              (1)
#define AC_CHEAT_CLEO_2              (2)
#define AC_CHEAT_CLEO_3              (3)
#define AC_CHEAT_CLEO_4              (4)
#define AC_CHEAT_CLEO_MOONLOADER_5   (5)
#define AC_CHEAT_CLEO_MOONLOADER_6   (6)
#define AC_CHEAT_CLEO_7              (7)
#define AC_CHEAT_SILENTPATCH         (8)
#define AC_CHEAT_SAMPFUNCS           (9)
#define AC_CHEAT_SAMPFUNCS_2         (10)
#define AC_CHEAT_S0BEIT_2            (11)
#define AC_CHEAT_MOD_VORBISFILE      (12)
#define AC_CHEAT_ULTRAWH             (13)
#define AC_CHEAT_SILENTAIM           (14)
#define AC_CHEAT_IMPROVED_DEAGLE     (15)
#define AC_CHEAT_STEALTH_REMASTERED  (16)
#define AC_CHEAT_SENSFIX             (17)
#define AC_CHEAT_S0BEIT_RAKNET       (18)
#define AC_CHEAT_FAKEMOBILE          (19)
#define AC_CHEAT_MODDED_CLIENT       (20)
#define AC_CHEAT_FAKER5              (21)

// actions (as configured per cheat)
#define AC_ACTION_IGNORE             (0)
#define AC_ACTION_WARN               (1)
#define AC_ACTION_KICK               (2)
#define AC_ACTION_BAN                (3)

// natives
native AC_SetEnabled(bool:toggle);
native bool:AC_IsEnabled();
native bool:AC_AddException(playerid, cheatid);
native bool:AC_RemoveException(playerid, cheatid);
native bool:AC_IsPlayerMobile(playerid);
native AC_GetLastCheat(playerid);
native bool:AC_GetCheatName(cheatid, name[], len = sizeof(name));

// called before the built-in action; return 0 to suppress it, 1 (default) to apply the configured action.
forward OnPlayerCheatDetected(playerid, cheatid, action);
