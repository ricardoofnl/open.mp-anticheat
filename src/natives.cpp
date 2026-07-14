// pawn natives + the onplayercheatdetected callback; the single tu that includes pawn_impl.hpp. compiled only when ac_enable_pawn is defined.

#ifdef AC_ENABLE_PAWN

#include <Server/Components/Pawn/Impl/pawn_natives.hpp>
#include <Server/Components/Pawn/Impl/pawn_impl.hpp>

#include "anticheat_component.hpp"
#include "detection/signatures.hpp"
#include "natives.hpp"

// natives

// ac_setenabled(bool:toggle) - turn detection on/off at runtime.
SCRIPT_API(AC_SetEnabled, bool(bool toggle))
{
	AntiCheatComponent::getInstance()->setEnabled(toggle);
	return true;
}

// ac_isenabled() - whether detection is currently running.
SCRIPT_API(AC_IsEnabled, bool())
{
	return AntiCheatComponent::getInstance()->enabled();
}

// ac_addexception(playerid, cheatid) - stop reporting `cheatid` for this player.
SCRIPT_API(AC_AddException, bool(int playerid, int cheatid))
{
	return AntiCheatComponent::getInstance()->addException(playerid, cheatid);
}

// ac_removeexception(playerid, cheatid) - undo ac_addexception.
SCRIPT_API(AC_RemoveException, bool(int playerid, int cheatid))
{
	return AntiCheatComponent::getInstance()->removeException(playerid, cheatid);
}

// ac_isplayermobile(playerid) - true if verified as a legit mobile client.
SCRIPT_API(AC_IsPlayerMobile, bool(int playerid))
{
	return AntiCheatComponent::getInstance()->isMobile(playerid);
}

// ac_getlastcheat(playerid) - id of the last cheat detected for this player.
SCRIPT_API(AC_GetLastCheat, int(int playerid))
{
	return AntiCheatComponent::getInstance()->lastCheat(playerid);
}

// ac_getcheatname(cheatid, name[], len) - human name for a cheat id.
SCRIPT_API(AC_GetCheatName, bool(int cheatid, OutputOnlyString& name))
{
	name = StringView(signatures::cheatInfo(static_cast<CheatId>(cheatid)).name);
	return true;
}

// component <-> pawn

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
	// filterscripts first, then the gamemode - a 0 return from any suppresses the built-in action.
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

#endif // ac_enable_pawn
