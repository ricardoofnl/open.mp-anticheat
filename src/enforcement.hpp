#pragma once

// Punishment mechanics (messages, delayed kicks, timed bans); policy lives in AntiCheatComponent::report.

#include <sdk.hpp>

class Config;
class Scheduler;

class Enforcement
{
public:
	Enforcement(ICore* core, const Config& cfg, Scheduler& sched)
		: core_(core)
		, cfg_(cfg)
		, sched_(sched)
	{
	}

	// Log + tell the player, but let them keep playing.
	void warn(IPlayer& player, const char* cheatName);

	// Message the player then kick after the configured delay.
	void kick(IPlayer& player, const char* cheatName);

	// Message the player, add a timed ban on their address, then kick.
	void ban(IPlayer& player, const char* cheatName);

private:
	void scheduleKick(IPlayer& player);

	ICore* core_;
	const Config& cfg_;
	Scheduler& sched_;
};
