#include "render/mesh_instanced_draw_service.h"

#include "core/math.h"
#include "core/resource_manager.h"
#include "core/settings.h"
#include "core/types.h"

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <array>
#include <cassert>
#include <memory>
#include <span>
#include <string>
#include <utility>

namespace gs::render
{
namespace
{
class ScopedShader
{
	::Shader shader_;

  public:
	explicit ScopedShader(const ::Shader& shader) : shader_(shader) { rlEnableShader(shader_.id); }
	ScopedShader(ScopedShader&)						 = delete;
	ScopedShader& operator=(const ScopedShader&)	 = delete;
	ScopedShader(ScopedShader&&) noexcept			 = delete;
	ScopedShader& operator=(ScopedShader&&) noexcept = delete;

	[[nodiscard]] int getLocationIndex(ShaderLocationIndex idx) const { return shader_.locs[idx]; }	 // NOLINT(*-pointer-arithmetic)
	[[nodiscard]] int getLocationAttr(const std::string& attrName) const { return rlGetLocationAttrib(shader_.id, attrName.c_str()); }

	~ScopedShader() { rlDisableShader(); }
};

class Material
{
	::Material material_;

  public:
	explicit Material(::Material material) : material_(material) {}

	static constexpr std::array<std::pair<MaterialMapIndex, ShaderLocationIndex>, 3> cubemapIndexes{
		std::make_pair(MATERIAL_MAP_CUBEMAP, SHADER_LOC_MAP_CUBEMAP),
		std::make_pair(MATERIAL_MAP_IRRADIANCE, SHADER_LOC_MAP_IRRADIANCE),
		std::make_pair(MATERIAL_MAP_PREFILTER, SHADER_LOC_MAP_PREFILTER),
	};

	static constexpr std::array<std::pair<MaterialMapIndex, ShaderLocationIndex>, 8> textureIndexes{
		std::make_pair(MATERIAL_MAP_ALBEDO, SHADER_LOC_MAP_ALBEDO),
		std::make_pair(MATERIAL_MAP_METALNESS, SHADER_LOC_MAP_METALNESS),
		std::make_pair(MATERIAL_MAP_NORMAL, SHADER_LOC_MAP_NORMAL),
		std::make_pair(MATERIAL_MAP_ROUGHNESS, SHADER_LOC_MAP_ROUGHNESS),
		std::make_pair(MATERIAL_MAP_OCCLUSION, SHADER_LOC_MAP_OCCLUSION),
		std::make_pair(MATERIAL_MAP_EMISSION, SHADER_LOC_MAP_EMISSION),
		std::make_pair(MATERIAL_MAP_HEIGHT, SHADER_LOC_MAP_HEIGHT),
		std::make_pair(MATERIAL_MAP_BRDF, SHADER_LOC_MAP_BRDF),
	};

	void bind(const ScopedShader& shader) const
	{
		for (const auto [materialIdx, shaderLoc] : cubemapIndexes)
		{
			if (const unsigned textureId = material_.maps[materialIdx].texture.id; textureId != 0)	// NOLINT(*-pointer-arithmetic)
			{
				const int textureSlot = materialIdx;
				rlActiveTextureSlot(textureSlot);
				rlEnableTextureCubemap(textureId);

				assert(shader.getLocationIndex(shaderLoc) != -1);
				rlSetUniform(shader.getLocationIndex(shaderLoc), &textureSlot, SHADER_UNIFORM_INT, 1);
			}
		}

		for (const auto [materialIdx, shaderLoc] : textureIndexes)
		{
			if (const unsigned textureId = material_.maps[materialIdx].texture.id; textureId != 0)	// NOLINT(*-pointer-arithmetic)
			{
				const int textureSlot = materialIdx;
				rlActiveTextureSlot(textureSlot);
				rlEnableTexture(textureId);

				assert(shader.getLocationIndex(shaderLoc) != -1);
				rlSetUniform(shader.getLocationIndex(shaderLoc), &textureSlot, SHADER_UNIFORM_INT, 1);
			}
		}
	}

