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
	MeshHandle sibenik = meshManager.LoadMesh("models\\sibenik\\sibenik.obj");
	scene.meshes.push_back(sibenik);

	{
		Entity entity = scene.coordinator.CreateEntity();

		Name name;
		name.name = "Sibenik";
		scene.coordinator.AddComponent<Name>(entity, name);

		Transform transform;
		transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
		transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		transform.scale = 0.1f;
		scene.coordinator.AddComponent<Transform>(entity, transform);

		Renderer renderer;
		renderer.meshHandle = sibenik;
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
