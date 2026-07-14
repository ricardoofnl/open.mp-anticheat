#pragma once

// ontick-driven deferred-task scheduler (replaces settimerex); tasks tagged by owner id (player id) for cancel-on-disconnect.

#include <chrono>
#include <functional>
#include <vector>

class Scheduler
{
public:
	using Clock = std::chrono::steady_clock;
	using Task = std::function<void()>;

	// run `task` after `delay`. `ownerid` lets us cancel it if that player
	// disconnects before it fires.
	void after(int ownerId, std::chrono::milliseconds delay, Task task);

	// fire every task whose deadline has passed. call once per server tick.
	void process();

	// drop all pending tasks belonging to `ownerid`.
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
