#include "vulkan_core.h"

#include "vulkan_utils.h"

namespace ec {

	void VulkanBuffer::create(const VulkanContext& context, uint64_t size, VkBufferUsageFlags usage, MemoryType type)
	{

		m_size = size;

		VkBufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		createInfo.size = size;
		createInfo.usage = usage;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocationCreateInfo = {};

		if (type == MemoryType::Auto) {
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		}
		else if (type == MemoryType::Device_local) {
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		}
		else if (type == MemoryType::Host_local) {
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		}

		allocationCreateInfo.flags = 0;

		VKA(vmaCreateBuffer(context.getData().allocator, &createInfo, &allocationCreateInfo, &m_buffer, &m_allocation, nullptr));


		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(context.getData().allocator, m_allocation, &allocationInfo);

		VkMemoryType vkMemoryType = context.getData().deviceMemoryProperties.memoryTypes[allocationInfo.memoryType];

		if (!(vkMemoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {

			VkBufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			createInfo.size = size;
			createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

			VmaAllocationCreateInfo allocationCreateInfo = {};
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

			//Create Buffer
			VKA(vmaCreateBuffer(context.getData().allocator, &createInfo, &allocationCreateInfo, &m_stagingBuffer, &m_stagingAllocation, nullptr));

		}

	}

	void VulkanBuffer::destroy(const VulkanContext& context)
	{

		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(context.getData().allocator, m_allocation, &allocationInfo);

		VkMemoryType vkMemoryType = context.getData().deviceMemoryProperties.memoryTypes[allocationInfo.memoryType];

		if (!(vkMemoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
			vmaDestroyBuffer(context.getData().allocator, m_stagingBuffer, m_stagingAllocation);
		}

		vmaDestroyBuffer(context.getData().allocator, m_buffer, m_allocation);

		
	}

	void VulkanBuffer::uploadData(const VulkanContext& context, void* data, uint32_t size, uint32_t offset)
	{

		assert(offset + size <= getSize(context));

		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(context.getData().allocator, m_allocation, &allocationInfo);

		VkMemoryType type = context.getData().deviceMemoryProperties.memoryTypes[allocationInfo.memoryType];

		if (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {

			void* dstPointer = 0;
			VKA(vmaMapMemory(context.getData().allocator, m_allocation, &dstPointer));
			dstPointer = ((uint8_t*)dstPointer) + offset;
			memcpy(dstPointer, data, size);
			vmaUnmapMemory(context.getData().allocator, m_allocation);

		}
		else {

			//Staging Buffer

			VkBufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			createInfo.size = size;
			createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

			VmaAllocationCreateInfo allocationCreateInfo = {};
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

			VkBuffer stagingBuffer = {};
			VmaAllocation stagingAllocation = {};

			//Create Buffer
			VKA(vmaCreateBuffer(context.getData().allocator, &createInfo, &allocationCreateInfo, &stagingBuffer, &stagingAllocation, nullptr));

			VmaAllocationInfo stagingInfo;
			vmaGetAllocationInfo(context.getData().allocator, stagingAllocation, &stagingInfo);

			//Copy data into staging Buffer
			void* dstPointer = nullptr;
			VKA(vmaMapMemory(context.getData().allocator, stagingAllocation, &dstPointer));
			memcpy(dstPointer, data, size);
			vmaUnmapMemory(context.getData().allocator, stagingAllocation);

			//Create Command buffers
			
			VkCommandPool transferCommandPool = {};
			VkCommandPoolCreateInfo commandPoolCreateInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
			commandPoolCreateInfo.queueFamilyIndex = context.getData().transferQueueFamilyIndex;
			commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
			VKA(vkCreateCommandPool(context.getData().device, &commandPoolCreateInfo, nullptr, &transferCommandPool));

			VkCommandBuffer transferCommandBuffer = {};
			VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
			allocateInfo.commandBufferCount = 1;
			allocateInfo.commandPool = transferCommandPool;
			allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

			VKA(vkAllocateCommandBuffers(context.getData().device, &allocateInfo, &transferCommandBuffer));

			VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(transferCommandBuffer, &beginInfo);

			VkBufferCopy copy = {};
			copy.dstOffset = offset;
			copy.srcOffset = 0;
			copy.size = size;
			vkCmdCopyBuffer(transferCommandBuffer, stagingBuffer, m_buffer, 1, &copy);

			vkEndCommandBuffer(transferCommandBuffer);

			VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &transferCommandBuffer;

			VkSemaphore signalSemaphore = nullptr;

			vkQueueSubmit(context.getData().transferQueue, 1, &submitInfo, 0);

			vkQueueWaitIdle(context.getData().transferQueue);

			vkDestroyCommandPool(context.getData().device, transferCommandPool, nullptr);
			vmaDestroyBuffer(context.getData().allocator, stagingBuffer, stagingAllocation);
			
			

		}
	}

	void VulkanBuffer::uploadFullData(const VulkanContext& context, VkCommandBuffer commandBuffer, void* data)
	{

		assert(offset + size <= getSize(context));

		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(context.getData().allocator, m_allocation, &allocationInfo);

		VkMemoryType type = context.getData().deviceMemoryProperties.memoryTypes[allocationInfo.memoryType];

		if (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {

			void* dstPointer = 0;
			VKA(vmaMapMemory(context.getData().allocator, m_allocation, &dstPointer));
			dstPointer = ((uint8_t*)dstPointer);
			memcpy(dstPointer, data, m_size);
			vmaUnmapMemory(context.getData().allocator, m_allocation);

		}
		else {

			VmaAllocationInfo stagingInfo;
			vmaGetAllocationInfo(context.getData().allocator, m_stagingAllocation, &stagingInfo);

			void* dstPointer = nullptr;
			VKA(vmaMapMemory(context.getData().allocator, m_stagingAllocation, &dstPointer));
			memcpy(dstPointer, data, m_size);
			vmaUnmapMemory(context.getData().allocator, m_stagingAllocation);

			VkBufferCopy copy = {};
			copy.dstOffset = 0;
			copy.srcOffset = 0;
			copy.size = m_size;
			vkCmdCopyBuffer(commandBuffer, m_stagingBuffer, m_buffer, 1, &copy);

			VkMemoryBarrier memoryBarrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER };
			memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			memoryBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 1, &memoryBarrier, 0, 0, 0, 0);

		}

	}

	uint64_t VulkanBuffer::getSize(const VulkanContext& context) const
	{

		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(context.getData().allocator, m_allocation, &allocationInfo);

		return allocationInfo.size;
	}

	const VkBuffer VulkanBuffer::getBuffer() const
	{
		return m_buffer;
	}

}