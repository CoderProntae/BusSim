#pragma once

#include <android/native_window.h>
#include <vulkan/vulkan.h>

#include "roadforge/core/FrameStats.hpp"
#include "roadforge/math/Transform.hpp"
#include "roadforge/renderer/RenderProxy.hpp"
#include "roadforge/renderer/VulkanResources.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace roadforge::renderer {

class VulkanRenderer final {
public:
    VulkanRenderer() = default;
    ~VulkanRenderer();

    VulkanRenderer(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;
    VulkanRenderer(VulkanRenderer&&) = delete;
    VulkanRenderer& operator=(VulkanRenderer&&) = delete;

    bool initialize(ANativeWindow* window);
    void shutdown();

    void onSurfaceChanged(int32_t width, int32_t height);
    void setTouchPulse(float seconds);
    void setInputDebug(float steering, float throttle, float brake, bool touchActive);
    void setSimulationTiming(double appTimeSeconds, uint64_t simulationTick, double interpolationAlpha);
    void setDebugRoadTransform(const math::Transform& transform);
    void setDebugCamera(const math::Vec3& eye, const math::Vec3& target, const math::Vec3& up, float fovYRadians);
    void setFrameStats(const core::FrameStatsSnapshot& stats);
    void setDebugRenderProxies(const std::vector<DebugRenderProxy>& proxies);
    void tick(float deltaSeconds);
    void drawFrame();

    [[nodiscard]] bool isReady() const { return initialized_; }

private:
    struct QueueFamilyIndices final {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        [[nodiscard]] bool complete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapchainSupport final {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    bool createInstance();
    bool createSurface(ANativeWindow* window);
    bool pickPhysicalDevice();
    bool createLogicalDevice();
    bool createDebugMeshResources();
    bool createDebugOverlayResources();
    bool createSwapchain();
    bool createImageViews();
    bool createDepthResources();
    bool createRenderPass();
    bool createGraphicsPipeline();
    bool createFramebuffers();
    bool createCommandPool();
    bool createCommandBuffers();
    bool createSyncObjects();

    void cleanupDebugMeshResources();
    void cleanupDebugOverlayResources();
    void cleanupDepthResources();
    void cleanupSwapchain();
    bool recreateSwapchain();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    bool updateDebugSceneBuffers(uint32_t frameIndex);
    bool updateDebugOverlayBuffers(uint32_t frameIndex);
    [[nodiscard]] VkShaderModule createShaderModule(const uint8_t* code, size_t size) const;
    [[nodiscard]] bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, DeviceBuffer& output) const;
    [[nodiscard]] bool createImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, DeviceImage& output) const;
    [[nodiscard]] VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const;
    [[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
    [[nodiscard]] VkFormat chooseDepthFormat() const;

    [[nodiscard]] bool isDeviceSuitable(VkPhysicalDevice device) const;
    [[nodiscard]] QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
    [[nodiscard]] bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
    [[nodiscard]] SwapchainSupport querySwapchainSupport(VkPhysicalDevice device) const;
    [[nodiscard]] VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) const;
    [[nodiscard]] VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& presentModes) const;
    [[nodiscard]] VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

    static constexpr uint32_t kMaxFramesInFlight = 2;

    bool initialized_ = false;
    bool framebufferResized_ = false;
    int32_t requestedWidth_ = 0;
    int32_t requestedHeight_ = 0;
    float touchPulseSeconds_ = 0.0F;
    double appTimeSeconds_ = 0.0;
    double interpolationAlpha_ = 0.0;
    uint64_t simulationTick_ = 0;
    float debugSteering_ = 0.0F;
    float debugThrottle_ = 0.0F;
    float debugBrake_ = 0.0F;
    float debugInputHoldSeconds_ = 0.0F;
    bool debugTouchActive_ = false;
    math::Transform debugRoadTransform_{};
    math::Vec3 debugCameraEye_{ 0.0F, 1.65F, -4.25F };
    math::Vec3 debugCameraTarget_{ 0.0F, 0.0F, 3.4F };
    math::Vec3 debugCameraUp_{ 0.0F, 1.0F, 0.0F };
    float debugCameraFovYRadians_ = 60.0F * 0.01745329252F;
    core::FrameStatsSnapshot frameStats_{};
    std::vector<DebugRenderProxy> debugRenderProxies_;

    ANativeWindow* window_ = nullptr;

    VkInstance instance_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue graphicsQueue_ = VK_NULL_HANDLE;
    VkQueue presentQueue_ = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    VkFormat swapchainImageFormat_ = VK_FORMAT_UNDEFINED;
    VkExtent2D swapchainExtent_{};
    VkFormat depthFormat_ = VK_FORMAT_UNDEFINED;
    DeviceImage depthImage_;
    VkRenderPass renderPass_ = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline_ = VK_NULL_HANDLE;
    VkCommandPool commandPool_ = VK_NULL_HANDLE;
    std::array<DeviceBuffer, kMaxFramesInFlight> debugSceneVertexBuffers_;
    std::array<DeviceBuffer, kMaxFramesInFlight> debugSceneIndexBuffers_;
    std::array<uint32_t, kMaxFramesInFlight> debugSceneIndexCounts_{};
    std::array<DeviceBuffer, kMaxFramesInFlight> overlayVertexBuffers_;
    std::array<DeviceBuffer, kMaxFramesInFlight> overlayIndexBuffers_;
    std::array<uint32_t, kMaxFramesInFlight> overlayIndexCounts_{};

    std::vector<VkImage> swapchainImages_;
    std::vector<VkImageView> swapchainImageViews_;
    std::vector<VkFramebuffer> framebuffers_;
    std::vector<VkCommandBuffer> commandBuffers_;
    std::vector<VkSemaphore> imageAvailableSemaphores_;
    std::vector<VkSemaphore> renderFinishedSemaphores_;
    std::vector<VkFence> inFlightFences_;
    uint32_t currentFrame_ = 0;
};

} // namespace roadforge::renderer
