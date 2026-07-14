#pragma once

// The interface every detection technique implements; the component fans events to modules and each reports via IACContext::report.

#include <sdk.hpp>

#include "../ac_types.hpp"

class PlayerACData;
class Config;
class Scheduler;

// Shared services handed to every module, plus the single reporting path.
struct IACContext
{
	virtual ~IACContext() { }

	virtual ICore* core() = 0;
	virtual const Config& config() const = 0;
	virtual Scheduler& scheduler() = 0;

	// A fresh randomised 4-aligned offset for a memory check (reference setMemOffset), shared by memory and version modules.
	virtual int nextMemOffset() = 0;

	// Report a detection: applies exceptions/dedup/config action/logging/Pawn callback/enforcement. Modules never kick directly.
	virtual void report(IPlayer& player, PlayerACData& data, CheatId cheat) = 0;
};

struct IDetectionModule
{
	virtual ~IDetectionModule() { }

	virtual const char* name() const = 0;

	// Whether this module runs, given the config.
	virtual bool enabled(const Config& cfg) const = 0;

	// A player connected: (re)set per-module state and fire any client checks.
	virtual void onConnect(IPlayer& player, PlayerACData& data) { }

	// A player spawned: used for the second wave of checks in the reference.
	virtual void onSpawn(IPlayer& player, PlayerACData& data) { }

	// The client answered a sendClientCheck; modules ignore action types that aren't theirs (0x5=memory, 0x45=version, 0x47/0x48=poison).
	virtual void onCheckResponse(IPlayer& player, PlayerACData& data, int actionType, int address, int results) { }

	// Deadline reached (~evaluate_delay_ms after connect): inspect collected responses and flag anything found.
	virtual void onEvaluate(IPlayer& player, PlayerACData& data) { }
};
