/**
 * @file reflection.h
 * @brief Static introspection of animatable types
 *
 * The reflection subsystem stores byte offsets for Property types within a Entry types,
 * as well as mappings between Scene and Entry types. The data is stored as a flat
 * tree to facilitate searching and traverse.
 ```
Scene (scene::Point)                  // used in dsl
├── Entry (ecs::component::Position)  // will be converted to std::span<std::byte> for writing
│   └── Property (Vec3)               // byte access via offset
│       ├── Property (float)          // byte access via offset
│       ├── Property (float)          // byte access via offset
│       └── Property (float)          // byte access via offset
├── Entry (ecs::component::MainColor) // will be converted to std::span<std::byte> for writing
│   ├── Property (Color)              // byte access via offset
│   │   ├── Property (float)          // byte access via offset
│   │   ├── Property (float)          // byte access via offset
│   │   ├── Property (float)          // byte access via offset
│   │   └── Property (float)          // byte access via offset
└── Entry (ecs::component::Radius)    // will be converted to std::span<std::byte> for writing
	└── Property (float)              // byte access via offset
```
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_REFLECTION_H
#define GEOMETRY_SANDBOX_REFLECTION_H

#include "core/ecs.h"
#include "core/types.h"

#include <array>
#include <cassert>
#include <optional>
#include <ranges>
#include <span>
#include <string_view>
#include <type_traits>

/** @brief Static introspection of animatable types */
namespace gs::reflection
{
enum class SceneType : std::uint8_t
{
	None,
	Point,
	Plane,
	Layout,
	Button,
	Camera,
};

enum class EntryType : std::uint8_t
{
	None,
	MainColor,
	Normal,
	Radius,
	Position,
	EndPosition,
	Thickness,
	UILayout,
	Button,
	Camera,
};

enum class PropertyType : std::uint8_t
{
	None,
	Float,
	Bool,
	Vec3,
	Color,
};

template <typename T>
constexpr SceneType sceneType()
{
	// clang-format off
	if constexpr (std::is_same_v<T, scene::Point>) return SceneType::Point;
	else if constexpr (std::is_same_v<T, scene::Plane>) return SceneType::Plane;
	else if constexpr (std::is_same_v<T, Camera>) return SceneType::Camera;
	else if constexpr (std::is_same_v<T, ui::Button::Props>) return SceneType::Button;
	else if constexpr (std::is_same_v<T, ui::Layout>) return SceneType::Layout;
	else { static_assert(sizeof(T) == 0, "There is no such scene type"); return SceneType::None; }
	// clang-format on
};

template <typename T>
constexpr PropertyType propertyType()
{
	// clang-format off
	if constexpr (std::is_same_v<T, float>) return PropertyType::Float;
	else if constexpr (std::is_same_v<T, bool>) return PropertyType::Bool;
	else if constexpr (std::is_same_v<T, Vec3>) return PropertyType::Vec3;
	else if constexpr (std::is_same_v<T, Color>) return PropertyType::Color;
	else { static_assert(sizeof(T) == 0, "There is no such property type"); return PropertyType::None; }
	// clang-format on
};

template <typename T>
constexpr EntryType entryType()
{
	// clang-format off
	if constexpr (std::is_same_v<T, ui::Button::Props>) return EntryType::Button;
	else if constexpr (std::is_same_v<T, ui::Layout>) return EntryType::UILayout;
	else if constexpr (std::is_same_v<T, Camera>) return EntryType::Camera;
	else if constexpr (std::is_same_v<T, ecs::component::MainColor>) return EntryType::MainColor;
	else if constexpr (std::is_same_v<T, ecs::component::Normal>) return EntryType::Normal;
	else if constexpr (std::is_same_v<T, ecs::component::Radius>) return EntryType::Radius;
	else if constexpr (std::is_same_v<T, ecs::component::Position>) return EntryType::Position;
	else if constexpr (std::is_same_v<T, ecs::component::EndPosition>) return EntryType::EndPosition;
	else if constexpr (std::is_same_v<T, ecs::component::Thickness>) return EntryType::Thickness;
	else { static_assert(sizeof(T) == 0, "There is no such entry type"); return EntryType::None; }
	// clang-format on
};

/** @brief member field metadata */
struct Property
{
	using Id = std::uint32_t;
	Id id;				  // unique property id
	int depth;			  // depth in entry object
	int offset;			  // offset in entry object
	SceneType sceneType;  // scene type on the scene or in the animation dsl
	EntryType entryType;  // defines which object is required as input
	PropertyType type;
	std::string_view name;	// to distinguish between fields of the same type
};

namespace detail
{
template <std::size_t N>
class PropertyRegistry	// it is like "join" table
{
  public:
	std::array<Property, N> properties = {};

