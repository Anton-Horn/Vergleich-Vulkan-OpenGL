#include "vulkan_core.h"

#include "vulkan_utils.h"

namespace ec {
	int count = 0;
	void VulkanPipeline::create(const VulkanContext& context, VulkanPipelineCreateInfo& createInfo) {

		VkPipelineShaderStageCreateInfo shaderStages[2];

		m_shaders.create(context, createInfo.vertexShaderFilePath, createInfo.fragmentShaderFilePath);
		shaderStages[0] = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = m_shaders.getVertexShader().getModule();
		shaderStages[0].pName = "main";

		shaderStages[1] = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = m_shaders.getFragementShader().getModule();
		shaderStages[1].pName = "main";

		std::vector<VkVertexInputAttributeDescription> vertexAttributes;
		vertexAttributes.resize(createInfo.vertexLayout.size());

		uint32_t offset = 0;

		for (uint32_t i = 0; i < vertexAttributes.size(); i++) {
			vertexAttributes[i] = { i, 0, createInfo.vertexLayout[i], offset };
			offset += getFormatSize(createInfo.vertexLayout[i]);
		}

		VkVertexInputBindingDescription binding = { 0, offset, VK_VERTEX_INPUT_RATE_VERTEX };

		VkPipelineVertexInputStateCreateInfo vertexInputState = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		vertexInputState.vertexBindingDescriptionCount = createInfo.vertexLayout.size() ? 1 : 0;
		vertexInputState.pVertexBindingDescriptions = &binding;
		vertexInputState.vertexAttributeDescriptionCount = (uint32_t)vertexAttributes.size();
		vertexInputState.pVertexAttributeDescriptions = vertexAttributes.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineMultisampleStateCreateInfo multisampleState = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multisampleState.rasterizationSamples = getSampleCount(createInfo.sampleCount);

		VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		rasterizationState.lineWidth = 1.0f;

		VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		depthStencilState.depthTestEnable = createInfo.depthTestEnabled ? VK_TRUE : VK_FALSE;
		depthStencilState.depthWriteEnable = createInfo.depthTestEnabled ? VK_TRUE : VK_FALSE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.minDepthBounds = 0.0f;
		depthStencilState.maxDepthBounds = 1.0f;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &colorBlendAttachment;

		VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		dynamicState.dynamicStateCount = ARRAY_COUNT(dynamicStates);
		dynamicState.pDynamicStates = dynamicStates;

		VkPipelineLayoutCreateInfo layoutCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

		layoutCreateInfo.setLayoutCount = (uint32_t)m_shaders.getLayouts().size();
		layoutCreateInfo.pSetLayouts = m_shaders.getLayouts().data();

		vkCreatePipelineLayout(context.getData().device, &layoutCreateInfo, nullptr, &m_pipelineLayout);

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		pipelineCreateInfo.stageCount = ARRAY_COUNT(shaderStages);

		pipelineCreateInfo.pStages = shaderStages;
		pipelineCreateInfo.pVertexInputState = &vertexInputState;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.layout = m_pipelineLayout;
		pipelineCreateInfo.renderPass = createInfo.renderpass->getRenderpass();
		pipelineCreateInfo.subpass = createInfo.subpassIndex;

		vkCreateGraphicsPipelines(context.getData().device, 0, 1, &pipelineCreateInfo, nullptr, &m_pipeline);

	}

	

	void VulkanPipeline::destroy(const VulkanContext& context)
	{

		m_shaders.destroy(context);
		vkDestroyPipeline(context.getData().device, m_pipeline, nullptr);
		vkDestroyPipelineLayout(context.getData().device, m_pipelineLayout, nullptr);

	}

	const VkPipelineLayout VulkanPipeline::getLayout() const
	{
		return m_pipelineLayout;
	}

	const VulkanShaderPack& VulkanPipeline::getShaders() const
	{
		return m_shaders;
	}

	const VkPipeline VulkanPipeline::getPipeline() const
	{
		return m_pipeline;
	}

}