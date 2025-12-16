#include <acceleration_structures.h>
#include <collision.h>
#include <components/camera.h>
#include <ecs/coordinator.h>
#include <engine.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui.h>
#include <sandbox.h>
#include <systems/camera_system.h>
#include <systems/render_system.h>
#include <systems/transform_gui_system.h>

Sandbox::Sandbox(Engine& engine) : Application(engine)
{
	mRenderContext.renderScale = 1.0f;
	mRenderContext.samplesPerPixel = 1;
	mRenderContext.frame = 0;
	isSimulating = true;
	mCurrentScene = 0;
}

void Sandbox::Awake()
{
	InitRt();

	mGlobalCoordinator.RegisterComponent<Camera>();

	{
		Entity entity = mGlobalCoordinator.CreateEntity();

		Camera camera{};
		camera.position = glm::vec3(0.0f, 2.0f, -3.0f);
		camera.sensitivity = 0.1f;
		camera.speed = 1.0f;
		camera.yaw = 90.0f;
		camera.fov = 60.0f;
		mGlobalCoordinator.AddComponent<Camera>(entity, camera);
	}

	mScenes.push_back(MirrorScene());
	mScenes.push_back(SponzaScene());

	mCurrentScene = 1;

	// NOTE(Sergei): Needs to happen after scenes are loaded
	InitRtDescriptorLayout();
	InitRtSceneDescriptorLayout();
	InitRtPipeline();
	InitRtSBT();
}

void Sandbox::Update(f64 deltaTime)
{
	CameraSystem::Update(mGlobalCoordinator.mEntityManager, mGlobalCoordinator.mComponentManager, mRenderContext.camera, deltaTime);
}

void Sandbox::PhysicsUpdate(f64 deltaTime) {}

void Sandbox::Render(VkCommandBuffer cmd, FrameData& currentFrame)
{
	Scene& scene = mScenes[mCurrentScene];

	mRenderContext.scene.mainLightPos = scene.mainLightPosition;
	mRenderContext.scene.mainLightColor = glm::vec4(scene.mainLightColor, scene.mainLightIntensity);
	mRenderContext.scene.skyColor = scene.skyColor;
	mRenderContext.scene.skyTextureId = scene.skyTextureIndex;
	mRenderContext.instances.clear();
	mRenderContext.light.lightCount = 0;
	mRenderContext.raytracing.tlas = scene.tlas.handle;

	RenderSystem::Update(cmd, currentFrame, mEngine, scene);

	TransitionImageLayout(cmd, mEngine.mColorTarget.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
	RenderRt(cmd, currentFrame);
	TransitionImageLayout(cmd, mEngine.mColorTarget.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
}

void Sandbox::OnGUI()
{
	ImGuiCustomStyle();

	if (ImGui::Begin("Settings"))
	{
		Scene& scene = mScenes[mCurrentScene];

		ImGuiApplication();
		CameraSystem::OnGUI(mGlobalCoordinator.mEntityManager, mGlobalCoordinator.mComponentManager);
		ImGuiScene(scene);
		TransformGUISystem::Update(scene.coordinator.mEntityManager, scene.coordinator.mComponentManager);
	}

	ImGui::End();
}

void Sandbox::Destroy() {}
