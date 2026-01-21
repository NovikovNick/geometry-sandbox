/**
 * @file float_span_adaptor.h
 * @brief adapters for scene types
 *
 * Used only animation DSL
 *
 * @author MetalHeart
 */

#ifndef GEOMETRY_SANDBOX_ANIMATION_FLOAT_SPAN_ADAPTER_H
#define GEOMETRY_SANDBOX_ANIMATION_FLOAT_SPAN_ADAPTER_H

#include "core/types.h"

#include <cassert>
#include <span>

namespace gs::animation
{
/** @brief Converts between scene types and flat float arrays for the animation pipeline */
template <typename T>
class FloatSpanAdapter
{
};

template <>
class FloatSpanAdapter<float>
{
  public:
	const static int channelCount = 1;
	static void toFloatSpan(float src, std::span<float> dst)
	{
		assert(dst.size() == channelCount);
		dst[0] = src;
	}

	static void toObj(const std::span<float> src, float& dst)
	{
		assert(src.size() == channelCount);
		dst = src[0];
	}
};

template <>
class FloatSpanAdapter<Vec3>
{
  public:
	const static int channelCount = 3;
	static void toFloatSpan(const Vec3& src, std::span<float> dst)
	{
		assert(dst.size() == channelCount);
		dst[0] = src.x();
		dst[1] = src.y();
		dst[2] = src.z();
	}

	static void toObj(const std::span<float> src, Vec3& dst)
	{
		assert(src.size() == channelCount);
		dst.x() = src[0];
		dst.y() = src[1];
		dst.z() = src[2];
	}
};

template <>
class FloatSpanAdapter<Color>
{
  public:
	const static int channelCount = 4;
	static void toFloatSpan(const Color& src, std::span<float> dst)
	{
		assert(dst.size() == channelCount);
		dst[0] = src.r;
		dst[1] = src.g;
		dst[2] = src.b;
		dst[3] = src.a;
	}

	static void toObj(const std::span<float> src, Color& dst)
	{
		assert(src.size() == channelCount);
		dst.r = src[0];
		dst.g = src[1];
		dst.b = src[2];
		dst.a = src[3];
	}
};

template <>
class FloatSpanAdapter<scene::Point>
{
  public:
	const static int channelCount = 8;
	static void toFloatSpan(const scene::Point& src, std::span<float> dst)
	{
		assert(dst.size() == channelCount);
		FloatSpanAdapter<Vec3>::toFloatSpan(src.position, dst.subspan(0, 3));
		FloatSpanAdapter<Color>::toFloatSpan(src.color, dst.subspan(3, 4));
		dst[7] = src.radius;
	}

	static void toObj(const std::span<float> src, scene::Point& dst)
	{
		assert(src.size() == channelCount);
		FloatSpanAdapter<Vec3>::toObj(src.subspan(0, 3), dst.position);
		FloatSpanAdapter<Color>::toObj(src.subspan(3, 4), dst.color);
		dst.radius = src[7];
	}
};

template <>
class FloatSpanAdapter<Camera>
{
  public:
	const static int channelCount = 10;
	static void toFloatSpan(const Camera& src, std::span<float> dst)
	{
		assert(dst.size() == channelCount);
		auto t = dst.subspan(0, 3);
		FloatSpanAdapter<Vec3>::toFloatSpan(src.position, dst.subspan(0, 3));
		FloatSpanAdapter<Vec3>::toFloatSpan(src.target, dst.subspan(3, 3));
		dst[6] = src.fov;
		dst[7] = src.zNear;
		dst[8] = src.zFar;
		dst[9] = src.perspective ? 1.0F : 0.0F;
	}

	static void toObj(const std::span<float> src, Camera& dst)
	{
		assert(src.size() == channelCount);
		FloatSpanAdapter<Vec3>::toObj(src.subspan(0, 3), dst.position);
		FloatSpanAdapter<Vec3>::toObj(src.subspan(3, 3), dst.target);
		dst.fov			= src[6];
		dst.zNear		= src[7];
		dst.zFar		= src[8];
		dst.perspective = src[9] > 0.0F;
	}
};

}  // namespace gs::animation

#endif	// GEOMETRY_SANDBOX_ANIMATION_FLOAT_SPAN_ADAPTER_H
