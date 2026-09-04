/**
 * @file types.h
 * @brief core types
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_TYPES_H
#define GEOMETRY_SANDBOX_TYPES_H

#include "core/math.h"

#include <bitset>
#include <chrono>
#include <cmath>
#include <functional>
#include <set>
#include <string>
#include <vector>

namespace gs
{
using Clock		   = std::chrono::steady_clock;
using Timepoint	   = Clock::time_point;
using Seconds	   = std::chrono::seconds;
using Milliseconds = std::chrono::milliseconds;
using Nanoseconds  = std::chrono::nanoseconds;

/**
 * @brief just a width-height pair.
 *
 * To make it easier to pass arguments to the function and avoid mixing them up.
 */
struct RectSize
{
	float width;
	float height;
};

/**
 * @brief RGBA color with float values [0, 255]
 *
 * @todo compress to 255UI8
 */
struct Color
{
	static constexpr float kMaxColorValue = 255.0F;

	float r, g, b, a;

	constexpr static Color red(float alpha = 255) { return Color{230, 41, 55, alpha}; }
	constexpr static Color yellow(float alpha = 255) { return Color{253, 249, 0, alpha}; }
	constexpr static Color green(float alpha = 255) { return Color{0, 228, 48, alpha}; }
	constexpr static Color cian(float alpha = 255) { return Color{0, 255, 255, alpha}; }
	constexpr static Color blue(float alpha = 255) { return Color{0, 121, 241, alpha}; }
	constexpr static Color magenta(float alpha = 255) { return Color{255, 0, 255, alpha}; }

	constexpr static Color white(float alpha = 255) { return Color{240, 240, 240, alpha}; }
	constexpr static Color gray(float alpha = 255) { return Color{200, 200, 200, alpha}; }
	constexpr static Color black(float alpha = 255) { return Color{0, 0, 0, alpha}; }

	constexpr static Color transparent() { return Color{0, 0, 0, 0}; }

	Color normalized() const { return Color{r / kMaxColorValue, g / kMaxColorValue, b / kMaxColorValue, a / kMaxColorValue}; }
};

/**
 * @brief Cartesian coordinate system handedness
 *
 * @see https://en.wikipedia.org/wiki/Cartesian_coordinate_system#Orientation_and_handedness
 */
enum class CoordinateHandedness
{
	Left,
	Right,
	Count
};

/** @brief Axis x, y, z and they pair combinations*/
enum class Axis : std::uint8_t
{
	None = 0b000,
	X	 = 0b001,
	XY	 = 0b011,
	Y	 = 0b010,
	YZ	 = 0b110,
	Z	 = 0b100,
	ZX	 = 0b101
};

/** @brief cursor type for UI */
enum class CursorType
{
	Arrow,
	PointingHand,
	Hidden
};

/** @brief All predefined model in applications */
enum class ModelType : std::uint8_t
{
	Point = 0,
	Duck,
	Camera,
	Count
};

/**
 * @brief Contains data classes for scene rendering
 *
 * These types are needed to make it easier to animate and create objects on the scene.
 */
namespace scene
{

/** @brief square 2D grid */
struct Grid2D
{
	Color majorLineColor;
	Color minorLineColor;
	int majorLineStep;
	float cellSize;
	float gridSize;
};

/** @brief Colored plane */
struct Plane
{
	Vec3 normal;
	Color color;
	float distance;
};

/** @brief Colored point */
struct Point
{
	Vec3 position;
	Color color;
	float radius;
};

/** @brief 3D dashed line */
struct DashedLine
{
	Vec3 begin;
	Vec3 end;
	Color color;
	float thickness;
	float dashLength;
	float gapLength;
};

/** @brief 3D text */
struct Text
{
	std::string text;
	Vec3 position;
	Color color;
	float fontSize;
};

/** @brief Vector as colored arrow */
struct Vector
{
	Vec3 begin;
	Vec3 end;
	Color color;
	float thickness;
	float arrowSize;
};

/** @brief Wired colored cube described as vector center position and size */
struct AABB
{
	Vec3 position;
	Color color;
	float size;
	float thickness;
};

/** @brief Colored point */
struct Model
{
	Vec3 position;
	Vec3 origin;
	float scale;
	ModelType type;
};
}  // namespace scene

