#include "core/camera_controller_service.h"

#include "core/camera_service.h"
#include "core/input_manager.h"
#include "core/math.h"
#include "core/settings.h"
#include "core/types.h"

#include <memory>

namespace gs
{
CameraControllerService::CameraControllerService(const std::shared_ptr<Settings>& settings,
												 const std::shared_ptr<IInputManager>& inputManager,
												 const std::shared_ptr<ICameraService>& cameraService)
	: settings_(settings), inputManager_(inputManager), cameraService_(cameraService)
{
	inputManager->onPress(InputKey::MouseRight, [&] { rotateCamera_ = !rotateCamera_; });
}

void CameraControllerService::update(Camera& camera) const
{
	const float moveSensitivity = settings_->cameraMoveSensitivity;

	if (camera.perspective)
	{
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
	}
	else
	{
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

	if (rotateCamera_ && camera.perspective)
	{
		const Vec2 delta = inputManager_->getCursorDelta() * settings_->cameraRotateSensitivity;
		cameraService_->rotateYaw(camera, delta.x());
		cameraService_->rotatePitch(camera, delta.y());
	}
}
}  // namespace gs
