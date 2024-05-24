#include <iostream>
#include <numeric>
#include <assert.h>
#include <vulkan/vulkan.h>

#include <string>
#include <fstream>
#include <filesystem>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "vulkan_impl/vulkan_core.h"
#include "vulkan_impl/vulkan_utils.h"

#define VULKAN_TEST 1
//#define USE_PRE_RECOREDED_COMMAND_BUFFER 1

using namespace ec;

struct Mat4 {

    float data[16];

};

struct Vertex {
    float x, y, z;
    struct Color {
        float r, g, b, a;
    } color;

};

const uint32_t drawCount = 500;
const uint32_t rows = 100;
const uint32_t columns = 100;
const uint32_t vertex_count = rows * columns * 6;

std::vector<Vertex> getTestData() {
  
    const float spacing = 0.1f;

    std::vector<Vertex> vertices;
    vertices.reserve(vertex_count);

    float max_coord = std::max(rows, columns) * spacing;

    for (uint32_t row = 0; row < rows; row++) {
        for (uint32_t col = 0; col < columns; col++) {

            float x = (col * spacing / max_coord) * 2.0f - 1.0f;
            float y = (row * spacing / max_coord) * 2.0f - 1.0f;
            float z = 0.0f;

            float nextX = ((col + 1) * spacing / max_coord) * 2.0f - 1.0f;
            float nextY = ((row + 1) * spacing / max_coord) * 2.0f - 1.0f;

            Vertex::Color color = { x, y, 1.0f, 1.0f };

            vertices.push_back({ x, y, z , color });
            vertices.push_back({ x, nextY, z , color });
            vertices.push_back({ nextX, y, z , color });

            // Dreieck 2
            vertices.push_back({ nextX, y, z ,color });
            vertices.push_back({ x, nextY, z ,color });
            vertices.push_back({ nextX, nextY, z ,color });



        }
    }
    return vertices;
}

std::string load_shader_source(const std::filesystem::path& path)
{

    std::string content;

    std::ifstream in(path.string());

    if (in.is_open()) {

        std::string line;

        while (std::getline(in, line)) {
            content += line + "\n";
        }
        in.close();

    }
    else {
        assert(false);
    }
    return content;
}

uint32_t get_shader_program(const std::string& vertex_path, const std::string& fragment_path) {

    std::string vertex_source = load_shader_source(vertex_path);
    std::string fragment_source = load_shader_source(fragment_path);

    uint32_t program = glCreateProgram();

    GLuint VertexID = glCreateShader(GL_VERTEX_SHADER);
    const char* vs = vertex_source.c_str();
    glShaderSource(VertexID, 1, &vs, 0);
    glCompileShader(VertexID);

    int result;

    glGetShaderiv(VertexID, GL_COMPILE_STATUS, &result);

    if (result != GL_TRUE) {

        int length = 0;

        glGetShaderiv(VertexID, GL_INFO_LOG_LENGTH, &length);

        char* message = new char[length];

        glGetShaderInfoLog(VertexID, length, &length, message);

        std::cout << message;

        assert(false);

        delete[] message;

    }

    GLuint FragID = glCreateShader(GL_FRAGMENT_SHADER);

    const char* fs = fragment_source.c_str();
    glShaderSource(FragID, 1, &fs, 0);
    glCompileShader(FragID);

    glGetShaderiv(FragID, GL_COMPILE_STATUS, &result);

    if (result != GL_TRUE) {

        int length = 0;

        glGetShaderiv(FragID, GL_INFO_LOG_LENGTH, &length);

        char* message = new char[length];

        glGetShaderInfoLog(FragID, length, &length, message);

        assert(false);

        delete[] message;

    }

    glAttachShader(program, VertexID);
    glAttachShader(program, FragID);

    glLinkProgram(program);

    glDetachShader(program, VertexID);
    glDetachShader(program, FragID);

    glDeleteShader(VertexID);
    glDeleteShader(FragID);

    return program;

}

int opengl_test() {

    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    std::string title = "OpenGL - Drawing " + std::to_string(vertex_count) + " Vertices";

    window = glfwCreateWindow(640, 480, title.c_str(), NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return -1;

    uint32_t vb = 0;

    uint32_t program = get_shader_program("vertex.vert", "fragment.frag");

    glUseProgram(program);

    glGenBuffers(1, &vb);
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    
    std::vector<Vertex> vertices = getTestData();
 
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

    glEnableVertexAttribArray(1);
    uint32_t offset = sizeof(float) * 3;
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offset);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glClearColor(0.1, 0.1, 0.1, 1.0);

    uint32_t timeQuery;
    glGenQueries(1, &timeQuery);

    double averageGpuTime = 0.0;
    double lastFrameTime = 0.0f;

    glfwSwapInterval(0.0f);

    while (!glfwWindowShouldClose(window))
    {
        
        glClear(GL_COLOR_BUFFER_BIT);

        glBeginQuery(GL_TIME_ELAPSED, timeQuery);

        
        for (uint32_t i = 0; i < drawCount; i++) {

            glDrawArrays(GL_TRIANGLES, 0, vertices.size());

        }      

        glEndQuery(GL_TIME_ELAPSED);

        GLuint64 elapsed_time;
        glGetQueryObjectui64v(timeQuery, GL_QUERY_RESULT, &elapsed_time);
        averageGpuTime = 0.99f * averageGpuTime + 0.01f * (elapsed_time / 1000000.0);
        std::cout << "GPU Time: " << averageGpuTime << " ms" << std::endl;

        glfwSwapBuffers(window);

        double time = glfwGetTime();

        std::cout << "Frame Time: " << (time - lastFrameTime) * 1000 << "\n";

        lastFrameTime = glfwGetTime();

        glfwPollEvents();

    }

    glfwTerminate();
    return 0;

}

