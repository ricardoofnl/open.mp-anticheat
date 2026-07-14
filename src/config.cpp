#include "config.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include "detection/signatures.hpp"

namespace
{

std::string trim(const std::string& s)
{
	size_t a = 0, b = s.size();
	while (a < b && std::isspace(static_cast<unsigned char>(s[a])))
		++a;
	while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1])))
		--b;
	return s.substr(a, b - a);
}

std::string lower(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c)
		{ return static_cast<char>(std::tolower(c)); });
	return s;
}

std::vector<std::string> splitCsv(const std::string& s)
{
	std::vector<std::string> out;
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, ','))
	{
		item = trim(item);
		if (!item.empty())
			out.push_back(item);
	}
	return out;
}

CheatAction parseAction(const std::string& v, CheatAction def)
{
	std::string s = lower(trim(v));
	if (s == "ignore" || s == "off" || s == "0")
		return CheatAction::Ignore;
	if (s == "warn" || s == "allow" || s == "log")
		return CheatAction::Warn;
	if (s == "kick")
		return CheatAction::Kick;
	if (s == "ban")
		return CheatAction::Ban;
	return def;
}

} // namespace

void Config::load(const std::string& path, ILogger* logger)
{
	std::ifstream f(path);
	if (f)
	{
		std::string line;
		while (std::getline(f, line))
		{
			// strip inline comments and surrounding whitespace.
			size_t hash = line.find('#');
			if (hash != std::string::npos)
				line = line.substr(0, hash);
			line = trim(line);
			if (line.empty())
				continue;
			size_t eq = line.find('=');
			if (eq == std::string::npos)
				continue;
			std::string key = lower(trim(line.substr(0, eq)));
			std::string val = trim(line.substr(eq + 1));
			if (!key.empty())
				kv_[key] = val;
		}
		if (logger)
			logger->printLn("[AC] loaded config from %s (%zu keys)", path.c_str(), kv_.size());
	}
	else if (logger)
	{
		logger->printLn("[AC] no config at %s, using defaults", path.c_str());
	}

	applyKnownKeys();
}

void Config::applyKnownKeys()
{
	enabled_ = getBool("enabled", enabled_);
	logOnly_ = getBool("log_only", logOnly_);
	announce_ = getBool("announce", announce_);
	banMinutes_ = getInt("ban_duration_minutes", banMinutes_);
	kickDelayMs_ = getInt("kick_delay_ms", kickDelayMs_);
	evalDelayMs_ = getInt("evaluate_delay_ms", evalDelayMs_);
	spawnDelayMs_ = getInt("spawn_recheck_delay_ms", spawnDelayMs_);

	modMemory_ = getBool("module.memory", modMemory_);
	modVersion_ = getBool("module.version", modVersion_);
	modPoison_ = getBool("module.poison", modPoison_);
	modMobile_ = getBool("module.mobile", modMobile_);
	modRaknet_ = getBool("module.raknet", modRaknet_);

	// allowed client versions: config override, else the reference whitelist.
	auto it = kv_.find("allowed_versions");
	if (it != kv_.end() && !it->second.empty())
		allowedVersions_ = splitCsv(it->second);
	if (allowedVersions_.empty())
	{
		for (int i = 0; i < signatures::kAllowedClientCount; ++i)
			allowedVersions_.emplace_back(signatures::kAllowedClients[i]);
	}

	mobileGpci_ = get("mobile_gpci", signatures::kMobileGpci);
	// mobile_checksum may be given as decimal or 0x-prefixed hex.
	{
		std::string cs = get("mobile_checksum", "");
		if (!cs.empty())
			mobileChecksum_ = static_cast<uint16_t>(std::strtoul(cs.c_str(), nullptr, 0));
		else
			mobileChecksum_ = signatures::kMobileChecksum;
	}
}

std::string Config::get(const std::string& key, const std::string& def) const
{
	auto it = kv_.find(key);
	return it != kv_.end() ? it->second : def;
}

bool Config::getBool(const std::string& key, bool def) const
{
	auto it = kv_.find(key);
	if (it == kv_.end())
		return def;
	std::string v = lower(trim(it->second));
	return v == "1" || v == "true" || v == "yes" || v == "on";
}

int Config::getInt(const std::string& key, int def) const
{
	auto it = kv_.find(key);
	if (it == kv_.end())
		return def;
	try
	{
		return std::stoi(it->second, nullptr, 0);
	}
	catch (...)
	{
		return def;
	}
}

CheatAction Config::actionFor(CheatId id) const
{
	const signatures::CheatInfo& info = signatures::cheatInfo(id);
	auto it = kv_.find(std::string("action.") + info.configKey);
	if (it != kv_.end())
		return parseAction(it->second, info.defaultAction);
	return info.defaultAction;
}

bool Config::isAllowedVersion(StringView version) const
{
	for (const std::string& v : allowedVersions_)
	{
		if (version == StringView(v.c_str()))
			return true;
	}
	return false;
}
