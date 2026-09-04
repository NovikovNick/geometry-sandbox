#include "core/json_service.h"

#include "core/math.h"

#include "nlohmann/json.hpp"

#include <algorithm>
#include <string_view>
#include <vector>

namespace gs
{
bool JsonService::isValidCoord2dArrayString(std::string_view input) const
{
	using nlohmann::json;

	const json data = json::parse(input, /*callback*/ nullptr, /*throw exceptions*/ false);
	if (data.is_discarded())
	{
		return false;
	}
	auto isValid = [](const auto& item) { return item.size() == 2 && item[0].is_number() && item[1].is_number(); };
	return std::ranges::all_of(data, isValid);
}

void JsonService::parseCoord2dArrayString(std::string_view input, std::vector<Vec2>& outPoints) const
{
	using nlohmann::json;

	for (const auto& item : json::parse(input))
	{
		const float x = item[0];
		const float y = item[1];
		outPoints.emplace_back(x, y);
	}
}
}  // namespace gs
