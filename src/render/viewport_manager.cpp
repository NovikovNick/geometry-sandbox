#include "render/viewport_manager.h"

#include "core/types.h"
#include "core/window_manager.h"
#include "ui/state_manager.h"

#include "raylib.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace gs::render
{
namespace
{
template <typename T>
void copyToMeshBuffer(T*& dest, const T* src, std::size_t count) noexcept
{
	if (count > 0)
	{
		dest = static_cast<T*>(MemAlloc(static_cast<unsigned>(count * sizeof(T))));
		if (dest)
		{
			std::memcpy(dest, src, count * sizeof(T));
		}
	}
}

// resourse manager?
::Mesh createPlane(const float halfWidth, const float halfHeight)
{
	::Mesh mesh		   = {.vertexCount = 0};
	mesh.vertexCount   = 4;
	mesh.triangleCount = 2;

	// clang-format off
	std::array vertices  {
        -halfWidth, -halfHeight, 0.0F, 
         halfWidth, -halfHeight, 0.0F,  
         halfWidth,  halfHeight, 0.0F, 
        -halfWidth,  halfHeight, 0.0F,
    };
	std::array texcoords = {
		0.0F, 0.0F,
		1.0F, 0.0F,  
		1.0F, 1.0F, 
		0.0F, 1.0F, 
	};

	std::array normals = {
		0.0F, 0.0F, 1.0F,
		0.0F, 0.0F, 1.0F,
		0.0F, 0.0F, 1.0F,
		0.0F, 0.0F, 1.0F,
	};
	// clang-format on

	std::array<std::uint16_t, 6> indices = {0, 1, 2, 0, 2, 3};	// NOLINT(*-magic-numbers)

	copyToMeshBuffer(mesh.vertices, vertices.data(), vertices.size());
	copyToMeshBuffer(mesh.texcoords, texcoords.data(), texcoords.size());
	copyToMeshBuffer(mesh.normals, normals.data(), normals.size());
	copyToMeshBuffer(mesh.indices, indices.data(), indices.size());

	UploadMesh(&mesh, false);

	return mesh;
}

}  // namespace

void ViewportManager::init()
{
	buildViewports();
	windowManager_->onResize(
		[&](int width, int height)
		{ rebuildViewports(RectSize{.width = static_cast<float>(width), .height = static_cast<float>(height)}); });
}

void ViewportManager::setupViewport(std::size_t viewportIndex)
{
	assert(viewportIndex < viewports_.size());
	BeginTextureMode(viewports_[viewportIndex].renderTarget);
}

void ViewportManager::cleanupViewport()
{
	EndTextureMode();
}

Model ViewportManager::getViewport3D(std::size_t viewportIndex) const
{
	assert(viewportIndex < viewports_.size());
	return viewports_[viewportIndex].plane;
}

Texture ViewportManager::getViewport2D(std::size_t viewportIndex) const
{
	assert(viewportIndex < viewports_.size());
	return viewports_[viewportIndex].renderTarget.texture;
}

ViewportManager::Viewport ViewportManager::createViewport(const RectSize& size)
{
	const float aspect					= size.width / size.height;
	const float top						= aspect;
	const float right					= 1.0F;

	constexpr float supersamplingFactor = 2.0F;

	ViewportManager::Viewport res{
		.plane		  = LoadModelFromMesh(createPlane(top, right)),
		.renderTarget = LoadRenderTexture(static_cast<int>(size.width * supersamplingFactor),
										  static_cast<int>(size.height * supersamplingFactor)),
	};

	SetTextureFilter(res.renderTarget.texture, TEXTURE_FILTER_BILINEAR);

	// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
	res.plane.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = res.renderTarget.texture;

	return res;
}

void ViewportManager::buildViewports()
{
	const ui::State ui = uiStateManager_->getState();
	viewports_.resize(ui.cameras.size());
	for (std::size_t i = 0; i < ui.cameras.size(); ++i)
	{
		const Camera& camera = ui.cameras[i];
		const RectSize size{.width = static_cast<float>(camera.width), .height = static_cast<float>(camera.height)};
		viewports_[i] = createViewport(size);
	}
}

void ViewportManager::rebuildViewports(const RectSize& size)
{
	const ui::State ui = uiStateManager_->getState();
	for (std::size_t i = 0; i < ui.cameras.size(); ++i)
	{
		UnloadRenderTexture(viewports_[i].renderTarget);
		UnloadModel(viewports_[i].plane);
		viewports_[i] = createViewport(size);
	}
}

}  // namespace gs::render
