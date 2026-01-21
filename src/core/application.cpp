#include "core/application.h"

#include "animation/manager.h"
#include "core/camera_controller_service.h"
#include "core/ecs.h"
#include "core/input_manager.h"
#include "core/log_manager.h"
#include "core/resource_manager.h"
#include "core/types.h"
#include "core/window_manager.h"
#include "interaction/service.h"
#include "render/facade.h"
#include "render/viewport_manager.h"
#include "ui/manager.h"

#include <memory>

namespace gs
{
Application::Application(const std::shared_ptr<ecs::Registry>& registry,
						 const std::shared_ptr<IInputManager>& inputManager,
						 const std::shared_ptr<IWindowManager>& windowManager,
						 const std::shared_ptr<IResourceManager>& resourceManager,
						 const std::shared_ptr<animation::IManager>& animationManager,
						 const std::shared_ptr<IUIManager>& uiManager,
						 const std::shared_ptr<render::IFacade>& renderer,
						 const std::shared_ptr<ICameraControllerService>& cameraController,
						 const std::shared_ptr<IInteractionService>& interationService,
						 const std::shared_ptr<render::IViewportManager>& viewportManager,
						 const std::shared_ptr<ILogManager>& logManager)
	: registry_(registry),					  //
	  inputManager_(inputManager),			  //
	  windowManager_(windowManager),		  //
	  resourceManager_(resourceManager),	  //
	  animationManager_(animationManager),	  //
	  uiManager_(uiManager),				  //
	  renderer_(renderer),					  //
	  cameraController_(cameraController),	  //
	  interationService_(interationService),  //
	  viewportManager_(viewportManager),	  //
	  logManager_(logManager)				  //
{
	windowManager_->initWindow();
	resourceManager_->load();  // should be inited after window created
	uiManager_->init();		   // should be inited after window created
	viewportManager_->init();  // should be inited after ui manager created
	interationService_->init();
}

void Application::drawNextFrame(Nanoseconds timeDelta)
{
	const Timepoint startedAt = Clock::now();
	ui::State& ui			  = uiManager_->getState();

	{  // update
		const Timepoint measurementStart = Clock::now();
		logManager_->clearFrameLog();
		windowManager_->tick();
		inputManager_->tick();
		uiManager_->tick();
		cameraController_->update(ui.cameras[ui.activeCameraIndex]);
		ui.performance.tick = Clock::now() - measurementStart;
	}

	{  // Animation
		const Timepoint measurementStart = Clock::now();
		animationManager_->tick(timeDelta);
		animationManager_->animate(*registry_, ui);
		ui.performance.animation = Clock::now() - measurementStart;
	}

	{  // Interaction
		const Timepoint measurementStart = Clock::now();
		interationService_->update();
		ui.performance.interaction = Clock::now() - measurementStart;
	}

	renderer_->render();
	ui.performance.total = Clock::now() - startedAt;
}

void Application::run()
{
	bool running = true;
	windowManager_->onClose([&] { running = false; });

	Nanoseconds timeDelta{0};
	Timepoint prev = Clock::now();
	while (running)
	{
		const Timepoint now = Clock::now();
		timeDelta			= now - prev;
		prev				= now;

		drawNextFrame(timeDelta);
	}
}
};	// namespace gs
