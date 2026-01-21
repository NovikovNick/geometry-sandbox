#include "animation/manager.h"

#include "animation/reflection.h"
#include "animation/types.h"
#include "core/ecs.h"
#include "core/types.h"

#include <cassert>
#include <cstddef>	// for std::byte, std::size_t
#include <cstdint>	// for std::uint32_t
#include <cstring>	// for std::memcpy
#include <span>
#include <vector>

namespace gs::animation
{
namespace
{
template <typename T>
std::span<std::byte> getBytes(T* ptr)
{
	auto* bytes = reinterpret_cast<std::byte*>(ptr);  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
	return std::span<std::byte>{bytes, sizeof(T)};
}

std::span<std::byte> getAsBytes(reflection::EntryType type, std::uint32_t id, ui::State& uiState, ecs::Registry& registry)
{
	switch (type)
	{
		case reflection::EntryType::UILayout: return getBytes(&uiState.layout);
		case reflection::EntryType::Button: return getBytes(&dynamic_cast<ui::Button*>(uiState.elements[id])->props);  // ugh
		case reflection::EntryType::Camera: return getBytes(&uiState.cameras.at(id));
		case reflection::EntryType::MainColor: return getBytes(&registry.get<ecs::component::MainColor>(ecs::Entity{id}));
		case reflection::EntryType::Normal: return getBytes(&registry.get<ecs::component::Normal>(ecs::Entity{id}));
		case reflection::EntryType::Radius: return getBytes(&registry.get<ecs::component::Radius>(ecs::Entity{id}));
		case reflection::EntryType::Position: return getBytes(&registry.get<ecs::component::Position>(ecs::Entity{id}));
		default: assert(false && "Unsupported animation component"); return {};
	}
}

template <typename T>
void write(void* dst, T value)
{
	std::memcpy(dst, &value, sizeof(T));
}

}  // namespace

void Manager::apply(ecs::Registry& registry,
					ui::State& ui,
					const Target& target,
					const std::vector<Asset::Channel>& channels,
					const std::span<float>& sample)
{
	assert(channels.size() == sample.size());
	for (int i = 0; i < channels.size(); ++i)
	{
		const Asset::Channel& channel		 = channels.at(i);

		const reflection::Property& property = reflection::getProperty(channel.propertyId);
		const std::span<std::byte> bytes	 = getAsBytes(property.entryType, target.entityId, ui, registry);

		if (property.type == reflection::PropertyType::Bool)
		{
			write<bool>(&bytes[property.offset], sample[i] > 0.0F);
		}
		else
		{
			write<float>(&bytes[property.offset], sample[i]);
		}
	}
}
}  // namespace gs::animation