#include "vulkan_utils.h"
#include <GLFW/glfw3.h>

namespace ec {
	VkSampler createSampler(const VulkanContext& context)
	{
		
		VkSamplerCreateInfo samplerCreateInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.mipLodBias = 0;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 1.0f;

		VkSampler sampler = {};

		VKA(vkCreateSampler(context.getData().device, &samplerCreateInfo, nullptr, &sampler));

		return sampler;

	}
	VkDescriptorPool createDesciptorPool(const VulkanContext& context, uint32_t maxSets, const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t flags) {

		VkDescriptorPool result = 0;

		VkDescriptorPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		createInfo.pPoolSizes = poolSizes.data();
		createInfo.poolSizeCount = (uint32_t)poolSizes.size();
		createInfo.maxSets = maxSets;
		createInfo.flags = (VkDescriptorPoolCreateFlags)flags;

		VKA(vkCreateDescriptorPool(context.getData().device, &createInfo, nullptr, &result));

		return result;
	}

	 VkDescriptorSetLayout createSetLayout(const VulkanContext& context, const std::vector<VkDescriptorSetLayoutBinding>& bindings) {

		VkDescriptorSetLayout result;

		VkDescriptorSetLayoutCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		createInfo.pBindings = bindings.data();
		createInfo.bindingCount = (uint32_t)bindings.size();

		VKA(vkCreateDescriptorSetLayout(context.getData().device, &createInfo, nullptr, &result));

		return result;
	}

	 VkDescriptorSet allocateDescriptorSet(const VulkanContext& context, VkDescriptorPool pool, VkDescriptorSetLayout layout) {

		VkDescriptorSet result;

		VkDescriptorSetAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocateInfo.descriptorPool = pool;
		allocateInfo.descriptorSetCount = 1;
		allocateInfo.pSetLayouts = &layout;

		VKA(vkAllocateDescriptorSets(context.getData().device, &allocateInfo, &result));

		return result;
	}

	 void writeDescriptorUniformBuffer(const VulkanContext& context, VkDescriptorSet descriptorSet, uint32_t binding, VulkanBuffer& buffer, bool dynamic, uint32_t offset, uint32_t range) {

		VkDescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = buffer.getBuffer();
		bufferInfo.offset = offset;
		bufferInfo.range = dynamic ? range : buffer.getSize(context);

		VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.descriptorCount = 1;
		write.descriptorType = dynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.dstBinding = binding;
		write.dstSet = descriptorSet;
		write.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(context.getData().device, 1, &write, 0, nullptr);

	}

	 void writeCombinedImageSampler(const VulkanContext& context, VkDescriptorSet descriptorSet, uint32_t binding, const VulkanImage& image, VkSampler sampler) {

		VkDescriptorImageInfo imageInfo;
		imageInfo.imageView = image.getImageView();
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.sampler = sampler;

		VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.dstBinding = binding;
		write.dstSet = descriptorSet;
		write.pImageInfo = &imageInfo;


		vkUpdateDescriptorSets(context.getData().device, 1, &write, 0, nullptr);

	}

	 uint32_t getFormatSize(VkFormat format) {
		switch (format) {
		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_B8G8R8A8_UNORM:
		case VK_FORMAT_R8G8B8A8_SNORM:
		case VK_FORMAT_B8G8R8A8_SNORM:
		case VK_FORMAT_R8G8B8A8_USCALED:
		case VK_FORMAT_B8G8R8A8_USCALED:
		case VK_FORMAT_R8G8B8A8_SSCALED:
		case VK_FORMAT_B8G8R8A8_SSCALED:
		case VK_FORMAT_R8G8B8A8_UINT:
		case VK_FORMAT_B8G8R8A8_UINT:
		case VK_FORMAT_R8G8B8A8_SINT:
		case VK_FORMAT_B8G8R8A8_SINT:
		case VK_FORMAT_R8G8B8A8_SRGB:
		case VK_FORMAT_B8G8R8A8_SRGB:
			return 4;
		case VK_FORMAT_R32G32B32A32_SFLOAT:
		case VK_FORMAT_R32G32B32A32_UINT:
		case VK_FORMAT_R32G32B32A32_SINT:
			return 16;
		case VK_FORMAT_R16G16B16A16_SFLOAT:
		case VK_FORMAT_R16G16B16A16_UNORM:
		case VK_FORMAT_R16G16B16A16_SNORM:
		case VK_FORMAT_R16G16B16A16_USCALED:
		case VK_FORMAT_R16G16B16A16_SSCALED:
		case VK_FORMAT_R16G16B16A16_UINT:
		case VK_FORMAT_R16G16B16A16_SINT:
		case VK_FORMAT_R32G32_SFLOAT:
			return 8;
		case VK_FORMAT_R8G8B8_UNORM:
		case VK_FORMAT_B8G8R8_UNORM:
		case VK_FORMAT_R8G8B8_SNORM:
		case VK_FORMAT_B8G8R8_SNORM:
		case VK_FORMAT_R8G8B8_USCALED:
		case VK_FORMAT_B8G8R8_USCALED:
		case VK_FORMAT_R8G8B8_SSCALED:
		case VK_FORMAT_B8G8R8_SSCALED:
		case VK_FORMAT_R8G8B8_UINT:
		case VK_FORMAT_B8G8R8_UINT:
		case VK_FORMAT_R8G8B8_SINT:
		case VK_FORMAT_B8G8R8_SINT:
		case VK_FORMAT_R8G8B8_SRGB:
		case VK_FORMAT_B8G8R8_SRGB:
			return 3;
		case VK_FORMAT_R8_UNORM:
		case VK_FORMAT_R8_SNORM:
		case VK_FORMAT_R8_USCALED:
		case VK_FORMAT_R8_SSCALED:
		case VK_FORMAT_R8_UINT:
		case VK_FORMAT_R8_SINT:
		case VK_FORMAT_R8_SRGB:
			return 1;
		case VK_FORMAT_R32G32B32_SFLOAT:
		case VK_FORMAT_R32G32B32_UINT:
		case VK_FORMAT_R32G32B32_SINT:
			return 12;
		default:
			assert(false);
			return 0;
		}
	}

