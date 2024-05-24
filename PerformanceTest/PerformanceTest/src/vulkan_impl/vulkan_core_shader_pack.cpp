#include "vulkan_core.h"
#include "vulkan_utils.h"


namespace ec {

	void VulkanShaderPack::create(const VulkanContext& context, const std::filesystem::path& vertexFilePath, const std::filesystem::path& fragmentFilePath) {

		m_vertexShader.create(context, vertexFilePath, VK_SHADER_STAGE_VERTEX_BIT);
		m_fragmentShader.create(context, fragmentFilePath,VK_SHADER_STAGE_FRAGMENT_BIT);

		createDescriptorSetLayouts(context);

	}

	void VulkanShaderPack::destroy(const VulkanContext& context) {

		m_vertexShader.destroy(context);

		m_fragmentShader.destroy(context);

		for (VkDescriptorSetLayout layout : m_layouts) {
			vkDestroyDescriptorSetLayout(context.getData().device, layout, nullptr);
		}
		m_layouts.clear();
	}

	const VulkanShaderModule& VulkanShaderPack::getVertexShader() const
	{
		return m_vertexShader;
	}

	const VulkanShaderModule& VulkanShaderPack::getFragementShader() const
	{
		return m_fragmentShader;
	}

	const std::vector<VkDescriptorSetLayout>& VulkanShaderPack::getLayouts() const
	{
		return m_layouts;
	}

	void VulkanShaderPack::createDescriptorSetLayouts(const VulkanContext& context)
	{

		std::vector<VulkanShaderResource> resources;
		resources.insert(resources.end(), m_vertexShader.getResources().begin(), m_vertexShader.getResources().end());
		resources.insert(resources.end(), m_fragmentShader.getResources().begin(), m_fragmentShader.getResources().end());

		std::vector<std::vector<VkDescriptorSetLayoutBinding>> setBindings;

		setBindings.resize(std::max(m_vertexShader.getDescriptorSetCount(), m_fragmentShader.getDescriptorSetCount()) + 1);

		for (const VulkanShaderResource& resource : resources) {

			VkDescriptorSetLayoutBinding binding = {};

			binding.binding = resource.binding;
			binding.descriptorCount = 1;
			binding.stageFlags = resource.shaderStage;

			switch (resource.type) {
				case VulkanShaderResourceType::UNIFROM_BUFFER:
					binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					break;
				case VulkanShaderResourceType::IMAGE_SAMPLER:
					binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					break;
				case VulkanShaderResourceType::DYNAMIC_UNIFORM_BUFFER:
					binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
					break;
				default:
					EC_ERROR("Descriptor type not implemented");
					assert(false);
			}

			binding.pImmutableSamplers = nullptr;

			setBindings[resource.set].push_back(binding);

		}

		uint32_t setIndex = 0;

		m_layouts.resize(setBindings.size());

		for (std::vector<VkDescriptorSetLayoutBinding>& bindings : setBindings) {

			VkDescriptorSetLayoutCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
			createInfo.bindingCount = (uint32_t) bindings.size();
			createInfo.pBindings = bindings.data();

			vkCreateDescriptorSetLayout(context.getData().device, &createInfo, nullptr, &m_layouts[setIndex]);

			setIndex++;
		}

	}
}