#include "mobile_check.hpp"

#include <cctype>

#include <bitstream.hpp>

#include "../config.hpp"
#include "../player_data.hpp"

namespace
{
bool equalsIgnoreCase(StringView a, StringView b)
{
	if (a.size() != b.size())
		return false;
	for (size_t i = 0; i < a.size(); ++i)
	{
		if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i])))
			return false;
	}
	return true;
}
}

bool MobileCheck::enabled(const Config& cfg) const
{
	return cfg.moduleMobile();
}

bool MobileCheck::onReceive(IPlayer& peer, NetworkBitStream& bs)
{
	// peek the trailing uint16 checksum, then restore the read pointer so the server processes the join normally. never block
	const int startOffset = bs.GetReadOffset();

	uint32_t version = 0, challenge = 0;
	uint8_t mod = 0, nickLen = 0, authLen = 0, cverLen = 0;
	uint16_t checksum = 0;

	bool ok = bs.readUINT32(version) && bs.readUINT8(mod) && bs.readUINT8(nickLen);
	if (ok)
	{
		bs.IgnoreBits(int(nickLen) * 8);
		ok = bs.readUINT32(challenge) && bs.readUINT8(authLen);
	}
	if (ok)
	{
		bs.IgnoreBits(int(authLen) * 8);
		ok = bs.readUINT8(cverLen);
	}
	if (ok)
	{
		bs.IgnoreBits(int(cverLen) * 8);
		ok = bs.readUINT16(checksum);
	}

	bs.SetReadOffset(startOffset);

	if (ok)
		pendingChecksum_[&peer] = checksum;
	return true;
}

void MobileCheck::onConnect(IPlayer& player, PlayerACData& data)
{
	uint16_t checksum = 0;
	auto it = pendingChecksum_.find(&player);
	if (it != pendingChecksum_.end())
	{
		checksum = it->second;
		pendingChecksum_.erase(it);
	}
	data.joinChecksum = checksum;

	const std::string& gpci = ctx_.config().mobileGpci();
	if (!equalsIgnoreCase(player.getSerial(), StringView(gpci.c_str())))
		return; // not a mobile serial - nothing to decide

	if (checksum == ctx_.config().mobileChecksum())
		data.mobilePlayer = true; // verified legit mobile client
	else
		ctx_.report(player, data, Cheat_FakeMobile); // mobile serial, wrong checksum
}
