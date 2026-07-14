#pragma once

// onTick-driven deferred-task scheduler (replaces SetTimerEx); tasks tagged by owner id (player id) for cancel-on-disconnect.

#include <chrono>
#include <functional>
#include <vector>

class Scheduler
{
public:
	using Clock = std::chrono::steady_clock;
	using Task = std::function<void()>;

	// Run `task` after `delay`. `ownerId` lets us cancel it if that player
	// disconnects before it fires.
	void after(int ownerId, std::chrono::milliseconds delay, Task task);

	// Fire every task whose deadline has passed. Call once per server tick.
	void process();

	// Drop all pending tasks belonging to `ownerId`.
	void cancelOwner(int ownerId);

	void clear() { entries_.clear(); }

private:
	struct Entry
	{
		Clock::time_point when;
		int ownerId;
		Task task;
	};

	std::vector<Entry> entries_;
};
