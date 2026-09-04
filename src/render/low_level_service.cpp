#include "render/low_level_service.h"

#include "core/camera_service.h"
#include "core/math.h"
#include "core/resource_manager.h"
#include "core/settings.h"
#include "core/types.h"

#include "raylib.h"
#include "rlgl.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace gs::render
{
namespace
{
::Color convert(Color src)
{
	return {
		.r = static_cast<unsigned char>(src.r),
		.g = static_cast<unsigned char>(src.g),
		.b = static_cast<unsigned char>(src.b),
		.a = static_cast<unsigned char>(src.a),
	};
}

::Vector3 convert(Vec3 src)
{
	return {.x = src.x(), .y = src.y(), .z = src.z()};
}

/** @brief Draw a grid centered at (0, 0, 0) */
void drawGrid(const scene::Grid2D& grid)
{
	const float gridExtent = grid.gridSize / 2.0F * grid.cellSize;

	auto setLineColor	   = [](const Color& color)
	{
		const Color normalized = color.normalized();
		rlColor3f(normalized.r, normalized.g, normalized.b);
	};

	auto drawLineOnX = [&](float x)
	{
		rlVertex3f(x, 0.0F, -gridExtent);
		rlVertex3f(x, 0.0F, gridExtent);
	};

	auto drawLineOnZ = [&](float z)
	{
		rlVertex3f(-gridExtent, 0.0F, z);
		rlVertex3f(gridExtent, 0.0F, z);
	};

	rlBegin(RL_LINES);

	const auto start = static_cast<int>(-grid.gridSize / 2);
	const auto end	 = static_cast<int>(+grid.gridSize / 2);
	for (int i = start; i <= end; i++)
	{
		setLineColor((i % grid.majorLineStep == 0) ? grid.majorLineColor : grid.minorLineColor);

		drawLineOnX(static_cast<float>(i) * grid.cellSize);
		drawLineOnZ(static_cast<float>(i) * grid.cellSize);
	}

	rlEnd();
}
}  // namespace

void LowLevelService::setupCamera(const Camera& camera)
{
	rlDrawRenderBatchActive();	// Update and draw internal render batch

	// 1. Projection
	rlMatrixMode(RL_PROJECTION);  // Switch to projection matrix
	rlPushMatrix();				  // Save previous matrix, which contains the settings for the 2d ortho projection
	rlLoadIdentity();			  // Reset current matrix (projection)

	rlMultMatrixf(cameraService_->getProjectionMatrix(camera).data());

	// 2. View
	rlMatrixMode(RL_MODELVIEW);	 // Switch back to modelview matrix
	rlLoadIdentity();			 // Reset current matrix (modelview)

	rlMultMatrixf(cameraService_->getViewMatrix(camera).data());

	rlEnableDepthTest();  // Enable DEPTH_TEST for 3D
}

void LowLevelService::cleanupCamera()
{
	rlDrawRenderBatchActive();	// Update and draw internal render batch

	rlMatrixMode(RL_PROJECTION);  // Switch to projection matrix
	rlPopMatrix();				  // Restore previous matrix (projection) from matrix stack

	rlMatrixMode(RL_MODELVIEW);	 // Switch back to modelview matrix
	rlLoadIdentity();			 // Reset current matrix (modelview)

	// if (rlGetActiveFramebuffer() == 0) rlMultMatrixf(MatrixToFloat(CORE.Window.screenScale));  // screen scaling

	rlDisableDepthTest();  // Disable DEPTH_TEST for 2D
}

void LowLevelService::clearBackground(const Color& color) const
{
	ClearBackground(convert(color));
}

void LowLevelService::drawLine(const Vec3& startWorld, const Vec3& endWorld, float thickness, const Color& color) const
{
	constexpr int kCylinderSides = 16;
	DrawCylinderEx(convert(startWorld),
				   convert(endWorld),
				   /*start radius*/ thickness / 2,
				   /*end radius*/ thickness / 2,
				   kCylinderSides,
				   convert(color));
}

void LowLevelService::drawDashedLine(const Vec3& startWorld,
									 const Vec3& endWorld,
									 const float thickness,
									 const Color& color,
									 const float dashLengthMax,
									 const float gapLength) const
{

	const Vec3 segment			 = endWorld - startWorld;
	const float segmentLength	 = segment.norm();
	const Vec3 segmentNormilized = segment.normalized();

	float offsetLength			 = 0;
	for (int i = 1; offsetLength < segmentLength; ++i)
	{
		const float dashLength = std::min(segmentLength - offsetLength, dashLengthMax);

		const Vec3 offset	   = segmentNormilized * offsetLength;
		const Vec3 dashEnd	   = offset + segmentNormilized * dashLength;

		const Vec3 begin	   = startWorld + offset;
		const Vec3 end		   = startWorld + dashEnd;

		drawLine(begin, end, thickness, color);
		offsetLength = std::min(static_cast<float>(i) * (dashLengthMax + gapLength), segmentLength);  // !
	}
}

void LowLevelService::drawArrow(const Vec3& startWorld,
								const Vec3& endWorld,
								const float thickness,
								const Color& color,
								float size) const
{
	constexpr int kCylinderSides = 16;
	const float headSize		 = thickness * size / 2;
	const Vec3 segment			 = (endWorld - startWorld);
	const Vec3 arrowStart		 = startWorld + segment.normalized() * (segment.norm() - headSize);

	// tail
	DrawCylinderEx(convert(startWorld),
				   convert(arrowStart),
				   /*start radius*/ thickness / 2,
				   /*end radius*/ thickness / 2,
				   kCylinderSides,
				   convert(color));
	// head
	DrawCylinderEx(convert(arrowStart),
				   convert(endWorld),
				   /*start radius*/ headSize / 2,
				   /*end radius*/ 0,
				   kCylinderSides,
				   convert(color));
}

void LowLevelService::drawAABB(const AABB& box, const Color& color) const
{
	const Vec3 size{box.max.x() - box.min.x(), box.max.y() - box.min.y(), box.max.z() - box.min.z()};
	const Vec3 center{box.min.x() + (size.x() / 2), box.min.y() + (size.y() / 2), box.min.z() + (size.z() / 2)};
	DrawCube(convert(center), size.x(), size.y(), size.z(), convert(color));
}

void LowLevelService::drawAABBFramed(const AABB& aabb, const Color& color, float thickness) const
{
	const Vec3 x{aabb.max.x() - aabb.min.x(), 0.0F, 0.0F};
	const Vec3 y{0.0F, aabb.max.y() - aabb.min.y(), 0.0F};
	const Vec3 z{0.0F, 0.0F, aabb.max.z() - aabb.min.z()};

	// vertices
	const Vec3 v000 = aabb.min;
	const Vec3 v100 = aabb.min + x;
	const Vec3 v010 = aabb.min + y;
	const Vec3 v110 = aabb.min + x + y;
	const Vec3 v001 = aabb.min + z;
	const Vec3 v101 = aabb.min + x + z;
	const Vec3 v011 = aabb.min + y + z;
	const Vec3 v111 = aabb.max;

	// --- face (z = min) ---
	drawLine(v000, v100, thickness, color);
	drawLine(v000, v010, thickness, color);
	drawLine(v100, v110, thickness, color);
	drawLine(v010, v110, thickness, color);

	// --- back (z = max) ---
	drawLine(v001, v101, thickness, color);
	drawLine(v001, v011, thickness, color);
	drawLine(v101, v111, thickness, color);
	drawLine(v011, v111, thickness, color);

	// --- Connecting ribs (along z axis) ---
	drawLine(v000, v001, thickness, color);
	drawLine(v100, v101, thickness, color);
	drawLine(v010, v011, thickness, color);
	drawLine(v110, v111, thickness, color);
}

void LowLevelService::drawPlane(const Plane& plane, const Color& color) const
{
	constexpr float width	= 3.0F;
	constexpr float height	= 3.0F;
	constexpr float opacity = 0.25F;
	const Vec3 modelNormal	= Vec3::UnitY();
	const Vector2 size{.x = width, .y = height};
	const Vector3 origin{.x = 0.0F, .y = 0.0F, .z = 0.0F};
	const Vec3 position	 = plane.normal * plane.distance;
	const auto drawPlane = [&](const Vec3& targetNormal)
	{
		const Vec3 rotateAxis = modelNormal.cross(targetNormal);
		const float angle	  = std::acos(modelNormal.dot(targetNormal)) * RAD2DEG;
		rlPushMatrix();
		{
			rlTranslatef(position.x(), position.y(), position.z());
			rlRotatef(angle, rotateAxis.x(), rotateAxis.y(), rotateAxis.z());
			DrawPlane(origin, size, convert(color));
		}
		rlPopMatrix();
	};

	drawPlane(plane.normal.normalized());		   // frontface
	drawPlane(plane.normal.normalized() * -1.0F);  // backface
}

void LowLevelService::drawText(const Camera& cam,
							   const std::string& str,
							   const Vec3& worldPos,
							   const Color& color,
							   const float fontSize) const
{
	const auto width	 = static_cast<float>(cam.width);
	const auto height	 = static_cast<float>(cam.height);
	const Font& font	 = resources_->defaultCanvasFont();
	const Vec3 screenPos = projectVector(worldPos,
										 cameraService_->getViewMatrix(cam),
										 cameraService_->getProjectionMatrix(cam),
										 width,
										 height);

	// frustum culling
	if (0 > screenPos.x() || screenPos.x() > width		// clip by right/left
		|| 0 > screenPos.y() || screenPos.y() > height	// clip by top/bottom
		|| std::abs(screenPos.z()) > 1)					// clip by near and far planes
	{
		return;
	}

	const float perspectiveMod = 1 / std::clamp(screenPos.z(), 0.25F, 1.0F);
	const float xOffset		   = static_cast<float>(str.size()) * fontSize * perspectiveMod / 4.0F;
	const float yOffset		   = (fontSize * perspectiveMod) + 25.0F;
	DrawTextEx(font,
			   str.c_str(),
			   {.x = screenPos.x() - xOffset, .y = screenPos.y() - yOffset},
			   fontSize * perspectiveMod,
			   0,
			   convert(color));
}

void LowLevelService::drawAxesGrid(Axis up, const scene::Grid2D& grid) const
{
	switch (up)
	{
		case Axis::X:
		{

			rlPushMatrix();
			{
				rlRotatef(90.0F, 0.0F, 0.0F, 1.0F);	 // NOLINT(*-magic-numbers)
				drawGrid(grid);						 // ZY
			}
			rlPopMatrix();
			break;
		}
		case Axis::Y:
		{
			drawGrid(grid);	 // XZ
			break;
		}
		case Axis::Z:
		{
			rlPushMatrix();
			{
				rlRotatef(90.0F, 1.0F, 0.0F, 0.0F);	 // NOLINT(*-magic-numbers)
				drawGrid(grid);						 // XY
			}
			rlPopMatrix();
		}
	}

	// Draw axis
	drawArrow(Vec3::Zero(), Vec3::UnitX() * grid.gridSize / 2, settings_->lineThickness, Color::red(), settings_->gizmoArrowSize);
	drawArrow(Vec3::Zero(), Vec3::UnitY() * grid.gridSize / 2, settings_->lineThickness, Color::green(), settings_->gizmoArrowSize);
	drawArrow(Vec3::Zero(), Vec3::UnitZ() * grid.gridSize / 2, settings_->lineThickness, Color::blue(), settings_->gizmoArrowSize);
}

void LowLevelService::drawSphereWires(const Vec3& position, const Color& color, float radius) const
{
	constexpr int kSphereRings	= 6;
	constexpr int kSphereSlices = 6;
	DrawSphereWires(convert(position), radius, kSphereRings, kSphereSlices, convert(color));
}

void LowLevelService::drawBoundingBox(const Vec3& min, const Vec3& max, const Color& color) const
{
	DrawBoundingBox(BoundingBox{.min = convert(min), .max = convert(max)}, convert(color));
}

void LowLevelService::drawModel(ModelType type, const Vec3& position, float scale) const
{
	DrawModel(resources_->getModel(type), convert(position), scale, ::WHITE);
}
}  // namespace gs::render
