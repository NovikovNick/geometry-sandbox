#include "resource_manager.h"

#include "core/job_manager.h"
#include "core/log_manager.h"
#include "core/settings.h"
#include "core/types.h"
#include "imgui.h"
#include "raylib.h"

#include <cstddef>
#include <filesystem>
#include <format>

namespace gs
{
namespace
{
ImFont* loadUIFont(const char* filename, float sizePixels)
{
	ImFontConfig config;
	config.SizePixels  = sizePixels;  // Base font size in pixels
	config.OversampleH = 4;			  // Horizontal anti-aliasing
	config.OversampleV = 4;			  // Vertical anti-aliasing
	config.PixelSnapH  = false;		  // Disable pixel snapping for smoother text

	return ImGui::GetIO().Fonts->AddFontFromFileTTF(filename, sizePixels, &config);
}
}  // namespace

ResourceManager::ResourceManager(const std::shared_ptr<Settings>& settings,
								 const std::shared_ptr<ILogManager>& log,
								 const std::shared_ptr<IJobManager>& jobManager)
	: BaseManager(settings, log), jobManager_(jobManager),	//
	  defaultUIFont_(nullptr), iconsFont_(nullptr), defaultCanvasFont_({}), pendingResourceCount_(0)
{
	log_->log("ResourceManager constructed");
}

void ResourceManager::load()
{
	const float kUIFontSizePixels = 20;
	const int kCanvasFontSize	  = 128;
	// imgui
	defaultUIFont_ = loadUIFont("resources/default_fnt.otf", kUIFontSizePixels);
	iconsFont_	   = loadUIFont("resources/Font Awesome 7 Free-Solid-900.otf", kUIFontSizePixels);

	// raylib
	defaultCanvasFont_ = LoadFontEx("resources/lmmonolt-regular-webfont.ttf", kCanvasFontSize, nullptr, 0);
	SetTextureFilter(defaultCanvasFont_.texture, TEXTURE_FILTER_BILINEAR);
}

Shader& ResourceManager::getShader(ResourceId id)
{
	assert(isAllResourcesLoaded());	 // there is no race condition cause resource manager is single-threaded
	assert(id.type == ResourceType::Shader);
	return shaders_.at(id.id);
}

Model& ResourceManager::getModel(ResourceId id)
{
	assert(isAllResourcesLoaded());	 // there is no race condition cause resource manager is single-threaded
	assert(id.type == ResourceType::Mesh);
	return models_.at(id.id);
}

ResourceId ResourceManager::loadShaderAsync(const ShaderPaths& shaderPaths, std::function<void(::Shader&)>&& onLoad)
{
	assert(shaders_.size() < std::numeric_limits<std::uint16_t>::max());

	ResourceId id{.id = static_cast<std::uint16_t>(shaders_.size()), .type = ResourceType::Shader};
	shaders_.emplace_back();

	auto guard = std::make_shared<PendingGuard>(*this);	 // ++pendingResourceCount_;

	jobManager_->schedule(
		[this, paths = shaderPaths, onLoad = std::move(onLoad), guard = std::move(guard), idx = id.id]
		{
			const std::filesystem::path basePath = std::filesystem::current_path() / "resources";

			shaders_.at(idx)					 = LoadShader(	//
				paths.vertex.empty() ? nullptr : (basePath / paths.vertex).string().c_str(),
				paths.fragment.empty() ? nullptr : (basePath / paths.fragment).string().c_str());

			onLoad(shaders_.at(idx));
		});

	return id;
}

ResourceId ResourceManager::generateModelAsync(std::function<::Model()>&& generator)
{
	ResourceId id{.id = static_cast<std::uint16_t>(models_.size()), .type = ResourceType::Mesh};
	models_.emplace_back();

	auto guard = std::make_shared<PendingGuard>(*this);	 // ++pendingResourceCount_;

	jobManager_->schedule([this, generator = std::move(generator), guard = std::move(guard), idx = id.id]
						  { models_.at(idx) = generator(); });

	return id;
}

ResourceId ResourceManager::loadResourceAsync(ResourceType type, const std::filesystem::path& resourcePath)
{
	ResourceId id{.id = 0, .type = type};
	switch (type)
	{
		case ResourceType::Mesh:
		{
			id.id = models_.size();
			models_.emplace_back();

			auto guard = std::make_shared<PendingGuard>(*this);	 // ++pendingResourceCount_;
			jobManager_->schedule(
				[this, path = resourcePath, guard = std::move(guard), idx = id.id]
				{
					const std::filesystem::path p = std::filesystem::current_path() / "resources" / path;
					models_.at(idx)				  = LoadModel(p.string().c_str());
				});

			break;
		}
		default: assert(false && "Unsupported resource type");
	}

	return id;
}

bool ResourceManager::isAllResourcesLoaded() const
{
	return pendingResourceCount_ == 0;
}

void ResourceManager::onAllResourcesLoaded() const
{
	log_->log("All resources loaded");
}

ResourceManager::~ResourceManager()
{
	UnloadFont(defaultCanvasFont_);
}
}  // namespace gs