	constexpr const Property& getProperty(Property::Id propertyId) const { return properties[propertyId]; }

	constexpr const Property& getPropertyRoot(Property::Id propertyId) const
	{
		assert(propertyId < properties.size());
		while (properties[propertyId].depth != 0)
		{
			assert(propertyId > 0);
			--propertyId;
		}
		return properties[propertyId];
	}

	constexpr const Property* findPropertyChild(Property::Id propertyId, std::string_view name) const
	{
		assert(propertyId < properties.size());

		const Property& root = properties[propertyId];

		while (++propertyId < properties.size())
		{
			const Property& property = properties[propertyId];

			if (property.depth <= root.depth)
			{
				return nullptr;
			}

			if (property.name == name)
			{
				return &property;
			}
		}
		return nullptr;
	}

  private:
	template <typename F, typename E, typename P>
	constexpr int registerPropertyLeaf(Property::Id propertyId, int depth, int offset, std::string_view name)
	{
		static_assert(std::is_standard_layout_v<P>, "Property type must be standard layout for offsetof");

		properties[propertyId] = Property{.id		 = propertyId,
										  .depth	 = depth,
										  .offset	 = offset,
										  .sceneType = sceneType<F>(),
										  .entryType = entryType<E>(),
										  .type		 = propertyType<P>(),
										  .name		 = name};
		++propertyId;
		++depth;
		if constexpr (std::is_same_v<P, Color>)
		{
			propertyId = registerPropertyLeaf<F, E, float>(propertyId, depth, offset + offsetof(P, r), "r");
			propertyId = registerPropertyLeaf<F, E, float>(propertyId, depth, offset + offsetof(P, g), "g");
			propertyId = registerPropertyLeaf<F, E, float>(propertyId, depth, offset + offsetof(P, b), "b");
			propertyId = registerPropertyLeaf<F, E, float>(propertyId, depth, offset + offsetof(P, a), "a");
		}
		else if constexpr (std::is_same_v<P, Vec3>)
		{
			propertyId = registerPropertyLeaf<F, E, float>(propertyId, depth, offset + sizeof(float) * 0, "x");
			propertyId = registerPropertyLeaf<F, E, float>(propertyId, depth, offset + sizeof(float) * 1, "y");
			propertyId = registerPropertyLeaf<F, E, float>(propertyId, depth, offset + sizeof(float) * 2, "z");
		}
		else if constexpr (!std::is_same_v<P, float> && !std::is_same_v<P, bool>)  // if NOT float or bool
		{
			static_assert(sizeof(P) == 0, "Type is not registered as a property type");
		}

		return propertyId;
	}

	template <typename F, typename E>
	constexpr int registerEntryNode(Property::Id propertyId, std::string_view name)
	{
		static_assert(std::is_standard_layout_v<F>, "Frontend type must be standard layout for offsetof");

		properties[propertyId] = Property{.id		 = propertyId,
										  .depth	 = 1,
										  .offset	 = 0,
										  .sceneType = sceneType<F>(),
										  .entryType = entryType<E>(),
										  .type		 = PropertyType::None,
										  .name		 = name};
		++propertyId;
		constexpr int depth = 2;
		if constexpr (std::is_same_v<E, ecs::component::Position>)
		{
			propertyId = registerPropertyLeaf<F, E, Vec3>(propertyId, depth, offsetof(E, val), "position");
		}
		else if constexpr (std::is_same_v<E, ecs::component::Radius>)
		{
			propertyId = registerPropertyLeaf<F, E, float>(propertyId, depth, offsetof(E, val), "radius");
		}
		else if constexpr (std::is_same_v<E, ecs::component::MainColor>)
		{
			propertyId = registerPropertyLeaf<F, E, Color>(propertyId, depth, offsetof(E, val), "color");
		}
		else if constexpr (std::is_same_v<E, ecs::component::Normal>)
		{
			propertyId = registerPropertyLeaf<F, E, Vec3>(propertyId, depth, offsetof(E, val), "normal");
		}
		else if constexpr (std::is_same_v<E, ui::Layout>)
		{
			propertyId = registerPropertyLeaf<F, E, float>(propertyId, depth, offsetof(E, opacity), "opacity");
		}
		else if constexpr (std::is_same_v<E, ui::Button::Props>)
		{
			propertyId = registerPropertyLeaf<F, E, Color>(propertyId, depth, offsetof(E, color), "color");
		}
		else if constexpr (std::is_same_v<E, Camera>)
		{
			propertyId = registerPropertyLeaf<F, E, Vec3>(propertyId, depth, offsetof(E, position), "position");
			propertyId = registerPropertyLeaf<F, E, Vec3>(propertyId, depth, offsetof(E, target), "target");
			propertyId = registerPropertyLeaf<F, E, float>(propertyId, depth, offsetof(E, fov), "fov");
			propertyId = registerPropertyLeaf<F, E, float>(propertyId, depth, offsetof(E, zNear), "zNear");
			propertyId = registerPropertyLeaf<F, E, float>(propertyId, depth, offsetof(E, zFar), "zFar");
			propertyId = registerPropertyLeaf<F, E, bool>(propertyId, depth, offsetof(E, perspective), "perspective");
		}
		else
		{
			static_assert(sizeof(E) == 0, "Type is not registered as an entry type");
		}
		return propertyId;
	};

