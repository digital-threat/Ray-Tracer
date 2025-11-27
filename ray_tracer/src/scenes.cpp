#include <components/name.h>
#include <components/renderer.h>
#include <components/transform.h>
#include <mesh_manager.h>
#include <sandbox.h>
#include <texture_manager.h>

Scene Sandbox::MirrorScene()
{
	Scene scene(mEngine);

	scene.coordinator.RegisterComponent<Transform>();
	scene.coordinator.RegisterComponent<Renderer>();
	scene.coordinator.RegisterComponent<Name>();

	scene.mainLightColor = glm::vec3(1, 1, 1);
	scene.mainLightPosition = glm::vec3(5, 10, 5);
	scene.mainLightIntensity = 0.5f;
	scene.skyColor = glm::vec4(0.27f, 0.69f, 0.86f, 1.0f);
	scene.skyTextureIndex = -1;

	std::string skyTextureName = "pink_sunset_equirect.png";
	TextureManager::Instance().LoadTexture("models", skyTextureName);
	scene.skyTextureIndex = TextureManager::Instance().GetTextureCount() - 1;

	MeshManager& meshManager = MeshManager::Instance();
	MeshHandle teapot = meshManager.LoadMesh("models\\teapot\\teapot.obj");
	MeshHandle cube = meshManager.LoadMesh("models\\mirror\\mirror.obj");
	MeshHandle plane = meshManager.LoadMesh("models\\plane\\plane.obj");
	scene.meshes.push_back(teapot);
	scene.meshes.push_back(cube);
	scene.meshes.push_back(plane);

	{
		Entity entity = scene.coordinator.CreateEntity();

		Name name;
		name.name = "Teapot";
		scene.coordinator.AddComponent<Name>(entity, name);

		Transform transform;
		transform.position = glm::vec3(0.0f, 0.5f, 0.0f);
		transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		transform.scale = 0.015f;
		scene.coordinator.AddComponent<Transform>(entity, transform);

		Renderer renderer;
		renderer.meshHandle = teapot;
		scene.coordinator.AddComponent<Renderer>(entity, renderer);
	}

	{
		Entity entity = scene.coordinator.CreateEntity();

		Name name;
		name.name = "Plane";
		scene.coordinator.AddComponent<Name>(entity, name);

		Transform transform;
		transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
		transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		transform.scale = 5.0f;
		scene.coordinator.AddComponent<Transform>(entity, transform);

		Renderer renderer;
		renderer.meshHandle = plane;
		scene.coordinator.AddComponent<Renderer>(entity, renderer);
	}

	{
		Entity entity = scene.coordinator.CreateEntity();

		Name name;
		name.name = "Mirror";
		scene.coordinator.AddComponent<Name>(entity, name);

		Transform transform;
		transform.position = glm::vec3(0.0f, 2.0f, 3.0f);
		transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		transform.scale = 1.0f;
		scene.coordinator.AddComponent<Transform>(entity, transform);

		Renderer renderer;
		renderer.meshHandle = cube;
		scene.coordinator.AddComponent<Renderer>(entity, renderer);
	}

	{
		Entity entity = scene.coordinator.CreateEntity();

		Name name;
		name.name = "Mirror";
		scene.coordinator.AddComponent<Name>(entity, name);

		Transform transform;
		transform.position = glm::vec3(0.0f, 2.0f, -3.0f);
		transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		transform.scale = 1.0f;
		scene.coordinator.AddComponent<Transform>(entity, transform);

		Renderer renderer;
		renderer.meshHandle = cube;
		scene.coordinator.AddComponent<Renderer>(entity, renderer);
	}

	scene.CreateBlas();
	scene.CreateTlas();

	return scene;
}

Scene Sandbox::SponzaScene()
{
	Scene scene(mEngine);

	scene.coordinator.RegisterComponent<Transform>();
	scene.coordinator.RegisterComponent<Renderer>();
	scene.coordinator.RegisterComponent<Name>();

	scene.mainLightColor = glm::vec3(1, 1, 1);
	scene.mainLightPosition = glm::vec3(5, 25, 5);
	scene.mainLightIntensity = 0.5f;
	scene.skyColor = glm::vec4(0.27f, 0.69f, 0.86f, 1.0f);
	scene.skyTextureIndex = -1;

	std::string skyTextureName = "blue_sunset_equirect.png";
	TextureManager::Instance().LoadTexture("models", skyTextureName);
	scene.skyTextureIndex = TextureManager::Instance().GetTextureCount() - 1;

	MeshManager& meshManager = MeshManager::Instance();
	MeshHandle sponza = meshManager.LoadMesh("models\\crytek_sponza\\sponza.obj");
	scene.meshes.push_back(sponza);

	{
		Entity entity = scene.coordinator.CreateEntity();

		Name name;
		name.name = "Sponza";
		scene.coordinator.AddComponent<Name>(entity, name);

		Transform transform;
		transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
		transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		transform.scale = 0.1f;
		scene.coordinator.AddComponent<Transform>(entity, transform);

		Renderer renderer;
		renderer.meshHandle = sponza;
		scene.coordinator.AddComponent<Renderer>(entity, renderer);
	}

	scene.CreateBlas();
	scene.CreateTlas();

	return scene;
}
