/**
 * @file service.h
 * @brief main service for interaction system
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_INTERACTION_SERVICE_H
#define GEOMETRY_SANDBOX_INTERACTION_SERVICE_H

#include "core/ecs.h"
#include "interaction/ecs_components.h"

#include "boost/di.hpp"

#include <memory>

namespace gs
{
struct Settings;
class IInputManager;
class IUIManager;
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
class InteractionService : public IInteractionService
{
	entt::entity selected_ = entt::null;
	entt::entity hovered_  = entt::null;

	std::shared_ptr<Settings> settings_;
	std::shared_ptr<ecs::Registry> registry_;
	std::shared_ptr<IInputManager> inputManager_;
	std::shared_ptr<IUIManager> uiManager_;
	std::shared_ptr<ICameraService> cameraService_;
	std::shared_ptr<ITranslateGizmoUpdateService> translateGizmoService_;

  public:
	InteractionService(std::shared_ptr<Settings> settings,
					   std::shared_ptr<ecs::Registry> registry,
					   std::shared_ptr<IInputManager> inputManager,
					   std::shared_ptr<IUIManager> uiManager,
					   std::shared_ptr<ICameraService> cameraService,
					   std::shared_ptr<ITranslateGizmoUpdateService> translateGizmoService)
		: settings_(settings),							 //
		  registry_(registry),							 //
		  inputManager_(inputManager),					 //
		  uiManager_(uiManager),						 //
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
