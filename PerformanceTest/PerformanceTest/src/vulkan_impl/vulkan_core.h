#pragma once
#include <filesystem>
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <assert.h>
#include <iostream>
#include "vk_mem_alloc.h"

#ifdef NDEBUG 

#define VKA(x) (x);

#else

#define VKA(x) assert((x) == VK_SUCCESS);

#endif





#define ARRAY_COUNT(x) (sizeof(x) / sizeof((x)[0]))

#define EC_ERROR(x) std::cout << x;
#define EC_INFO(x) std::cout << x;
#define EC_ABORT(x) {std::cout << x; __debugbreak();}

struct SpvReflectDescriptorBinding;
struct GLFWwindow;

namespace ec {

	uint32_t alignToPow2(uint32_t alignment, uint32_t value);

	struct VulkanContextData {

		VkDevice device;
		VkPhysicalDevice physicalDevice;
		VkPhysicalDeviceProperties deviceProperties;
		VkInstance instance;
		VkQueue graphicsQueue;
		uint32_t graphicsQueueFamilyIndex = 0;
		VkDebugUtilsMessengerEXT debugCallback;
		VmaAllocator allocator;
		VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
		VkDescriptorPool generalDescriptorPool;

		VkQueue transferQueue;
		uint32_t transferQueueFamilyIndex = 0;

	};

	class VulkanContext {

	public:

		VulkanContext() = default;
		~VulkanContext() = default;

		VulkanContext(const VulkanContext&) = delete;
		VulkanContext& operator=(const VulkanContext&) = delete;

		VulkanContext(const VulkanContext&&) = delete;
		VulkanContext& operator=(const VulkanContext&&) = delete;

		void createDefaultVulkanContext(const std::string& applicationName, std::vector<const char*>& additionalWindowInstanceExtensions);
		void create(const std::string& applicationName, const std::vector<const char*>& layers, const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& deviceExtensions);
		void destroy();

		const VulkanContextData& getData() const;

	private:

		VulkanContextData m_data;

		void createInstance(const std::string& applicationName, const std::vector<const char*>& layers, const std::vector<const char*>& instanceExtensions);
		void createDevice(const std::vector<const char*>& deviceExtensions);
		void registerDebugCallback();

	};

	enum class MemoryType {

		Device_local,
		Auto,
		Host_local

	};

	class VulkanBuffer {

	public:

		VulkanBuffer() = default;
		~VulkanBuffer() = default;

		VulkanBuffer(const VulkanBuffer&) = delete;
		VulkanBuffer& operator=(const VulkanBuffer&) = delete;

		VulkanBuffer(const VulkanBuffer&&) = delete;
		VulkanBuffer& operator=(const VulkanBuffer&&) = delete;

		void create(const VulkanContext& context, uint64_t size, uint32_t usage, MemoryType type = MemoryType::Auto);
		void destroy(const VulkanContext& context);

		void uploadData(const VulkanContext& context, void* data, uint32_t size, uint32_t offset = 0);
		uint64_t getSize(const VulkanContext& context) const;
		const VkBuffer getBuffer() const;

	private:

		VkBuffer m_buffer;
		VmaAllocation m_allocation;
		VkBufferUsageFlags m_usage;

	};

	// The Uniform buffer needs to be at binding 0
	template<typename T>
	struct UniformBuffer {

	public:
		
		void create(const VulkanContext& context, MemoryType type = MemoryType::Auto, uint32_t count = 1) {

			alignedSize = alignToPow2((uint32_t)context.getData().deviceProperties.limits.minUniformBufferOffsetAlignment, sizeof(T));
			buffer.create(context, alignedSize * count, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, type);
		}

		void destroy(const VulkanContext& context) {

			buffer.destroy(context);

		}

		VulkanBuffer buffer;
		uint32_t alignedSize = 0;

	};

	class VulkanRenderpass {

	public:

		VulkanRenderpass() = default;
		~VulkanRenderpass() = default;

		VulkanRenderpass(const VulkanRenderpass&) = delete;
		VulkanRenderpass& operator=(const VulkanRenderpass&) = delete;

