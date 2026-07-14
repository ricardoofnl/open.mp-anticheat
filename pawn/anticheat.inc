/*
 *  anticheat.inc - Pawn API for the open.mp Anti-Cheat component.
 *
 *  Put this on your compiler's include path and `#include <anticheat>` in your
 *  gamemode/filterscript. Requires the anticheat component built with
 *  AC_ENABLE_PAWN (the default).
 */

#if defined _INC_anticheat
	#endinput
#endif
#define _INC_anticheat

// ---- cheat ids (mirror the component's CheatId enum) ----
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

// ---- actions (as configured per cheat) ----
#define AC_ACTION_IGNORE             (0)
#define AC_ACTION_WARN               (1)
#define AC_ACTION_KICK               (2)
#define AC_ACTION_BAN                (3)

// ---- natives ----
native AC_SetEnabled(bool:toggle);
native bool:AC_IsEnabled();
native bool:AC_AddException(playerid, cheatid);
native bool:AC_RemoveException(playerid, cheatid);
native bool:AC_IsPlayerMobile(playerid);
native AC_GetLastCheat(playerid);
native bool:AC_GetCheatName(cheatid, name[], len = sizeof(name));

/*
 *  Called when a cheat is detected, BEFORE the built-in action runs.
 *  Return 0 to suppress the built-in action (you handle it yourself);
 *  return 1 (default) to let the component apply the configured action.
 */
forward OnPlayerCheatDetected(playerid, cheatid, action);
