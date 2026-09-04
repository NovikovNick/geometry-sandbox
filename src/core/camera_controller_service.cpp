#include "core/camera_controller_service.h"

#include "core/base_app_component.h"
#include "core/camera_service.h"
#include "core/input_manager.h"
#include "core/math.h"
#include "core/settings.h"
#include "core/types.h"

#include <memory>
#include <stdexcept>

namespace gs
{
CameraControllerService::CameraControllerService(const std::shared_ptr<Settings>& settings,
												 const std::shared_ptr<ILogManager>& log,
												 const std::shared_ptr<IInputManager>& inputManager,
												 const std::shared_ptr<ICameraService>& cameraService)
	: BaseService(settings, log), controllerType_(CameraControllerType::Free), inputManager_(inputManager),
	  cameraService_(cameraService)
{
}

void CameraControllerService::update(Camera& camera) const
{
	switch (controllerType_)
	{
		case CameraControllerType::Free: updateFree(camera); break;
		case CameraControllerType::Orbit: updateOrbit(camera); break;
		default: throw std::runtime_error("invalid camera controller type");
	}
}

void gs::CameraControllerService::updateFree(Camera& camera) const
{
	const float moveSensitivity = settings_->cameraMoveSensitivity;
	if (inputManager_->isKeyPressed(InputKey::Forward))
	{
		const Vec3 offset = cameraService_->getForward(camera) * moveSensitivity;

		camera.position += offset;
		camera.target += offset;
	}
	if (inputManager_->isKeyPressed(InputKey::Backward))
	{
		const Vec3 offset = cameraService_->getForward(camera) * -moveSensitivity;

		camera.position += offset;
		camera.target += offset;
	}
	if (inputManager_->isKeyPressed(InputKey::Left))
	{
		const Vec3 offset = cameraService_->getRight(camera) * -moveSensitivity;
		camera.position += offset;
		camera.target += offset;
	}
	if (inputManager_->isKeyPressed(InputKey::Right))
	{
		const Vec3 offset = cameraService_->getRight(camera) * moveSensitivity;
		camera.position += offset;
		camera.target += offset;
	}

	if (inputManager_->isKeyPressed(InputKey::MouseRight))
	{
		const Vec2 delta = inputManager_->getCursorDelta() * settings_->cameraRotateSensitivity;
		cameraService_->rotateYaw(camera, delta.x());
		cameraService_->rotatePitch(camera, delta.y());
	}
}

void gs::CameraControllerService::updateOrbit(Camera& camera) const
{
	const float moveSensitivity = settings_->cameraMoveSensitivity;

	if (inputManager_->isKeyPressed(InputKey::Forward))
	{
		camera.position += cameraService_->getUp(camera) * moveSensitivity;
	}
	if (inputManager_->isKeyPressed(InputKey::Backward))
	{
		camera.position += cameraService_->getUp(camera) * -moveSensitivity;
	}
	if (inputManager_->isKeyPressed(InputKey::Left))
	{
		camera.position += cameraService_->getRight(camera) * -moveSensitivity;
	}
	if (inputManager_->isKeyPressed(InputKey::Right))
	{
		camera.position += cameraService_->getRight(camera) * moveSensitivity;
	}
}

}  // namespace gs
