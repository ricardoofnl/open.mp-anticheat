#include "version_check.hpp"

#include <string>

#include "../config.hpp"
#include "../player_data.hpp"
#include "signatures.hpp"

// signatures::versionIndex wants a null-terminated string; the SDK StringView isn't guaranteed to be.
static std::string verName(IPlayer& player)
{
	StringView v = player.getClientVersionName();
	return std::string(v.data(), v.size());
}

bool VersionCheck::enabled(const Config& cfg) const
{
	return cfg.moduleVersion();
}

void VersionCheck::onConnect(IPlayer& player, PlayerACData& data)
{
	// read the four client addresses; they must stay constant across reads.
	for (int i = 0; i < 4; ++i)
	{
		data.clientAddrSent[i] = signatures::kClientAddr[i];
		data.lastClientRetn[i] = 0;
		player.sendClientCheck(0x45, signatures::kClientAddr[i], 0x0, 0x4);
	}

	const int idx = signatures::versionIndex(verName(player).c_str());
	if (idx < 0)
		return;

	data.sampCheckAddr = signatures::kSampAddr[idx];
	for (int k = 0; k < 3; ++k)
		player.sendClientCheck(0x45, data.sampCheckAddr, 0x0, 0x4);

	// extra connect-wave samp.dll check - non-omp clients only (reference !isplayerusingomp guard; the omp client would false-positive).
	if (!player.isUsingOmp())
	{
		const int off = ctx_.nextMemOffset();
		const int base = signatures::kAddCheckConnect[idx][0] + signatures::kAddCheckConnect[idx][1];
		data.addCheckConnectAddr = base - off;
		player.sendClientCheck(0x45, base - off, off, 0x4);
	}
}

void VersionCheck::onSpawn(IPlayer& player, PlayerACData& data)
{
	const int idx = signatures::versionIndex(verName(player).c_str());
	if (idx < 0 || player.isUsingOmp())
		return;

	const int off = ctx_.nextMemOffset();
	const int base = signatures::kAddCheckSpawn[idx][0] + signatures::kAddCheckSpawn[idx][1];
	data.addCheckSpawnAddr = base - off;
	player.sendClientCheck(0x45, base - off, off, 0x4);
}

void VersionCheck::onCheckResponse(IPlayer& player, PlayerACData& data, int actionType, int address, int results)
{
	(void)player;
	if (actionType != 0x45)
		return;

	// connect-wave extra check: a clean client answers 128.
	if (data.addCheckConnectAddr != 0 && address == data.addCheckConnectAddr && (results & 0xFF) != 128)
		data.pending[Cheat_CLEO_7] = true;

	// spawn-wave extra check: a clean client answers 192.
	if (data.addCheckSpawnAddr != 0 && address == data.addCheckSpawnAddr && (results & 0xFF) != 192)
		data.pending[Cheat_ModdedClient] = true;

	// repeated client-address reads must return the same value every time.
	for (int i = 0; i < 4; ++i)
	{
		if (address == data.clientAddrSent[i])
		{
			if (data.lastClientRetn[i] == 0)
				data.lastClientRetn[i] = results;
			else if (data.lastClientRetn[i] != results)
				data.clientAnomaly = true;
			break;
		}
	}
}

void VersionCheck::onEvaluate(IPlayer& player, PlayerACData& data)
{
	(void)player;
	if (data.clientAnomaly)
		data.pending[Cheat_S0beit_RakNet] = true;
}
