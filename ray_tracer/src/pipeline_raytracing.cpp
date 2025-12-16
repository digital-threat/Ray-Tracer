#include <engine.h>
#include <glm/gtc/quaternion.hpp>
#include <sandbox.h>
#include <utility.h>
#include <vk_buffers.h>
#include <vk_pipelines.h>

struct RtPushConstants
{
	u32 samplesPerPixel;
	u32 frame;
};

void Sandbox::InitRt()
{
	VkPhysicalDeviceProperties2 properties2{};
	properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	properties2.pNext = &mRtProperties;
	vkGetPhysicalDeviceProperties2(mEngine.mPhysicalDevice, &properties2);
}

void Sandbox::InitRtDescriptorLayout()
{
	DescriptorLayoutBuilder builder;
	builder.AddBinding(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
					   VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
	builder.AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
	mRtDescriptorLayout = builder.Build(mEngine.mDevice);
}

void Sandbox::InitRtSceneDescriptorLayout()
{
	u32 textureCount = TextureManager::Instance().GetTextureCount();

	DescriptorLayoutBuilder builder;
	builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					   VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR);
	builder.AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR);
	builder.AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, textureCount,
					   VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR);
	mRtSceneDescriptorLayout = builder.Build(mEngine.mDevice);
}

// TODO(Sergei): Add "AddShaderStage" to pipeline builder
void Sandbox::InitRtPipeline()
{
	VkShaderModule raygenShader, missShader, closestHitShader, shadowMissShader, anyHitShader;
	LoadShaderModule("shaders/rt.rgen.spv", mEngine.mDevice, &raygenShader);
	LoadShaderModule("shaders/rt.rmiss.spv", mEngine.mDevice, &missShader);
	LoadShaderModule("shaders/rt.rchit.spv", mEngine.mDevice, &closestHitShader);
	LoadShaderModule("shaders/rt_shadow.rmiss.spv", mEngine.mDevice, &shadowMissShader);
	LoadShaderModule("shaders/rt.rahit.spv", mEngine.mDevice, &anyHitShader);

	std::array<VkPipelineShaderStageCreateInfo, 5> stages;
	VkPipelineShaderStageCreateInfo stage{};
	stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage.pName = "main";

	stage.module = raygenShader;
	stage.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	stages[0] = stage;

	stage.module = missShader;
	stage.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
	stages[1] = stage;

	stage.module = shadowMissShader;
	stage.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
	stages[2] = stage;

	stage.module = closestHitShader;
	stage.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	stages[3] = stage;

	stage.module = anyHitShader;
	stage.stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
	stages[4] = stage;

	VkRayTracingShaderGroupCreateInfoKHR group{};
	group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	group.anyHitShader = VK_SHADER_UNUSED_KHR;
	group.closestHitShader = VK_SHADER_UNUSED_KHR;
	group.generalShader = VK_SHADER_UNUSED_KHR;
	group.intersectionShader = VK_SHADER_UNUSED_KHR;

	group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	group.generalShader = 0;
	mRtShaderGroups.push_back(group);

	group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	group.generalShader = 1;
	mRtShaderGroups.push_back(group);

	group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	group.generalShader = 2;
	mRtShaderGroups.push_back(group);

	group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
	group.generalShader = VK_SHADER_UNUSED_KHR;
	group.closestHitShader = 3;
	group.anyHitShader = 4;
	mRtShaderGroups.push_back(group);

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;
	pushConstantRange.size = sizeof(RtPushConstants);

	std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {mRtDescriptorLayout, mRtSceneDescriptorLayout};

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<u32>(descriptorSetLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	vkCreatePipelineLayout(mEngine.mDevice, &pipelineLayoutCreateInfo, nullptr, &mRtPipelineLayout);

	VkRayTracingPipelineCreateInfoKHR pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	pipelineCreateInfo.stageCount = static_cast<u32>(stages.size());
	pipelineCreateInfo.pStages = stages.data();
	pipelineCreateInfo.groupCount = static_cast<u32>(mRtShaderGroups.size());
	pipelineCreateInfo.pGroups = mRtShaderGroups.data();
	pipelineCreateInfo.maxPipelineRayRecursionDepth = 2;
	pipelineCreateInfo.layout = mRtPipelineLayout;
	mEngine.mVkbDT.fp_vkCreateRayTracingPipelinesKHR(mEngine.mDevice, {}, {}, 1, &pipelineCreateInfo, nullptr, &mRtPipeline);

	if (mRtProperties.maxRayRecursionDepth <= 1)
	{
		throw std::runtime_error("Device does not support ray recursion (m_rtProperties.maxRayRecursionDepth <= 1).");
	}

	for (u32 i = 0; i < stages.size(); i++)
	{
		vkDestroyShaderModule(mEngine.mDevice, stages[i].module, nullptr);
	}
}

void Sandbox::InitRtSBT()
{
	u32 missCount = 2;
	u32 hitCount = 1;
	u32 handleCount = 1 + missCount + hitCount;
	u32 handleSize = mRtProperties.shaderGroupHandleSize;

	u32 handleSizeAligned = AlignUp(handleSize, mRtProperties.shaderGroupHandleAlignment);

	mRgenRegion.stride = AlignUp(handleSizeAligned, mRtProperties.shaderGroupBaseAlignment);
	mRgenRegion.size = mRgenRegion.stride;
	mMissRegion.stride = handleSizeAligned;
	mMissRegion.size = AlignUp(missCount * handleSizeAligned, mRtProperties.shaderGroupBaseAlignment);
	mHitRegion.stride = handleSizeAligned;
	mHitRegion.size = AlignUp(hitCount * handleSizeAligned, mRtProperties.shaderGroupBaseAlignment);
	mCallRegion.stride = 0;
	mCallRegion.size = 0;

	u32 dataSize = handleCount * handleSize;
	std::vector<u8> handles(dataSize);
	VkResult result = mEngine.mVkbDT.fp_vkGetRayTracingShaderGroupHandlesKHR(mEngine.mDevice, mRtPipeline, 0, handleCount, dataSize,
																			 handles.data());
	assert(result == VK_SUCCESS);

	VkDeviceSize sbtSize = mRgenRegion.size + mMissRegion.size + mHitRegion.size + mCallRegion.size;
	VkBufferUsageFlags usageFlags;
	usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	usageFlags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	usageFlags |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
	mRtSBTBuffer = CreateBuffer(mEngine.mAllocator, sbtSize, usageFlags, VMA_MEMORY_USAGE_CPU_ONLY);

	VkBufferDeviceAddressInfo info{};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	info.buffer = mRtSBTBuffer.buffer;

	VkDeviceAddress sbtAddress = vkGetBufferDeviceAddress(mEngine.mDevice, &info);
	mRgenRegion.deviceAddress = sbtAddress;
	mMissRegion.deviceAddress = sbtAddress + mRgenRegion.size;
	mHitRegion.deviceAddress = sbtAddress + mRgenRegion.size + mMissRegion.size;

	u8* sbtBuffer = reinterpret_cast<u8*>(mRtSBTBuffer.info.pMappedData);
	u32 handleId = 0;

	u8* dst = sbtBuffer;
	memcpy(dst, handles.data() + handleId * handleSize, handleSize);
	handleId++;

	dst = sbtBuffer + mRgenRegion.size;
	for (u32 i = 0; i < missCount; i++)
	{
		memcpy(dst, handles.data() + handleId * handleSize, handleSize);
		handleId++;
		dst += mMissRegion.stride;
	}

	dst = sbtBuffer + mRgenRegion.size + mMissRegion.size;
	for (u32 i = 0; i < hitCount; i++)
	{
		memcpy(dst, handles.data() + handleId * handleSize, handleSize);
		handleId++;
		dst += mHitRegion.stride;
	}
}

void Sandbox::UpdateRtSceneDescriptorSet(VkDescriptorSet sceneSet, FrameData& currentFrame)
{
	float aspect = static_cast<float>(mEngine.mRenderExtent.width) / static_cast<float>(mEngine.mRenderExtent.height);

	SceneRenderData sceneRenderData = mRenderContext.scene;
	glm::mat4 mainLightP = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 100.0f, 0.1f);
	glm::mat4 mainLightV = glm::lookAt(sceneRenderData.mainLightPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));

	CameraRenderData& camera = mRenderContext.camera;
	SceneData scene{};
	scene.matrixV = glm::lookAt(camera.pos, camera.pos + camera.forward, camera.up);
	scene.matrixP = glm::perspective(glm::radians(camera.fov), aspect, 100.0f, 0.1f);
	scene.matrixIV = glm::inverse(scene.matrixV);
	scene.matrixIP = glm::inverse(scene.matrixP);
	scene.matrixVP = scene.matrixP * scene.matrixV;
	scene.mainLightVP = mainLightP * mainLightV;
	scene.mainLightColor = sceneRenderData.mainLightColor;
	scene.mainLightDir = glm::normalize(glm::vec4(sceneRenderData.mainLightPos - glm::vec3(0.0f), 1.0f));
	scene.skyColor = glm::vec4(sceneRenderData.skyColor, 1.0f);
	scene.skyTextureId = sceneRenderData.skyTextureId;
	scene.lightBuffer = mRenderContext.light.lightBuffer;
	scene.lightCount = mRenderContext.light.lightCount;
	scene.cameraPos = glm::vec4(camera.pos, 0.0f);

	memcpy(currentFrame.sceneDataBuffer.info.pMappedData, &scene, sizeof(SceneData));

	// TODO(Sergei): Object data buffer shouldn't be updated every frame, only on scene load.
	u32 meshCount = MeshManager::Instance().mMeshes.size();
	std::vector<ObjectData> objects;
	objects.reserve(meshCount);

	for (u32 j = 0; j < meshCount; j++)
	{
		GpuMesh& mesh = MeshManager::Instance().mMeshes[j];

		ObjectData renderObject{};
		renderObject.textureOffset = mesh.textureOffset;
		renderObject.vertexBufferAddress = mesh.vertexBufferAddress;
		renderObject.indexBufferAddress = mesh.indexBufferAddress;
		renderObject.materialBufferAddress = mesh.materialBufferAddress;
		renderObject.matIdBufferAddress = mesh.matIdBufferAddress;

		objects.push_back(renderObject);
	}

	memcpy(currentFrame.objectDataBuffer.info.pMappedData, objects.data(), objects.size() * sizeof(ObjectData));

	// TODO(Sergei): Textures should be updated either on scene load or never.
	u32 textureCount = TextureManager::Instance().mTextures.size();
	std::vector<VkDescriptorImageInfo> imageInfos;
	for (u32 i = 0; i < textureCount; i++)
	{
		Texture& texture = TextureManager::Instance().mTextures[i];

		VkDescriptorImageInfo imageInfo{};
		imageInfo.sampler = texture.sampler;
		imageInfo.imageView = texture.image.imageView;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		imageInfos.emplace_back(imageInfo);
	}

	DescriptorWriter writer;
	writer.WriteBuffer(0, currentFrame.sceneDataBuffer.buffer, sizeof(SceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	writer.WriteBuffer(1, currentFrame.objectDataBuffer.buffer, objects.size() * sizeof(ObjectData), 0,
					   VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	writer.WriteImages(2, imageInfos.data(), textureCount, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	writer.UpdateSet(mEngine.mDevice, sceneSet);
}

void Sandbox::RenderRt(VkCommandBuffer cmd, FrameData& currentFrame)
{
	VkDescriptorSet rtSet = currentFrame.descriptorAllocator.Allocate(mEngine.mDevice, mRtDescriptorLayout);
	VkDescriptorSet sceneSet = currentFrame.descriptorAllocator.Allocate(mEngine.mDevice, mRtSceneDescriptorLayout);
	std::vector<VkDescriptorSet> descSets = {rtSet, sceneSet};

	VkWriteDescriptorSetAccelerationStructureKHR writeAS{};
	writeAS.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	writeAS.accelerationStructureCount = 1;
	writeAS.pAccelerationStructures = &mRenderContext.raytracing.tlas;

	DescriptorWriter writer;
	writer.WriteTlas(0, &writeAS);
	writer.WriteImage(1, mEngine.mColorTarget.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	writer.UpdateSet(mEngine.mDevice, rtSet);

	UpdateRtSceneDescriptorSet(sceneSet, currentFrame);

	// TODO(Sergei): Find a better place for this!
	{
		static glm::mat4 oldMatrixV;
		static float oldFov = 0;

		CameraRenderData& camera = mRenderContext.camera;
		glm::mat4 newMatrixV = glm::lookAt(camera.pos, camera.pos + camera.forward, camera.up);
		float newFov = camera.fov;

		if (oldMatrixV != newMatrixV || oldFov != newFov)
		{
			mRenderContext.frame = -1;
			oldMatrixV = newMatrixV;
			oldFov = newFov;
		}

		mRenderContext.frame++;
	}

	RtPushConstants pushConstants{};
	pushConstants.samplesPerPixel = static_cast<u32>(mRenderContext.samplesPerPixel);
	pushConstants.frame = mRenderContext.frame;

	u32 width = mEngine.mColorTarget.extent.width;
	u32 height = mEngine.mColorTarget.extent.height;

	VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
	VkShaderStageFlags stages = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;

	vkCmdBindPipeline(cmd, bindPoint, mRtPipeline);
	vkCmdBindDescriptorSets(cmd, bindPoint, mRtPipelineLayout, 0, static_cast<u32>(descSets.size()), descSets.data(), 0, nullptr);
	vkCmdPushConstants(cmd, mRtPipelineLayout, stages, 0, sizeof(RtPushConstants), &pushConstants);
	mEngine.mVkbDT.fp_vkCmdTraceRaysKHR(cmd, &mRgenRegion, &mMissRegion, &mHitRegion, &mCallRegion, width, height, 1);
}