		VulkanRenderpass(const VulkanRenderpass&&) = delete;
		VulkanRenderpass& operator=(const VulkanRenderpass&&) = delete;

		void create(const VulkanContext& context, const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& dependencies = {});
		void destroy(const VulkanContext& context);

		const VkRenderPass getRenderpass() const;

	private:

		VkRenderPass m_renderpass;

	};


	class VulkanImage {

	public:

		VulkanImage() = default;
		~VulkanImage() = default;

		VulkanImage(const VulkanImage&) = delete;
		VulkanImage& operator=(const VulkanImage&) = delete;

		VulkanImage(const VulkanImage&&) {};
		VulkanImage& operator=(const VulkanImage&&) {};

		void create(const VulkanContext& context, uint32_t width, uint32_t height, uint32_t format = VK_FORMAT_R8G8B8A8_UNORM, uint32_t usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, uint32_t sampleCount = 1);
		void create(VkImage image, VkImageView view, uint32_t width, uint32_t height);
		void destroy(const VulkanContext& context);
		void uploadData(const VulkanContext& context, void* data, uint32_t width, uint32_t height, uint32_t bytePerPixel, uint32_t layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		// bad performance, uses own queue submit and waits for it being finished!
		void switchLayout(const VulkanContext& context, uint32_t newLayout);

		const VkImage getImage() const;
		const VkImageView getImageView() const;
		const uint32_t getWidth() const;
		const uint32_t getHeight() const;

	private:

		VkImage m_image;
		VkImageView m_imageView;
		VmaAllocation m_allocation;
		uint32_t m_imageWidth, m_imageHeight;
		bool m_mutable = false;
		VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;

	};

	class VulkanFramebuffer {

	public:

		VulkanFramebuffer() = default;
		~VulkanFramebuffer() = default;

		VulkanFramebuffer(const VulkanFramebuffer&) = delete;
		VulkanFramebuffer& operator=(const VulkanFramebuffer&) = delete;

		VulkanFramebuffer(const VulkanFramebuffer&&) noexcept {};
		VulkanFramebuffer& operator=(const VulkanFramebuffer&&) noexcept {};

		void create(const VulkanContext& context, VulkanRenderpass& renderpass, const std::vector<const VulkanImage*>& images);
		void destroy(const VulkanContext& context);

		const VkFramebuffer getFramebuffer() const;

	private:

		VkFramebuffer m_framebuffer;

	};

	class VulkanSwapchain {

	public:

		VulkanSwapchain() = default;
		~VulkanSwapchain() = default;

		VulkanSwapchain(const VulkanSwapchain&) = delete;
		VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;

		VulkanSwapchain(const VulkanSwapchain&&) = delete;
		VulkanSwapchain& operator=(const VulkanSwapchain&&) = delete;

		void create(const VulkanContext& context, VkSurfaceKHR surface);
		void recreate(const VulkanContext& context, VkSurfaceKHR surface);
		void destroy(const VulkanContext& context);

		
		void aquireNextImage(const VulkanContext& context, VkSemaphore signalSemaphore, bool& recreateSwapchain);
		void aquireNextImage(const VulkanContext& context, bool& recreateSwapchain);
		
		void present(const VulkanContext& context, const std::vector<VkSemaphore>& waitSemaphores, bool& recreateSwapchain);

		uint32_t getWidth() const;
		uint32_t getHeight() const;
		uint32_t getCurrentIndex() const;

		const VulkanRenderpass& getRenderpass() const;
		const std::vector<VulkanFramebuffer>& getFramebuffers() const;

		VkFormat getFormat() const;
		const std::vector<VulkanImage>& getImages() const;
		const VkSwapchainKHR getSwapchain() const;

	private:

		VkSwapchainKHR m_swapchain;

		std::vector<VulkanImage> m_images;

		// should be used for last pass into the swapchain
		std::vector<VulkanFramebuffer> m_framebuffers;
		VulkanRenderpass m_renderpass;

		uint32_t m_swapchainWidth;
		uint32_t m_swapchainHeight;
		VkFormat m_format;
		uint32_t m_currentSwapchainImageIndex;

	};

	

	enum class VulkanShaderResourceType {

