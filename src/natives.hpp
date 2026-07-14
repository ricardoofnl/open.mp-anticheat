#pragma once

/*
 *  Thin bridge between the component and the Pawn scripting layer. All of the
 *  actual pawn-natives machinery (which pulls in pawn_impl.hpp exactly once)
 *  lives in natives.cpp, so the rest of the component never includes it.
 *  Compiled only when AC_ENABLE_PAWN is defined.
 */

#ifdef AC_ENABLE_PAWN

#include <sdk.hpp>
#include <Server/Components/Pawn/pawn.hpp>

namespace ac_pawn
{
// Set up the amx function pointers + parameter lookups (call from onInit).
void onInit(IComponentList* components, IPawnComponent* pawn);

// Register this component's natives with a newly loaded script.
void onAmxLoad(IPawnScript& script);

// Fire public OnPlayerCheatDetected(playerid, cheatid, action) in every script.
// Returns 0 if any script returned 0 (suppress the built-in action), else 1.
int fireDetected(IPawnComponent* pawn, int playerid, int cheatid, int action);
}

#endif // AC_ENABLE_PAWN