	template <typename F>
	constexpr Property::Id registerFrontendRoot(Property::Id propertyId)
	{
		properties[propertyId] = Property{.id		 = propertyId,
										  .depth	 = 0,
										  .offset	 = 0,
										  .sceneType = sceneType<F>(),
										  .entryType = EntryType::None,
										  .type		 = PropertyType::None};

		++propertyId;

		if constexpr (std::is_same_v<F, scene::Point>)
		{
			propertyId = registerEntryNode<F, ecs::component::Position>(propertyId, "position");
			propertyId = registerEntryNode<F, ecs::component::MainColor>(propertyId, "color");
			propertyId = registerEntryNode<F, ecs::component::Radius>(propertyId, "radius");
		}
		else if constexpr (std::is_same_v<F, scene::Plane>)
		{
			propertyId = registerEntryNode<F, ecs::component::Normal>(propertyId, "normal");
			propertyId = registerEntryNode<F, ecs::component::MainColor>(propertyId, "color");
		}
		else if constexpr (std::is_same_v<F, ui::Layout>)
		{
			propertyId = registerEntryNode<F, F>(propertyId, "ui");
		}
		else if constexpr (std::is_same_v<F, ui::Button::Props>)
		{
			propertyId = registerEntryNode<F, F>(propertyId, "button");
		}
		else if constexpr (std::is_same_v<F, Camera>)
		{
			propertyId = registerEntryNode<F, F>(propertyId, "camera");
		}
		else
		{
			static_assert(sizeof(F) == 0, "Type is not registered as a scene type");
		}

		return propertyId;
	}

  public:
	template <typename... F>
	constexpr PropertyRegistry(std::type_identity<F>...)
	{
		Property::Id propertyId = 0;
		((propertyId = registerFrontendRoot<F>(propertyId)), ...);
	};
};

constexpr PropertyRegistry<64> propertyRegistry{std::type_identity<scene::Point>{},
												std::type_identity<scene::Plane>{},
												std::type_identity<ui::Layout>{},
												std::type_identity<ui::Button::Props>{},
												std::type_identity<Camera>{}};
}  // namespace detail

template <typename F>
constexpr const Property& getSceneTypeProperty()
{
	const auto predicate = [](const Property& p) { return p.depth == 0 && p.sceneType == sceneType<F>(); };
	constexpr auto it	 = std::ranges::find_if(detail::propertyRegistry.properties, predicate);

	if constexpr (it != detail::propertyRegistry.properties.end())
	{
		return *it;
	}
	else
	{
		static_assert(sizeof(F) == 0, "Type is not registered in PropertyRegistry");
	}
}

/** @brief for property path build */
constexpr const Property* findChildProperty(Property::Id propertyId, std::string_view name)
{
	return detail::propertyRegistry.findPropertyChild(propertyId, name);
}

/** @brief for key build */
constexpr const Property& getPropertyRoot(Property::Id propertyId)
{
	return detail::propertyRegistry.getPropertyRoot(propertyId);
}

/** @brief for channels search */
constexpr auto nextFloatProperties(Property::Id propertyId, std::size_t count)
{
	// clang-format off
	return detail::propertyRegistry.properties																	   
		   | std::views::drop_while([propertyId](const Property& property) { return property.id != propertyId; })  
		   | std::views::filter([](const Property& property) { return property.type == PropertyType::Float ||  property.type == PropertyType::Bool; })			   
		   | std::views::take(count);
	// clang-format on
}

/** @brief for apply animation */
constexpr const Property& getProperty(Property::Id propertyId)
{
	return detail::propertyRegistry.getProperty(propertyId);
}

}  // namespace gs::reflection

#endif	// GEOMETRY_SANDBOX_REFLECTION_H