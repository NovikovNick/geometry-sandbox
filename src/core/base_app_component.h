/**
 * @file base_app_component.h
 * @brief Base class with logger and settings
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_BASE_APP_COMPONENT_H
#define GEOMETRY_SANDBOX_BASE_APP_COMPONENT_H

#include <memory>

namespace gs
{
struct Settings;
class ILogManager;

/** @brief Contains logger and settings */
class BaseAppComponent
{
  protected:
	std::shared_ptr<Settings> settings_;
	std::shared_ptr<ILogManager> log_;

  public:
	BaseAppComponent(const std::shared_ptr<Settings>& settings, const std::shared_ptr<ILogManager>& log)
		: settings_(settings), log_(log)
	{
	}

	virtual ~BaseAppComponent() = default;
};

/** @brief Base for each class that contains state and logic.*/
using BaseManager = BaseAppComponent;

/** @brief Base for each class that contains only logic.*/
using BaseService = BaseAppComponent;

}  // namespace gs
#endif	// GEOMETRY_SANDBOX_BASE_APP_COMPONENT_H
