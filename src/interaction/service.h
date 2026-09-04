/**
 * @file service.h
 * @brief main service for interaction system
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_INTERACTION_SERVICE_H
#define GEOMETRY_SANDBOX_INTERACTION_SERVICE_H

#include "core/base_app_component.h"
#include "core/ecs.h"
#include "interaction/ecs_components.h"

#include "boost/di.hpp"

#include <memory>

namespace gs
{
struct Settings;
class IInputManager;
class IUIManager;
class IUIStateManager;
class ICameraService;
class ITranslateGizmoUpdateService;

/**
 * @brief Interaction system facade
 *
 * @todo rename to interaction::IFacade or IInteractionFacade?
 */
class IInteractionService
{
  public:
	virtual void init()			   = 0;
	virtual void update()		   = 0;
	virtual ~IInteractionService() = default;
};

/** @brief basic IInteractionService implementation */
class InteractionService : public BaseService, public IInteractionService
{
	entt::entity selected_ = entt::null;
	entt::entity hovered_  = entt::null;

	std::shared_ptr<ecs::Registry> registry_;
	std::shared_ptr<IInputManager> inputManager_;
	std::shared_ptr<IUIManager> uiManager_;
	std::shared_ptr<IUIStateManager> uiStateManager_;
	std::shared_ptr<ICameraService> cameraService_;
	std::shared_ptr<ITranslateGizmoUpdateService> translateGizmoService_;

  public:
	InteractionService(const std::shared_ptr<Settings>& settings,
					   const std::shared_ptr<ILogManager>& log,
					   const std::shared_ptr<ecs::Registry>& registry,
					   const std::shared_ptr<IInputManager>& inputManager,
					   const std::shared_ptr<IUIManager>& uiManager,
					   const std::shared_ptr<IUIStateManager>& uiStateManager,
					   const std::shared_ptr<ICameraService>& cameraService,
					   const std::shared_ptr<ITranslateGizmoUpdateService>& translateGizmoService)
		: BaseManager(settings, log),					 //
		  registry_(registry),							 //
		  inputManager_(inputManager),					 //
		  uiManager_(uiManager),						 //
		  uiStateManager_(uiStateManager),				 //
		  cameraService_(cameraService),				 //
		  translateGizmoService_(translateGizmoService)	 //
	{
	}

	virtual void init() override;
	virtual void update() override;
};

namespace di
{
inline auto interactionService() noexcept
{
	return boost::di::bind<IInteractionService>.to<InteractionService>();
}
}  // namespace di
}  // namespace gs

#endif	// GEOMETRY_SANDBOX_INTERACTION_SERVICE_H
