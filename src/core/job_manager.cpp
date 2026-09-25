#include "core/job_manager.h"

#include "core/log_manager.h"
#include "core/types.h"

namespace gs
{
JobManager::JobManager(const std::shared_ptr<Settings>& settings, const std::shared_ptr<ILogManager>& log) : BaseManager(settings, log)
{
	log_->log("JobManager constructed");
}
void JobManager::tick(Nanoseconds budget)
{
	const Timepoint t0 = Clock::now();

	while ((Clock::now() - t0) < budget && !queue_.empty())
	{
		log_->log("Job begin");
		queue_.front()();
		queue_.pop();
		log_->log("Job end");
	}
}

void JobManager::schedule(Job&& job)
{
	queue_.push(std::move(job));
}

}  // namespace gs