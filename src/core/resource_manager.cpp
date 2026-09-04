#include "core/resource_manager.h"

#include "core/types.h"
#include "imgui.h"
#include "raylib.h"

#include <cstddef>
#include <format>

#ifdef PLATFORM_DESKTOP
constexpr int kGlslVersion = 330;
#else
constexpr int kGlslVersion = 100;
#endif

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

	// meshes
	models_.at(static_cast<std::size_t>(ModelType::Camera)) = LoadModel("resources/models/camera.glb");
	models_.at(static_cast<std::size_t>(ModelType::Duck))	= LoadModel("resources/models/little_duck.glb");

	// Mesh instancing shader
	{
		const Shader shader = LoadShader(std::format("resources/shaders/glsl{}/shader_instanced_color.vs", kGlslVersion).c_str(),
										 std::format("resources/shaders/glsl{}/shader_instanced_color.fs", kGlslVersion).c_str());

		// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		shader.locs[SHADER_LOC_MATRIX_MVP]	 = GetShaderLocation(shader, "mvp");
		shader.locs[SHADER_LOC_VECTOR_VIEW]	 = GetShaderLocation(shader, "viewPos");
		shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(shader, "instanceTransform");
		// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

		shaders_[static_cast<std::size_t>(ShaderType::MeshInstancing)] = shader;
	}

	// Bloom shader
	{
		shaders_[static_cast<std::size_t>(ShaderType::Blur)] = LoadShader(nullptr,
																		  std::format("resources/shaders/glsl{}/blur.fs", kGlslVersion)
																			  .c_str());
	}

	// FXAA shader
	{
		shaders_[static_cast<std::size_t>(ShaderType::FXAA)] = LoadShader(nullptr,
																		  std::format("resources/shaders/glsl{}/fxaa.fs", kGlslVersion)
																			  .c_str());
	}

	// point
	{
		constexpr int kSphereRings	= 16;
		constexpr int kSphereSlices = 16;
		meshes_.push_back(GenMeshSphere(1.0F, kSphereRings, kSphereSlices));
		const Mesh& mesh									   = meshes_.back();

		models_.at(static_cast<std::size_t>(ModelType::Point)) = LoadModelFromMesh(mesh);
		const Model& model									   = models_.at(static_cast<std::size_t>(ModelType::Point));
		Material& material						  = model.materials[0];	 // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		material.maps[MATERIAL_MAP_DIFFUSE].color = ::BLACK;			 // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		material.shader							  = getShader(ShaderType::MeshInstancing);
	}
}

const Shader& ResourceManager::getShader(ShaderType type) const
{
	return shaders_.at(static_cast<std::size_t>(type));
}
const Model& ResourceManager::getModel(ModelType type) const
{
	return models_.at(static_cast<std::size_t>(type));
}

ResourceManager::~ResourceManager()
{
	UnloadFont(defaultCanvasFont_);
}

}  // namespace gs