/**
 * @brief Basic vieport widget to translate scene objects
 *
 * @todo In the next release, a rotation gizmo will be added, and scene objects will move
 * by updating the Transform matrix, not just the position vectors as now.
 */
struct TranslateGizmo
{
	bool active;
	bool constrained;
	bool constrainedOnSingleAxis;
	Axis hoveredAxis;

	Vec3 position;

	Plane constraintPlane;
	Vec3 constraintAxis;
	Vec3 dragOffset;

	bool isHovered() const { return hoveredAxis != Axis::None; }
	bool isActive() const { return active; }
	bool isConstrained() const { return constrained; }
};

/** @brief Camera with coordinate handedness property */
struct Camera
{
	CoordinateHandedness handedness;
	Vec3 position;
	Vec3 target;
	Axis upAxis;
	int width;		   // viewport's width, to calculate aspect ratio, viewport texture
	int height;		   // viewport's height
	float fov;		   // field of view by Y axis
	float zNear;	   // near clipping plane distance from position
	float zFar;		   // far clipping plane distance from position
	bool perspective;  // perspective or orthographic
};

/** @brief Holds per-frame timings for each stage of the update/render pipeline  */
struct PerformanceStats
{
	Nanoseconds tick;
	Nanoseconds animation;
	Nanoseconds interaction;
	Nanoseconds render;
	Nanoseconds total;
};

/** @brief UI system with ui elements, state, services and managers */
namespace ui
{

/** @brief main UI layout properties. For now only opacity */
struct Layout
{
	float opacity = 1.0F;
};

/** @brief parent class for UI elements with id and callbacks */
struct Element
{
	int id = -1;
	mutable bool hovered;
	std::function<void(bool)> onHover;
	std::function<void()> onClick;

	virtual ~Element() = default;
};

/** @brief Button UI element */
struct Button : public Element
{
	/** @brief Nested type for animatable properties */
	struct Props
	{
		Color color;
	} props;

	RectSize size;
	float fontSize;
	float border;
	std::string_view icon;
	bool active;
	mutable bool pressed;
};

/** @brief Custom slide properties */
struct Slider : public Element
{
	Color backgroudColor;
	Color fillColor;
	Color grabberColor;
	Color grabberColorActive;
	float grabberSize;
	float roundingSize;
	float height;
	float value;

	std::set<float> markers;
	std::function<void(float)> onUpdate;
	std::function<std::string(float)> tooltipConverter;
};

/** @brief Player widget. Contains prev/play/next buttons and progress slider */
struct AnimationPlayer : public Element
{
	std::string currentTime = "00:00.000";
	std::string endTime		= "00:00.000";
	bool isForward			= true;
	bool isPlaying			= false;

	Button prevButton;
	Button playButton;
	Button nextButton;
	Slider timelineSlider;
};

/** @brief Main UI state with all UI elements, viewport widgets and layout properties */
struct State
{
	int width;
	int height;
	ui::Layout layout;
	CursorType cursorType = CursorType::Arrow;
	std::string algorithmData;

	TranslateGizmo transformGizmo;
	PerformanceStats performance;

	std::vector<ui::Element*> elements;

	std::vector<Camera> cameras;
	int activeCameraIndex;
	Ray mouseRay;

	AnimationPlayer player;
	Button settingsButton;
	Button forwardButton;
	Button leftButton;
	Button backwardButton;
	Button rightButton;
};
}  // namespace ui
}  // namespace gs

namespace std
{
inline gs::Vec3 lerp(const gs::Vec3& lhs, const gs::Vec3& rhs, float t)
{
	return {
		std::lerp(lhs.x(), rhs.x(), t),
		std::lerp(lhs.y(), rhs.y(), t),
		std::lerp(lhs.z(), rhs.z(), t),
	};
}
}  // namespace std

#endif	// GEOMETRY_SANDBOX_TYPES_H
