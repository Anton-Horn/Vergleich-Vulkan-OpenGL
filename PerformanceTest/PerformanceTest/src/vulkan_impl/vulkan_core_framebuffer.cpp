#include "vulkan_core.h"

namespace ec {

	void VulkanFramebuffer::create(const VulkanContext& context, VulkanRenderpass& renderpass, const std::vector<const VulkanImage*>& images) {

		assert(images.size());

		std::vector<VkImageView> imageViews;
		imageViews.resize(images.size());

		for (uint32_t i = 0; i < images.size(); i++) {
			imageViews[i] = images[i]->getImageView();
		}

		VkFramebufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		createInfo.width = images[0]->getWidth();
		createInfo.height = images[0]->getHeight();
		createInfo.renderPass = renderpass.getRenderpass();
		createInfo.attachmentCount = (uint32_t) imageViews.size();
		createInfo.pAttachments = imageViews.data();
		createInfo.layers = 1;

		vkCreateFramebuffer(context.getData().device, &createInfo, nullptr, &m_framebuffer);

	}

	void VulkanFramebuffer::destroy(const VulkanContext& context)
	{
		vkDestroyFramebuffer(context.getData().device, m_framebuffer, nullptr);
	}

	const VkFramebuffer VulkanFramebuffer::getFramebuffer() const
	{
		return m_framebuffer;
	}

}
