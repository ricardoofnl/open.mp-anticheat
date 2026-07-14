#include "poison_check.hpp"

#include "../config.hpp"
#include "../player_data.hpp"

bool PoisonCheck::enabled(const Config& cfg) const
{
	return cfg.modulePoison();
}

void PoisonCheck::onConnect(IPlayer& player, PlayerACData& data)
{
	if (data.mobilePlayer)
		return;

	// seed suspicion, then send the real 0x47/0x48 reads of address 0; a clean client's answer (address 0) clears it in oncheckresponse.
	data.suspicious = true;
	player.sendClientCheck(0x47, 0x0, 0x0, 0x4);
	player.sendClientCheck(0x48, 0x0, 0x0, 0x4);
}

void PoisonCheck::onCheckResponse(IPlayer& player, PlayerACData& data, int actionType, int address, int results)
{
	(void)player;
	// only a client that reflects the poisoned address/value keeps suspicion.
	if (actionType == 0x47)
		data.suspicious = (address == 0xCECECE && (results & 0xFF) == 255);
	else if (actionType == 0x48)
		data.suspicious = (address == 0xDEDEDE && (results & 0xFF) == 255);
}

void PoisonCheck::onEvaluate(IPlayer& player, PlayerACData& data)
{
	(void)player;
	if (data.suspicious && !data.mobilePlayer)
		data.pending[Cheat_ModdedClient] = true;
}
