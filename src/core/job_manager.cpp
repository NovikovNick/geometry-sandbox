#include "core/job_manager.h"

#include "core/types.h"

namespace gs
{
void JobManager::tick(Nanoseconds budget)
{
	const Timepoint t0 = Clock::now();

	while ((Clock::now() - t0) < budget && !queue_.empty())
	{
		queue_.front()();
		queue_.pop();
	}
}

void JobManager::schedule(Job&& job)
{
	queue_.push(std::move(job));
}

}  // namespace gs