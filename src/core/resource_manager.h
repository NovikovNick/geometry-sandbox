/**
 * @file resource_manager.h
 * @brief load and access to all resources: fonts, meshes, shaders
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_RESOURCE_MANAGER_H
#define GEOMETRY_SANDBOX_RESOURCE_MANAGER_H

#include "core/base_app_component.h"
#include "core/types.h"

#include "boost/di.hpp"
#include "raylib.h"

#include <filesystem>
#include <functional>
#include <memory>
#include <vector>

// imgui types
struct ImFont;

namespace gs
{
class IJobManager;

struct ShaderPaths
{
	std::filesystem::path vertex;
	std::filesystem::path fragment;
};

/**
 * @brief load and access to all resources: fonts, meshes, shaders
 *
 * As for me "asset manager" is close to editor mode, and "resource manager" is closer to runtime
 * All resources are expected to be loaded on startup. There is no release functionality.
 */
class IResourceManager
{
  public:
	// -- predefined resources --

	virtual void load()							  = 0;
	virtual const Font& defaultCanvasFont() const = 0;
	virtual ImFont* defaultUIFont() const		  = 0;
	virtual ImFont* iconFont() const			  = 0;

	virtual ResourceId addModel(::Model)		  = 0;

	// -- sample-specific resources --

	virtual ResourceId loadShaderAsync(const ShaderPaths&, std::function<void(::Shader&)>&& onLoad = [](::Shader&) {}) = 0;
	virtual ResourceId loadResourceAsync(const std::filesystem::path&, ResourceType)								   = 0;
	virtual bool isAllResourcesLoaded() const																		   = 0;

	virtual ::Shader& getShader(ResourceId)																			   = 0;
	virtual ::Model& getModel(ResourceId)																			   = 0;

	virtual ~IResourceManager()																						   = default;
};

/** @brief basic IResourceManager implementation */
class ResourceManager : public BaseManager, public IResourceManager
{
	std::shared_ptr<IJobManager> jobManager_;

	ImFont* defaultUIFont_;
	ImFont* iconsFont_;

	Font defaultCanvasFont_;

	std::vector<unsigned char> resourceBuffer_;
	int pendingResourceCount_;

	std::vector<Shader> shaders_;
	std::vector<Model> models_;

  public:
	ResourceManager(const std::shared_ptr<Settings>& settings,
					const std::shared_ptr<ILogManager>& log,
					const std::shared_ptr<IJobManager>& jobManager)
		: BaseManager(settings, log), jobManager_(jobManager), pendingResourceCount_(0)
	{
	}

	virtual void load() override;

	virtual const Font& defaultCanvasFont() const override { return defaultCanvasFont_; };
	virtual ImFont* defaultUIFont() const override { return defaultUIFont_; };
	virtual ImFont* iconFont() const override { return iconsFont_; };

	virtual ResourceId addModel(::Model) override;

	virtual ResourceId loadShaderAsync(const ShaderPaths&, std::function<void(::Shader&)>&& onLoad) override;
	virtual ResourceId loadResourceAsync(const std::filesystem::path&, ResourceType) override;
	virtual bool isAllResourcesLoaded() const override;

	virtual ::Shader& getShader(ResourceId) override;
	virtual ::Model& getModel(ResourceId) override;

	virtual ~ResourceManager() override;
};

namespace di
{
inline auto resourceManager() noexcept
{
	return boost::di::bind<IResourceManager>.to<ResourceManager>();
}
}  // namespace di
}  // namespace gs

#endif	// GEOMETRY_SANDBOX_RESOURCE_MANAGER_H