	void unbind() const
	{
		for (const auto [materialIdx, shaderLoc] : cubemapIndexes)
		{
			if (material_.maps[materialIdx].texture.id != 0)  // NOLINT(*-pointer-arithmetic)
			{
				const int textureSlot = materialIdx;
				rlActiveTextureSlot(textureSlot);
				rlDisableTexture();
			}
		}

		for (const auto [materialIdx, shaderLoc] : textureIndexes)
		{
			if (material_.maps[materialIdx].texture.id != 0)  // NOLINT(*-pointer-arithmetic)
			{
				const int textureSlot = materialIdx;
				rlActiveTextureSlot(textureSlot);
				rlDisableTexture();
			}
		}
	}
};

unsigned uploadTransformsToGPU(const std::span<Mat4>& transforms, const ScopedShader& shader)
{
	const unsigned vboId = rlLoadVertexBuffer(transforms.data(),
											  static_cast<int>(transforms.size() * sizeof(Mat4)),
											  /* dynamic */ false);
	for (int i = 0; i < 4; i++)
	{
		const int loc = shader.getLocationIndex(SHADER_LOC_MATRIX_MODEL) + i;
		assert(loc != -1);
		rlEnableVertexAttribute(loc);
		rlSetVertexAttribute(loc, 4, RL_FLOAT, false, static_cast<int>(sizeof(Matrix)), i * static_cast<int>(sizeof(Vector4)));
		rlSetVertexAttributeDivisor(loc, 1);
	}
	return vboId;
}

unsigned uploadColorsToGPU(const std::span<Color>& colors, const ScopedShader& shader)
{
	const int loc = shader.getLocationAttr("instanceColor");
	assert(loc != -1);

	const unsigned vboId = rlLoadVertexBuffer(colors.data(), static_cast<int>(colors.size() * sizeof(Color)), /* dynamic */ false);

	rlEnableVertexAttribute(loc);
	rlSetVertexAttribute(loc, 4, RL_FLOAT, false, 0, 0);
	rlSetVertexAttributeDivisor(loc, 1);
	return vboId;
}

void bindMesh(const ::Mesh& mesh, const ScopedShader& shader)
{
	assert(shader.getLocationIndex(SHADER_LOC_VERTEX_POSITION) != -1);
	assert(shader.getLocationIndex(SHADER_LOC_VERTEX_TEXCOORD01) != -1);

	// NOLINTBEGIN(*-pointer-arithmetic)
	if (!rlEnableVertexArray(mesh.vaoId))
	{
		rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION]);
		rlSetVertexAttribute(shader.getLocationIndex(SHADER_LOC_VERTEX_POSITION), 3, RL_FLOAT, false, 0, 0);
		rlEnableVertexAttribute(shader.getLocationIndex(SHADER_LOC_VERTEX_POSITION));

		rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD]);
		rlSetVertexAttribute(shader.getLocationIndex(SHADER_LOC_VERTEX_TEXCOORD01), 2, RL_FLOAT, false, 0, 0);
		rlEnableVertexAttribute(shader.getLocationIndex(SHADER_LOC_VERTEX_TEXCOORD01));

		if (const int loc = shader.getLocationIndex(SHADER_LOC_VERTEX_NORMAL); loc != -1)
		{
			rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL]);
			rlSetVertexAttribute(loc, 3, RL_FLOAT, false, 0, 0);
			rlEnableVertexAttribute(loc);
		}

		if (mesh.indices != nullptr)
		{
			rlEnableVertexBufferElement(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES]);
		}
	}
	// NOLINTEND(*-pointer-arithmetic)
}
}  // namespace

MeshInstancedDrawService::MeshInstancedDrawService(const std::shared_ptr<Settings>& settings,
												   const std::shared_ptr<ILogManager>& log,
												   const std::shared_ptr<IResourceManager>& resources)
	: BaseService(settings, log), resources_(resources)
{
	shaderId_ = resources_->loadShaderAsync({.vertex   = settings->resourcePathInstancingVertexShader,
											 .fragment = settings->resourcePathInstancingFragmentShader},
											[](Shader& shader)
											{
												// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
												shader.locs[SHADER_LOC_MATRIX_MVP]	 = GetShaderLocation(shader, "mvp");
												shader.locs[SHADER_LOC_VECTOR_VIEW]	 = GetShaderLocation(shader, "viewPos");
												shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(shader,
																											   "instanceTransform");
												// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
											});
}

/** @brief adapt raylib's DrawMeshInstanced */
void MeshInstancedDrawService::drawMeshInstanced(ResourceId resourceId,
												 const Vec3& cameraPosition,
												 const std::span<Mat4>& transforms,
												 const std::span<Color>& colors) const
{
	assert(resources_->isAllResourcesLoaded());
	assert(resourceId.type == ResourceType::Mesh);
	assert(transforms.size() == colors.size());

	::Shader s = resources_->getShader(shaderId_);
	SetShaderValue(s,
				   s.locs[SHADER_LOC_VECTOR_VIEW],	// NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
				   cameraPosition.data(),
				   SHADER_UNIFORM_VEC3);

	const ScopedShader shader{s};
	assert(shader.getLocationIndex(SHADER_LOC_MATRIX_MVP) != -1);

	const ::Model& model = resources_->getModel(resourceId);
	const ::Mesh& mesh	 = model.meshes[0];	 // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

	rlEnableVertexArray(mesh.vaoId);

	const unsigned transformsVboId = uploadTransformsToGPU(transforms, shader);
	const unsigned colorsVboId	   = uploadColorsToGPU(colors, shader);

	rlDisableVertexBuffer();
	rlDisableVertexArray();

	const Material mat{model.materials[0]};	 // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
	mat.bind(shader);

	bindMesh(mesh, shader);

	rlSetUniformMatrix(shader.getLocationIndex(SHADER_LOC_MATRIX_MVP),
					   MatrixMultiply(rlGetMatrixModelview(), rlGetMatrixProjection()));

	const int instanceCount = static_cast<int>(transforms.size());
	if (mesh.indices != nullptr)
	{
		rlDrawVertexArrayElementsInstanced(0, mesh.triangleCount * 3, nullptr, instanceCount);
	}
	else
	{
		rlDrawVertexArrayInstanced(0, mesh.vertexCount, instanceCount);
	}

	mat.unbind();

	rlDisableVertexBufferElement();
	rlDisableVertexBuffer();
	rlDisableVertexArray();

	rlUnloadVertexBuffer(transformsVboId);
	rlUnloadVertexBuffer(colorsVboId);
}
}  // namespace gs::render
