/**
 * @file settings.h
 * @brief Contains all configurable application parameters
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_SETTINGS_H
#define GEOMETRY_SANDBOX_SETTINGS_H

#include "core/types.h"

#include <string>

namespace gs
{

/** @brief Contains all configurable application parameters */
struct Settings
{
	std::string title;
	int width;
	int height;
	bool fullscreen;
	bool multiSampleAntiAliasing4X;
	bool vSync;

	float footerHeight;
	float detailsWidth;

	bool showConsole;
	bool showCollisions;
	bool showCursorRay;

	constexpr static float kAnimationSpeedMin = 0.01f;
	constexpr static float kAnimationSpeedMax = 10.00f;
	float animationSpeed;

	std::string_view iconPlayerBackwardFast;
	std::string_view iconPlayerBackwardStep;
	std::string_view iconPlayerPlay;
	std::string_view iconPlayerPause;
	std::string_view iconPlayerForwardStep;
	std::string_view iconPlayerForwardFast;
	std::string_view iconSettings;

	bool enableQuaternionCameraRotation;
	bool resetTimelineAfterDataChange;

	float gizmoArrowSize;
	float gizmoColliderAlpha;
	float gizmoColliderAlphaSelected;
	Color gizmoTranslateAxisX;
	Color gizmoTranslateAxisXSelected;
	Color gizmoTranslateAxesXY;
	Color gizmoTranslateAxesXYSelected;
	Color gizmoTranslateAxisY;
	Color gizmoTranslateAxisYSelected;
	Color gizmoTranslateAxesYZ;
	Color gizmoTranslateAxesYZSelected;
	Color gizmoTranslateAxisZ;
	Color gizmoTranslateAxisZSelected;
	Color gizmoTranslateAxesZX;
	Color gizmoTranslateAxesZXSelected;
	float gizmoColliderPlaneThickness;
	float gizmoColliderAxisThickness;
	float gizmoColliderAxisLength;
	float gizmoColliderPlaneSize;

	float canvasFontSize;

	float uiFontSize;
	Color uiButtonColor;
	Color canvasBackgroundColor;

	Color uiPlayerTimelineBackgroundColor;
	Color uiPlayerTimelineFillColor;
	Color uiPlayerTimelineGrabberColor;
	Color uiPlayerTimelineGrabberColorActive;
	float uiPlayerTimelineGrabberSize;
	float uiPlayerTimelineRoundingSize;
	float uiPlayerTimelineHeight;
	float uiPlayerControlButtonWidth;
	float uiPlayerControlTextWidth;
	float uiPlayerHeight;

	float lineThick;
	float dashLength;

	Camera defaultCamera;

	scene::Grid2D grid;

	float idleRotationAnglePerFrame;
	Seconds idleRotationAnimationStartDelay;
	Seconds idleRotationTransitionDuration;
	float idleRotationDistanceMin;
	float idleRotationDistanceMod;
	static float kCameraRotateSensitivityMin;
	static float kCameraRotateSensitivityMax;
	float cameraRotateSensitivity;

	constexpr static float kCameraMoveSensitivityMin = 0.02f;
	constexpr static float kCameraMoveSensitivityMax = 1.0f;
	float cameraMoveSensitivity;
};

}  // namespace gs
#endif	// GEOMETRY_SANDBOX_SETTINGS_H
