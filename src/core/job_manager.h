/**
 * @file job_manager.h
 * @brief single-threaded, frame budgeted job manager
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_JOB_MANAGER_H
#define GEOMETRY_SANDBOX_JOB_MANAGER_H

#include "core/base_app_component.h"
#include "core/types.h"

#include "boost/di.hpp"

#include <functional>
#include <memory>
#include <queue>

namespace gs
{
/**
 * @brief single-threaded, frame budgeted job manager
 *
 * The design is dictated by the target environment: under Emscripten/WebAssembly,
 * browsers that have implemented and enabled SharedArrayBuffer, are gating it behind
 * Cross Origin Opener Policy (COOP) and Cross Origin Embedder Policy (COEP) headers.
 * Pthreads code will not work in deployed environment unless these headers are
 * correctly set. https://emscripten.org/docs/porting/pthreads.html.
 *
 * As a result, real threading cannot be relied upon, and cooperative, budget-limited
 * execution within each frame is used instead.
 */
class IJobManager
{
  public:
	using Job = std::function<void()>;

	/** @brief execute jobs in the current thread with the limited budget */
	virtual void tick(Nanoseconds budget) = 0;

	/** @brief schedule the job */
	virtual void schedule(Job&&) = 0;

	virtual ~IJobManager()		 = default;
};

/** @brief basic IJobManager implementation */
class JobManager : public BaseManager, public IJobManager
{
	std::queue<Job> queue_;

  public:
	JobManager(const std::shared_ptr<Settings>& settings, const std::shared_ptr<ILogManager>& log) : BaseManager(settings, log) {}

	virtual void tick(Nanoseconds budget) override;
	virtual void schedule(Job&&) override;
};

namespace di
{
inline auto jobManager() noexcept
{
	return boost::di::bind<IJobManager>.to<JobManager>();
}
}  // namespace di
}  // namespace gs
#endif	// GEOMETRY_SANDBOX_JOB_MANAGER_H
