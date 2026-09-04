/**
 * @file log_manager.h
 * @brief Provides frame-scoped logs and general logs
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_LOG_MANAGER_H
#define GEOMETRY_SANDBOX_LOG_MANAGER_H

#include "core/types.h"

#include "boost/di.hpp"

#include <format>
#include <iostream>
#include <string>
#include <vector>

namespace gs
{

/** @brief Provides frame-scoped logs and general logs */
class ILogManager
{
  public:
	/** @brief log, which is cleared every frame */
	template <class... Args>
	void frameLog(const std::format_string<Args...> format, Args&&... args)
	{
		addFrameLog(std::vformat(format.get(), std::make_format_args(args...)));
	}

	/**
	 * @brief It is convenient to display this log in the overlay console.
	 */
	virtual const std::vector<std::string>& getFrameLog() const = 0;

	/**
	 * @brief should be called at the beginning of each frame.
	 */
	virtual void clearFrameLog() = 0;

	/**
	 * @brief general log
	 */
	template <class... Args>
	void log(const std::format_string<Args...> format, Args&&... args)
	{
		addLog(std::vformat(format.get(), std::make_format_args(args...)));
	}

	virtual ~ILogManager() = default;

  protected:
	virtual void addFrameLog(const std::string&) = 0;
	virtual void addLog(const std::string&)		 = 0;
};

/** @brief basic ILogManager implementation */
class LogManager : public ILogManager
{
	std::vector<std::string> frameLogs_;

  public:
	virtual const std::vector<std::string>& getFrameLog() const override { return frameLogs_; };
	virtual void clearFrameLog() override { frameLogs_.clear(); };

  protected:
	virtual void addFrameLog(const std::string& str) override { frameLogs_.push_back(str); };
	virtual void addLog(const std::string& str) override { std::cout << str << std::endl; };
};

namespace di
{
inline auto logManager() noexcept
{
	return boost::di::bind<ILogManager>.to<LogManager>();
}
}  // namespace di
}  // namespace gs
#endif	// GEOMETRY_SANDBOX_LOG_MANAGER_H
