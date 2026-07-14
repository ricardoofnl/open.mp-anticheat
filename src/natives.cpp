/*
 *  Pawn natives + the OnPlayerCheatDetected callback. Compiled only when
 *  AC_ENABLE_PAWN is defined; this is the single translation unit that includes
 *  pawn_impl.hpp.
 */

#ifdef AC_ENABLE_PAWN

#include <Server/Components/Pawn/Impl/pawn_natives.hpp>
#include <Server/Components/Pawn/Impl/pawn_impl.hpp>

#include "anticheat_component.hpp"
#include "detection/signatures.hpp"
#include "natives.hpp"

// ------------------------------------------------------------------- natives

// AC_SetEnabled(bool:toggle) - turn detection on/off at runtime.
SCRIPT_API(AC_SetEnabled, bool(bool toggle))
{
	AntiCheatComponent::getInstance()->setEnabled(toggle);
	return true;
}

// AC_IsEnabled() - whether detection is currently running.
SCRIPT_API(AC_IsEnabled, bool())
{
	return AntiCheatComponent::getInstance()->enabled();
}

// AC_AddException(playerid, cheatid) - stop reporting `cheatid` for this player.
SCRIPT_API(AC_AddException, bool(int playerid, int cheatid))
{
	return AntiCheatComponent::getInstance()->addException(playerid, cheatid);
}

// AC_RemoveException(playerid, cheatid) - undo AC_AddException.
SCRIPT_API(AC_RemoveException, bool(int playerid, int cheatid))
{
	return AntiCheatComponent::getInstance()->removeException(playerid, cheatid);
}

// AC_IsPlayerMobile(playerid) - true if verified as a legit mobile client.
SCRIPT_API(AC_IsPlayerMobile, bool(int playerid))
{
	return AntiCheatComponent::getInstance()->isMobile(playerid);
}

// AC_GetLastCheat(playerid) - id of the last cheat detected for this player.
SCRIPT_API(AC_GetLastCheat, int(int playerid))
{
	return AntiCheatComponent::getInstance()->lastCheat(playerid);
}

// AC_GetCheatName(cheatid, name[], len) - human name for a cheat id.
SCRIPT_API(AC_GetCheatName, bool(int cheatid, OutputOnlyString& name))
{
	name = StringView(signatures::cheatInfo(static_cast<CheatId>(cheatid)).name);
	return true;
}

// ------------------------------------------------------- component <-> pawn

namespace ac_pawn
{
void onInit(IComponentList* components, IPawnComponent* pawn)
{
	setAmxFunctions(pawn->getAmxFunctions());
	setAmxLookups(components);
}

void onAmxLoad(IPawnScript& script)
{
	pawn_natives::AmxLoad(script.GetAMX());
}

int fireDetected(IPawnComponent* pawn, int playerid, int cheatid, int action)
{
	int result = 1;
	// Filterscripts first, then the gamemode - a 0 return from any suppresses
	// the built-in action.
	for (IPawnScript* script : pawn->sideScripts())
	{
		if (script->Call("OnPlayerCheatDetected", DefaultReturnValue_True, playerid, cheatid, action) == 0)
			result = 0;
	}
	if (IPawnScript* main = pawn->mainScript())
	{
		if (main->Call("OnPlayerCheatDetected", DefaultReturnValue_True, playerid, cheatid, action) == 0)
			result = 0;
	}
	return result;
}
}

#endif // AC_ENABLE_PAWN
