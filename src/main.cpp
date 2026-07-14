// open.mp anti-cheat component - c++ port of pevenaider/antycheat on the open.mp sdk.

#include <sdk.hpp>

#include "anticheat_component.hpp"

// called by the server when it loads the binary; returns the component singleton.
COMPONENT_ENTRY_POINT()
{
	return AntiCheatComponent::getInstance();
}