		UNIFROM_BUFFER,
		DYNAMIC_UNIFORM_BUFFER,
		IMAGE_SAMPLER

	};

	struct VulkanShaderResource {

		VulkanShaderResourceType type;
		uint32_t set;
		uint32_t binding;
		VkShaderStageFlags shaderStage;

	};

	class VulkanShaderModule {

	public:

		VulkanShaderModule() = default;
		~VulkanShaderModule();

		VulkanShaderModule(const VulkanShaderModule&) = delete;
		VulkanShaderModule& operator=(const VulkanShaderModule&) = delete;

		VulkanShaderModule(const VulkanShaderModule&&) = delete;
		VulkanShaderModule& operator=(const VulkanShaderModule&&) = delete;

		void create(const VulkanContext& context, const std::filesystem::path& filePath, VkShaderStageFlags shaderStage);
		void destroy(const VulkanContext& context);

		const VkShaderModule getModule() const;
		const std::vector<VulkanShaderResource>& getResources() const;
		uint32_t getDescriptorSetCount();

	private:

		std::vector<VulkanShaderResource> m_resources;
		VkShaderModule m_module;
		uint32_t m_setCount;

	};

	class VulkanShaderPack {

	public:

		VulkanShaderPack() = default;
		~VulkanShaderPack() = default;

		VulkanShaderPack(const VulkanShaderPack&) = delete;
		VulkanShaderPack& operator=(const VulkanShaderPack&) = delete;

		VulkanShaderPack(const VulkanShaderPack&&) = delete;
		VulkanShaderPack& operator=(const VulkanShaderPack&&) = delete;

		void create(const VulkanContext& context, const std::filesystem::path& vertexFilePath, const std::filesystem::path& fragmentFilePath);
		void destroy(const VulkanContext& context);

		const VulkanShaderModule& getVertexShader() const;
		const VulkanShaderModule& getFragementShader() const;
		const std::vector<VkDescriptorSetLayout>& getLayouts() const;

	private:

		VulkanShaderModule m_vertexShader;
		VulkanShaderModule m_fragmentShader;
		std::vector<VkDescriptorSetLayout> m_layouts;

		void createDescriptorSetLayouts(const VulkanContext& context);

	};

	struct VulkanPipelineCreateInfo {

		std::filesystem::path vertexShaderFilePath;
		std::filesystem::path fragmentShaderFilePath;

		VulkanRenderpass* renderpass;
		uint32_t subpassIndex;

		std::vector<VkFormat> vertexLayout;

		bool depthTestEnabled = false;
		uint8_t sampleCount = 1;

	};

	class VulkanMaterialTemplate {

	public:

		VulkanMaterialTemplate() = default;
		~VulkanMaterialTemplate() = default;

		VulkanMaterialTemplate(const VulkanMaterialTemplate&) = delete;
		VulkanMaterialTemplate& operator=(const VulkanMaterialTemplate&) = delete;

		VulkanMaterialTemplate(const VulkanMaterialTemplate&&) = delete;
		VulkanMaterialTemplate& operator=(const VulkanMaterialTemplate&&) = delete;

		void create(const VulkanContext& context);
		void destroy(const VulkanContext& context);

	};

	class VulkanPipeline {

	public:

		VulkanPipeline() = default;
		~VulkanPipeline() = default;

		VulkanPipeline(const VulkanPipeline&) = delete;
		VulkanPipeline& operator=(const VulkanPipeline&) = delete;

		VulkanPipeline(const VulkanPipeline&&) = delete;
		VulkanPipeline& operator=(const VulkanPipeline&&) = delete;


		void create(const VulkanContext& context, VulkanPipelineCreateInfo& createInfo);
		void destroy(const VulkanContext& context);

		const VkPipelineLayout getLayout() const;
		const VulkanShaderPack& getShaders() const;
		const VkPipeline getPipeline() const;

	private:

		VkPipeline m_pipeline;
		VkPipelineLayout m_pipelineLayout;
		VulkanShaderPack m_shaders;

	};

	class Window;

	struct VulkanWindow {

		GLFWwindow* window;
		VkSurfaceKHR surface;
		VulkanSwapchain swapchain;

	};

}

