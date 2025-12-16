#pragma once

#include <glm/glm.hpp>
#include <lights.h>
#include <render_object.h>

#include <array>
#include <vector>

struct SceneRenderData
{
	glm::vec3 mainLightPos;
	glm::vec4 mainLightColor;
	glm::vec3 skyColor;
	i32 skyTextureId;
};

struct CameraRenderData
{
	glm::vec3 pos;
	glm::vec3 forward;
	glm::vec3 up;
	float fov;
};

struct LightRenderData
{
	std::array<LightData, MAX_LIGHTS> lightBuffer;
	u32 lightCount;
};

struct RaytracingData
{
	VkAccelerationStructureKHR tlas;
};

struct RenderContext
{
	std::vector<RenderInstance> instances;
	SceneRenderData scene;
	CameraRenderData camera;
	LightRenderData light;
	RaytracingData raytracing;
	float renderScale;
	i32 samplesPerPixel;
	u32 frame;
};
