/**
 * @file dsl.h
 * @brief DSL for animation system with compile-time validation
 *
 * The animation DSL is validated **at compile time** — syntax errors, invalid property paths, and type mismatches
 *
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_ANIMATION_DSL_H
#define GEOMETRY_SANDBOX_ANIMATION_DSL_H

#include "animation/float_span_adaptor.h"
#include "animation/reflection.h"
#include "animation/types.h"

#include <array>
#include <ranges>
#include <unordered_map>

namespace gs::animation
{
template <typename T>
using Interpolator = std::function<T(const T&, const T&, float)>;

/** @brief DSL. All required data to define value for simple float or complex animated type */
struct Key
{
	Target target;
	std::uint32_t rootPropertyId;
	Milliseconds time;
	std::vector<float> values;
	float (*easing)(float);
	SampleInterpolator interpolator;
};

/** @brief DSL. DTO with collection of keys for specific time */
struct Keyframe
{
	Milliseconds time;
	std::vector<Key> keys;
};

/** @brief DSL. DTO with interpolation data and T property type value */
template <typename T>
struct PropertyValue
{
	T value;
	float (*easing)(float);
	Interpolator<T> interpolator;
};

/** @brief DSL. Collection of keys for specific time in milliseconds from animation start */
struct KeyframeCollection
{
	std::vector<Key> keys;
	Milliseconds duration = Milliseconds{0};

	void add(Keyframe&& keyframe)
	{
		duration = std::max(duration, keyframe.time);
		keys.reserve(keys.size() + keyframe.keys.size());
		std::move(keyframe.keys.begin(), keyframe.keys.end(), std::back_inserter(keys));  // instead of std::copy
		keyframe.keys.clear();
	}
};

/**
 * @brief DSL. Implementation of Lenses composition pattern
 *
 * Lenses are a functional programming pattern that provides a composable way
 * to access and modify nested data structures in an immutable way.
 */
template <typename Parent, typename Property>
struct SubProperty
{
	std::string_view propertyName;

	constexpr SubProperty(Property Parent::*, const std::string_view& name) : propertyName(name) {}
	constexpr SubProperty(const std::string_view& name) : propertyName(name) {}
};

/** @brief DSL. Collects path to specific property (any nested level) and combines with PropertyValue<T> to make a Key */
template <typename Parent, typename Property>
struct PropertyPath
{
	Target target;
	explicit constexpr PropertyPath(const Target& t) : target(t) {}

	template <typename Prop, typename SubProp>
	constexpr PropertyPath<Prop, SubProp> then(SubProperty<Prop, SubProp> next) const
	{
		if (const reflection::Property* property = reflection::findChildProperty(target.propertyId, next.propertyName); property)
		{
			return PropertyPath<Prop, SubProp>{Target{.entityId = target.entityId, .propertyId = property->id}};
		}
		else
		{
			throw std::runtime_error("wrong property path");
		};
	}

	template <typename Prop>
	Key to(PropertyValue<Prop> val) const
	{
		Key res;
		res.target		   = target;
		res.rootPropertyId = reflection::getPropertyRoot(target.propertyId).id;
		res.easing		   = val.easing == nullptr ? linear::in : val.easing;

		if (val.interpolator)
		{
			res.interpolator = [interpolator = val.interpolator](const std::span<float> lhs,
																 const std::span<float> rhs,
																 std::span<float> out,
																 float t)
			{
				Prop lhsObj, rhsObj;
				FloatSpanAdapter<Prop>::toObj(lhs, lhsObj);
				FloatSpanAdapter<Prop>::toObj(rhs, rhsObj);
				FloatSpanAdapter<Prop>::toFloatSpan(interpolator(lhsObj, rhsObj, t), out);
			};
		}

		res.values.resize(FloatSpanAdapter<Prop>::channelCount);
		FloatSpanAdapter<Prop>::toFloatSpan(val.value, std::span{res.values});

		return res;
	}
};

