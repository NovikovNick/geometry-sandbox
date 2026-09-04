#include "render/frustum_draw_service.h"

#include "core/camera_service.h"
#include "core/settings.h"
#include "render/low_level_service.h"
#include "render/viewport_manager.h"

#include "raylib.h"
#include "rlgl.h"

#include "core/math.h"
#include "core/types.h"

#include <array>
#include <cmath>
#include <cstddef>

namespace gs::render
{

/** @brief adapt raylib's DrawMeshInstanced */
void FrustumDrawService::drawFrustum(std::size_t cameraIndex, const Camera& camera) const
{
	drawFrustumWires(camera);

	rlPushMatrix();
	{
		// Translate and rotate to camera position and rotation
		const Mat4 model = cameraService_->getViewMatrix(camera).inverse();
		rlMultMatrixf(model.data());

		rlPushMatrix();
		{
			//  temporary solution. Should be removed after rotation implement
			if (camera.handedness == settings_->defaultCamera.handedness)
			{
				rlRotatef(180.0F, 0, 1, 0);	 // NOLINT(*-magic-numbers)
			}

			// Move it a little so that the camera origin is right in the lens area.
			rlTranslatef(0, 0, -0.25F);	 // NOLINT(*-magic-numbers)

			graphic_->drawModel(ModelType::Camera, /*pos*/ Vec3::Zero(), 1.0F);
		}
		rlPopMatrix();

		if (camera.handedness == CoordinateHandedness::Right)
		{
			// translate to near plane position. I don't know why am I MULTIPLY by -1 to fix old version!
			rlTranslatef(0, 0, -camera.zNear);
		}
		else
		{
			rlTranslatef(0, 0, camera.zNear);
			// Flip to show orientation correctly in another handedness
			rlRotatef(180.0F, 0, 1, 0);	 // NOLINT(*-magic-numbers)
		}

		const float zoom = camera.zNear * std::tan(degToRad(camera.fov / 2));
		DrawModel(viewports_->getViewport3D(cameraIndex),
				  /*pos*/ Vector3{.x = 0, .y = 0, .z = 0},
				  zoom,
				  ::WHITE);
	}
	rlPopMatrix();
}
void FrustumDrawService::drawFrustumWires(const Camera& camera) const
{
	constexpr int kSliceCount		 = 2;  // was 11 to show z-fighting
	constexpr int kSliceCornersCount = kSliceCount * 4;
	constexpr Color kColor			 = Color::yellow();

	const Mat4 view					 = cameraService_->getViewMatrix(camera);
	const Mat4 model				 = view.inverse();
	const Mat4 projection			 = cameraService_->getProjectionMatrix(camera);

	const float thickness			 = settings_->lineThickness;

	auto drawPolygonWires			 = [&](const Color& color,	//
								   const float thickness,
								   const Vec3& p0,
								   const Vec3& p1,
								   const Vec3& p2,
								   const Vec3& p3)
	{
		graphic_->drawLine(p0, p1, thickness, color);
		graphic_->drawLine(p1, p2, thickness, color);
		graphic_->drawLine(p2, p3, thickness, color);
		graphic_->drawLine(p3, p0, thickness, color);
	};

	std::array<Vec3, kSliceCornersCount> coord;
	{
		const float step = 2.0F / (kSliceCount - 1);  // 2 is a distance between -1 and 1
		float z			 = -1.0F;					  // begin from -1

		// convert ndc coords to world
		const Mat4 combinedInverted = (projection * view).inverse();
		for (int i = 0; i < kSliceCornersCount; i += 4)
		{
			coord.at(i + 0) = (combinedInverted * Vec4{-1, +1, z, 1}).hnormalized();  // top-left
			coord.at(i + 1) = (combinedInverted * Vec4{-1, -1, z, 1}).hnormalized();  // bottom-left
			coord.at(i + 2) = (combinedInverted * Vec4{+1, -1, z, 1}).hnormalized();  // bottom-right
			coord.at(i + 3) = (combinedInverted * Vec4{+1, +1, z, 1}).hnormalized();  // top-right
			z += step;
		}
	}

	// draw near plane
	drawPolygonWires(kColor, thickness, coord[0], coord[1], coord[2], coord[3]);

	for (int i = 4; i < kSliceCornersCount; i += 4)
	{
		// draw slice plane to show z-fighting
		drawPolygonWires(kColor, thickness / 3, coord.at(i + 0), coord.at(i + 1), coord.at(i + 2), coord.at(i + 3));
	}

	// draw far plane
	drawPolygonWires(kColor,
					 thickness * 2,
					 coord.at(kSliceCornersCount - 4),
					 coord.at(kSliceCornersCount - 3),
					 coord.at(kSliceCornersCount - 2),
					 coord.at(kSliceCornersCount - 1));

	// draw side planes
	graphic_->drawLine(coord[0], coord[kSliceCornersCount - 4], thickness, kColor);
	graphic_->drawLine(coord[1], coord[kSliceCornersCount - 3], thickness, kColor);
	graphic_->drawLine(coord[2], coord[kSliceCornersCount - 2], thickness, kColor);
	graphic_->drawLine(coord[3], coord[kSliceCornersCount - 1], thickness, kColor);
}
}  // namespace gs::render
