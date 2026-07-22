#pragma once

// config loaded from a key=value file (components/anticheat.cfg); every key is optional.

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "ac_types.hpp"

class Config
{
public:
	// parse `path` if it exists; otherwise keep defaults.
	void load(const std::string& path);

	bool enabled() const { return enabled_; }
	bool logOnly() const { return logOnly_; } // detect + log but never punish
	bool announceToAdmins() const { return announce_; }

	int banDurationMinutes() const { return banMinutes_; }
	int kickDelayMs() const { return kickDelayMs_; }
	int evaluateDelayMs() const { return evalDelayMs_; }
	int spawnRecheckDelayMs() const { return spawnDelayMs_; }

	bool moduleMemory() const { return modMemory_; }
	bool moduleVersion() const { return modVersion_; }
	bool modulePoison() const { return modPoison_; }
	bool moduleMobile() const { return modMobile_; } // raw raknet, off by default
	bool moduleRaknet() const { return modRaknet_; } // raw raknet, off by default
	bool moduleFaker5() const { return modFaker5_; } // faker5 / clientcheck-spoofer detector

	// resolve the configured action for a cheat (per-cheat override, else the
	// signature's built-in default).
	CheatAction actionFor(CheatId id) const;

	bool isAllowedVersion(const char* version) const;
	const std::vector<std::string>& allowedVersions() const { return allowedVersions_; }
	const std::string& mobileGpci() const { return mobileGpci_; }
	uint16_t mobileChecksum() const { return mobileChecksum_; }

private:
	std::string get(const std::string& key, const std::string& def) const;
	bool getBool(const std::string& key, bool def) const;
	int getInt(const std::string& key, int def) const;
	void applyKnownKeys();

	std::unordered_map<std::string, std::string> kv_;

	bool enabled_ = true;
	bool logOnly_ = false;
	bool announce_ = true;
	int banMinutes_ = 180;
	int kickDelayMs_ = 1500;
	int evalDelayMs_ = 2500;
	int spawnDelayMs_ = 1800;
	bool modMemory_ = true;
	bool modVersion_ = true;
	bool modPoison_ = true;
	bool modMobile_ = false;
	bool modRaknet_ = false;
	bool modFaker5_ = true;
	std::vector<std::string> allowedVersions_;
	std::string mobileGpci_;
	uint16_t mobileChecksum_ = 0xBEEF;
};