/** @brief animation system's domain-specific language */
namespace dsl
{
using namespace std::chrono_literals;  // significantly improves usability.

namespace detail
{
template <typename F>
constexpr auto propertyPath(auto id)
{
	return PropertyPath<F, F>{Target{.entityId	 = static_cast<std::uint32_t>(id),	//
									 .propertyId = reflection::getSceneTypeProperty<F>().id}};
};
}  // namespace detail

inline void operator+=(KeyframeCollection& keyframeCollection, Keyframe&& keyframe)
{
	keyframeCollection.add(std::move(keyframe));
};

template <typename Parent, typename Prop, typename SubProp>
constexpr PropertyPath<Prop, SubProp> operator/(const PropertyPath<Parent, Prop>& path, SubProperty<Prop, SubProp> next)
{
	return path.then(next);
}

template <typename Parent, typename Prop>
inline Key operator<<(const PropertyPath<Parent, Prop>& path, const PropertyValue<Prop>& val)
{
	return path.to(val);
}

template <typename Parent, typename Prop>
inline Key operator<<(const PropertyPath<Parent, Prop>& path, const Prop& val)
{
	return operator<<(path, PropertyValue{val});
}

constexpr Keyframe operator|(Keyframe&& keyframe, Key&& key)  // is this a copy?
{
	key.time = keyframe.time;
	keyframe.keys.push_back(std::move(key));
	return keyframe;
}

template <typename T>
constexpr PropertyValue<T> key(const T& val, float (*easing)(float) = nullptr)
{
	return PropertyValue<T>{val, easing};
}

template <typename T>
PropertyValue<T> key(const T& val, Interpolator<T> interpolator)
{
	return PropertyValue<T>{val, nullptr, interpolator};
}

inline Keyframe keyframe(const Milliseconds& ms)
{
	return Keyframe{.time = ms};
}

/** @brief accessor functions for specific scene type */
namespace accessor
{
constexpr auto point(auto id)
{
	return detail::propertyPath<scene::Point>(id);
};
constexpr auto plane(auto id)
{
	return detail::propertyPath<scene::Plane>(id);
};
constexpr auto ui()
{
	return detail::propertyPath<ui::Layout>(0);
};

constexpr auto button(auto id)
{
	return detail::propertyPath<ui::Button::Props>(id);
};
constexpr auto camera(auto id)
{
	return detail::propertyPath<Camera>(id);
};
}  // namespace accessor

/** @brief @ref gs::Vec3 mapping */
namespace vec3
{
constexpr auto x = SubProperty<Vec3, float>("x");
constexpr auto y = SubProperty<Vec3, float>("y");
constexpr auto z = SubProperty<Vec3, float>("z");
}  // namespace vec3

/** @brief @ref gs::Color mapping */
namespace color
{
constexpr auto r = SubProperty{&Color::r, "r"};
constexpr auto g = SubProperty{&Color::g, "g"};
constexpr auto b = SubProperty{&Color::b, "b"};
constexpr auto a = SubProperty{&Color::a, "a"};
}  // namespace color

/** @brief @ref gs::scene::Point mapping */
namespace point
{
constexpr auto position = SubProperty{&scene::Point::position, "position"};
constexpr auto color	= SubProperty{&scene::Point::color, "color"};
constexpr auto radius	= SubProperty{&scene::Point::radius, "radius"};
}  // namespace point

/** @brief @ref gs::scene::Plane mapping */
namespace plane
{
constexpr auto color  = SubProperty{&scene::Plane::color, "color"};	   // SubProperty?
constexpr auto normal = SubProperty{&scene::Plane::normal, "normal"};  // SubProperty?
}  // namespace plane

/** @brief @ref gs::ui::Layout mapping */
namespace layout
{
constexpr auto opacity = SubProperty{&ui::Layout::opacity, "opacity"};
}  // namespace layout

/** @brief @ref gs::ui::Button::Props mapping */
namespace btn
{
constexpr auto color = SubProperty{&ui::Button::Props::color, "color"};
}  // namespace btn

/** @brief @ref gs::Camera mapping */
namespace camera
{
constexpr auto position = SubProperty{&Camera::position, "position"};
constexpr auto target	= SubProperty{&Camera::target, "target"};
}  // namespace camera
}  // namespace dsl
}  // namespace gs::animation
#endif	// GEOMETRY_SANDBOX_ANIMATION_DSL_H
