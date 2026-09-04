/**
 * @file json_service.h
 * @brief validates and parses json string to app's types
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_JSON_SERVICE_H
#define GEOMETRY_SANDBOX_JSON_SERVICE_H

#include "core/base_app_component.h"
#include "core/types.h"

#include "boost/di.hpp"

#include <memory>
#include <string_view>
#include <vector>

namespace gs
{
/** @brief validates and parses json string to app's types */
class IJsonService
{
  public:
	/**
	 * @brief validate array of 2d points in format [[x1,y1], [x2,y2], ...]
	 */
	virtual bool isValidCoord2dArrayString(std::string_view input) const = 0;

	/**
	 * @brief parse array of 2d points in format [[x1,y1], [x2,y2], ...]
	 */
	virtual void parseCoord2dArrayString(std::string_view input, std::vector<Vec2>& outPoints) const = 0;

	virtual ~IJsonService()																			 = default;
};

/** @brief basic IJsonService implementation */
class JsonService : public BaseService, public IJsonService
{
  public:
	JsonService(const std::shared_ptr<Settings>& settings, const std::shared_ptr<ILogManager>& log) : BaseManager(settings, log) {}

	virtual bool isValidCoord2dArrayString(std::string_view input) const override;
	virtual void parseCoord2dArrayString(std::string_view input, std::vector<Vec2>& outPoints) const override;
};

namespace di
{
inline auto jsonService() noexcept
{
	return boost::di::bind<IJsonService>.to<JsonService>();
}
}  // namespace di
}  // namespace gs
#endif	// GEOMETRY_SANDBOX_JSON_SERVICE_H
