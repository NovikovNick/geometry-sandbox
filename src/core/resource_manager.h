/**
 * @file resource_manager.h
 * @brief load and access to all resources: fonts, meshes, shaders
 *
 * If dynamic loading from file system were used, it could be called an asset manager.
 * But all resources are static and loaded at startup, so the name is appropriate.
 *
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_RESOURCE_MANAGER_H
#define GEOMETRY_SANDBOX_RESOURCE_MANAGER_H

#include "core/types.h"

#include "boost/di.hpp"
#include "raylib.h"

#include <array>
#include <functional>
#include <list>
#include <memory>

// imgui types
struct ImFont;

namespace gs
{
enum class ShaderType
{
	MeshInstancing,
	Blur,
	FXAA,
	Count
};

/** @brief load and access to all resources: fonts, meshes, shaders */
class IResourceManager
{
  public:
	virtual void load()								  = 0;
	virtual const Font& defaultCanvasFont() const	  = 0;
	virtual ImFont* defaultUIFont() const			  = 0;
	virtual ImFont* iconFont() const				  = 0;

	virtual const Shader& getShader(ShaderType) const = 0;
	virtual const Model& getModel(ModelType) const	  = 0;

	virtual ~IResourceManager()						  = default;
};

/** @brief basic IResourceManager implementation */
class ResourceManager : public IResourceManager
{
	ImFont* defaultUIFont_;
	ImFont* iconsFont_;

	Font defaultCanvasFont_;

	std::array<Shader, static_cast<std::size_t>(ShaderType::Count)> shaders_;
	std::array<Model, static_cast<std::size_t>(ModelType::Count)> models_;
	std::list<Mesh> meshes_;
	// std::list<Material> materials_;

  public:
	ResourceManager() {};

	virtual void load() override;

	virtual const Font& defaultCanvasFont() const override { return defaultCanvasFont_; };
	virtual ImFont* defaultUIFont() const override { return defaultUIFont_; };
	virtual ImFont* iconFont() const override { return iconsFont_; };

	virtual const Shader& getShader(ShaderType) const override;
	virtual const Model& getModel(ModelType) const override;

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
