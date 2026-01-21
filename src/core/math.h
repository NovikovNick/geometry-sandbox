/**
 * @file math.h
 * @brief Defines math types, geometric primitives, utility functions
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_MATH_H
#define GEOMETRY_SANDBOX_MATH_H

#include <Eigen/Dense>

#include <concepts>	 // for std::floating_point
#include <numbers>	 // for std::numbers::pi

namespace gs
{
using Vec2 = Eigen::Vector2f;
using Vec3 = Eigen::Vector3f;
using Vec4 = Eigen::Vector4f;
using Quat = Eigen::Quaternion<float>;
using Mat4 = Eigen::Matrix<float, 4, 4, Eigen::ColMajor>;

/** @brief Math ray: vector position and direction */
struct Ray
{
	Vec3 origin, direction;
};

/** @brief Math sphere: vector center position and radius */
struct Sphere
{
	Vec3 center;
	float radius;
};

/**
 * @brief Axis-aligned bounding box. Minimum and maximum vector position along each axis
 *
 * This is a simple box whose sides are always parallel to the world axes (x, y, z)
 */
struct AABB
{
	Vec3 min;
	Vec3 max;
};

/** @brief Math plane: normal vector and distance to the projection of an origin onto the plane */
struct Plane
{
	Vec3 normal;
	float distance = 0;	 // also known as - d, offset
};

/** @brief Indicates intersection by bool hit flag and contains intersection point and distance to it */
struct Intersection
{
	bool hit;
	Vec3 point;
	float distance;
	// Vec3 normal?
};

/**
 * @todo check maybe all functions should be constexpr instead of inline?
 */
inline bool almostEquals(float lhs, float rhs, float tolerance = 1e-4f)
{
	return std::abs(lhs - rhs) < tolerance;
}

template <std::floating_point T = double>
constexpr T degToRad(T degrees) noexcept
{
	return degrees * (std::numbers::pi_v<T> / T(180));
}

template <std::floating_point T = double>
constexpr T radToDeg(T radian) noexcept
{
	return radian * (T(180) / std::numbers::pi_v<T>);
}

// from world space into screen space
inline Vec3 projectVector(const Vec3& world, const Mat4& view, const Mat4& projection, const float width, const float height)
{
	Vec4 res = world.homogeneous();		 // add w=1 component
	res		 = projection * view * res;	 // multiply by view and projection
	Vec3 ndc = res.hnormalized();		 // res / res.w() to Normalized Device Coords

	Vec3 viewport;
	viewport.x() = (ndc.x() + 1.0f) / 2.0f * width;	  // add 1.0f because ndc can be from -1 to 1
	viewport.y() = (1.0f - ndc.y()) / 2.0f * height;  // invert Y, as it points downwards on screens ¯\_(ツ)_/¯
	viewport.z() = ndc.z();							  // usefull data, for frustum culling for example...
	return viewport;
}

// from screen space into object world
inline Vec3 deprojectVector(const Vec3& screen, const Mat4& view, const Mat4& projection)
{
	Vec4 res = screen.homogeneous();				 // add w=1 component
	res		 = (projection * view).inverse() * res;	 // multiply by unprojected matrix
	return res.hnormalized();						 // res / res.w() to Normalized Device Coords
}

inline Intersection intersect(const Ray& ray, const Sphere& sphere)
{
	Intersection res;
	constexpr float tMin = 0.0f;
	constexpr float tMax = std::numeric_limits<float>::max();

	Eigen::Vector3f oc	 = ray.origin - sphere.center;

	float a				 = ray.direction.squaredNorm();
	float b				 = 2.0f * oc.dot(ray.direction);
	float c				 = oc.squaredNorm() - sphere.radius * sphere.radius;

	float discriminant	 = b * b - 4.0f * a * c;

	if (discriminant < 0.0f)
	{
		res.hit = false;
		return res;
	}

	float sqrtDisc = std::sqrt(discriminant);
	float t		   = (-b - sqrtDisc) / (2.0f * a);
	if (t < tMin || t > tMax)
	{
		t = (-b + sqrtDisc) / (2.0f * a);
		if (t < tMin || t > tMax)
		{
			res.hit = false;
			return res;
		}
	}

	res.hit		 = true;
	res.point	 = ray.origin + ray.direction * t;
	res.distance = t;
	return res;
}

inline Intersection intersect(const Ray& ray, const Plane& plane)
{
	Intersection res;
	res.hit = false;
	if (const float denominator = ray.direction.dot(plane.normal); denominator != 0)
	{
		const float distance = -(plane.normal.dot(ray.origin) + plane.distance) / denominator;

		res.hit				 = distance > 0;
		if (res.hit)
		{
			res.point	 = ray.origin + ray.direction * distance;
			res.distance = distance;
		}
	}
	return res;
}

// copypasted from raylib
inline Intersection intersect(Ray ray, const AABB& box)
{
	Intersection res;
	bool insideBox = (ray.origin.x() > box.min.x()) && (ray.origin.x() < box.max.x()) && (ray.origin.y() > box.min.y()) &&
					 (ray.origin.y() < box.max.y()) && (ray.origin.z() > box.min.z()) && (ray.origin.z() < box.max.z());

	if (insideBox) ray.direction = ray.direction * -1;

	float t[11]	 = {0};

	t[8]		 = 1.0f / ray.direction.x();
	t[9]		 = 1.0f / ray.direction.y();
	t[10]		 = 1.0f / ray.direction.z();

	t[0]		 = (box.min.x() - ray.origin.x()) * t[8];
	t[1]		 = (box.max.x() - ray.origin.x()) * t[8];
	t[2]		 = (box.min.y() - ray.origin.y()) * t[9];
	t[3]		 = (box.max.y() - ray.origin.y()) * t[9];
	t[4]		 = (box.min.z() - ray.origin.z()) * t[10];
	t[5]		 = (box.max.z() - ray.origin.z()) * t[10];
	t[6]		 = (float)fmax(fmax(fmin(t[0], t[1]), fmin(t[2], t[3])), fmin(t[4], t[5]));
	t[7]		 = (float)fmin(fmin(fmax(t[0], t[1]), fmax(t[2], t[3])), fmax(t[4], t[5]));

	res.hit		 = !((t[7] < 0) || (t[6] > t[7]));
	res.distance = t[6];
	return res;
}

inline Vec3 rotateVector(const Vec3& v, Vec3 axis, float angle)
{
	// Using Euler-Rodrigues Formula
	// Ref.: https://en.wikipedia.org/w/index.php?title=Euler%E2%80%93Rodrigues_formula

	Vec3 result = v;
	axis		= axis.normalized();

	angle /= 2;
	float a		   = sinf(angle);
	const float b  = axis.x() * a;
	const float c  = axis.y() * a;
	const float d  = axis.z() * a;
	a			   = cosf(angle);

	const Vec3 w   = {b, c, d};
	const Vec3 wv  = w.cross(v);
	const Vec3 wwv = w.cross(wv);

	result += wv * 2 * a + wwv * 2;

	return result;
}

}  // namespace gs
#endif	// GEOMETRY_SANDBOX_MATH_H
