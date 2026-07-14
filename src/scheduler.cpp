#include "scheduler.hpp"

#include <utility>

void Scheduler::after(int ownerId, std::chrono::milliseconds delay, Task task)
{
	entries_.push_back(Entry { Clock::now() + delay, ownerId, std::move(task) });
}

void Scheduler::process()
{
	if (entries_.empty())
		return;

	const Clock::time_point now = Clock::now();

	// Move due tasks out first so a task that schedules more work (or cancels an owner) can't invalidate the container.
	std::vector<Task> due;
	for (size_t i = 0; i < entries_.size();)
	{
		if (entries_[i].when <= now)
		{
			due.push_back(std::move(entries_[i].task));
			if (i != entries_.size() - 1)
				entries_[i] = std::move(entries_.back());
			entries_.pop_back();
		}
		else
		{
			++i;
		}
	}

	for (Task& t : due)
	{
		if (t)
			t();
	}
}

void Scheduler::cancelOwner(int ownerId)
{
	for (size_t i = 0; i < entries_.size();)
	{
		if (entries_[i].ownerId == ownerId)
		{
			if (i != entries_.size() - 1)
				entries_[i] = std::move(entries_.back());
			entries_.pop_back();
		}
		else
		{
			++i;
		}
	}
}
