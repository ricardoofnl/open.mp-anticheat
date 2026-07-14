#pragma once

// the interface every detection technique implements; the component fans events to modules and each reports via iaccontext::report.

#include <sdk.hpp>

#include "../ac_types.hpp"

class PlayerACData;
class Config;
class Scheduler;

// shared services handed to every module, plus the single reporting path.
struct IACContext
{
	virtual ~IACContext() { }

	virtual ICore* core() = 0;
	virtual const Config& config() const = 0;
	virtual Scheduler& scheduler() = 0;

	// a fresh randomised 4-aligned offset for a memory check (reference setmemoffset), shared by memory and version modules.
	virtual int nextMemOffset() = 0;

	// report a detection: applies exceptions/dedup/config action/logging/pawn callback/enforcement. modules never kick directly.
	virtual void report(IPlayer& player, PlayerACData& data, CheatId cheat) = 0;
};

struct IDetectionModule
{
	virtual ~IDetectionModule() { }

	virtual const char* name() const = 0;

	// whether this module runs, given the config.
	virtual bool enabled(const Config& cfg) const = 0;

	// a player connected: (re)set per-module state and fire any client checks.
	virtual void onConnect(IPlayer& player, PlayerACData& data) { }

	// a player spawned: used for the second wave of checks in the reference.
	virtual void onSpawn(IPlayer& player, PlayerACData& data) { }

	// the client answered a sendclientcheck; modules ignore action types that aren't theirs (0x5=memory, 0x45=version, 0x47/0x48=poison).
	virtual void onCheckResponse(IPlayer& player, PlayerACData& data, int actionType, int address, int results) { }

	// deadline reached (~evaluate_delay_ms after connect): inspect collected responses and flag anything found.
	virtual void onEvaluate(IPlayer& player, PlayerACData& data) { }
};