	 VkSampleCountFlagBits getSampleCount(uint8_t sampleCount)
	{

		switch (sampleCount) {
		case 1: return VK_SAMPLE_COUNT_1_BIT;
		case 2: return VK_SAMPLE_COUNT_2_BIT;
		case 4: return VK_SAMPLE_COUNT_4_BIT;
		case 8: return VK_SAMPLE_COUNT_8_BIT;
		case 16: return VK_SAMPLE_COUNT_16_BIT;
		case 32: return VK_SAMPLE_COUNT_32_BIT;
		default:

			EC_ERROR("unknown sample count: {0}", sampleCount);
			assert(false);
			return VK_SAMPLE_COUNT_1_BIT;
		}
	}

	 VkAttachmentDescription createColorAttachment(uint32_t sampleCount)
	{
		VkAttachmentDescription result = {};
		result.format = VK_FORMAT_R8G8B8A8_UNORM;
		result.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		result.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		result.samples = getSampleCount(sampleCount);
		result.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		result.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		return result;
	}

	 VkAttachmentDescription createDepthAttachment(uint32_t sampleCount)
	{
		VkAttachmentDescription result = {};
		result.format = VK_FORMAT_D32_SFLOAT;
		result.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		result.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		result.samples = getSampleCount(sampleCount);
		result.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		result.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		return result;
	}

	VkAttachmentDescription createAttachment(uint32_t sampleCount, VkFormat format, VkImageLayout initialLayout, VkImageLayout finalLayout, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp)
	{
		VkAttachmentDescription result = {};
		result.format = format;
		result.finalLayout = finalLayout;
		result.initialLayout = initialLayout;
		result.samples = getSampleCount(sampleCount);
		result.loadOp = loadOp;
		result.storeOp = storeOp;
		return result;
	}

	VkSubpassDescription createSubpass(std::vector<VkAttachmentReference>& colorAttachments, std::vector<VkAttachmentReference>& resolveAttachments, std::vector<VkAttachmentReference>& inputAttachments, std::optional<VkAttachmentReference> depthStencilAttachment) {


		VkSubpassDescription description = {};
		description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		description.colorAttachmentCount = (uint32_t)colorAttachments.size();
		description.pColorAttachments = colorAttachments.data();

		description.pDepthStencilAttachment = depthStencilAttachment.has_value() ? &depthStencilAttachment.value() : nullptr;
		description.pResolveAttachments = resolveAttachments.data();

		description.inputAttachmentCount = (uint32_t)inputAttachments.size();
		description.pInputAttachments = inputAttachments.data();
		

		return description;

	}

	 VkCommandPool createCommandPool(const VulkanContext& context, uint32_t flags) {

		VkCommandPool result = {};

		VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		createInfo.queueFamilyIndex = context.getData().graphicsQueueFamilyIndex;
		createInfo.flags = flags;
		

		VKA(vkCreateCommandPool(context.getData().device, &createInfo, nullptr, &result));
		return result;

	}

	 VkCommandBuffer allocateCommandBuffer(const VulkanContext& context, VkCommandPool commandPool) {

		VkCommandBuffer result = {};

		VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		allocateInfo.commandPool = commandPool;
		allocateInfo.commandBufferCount = 1;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VKA(vkAllocateCommandBuffers(context.getData().device, &allocateInfo, &result));

		return result;

	}

	 void beginCommandBuffer(VkCommandBuffer buffer, uint32_t flags)
	 {

		 VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		 beginInfo.flags = flags;

		 vkBeginCommandBuffer(buffer, &beginInfo);

	 }

	 void endCommandBuffer(VkCommandBuffer buffer)
	 {
		 vkEndCommandBuffer(buffer);
	 }

	 VkFence createFence(const VulkanContext& context, bool signaled) {

		VkFence result = {};

		VkFenceCreateInfo createInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		if (signaled)
			createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		else
			createInfo.flags = 0;
		VKA(vkCreateFence(context.getData().device, &createInfo, nullptr, &result));
		return result;
	}

	 VkSemaphore createSemaphore(const VulkanContext& context) {

		VkSemaphore result = {};

		VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

		VKA(vkCreateSemaphore(context.getData().device, &createInfo, nullptr, &result));
		return result;
	}

	 VkSurfaceKHR createSurface(const VulkanContext& context, GLFWwindow* window) {

		 VkSurfaceKHR surface = {};
		VKA(glfwCreateWindowSurface(context.getData().instance, window, nullptr, &surface));
		return surface;

	}

	 void destroySurface(const VulkanContext& context, VkSurfaceKHR surface) {

		vkDestroySurfaceKHR(context.getData().instance, surface, nullptr);
	}


	 std::vector<const char*> getInstanceExtensions() {

		uint32_t glfwInstanceExtensionsCount = 0;
		glfwGetRequiredInstanceExtensions(&glfwInstanceExtensionsCount);

		const char** glfwInstanceExtensions = glfwGetRequiredInstanceExtensions(&glfwInstanceExtensionsCount);

		std::vector<const char*> enabledInstanceExtensions;
		enabledInstanceExtensions.resize(glfwInstanceExtensionsCount);

		for (uint32_t i = 0; i < glfwInstanceExtensionsCount; i++) {
			enabledInstanceExtensions[i] = glfwInstanceExtensions[i];
		}

		return enabledInstanceExtensions;

	}

}