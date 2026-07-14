#pragma once

// thin bridge to the pawn layer; the pawn-natives machinery lives in natives.cpp. compiled only when ac_enable_pawn is defined.

#ifdef AC_ENABLE_PAWN

#include <sdk.hpp>
#include <Server/Components/Pawn/pawn.hpp>

namespace ac_pawn
{
// set up the amx function pointers + parameter lookups (call from oninit).
void onInit(IComponentList* components, IPawnComponent* pawn);

// register this component's natives with a newly loaded script.
void onAmxLoad(IPawnScript& script);

// fire onplayercheatdetected(playerid, cheatid, action) in every script; returns 0 if any script returned 0 (suppress), else 1.
int fireDetected(IPawnComponent* pawn, int playerid, int cheatid, int action);
}

#endif // ac_enable_pawn
