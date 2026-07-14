/*
 *  ac_test.pwn - minimal gamemode/filterscript to exercise the anti-cheat's
 *  Pawn API. Compile with the open.mp compiler and run it alongside the
 *  anticheat component.
 */

#include <open.mp>
#include <anticheat>

public OnPlayerCheatDetected(playerid, cheatid, action)
{
	new name[48];
	AC_GetCheatName(cheatid, name);

	new pname[MAX_PLAYER_NAME];
	GetPlayerName(playerid, pname, sizeof(pname));

	printf("[AC-TEST] %s (id %d) -> cheat %d '%s', action %d", pname, playerid, cheatid, name, action);

	// Example policy: let RCON admins bypass everything.
	if (IsPlayerAdmin(playerid))
	{
		printf("[AC-TEST] %s is admin - suppressing action", pname);
		return 0; // suppress the built-in kick/ban
	}
	return 1; // let the component do its thing
}

public OnPlayerConnect(playerid)
{
	// Example: this server tolerates SilentPatch, so never report it here.
	AC_AddException(playerid, AC_CHEAT_SILENTPATCH);

	if (AC_IsPlayerMobile(playerid))
	{
		SendClientMessage(playerid, 0x20DD6AFF, "Welcome, mobile player!");
	}
	return 1;
}
