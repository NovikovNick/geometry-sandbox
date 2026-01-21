/**
 * @file slot_map.h
 * @brief stores values and returns a unique, stable, generation-checked key
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_SLOT_MAP_H
#define GEOMETRY_SANDBOX_SLOT_MAP_H

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <deque>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <vector>

/**
 * @brief stores values and returns a unique, stable, generation-checked key
 *
 * O(1) access, deletion, and insertion
 */
template <typename T>
class SlotMap final
{
	static constexpr float kGrowthFactor		  = 2.0F;
	static constexpr std::size_t kInitialCapacity = 16;

	struct Slot
	{
		T data;
		std::uint32_t generation;
		bool occupied;

		Slot() : data(T{}), generation(0), occupied(false) {}
		Slot(const T& value) : data(value), generation(0), occupied(true) {}
	};

	std::vector<Slot> data_;
	std::uint32_t nextFreeSlot_;

  public:
	struct Key
	{
		uint32_t slotIndex				   = 0;
		uint32_t generation				   = 0;

		auto operator<=>(const Key&) const = default;
	};

	explicit SlotMap(std::size_t initial_capacity = kInitialCapacity) : data_(initial_capacity), nextFreeSlot_(0) {}

	std::vector<Slot>& data() { return data_; }

	const std::size_t size() const { return data_.size(); }

	bool isValid(const Key& key) const
	{
		return key.slotIndex < data_.size()							 //
			   && data_[key.slotIndex].generation == key.generation	 //
			   && data_[key.slotIndex].occupied;
	}

	auto forEach()
	{
		// clang-format off
		return data_
			   | enumerate()
			   | std::views::filter([](const auto& it) { return std::get<1>(it).occupied; }) 
			   | std::views::transform([](auto&& it) { return std::make_pair(std::get<0>(it), std::ref(std::get<1>(it).data)); });
		// clang-format on
	}

	/**
	 * @return index and generation
	 */
	Key insert(T&& value)
	{
		std::uint32_t slotIndex = nextFreeSlot_;

		// find next free slot
		do
		{
			++nextFreeSlot_;
		} while (nextFreeSlot_ < data_.size() && data_[nextFreeSlot_].occupied);

		// reallocate if needed
		if (data_.size() == nextFreeSlot_)
		{
			data_.resize(data_.size() * kGrowthFactor);
		}

		// occupy slot
		Slot& slot	  = data_[slotIndex];
		slot.occupied = true;
		slot.data	  = std::move(value);
		++slot.generation;

		return Key{slotIndex, slot.generation};
	}

	void remove(std::uint32_t slotIndex)
	{
		assert(slotIndex < data_.size() && data_[slotIndex].occupied);

		data_[slotIndex].occupied = false;
		nextFreeSlot_			  = std::min(nextFreeSlot_, slotIndex);
	}

	T& at(const Key& key)
	{
		assert(isValid(key));
		return data_[key.slotIndex].data;
	}

	const T& at(const Key& key) const
	{
		assert(isValid(key));
		return data_[key.slotIndex].data;
	}

  private:
	auto enumerate()
	{
#if defined(__cpp_lib_ranges_enumerate)
		return std::views::enumerate;
#else
		return std::views::transform(
			[&](auto& item)
			{
				static_assert(std::contiguous_iterator<decltype(data_.begin())>,
							  "SlotMap requires contiguous container for address arithmetic. Or c++23");
				return std::make_pair(&item - data_.data(), std::ref(item));
			});
#endif
	}
};

#endif	// GEOMETRY_SANDBOX_SLOT_MAP_H