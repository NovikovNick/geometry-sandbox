#ifndef GEOMETRY_SANDBOX_CAMERA_HANDEDNESS_SAMPLE_H
#define GEOMETRY_SANDBOX_CAMERA_HANDEDNESS_SAMPLE_H

#include <vector>

namespace gs
{
struct Matrix4x4DTO
{
	std::vector<float> data;
};

Matrix4x4DTO getModel();

Matrix4x4DTO getView();

Matrix4x4DTO getProjection();

void init();

void drawNextFrame();

void startInfiniteLoop();

}  // namespace gs

#endif	// GEOMETRY_SANDBOX_CAMERA_HANDEDNESS_SAMPLE_H