int vulkan_test() {

    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

    std::string title = "Vulkan - Drawing " + std::to_string(vertex_count) + " Vertices";

    window = glfwCreateWindow(640, 480, title.c_str(), NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    VulkanContext context;
    auto instanceExtensions = getInstanceExtensions();
    context.createDefaultVulkanContext("PerformanceTest", instanceExtensions);

    VulkanWindow vulkanWindow;
    vulkanWindow.window = window;
    vulkanWindow.surface = createSurface(context, window);
    vulkanWindow.swapchain.create(context, vulkanWindow.surface);

    VkSemaphore submitSemaphore = createSemaphore(context);
    VkSemaphore acquireSemaphore = createSemaphore(context);

    VkFence fence = createFence(context);

#ifdef USE_PRE_RECOREDED_COMMAND_BUFFER

    VkCommandPool commandPool = createCommandPool(context, 0);

    std::vector<VkCommandBuffer> commandBuffers;
    commandBuffers.resize(vulkanWindow.swapchain.getImages().size());

    for (VkCommandBuffer& commandBuffer : commandBuffers) {

        commandBuffer = allocateCommandBuffer(context, commandPool);

    }

#else

    VkCommandPool commandPool = createCommandPool(context);
    VkCommandBuffer commandBuffer = allocateCommandBuffer(context, commandPool);

#endif

    

    VulkanPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.subpassIndex = 0;
     
    pipelineCreateInfo.renderpass = (VulkanRenderpass*)&vulkanWindow.swapchain.getRenderpass();
    pipelineCreateInfo.depthTestEnabled = false;
    pipelineCreateInfo.sampleCount = 1;
    pipelineCreateInfo.vertexShaderFilePath = "vertex.spv";
    pipelineCreateInfo.fragmentShaderFilePath = "fragment.spv";
   
    pipelineCreateInfo.vertexLayout = { VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT };

    VulkanPipeline pipeline;
    pipeline.create(context, pipelineCreateInfo);

    std::vector<Vertex> vertices = getTestData();

    VulkanBuffer vertexBuffer;
    vertexBuffer.create(context, vertices.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MemoryType::Device_local);
    vertexBuffer.uploadData(context, vertices.data(), vertices.size() * sizeof(Vertex));

    VkQueryPool timestampQueryPool = {};

    VkQueryPoolCreateInfo queryPoolCreateInfo{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
    queryPoolCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
    queryPoolCreateInfo.queryCount = 64;
    VKA(vkCreateQueryPool(context.getData().device, &queryPoolCreateInfo, nullptr, &timestampQueryPool));

    double averageGpuTime = 0.0f;
    double lastFrameTime = 0.0f;

#ifdef USE_PRE_RECOREDED_COMMAND_BUFFER

    for (uint32_t i = 0; i < vulkanWindow.swapchain.getImages().size(); i++) {

        VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

        VKA(vkBeginCommandBuffer(commandBuffers[i], &beginInfo));

        vkCmdResetQueryPool(commandBuffers[i], timestampQueryPool, 0, 64);

        vkCmdWriteTimestamp(commandBuffers[i], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, timestampQueryPool, 0);

        VkRenderPassBeginInfo renderpassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        renderpassBeginInfo.renderPass = vulkanWindow.swapchain.getRenderpass().getRenderpass();
        renderpassBeginInfo.framebuffer = vulkanWindow.swapchain.getFramebuffers()[i].getFramebuffer();
        renderpassBeginInfo.renderArea = { 0,0, vulkanWindow.swapchain.getWidth(), vulkanWindow.swapchain.getHeight() };
        renderpassBeginInfo.clearValueCount = 1;
        VkClearValue clearValue = { 0.1f, 0.1f, 0.102f, 1.0f };
        renderpassBeginInfo.pClearValues = &clearValue;

        vkCmdBeginRenderPass(commandBuffers[i], &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getPipeline());

        VkViewport viewport = { 0.0f, 0.0f, (float)vulkanWindow.swapchain.getWidth(), (float)vulkanWindow.swapchain.getHeight(), 0.0f, 1.0f };
        VkRect2D scissor = { {0,0}, {vulkanWindow.swapchain.getWidth(), vulkanWindow.swapchain.getHeight()} };

        vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
        vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

        VkDeviceSize offsets[] = { 0 };

        const VkBuffer buffer = vertexBuffer.getBuffer();
        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &buffer, offsets);


        for (uint32_t j = 0; j < drawCount; j++) {

            vkCmdDraw(commandBuffers[i], vertices.size(), 1, 0, 0);
        }

        vkCmdEndRenderPass(commandBuffers[i]);

        vkCmdWriteTimestamp(commandBuffers[i], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, timestampQueryPool, 1);

        vkEndCommandBuffer(commandBuffers[i]);

    }
#endif

    while (!glfwWindowShouldClose(window))
    {
        
        vkWaitForFences(context.getData().device, 1, &fence, true, UINT64_MAX);
        vkResetFences(context.getData().device, 1, &fence); 

        bool recreateSwapchain = false;
        vulkanWindow.swapchain.aquireNextImage(context, acquireSemaphore, recreateSwapchain);

        uint64_t timestamps[2] = {};
        VkResult timestampsValid = vkGetQueryPoolResults(context.getData().device, timestampQueryPool, 0, ARRAY_COUNT(timestamps), sizeof(timestamps), timestamps, sizeof(timestamps[0]), VK_QUERY_RESULT_64_BIT);
        if (timestampsValid == VK_SUCCESS) {
            double frameGpuBegin = double(timestamps[0]) * context.getData().deviceProperties.limits.timestampPeriod * 1e-6;
            double frameGpuEnd = double(timestamps[1]) * context.getData().deviceProperties.limits.timestampPeriod * 1e-6;

            averageGpuTime = 0.99f * averageGpuTime + 0.01f * (frameGpuEnd - frameGpuBegin);

            std::cout << "Average GPU Time: " << averageGpuTime << "ms\n";
        }

#ifndef USE_PRE_RECOREDED_COMMAND_BUFFER

        vkResetCommandPool(context.getData().device, commandPool, 0);

        VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; 

        VKA(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        vkCmdResetQueryPool(commandBuffer, timestampQueryPool, 0, 64);

        vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, timestampQueryPool, 0);

        VkRenderPassBeginInfo renderpassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        renderpassBeginInfo.renderPass =  vulkanWindow.swapchain.getRenderpass().getRenderpass();
        renderpassBeginInfo.framebuffer = vulkanWindow.swapchain.getFramebuffers()[vulkanWindow.swapchain.getCurrentIndex()].getFramebuffer();
        renderpassBeginInfo.renderArea = { 0,0, vulkanWindow.swapchain.getWidth(), vulkanWindow.swapchain.getHeight() };
        renderpassBeginInfo.clearValueCount = 1;
        VkClearValue clearValue = { 0.1f, 0.1f, 0.102f, 1.0f };
        renderpassBeginInfo.pClearValues = &clearValue;

        vkCmdBeginRenderPass(commandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getPipeline());

        VkViewport viewport = { 0.0f, 0.0f, (float) vulkanWindow.swapchain.getWidth(), (float)vulkanWindow.swapchain.getHeight(), 0.0f, 1.0f };
        VkRect2D scissor = { {0,0}, {vulkanWindow.swapchain.getWidth(), vulkanWindow.swapchain.getHeight()} };

        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkDeviceSize offsets[] = { 0 };

        const VkBuffer buffer = vertexBuffer.getBuffer();
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, offsets);   


        for (uint32_t i = 0; i < drawCount; i++) {

            vkCmdDraw(commandBuffer, vertices.size(), 1, 0, 0);
        }

        vkCmdEndRenderPass(commandBuffer);

        vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, timestampQueryPool, 1);

        vkEndCommandBuffer(commandBuffer);

#endif

        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;

#ifdef USE_PRE_RECOREDED_COMMAND_BUFFER

        submitInfo.pCommandBuffers = &commandBuffers[vulkanWindow.swapchain.getCurrentIndex()];

#else

        submitInfo.pCommandBuffers = &commandBuffer;

#endif

        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &submitSemaphore;

        VkPipelineStageFlags waitMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submitInfo.pWaitDstStageMask = &waitMask;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &acquireSemaphore;
       
        VKA(vkQueueSubmit(context.getData().graphicsQueue, 1, &submitInfo, fence));

        recreateSwapchain = false;
        vulkanWindow.swapchain.present(context, { submitSemaphore }, recreateSwapchain);

        double time = glfwGetTime();

        std::cout << "Frame Time: " << (time - lastFrameTime) * 1000 << "\n";

        lastFrameTime = glfwGetTime();

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;

}

int main()
{
    
#ifdef OPENGL_TEST
    return opengl_test();
#elif VULKAN_TEST
    return vulkan_test();
#else
#error "please define OPENGL_TEST or VULKAN_TEST"
#endif // OPENGL_TEST
    
}



