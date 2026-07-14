// open.mp Anti-Cheat component - C++ port of Pevenaider/AntyCheat on the open.mp SDK.

#include <sdk.hpp>

#include "anticheat_component.hpp"

// Called by the server when it loads the binary; returns the component singleton.
COMPONENT_ENTRY_POINT()
{
	return AntiCheatComponent::getInstance();
}
