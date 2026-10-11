/**
 * @file sample.cpp
 * @brief research meshes in details...
 *
 * @author MetalHeart
 */
#include "sample.h"

#include "animation/dsl.h"
#include "animation/handle.h"
#include "animation/manager.h"
#include "animation/player_manager.h"
#include "core/application.h"
#include "core/camera_controller_service.h"
#include "core/configuration.h"
#include "core/ecs.h"
#include "core/input_manager.h"
#include "core/job_manager.h"
#include "core/math.h"
#include "core/scene_service.h"
#include "core/types.h"
#include "feature/camera_idle_rotation_manager.h"
#include "interaction/ecs_components.h"
#include "interaction/service.h"
#include "render/low_level_service.h"
#include "ui/manager.h"
#include "ui/state_manager.h"
#include "ui/ui.h"

#include "glad.h"
#include "imgui.h"

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <utility>
#include <vector>

namespace gs
{
namespace
{
// tmp begin
::Vector3 convert(Vec3 src)
{
	return {.x = src.x(), .y = src.y(), .z = src.z()};
}

// tmp end

/** @brief Parameters that the user configures through the UI update or dragging scene objects */
struct Parameters
{
	bool loaded = false;
	Model model{};
	ModelAnimation* animations{};
};

/** @brief Identifiers of scene objects related to the camera */
struct Entities
{
};

/** @brief Create scene objecs and define colors, radiuses, font sizes and etc */
void setupScene(Parameters& params)
{
	// NOLINTBEGIN(*-magic-numbers)
	auto& ctx					  = di::getContext();
	auto& app					  = ctx.create<Application&>();	 // Application should be created first
	auto& uiStateManager		  = ctx.create<IUIStateManager&>();
	auto& sceneService			  = ctx.create<ISceneService&>();
	auto& renderer				  = ctx.create<render::IFacade&>();
	auto& graphic				  = ctx.create<render::ILowLevelService&>();
	auto& cameraService			  = ctx.create<ICameraService&>();
	auto& cameraControllerService = ctx.create<ICameraControllerService&>();
	auto& input					  = ctx.create<IInputManager&>();
	auto& jobs					  = ctx.create<IJobManager&>();
	auto& log					  = ctx.create<ILogManager&>();
	auto& settings				  = ctx.create<Settings&>();

	settings.showPerformance	  = true;

	constexpr int count			  = 4;
	constexpr float radius		  = 0.05F;
	constexpr Color color		  = Color::yellow();

	Camera& camera				  = uiStateManager.getState().cameras[0];
	camera.position				  = Vec3{7, 7, 7};
	cameraService.lookAt(camera, Vec3{0, 1, 0});

	int pointCount = 0;
	for (float x = 0; x < 25; x += 1)
	{
		for (float y = 0; y < 25; y += 1)
		{
			for (float z = 0; z < 25; z += 1)
			{
				sceneService.addPoint({.position = Vec3{x / 5, y / 5, z / 5},  //
									   .color	 = Color::yellow(),
									   .radius	 = 0.02F});
				++pointCount;
			}
		}
	}
	log.log("Point count - {}", pointCount);

// #define TEST_OPENGL_MESH
#ifdef TEST_OPENGL_MESH

	const GLchar* vertexShaderSource   = "#version 330 core\n"
										 "layout (location = 0) in vec3 position;\n"
										 "layout (location = 1) in vec3 color;\n"
										 "out vec4 fragColor;\n"
										 "void main()\n"
										 "{\n"
										 "fragColor = vec4(color, 1.0);\n"
										 "gl_Position = vec4(position, 1.0);\n"
										 "}\0";
	const GLchar* fragmentShaderSource = "#version 330 core\n"
										 "in vec4 fragColor;\n"
										 "out vec4 color;\n"
										 "void main()\n"
										 "{\n"
										 "color = fragColor;\n"
										 "}\n\0";

	// clang-format off
	GLfloat vertices[] = {
		// coords			// color
		-0.9F,  0.9F, 0.0F, 1.0F, 0.0F,  0.0F, // top left
		-0.9F, -0.9F, 0.0F, 0.0F, 1.0F,  0.0F, // bottom left
		 0.9F,  0.9F, 0.0F, 0.0F, 0.0F,  1.0F  // top right
	};
	// clang-format on
	GLuint VBO;
	glGenBuffers(1, &VBO);

	GLuint indices[] = {0, 1, 2};
	GLuint IBO;
	glGenBuffers(1, &IBO);

	static GLuint VAO;
	glGenVertexArrays(1, &VAO);
	{
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0,					// location in shader
							  3,					// component count, 3 is for vector xyz
							  GL_FLOAT,				// type
							  GL_FALSE,				// to normalize or not, int to [0, 1], float to [-1, 1]
							  6 * sizeof(GLfloat),	// step between vertex
							  (GLvoid*)0);			// offset in bytes
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1,								// location in shader
							  3,								// component count, 3 is for vector xyz
							  GL_FLOAT,							// type
							  GL_FALSE,							// to normalize or not, int to [0, 1], float to [-1, 1]
							  6 * sizeof(GLfloat),				// step between vertex
							  (GLvoid*)(3 * sizeof(GLfloat)));	// offset in bytes
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	GLuint vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	GLuint fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	static GLuint shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	input.onPress(InputKey::MouseRight, [&] {});

	renderer.onRender(
		[&]
		{
			glUseProgram(shaderProgram);
			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		});

#else

	static int animCount = 1;
	// 1.
	/*
	params.model	  = LoadModel("resources/robot.glb");
	params.animations = LoadModelAnimations("resources/robot.glb", &animCount);
	*/

	// 2.

	static int animFrame = 0;
	static int animIndex = 0;
	input.onPress(InputKey::MouseRight,
				  [&]()
				  {
					  /*params.model		= LoadModel("resources/robot.glb");
					  params.animations = LoadModelAnimations("resources/robot.glb", &animCount);*/
				  });

	renderer.onRender(
		[&]
		{
			if (params.loaded)
			{
				log.frameLog("Mesh loaded. animCount = {}", animCount);
				/*
				animFrame = (animFrame + 1) % params.animations[animIndex].frameCount;
				UpdateModelAnimation(params.model, params.animations[animIndex], animFrame);
				*/
				DrawModel(params.model, convert(Vec3::Zero()), /*scale*/ 1.0F, ::WHITE);
			}
		});

#endif	// TEST_OPENGL_MESH

	// NOLINTEND(*-magic-numbers)
}

void setupPlaybackAnimation() {}

void updateScene(const Parameters& params, const Entities& entities) {}

void setupInteractionLogic(Parameters& params, const Entities& entities) {}

void setupUI(Parameters& params, const Entities& entities) {}

Application& getApp()
{
	static auto instance = di::getContext().create<Application>();
	return instance;
}
}  // namespace

void init()
{
	static Entities entities;
	static Parameters params;

	setupScene(params);
	setupInteractionLogic(params, entities);
	setupUI(params, entities);
	setupPlaybackAnimation();

	updateScene(params, entities);
}

void drawNextFrame()
{
	static Timepoint prev		= Clock::now();
	const Timepoint now			= Clock::now();
	const Nanoseconds timeDelta = now - prev;
	prev						= now;

	getApp().drawNextFrame(timeDelta);
}

void startInfiniteLoop()
{
	getApp().run();
}

}  // namespace gs
