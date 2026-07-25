#include "roadforge/renderer/VulkanRenderer.hpp"

#include "roadforge/core/Log.hpp"
#include "roadforge/math/Frustum.hpp"
#include "roadforge/math/Mat4.hpp"
#include "roadforge/math/Transform.hpp"
#include "roadforge/math/Vec.hpp"
#include "roadforge/renderer/DebugMeshAssets.hpp"
#include "roadforge/renderer/GeneratedShaders.hpp"

#include <vulkan/vulkan_android.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <set>
#include <vector>

namespace roadforge::renderer {

namespace {

constexpr std::array<const char*, 2> kInstanceExtensions = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
};

constexpr std::array<const char*, 1> kDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

struct PushConstants final {
    float mvp[16];
};

const std::array<uint8_t, 7>& glyphRows(char c) {
    static constexpr std::array<uint8_t, 7> kA = { 0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001 };
    static constexpr std::array<uint8_t, 7> kB = { 0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110 };
    static constexpr std::array<uint8_t, 7> kD = { 0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110 };
    static constexpr std::array<uint8_t, 7> kF = { 0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000 };
    static constexpr std::array<uint8_t, 7> kG = { 0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01110 };
    static constexpr std::array<uint8_t, 7> kI = { 0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b11111 };
    static constexpr std::array<uint8_t, 7> kL = { 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111 };
    static constexpr std::array<uint8_t, 7> kM = { 0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001 };
    static constexpr std::array<uint8_t, 7> kO = { 0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110 };
    static constexpr std::array<uint8_t, 7> kP = { 0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000 };
    static constexpr std::array<uint8_t, 7> kR = { 0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001 };
    static constexpr std::array<uint8_t, 7> kS = { 0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110 };
    static constexpr std::array<uint8_t, 7> kT = { 0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100 };
    static constexpr std::array<uint8_t, 7> kU = { 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110 };
    static constexpr std::array<uint8_t, 7> kX = { 0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001 };
    static constexpr std::array<uint8_t, 7> kBlank = { 0, 0, 0, 0, 0, 0, 0 };

    switch (c) {
        case 'A': return kA;
        case 'B': return kB;
        case 'D': return kD;
        case 'F': return kF;
        case 'G': return kG;
        case 'I': return kI;
        case 'L': return kL;
        case 'M': return kM;
        case 'O': return kO;
        case 'P': return kP;
        case 'R': return kR;
        case 'S': return kS;
        case 'T': return kT;
        case 'U': return kU;
        case 'X': return kX;
        default: return kBlank;
    }
}

void appendOverlayQuad(std::vector<DebugVertex>& vertices,
                       std::vector<uint16_t>& indices,
                       float minX,
                       float minY,
                       float maxX,
                       float maxY,
                       float z,
                       const std::array<float, 3>& color) {
    // Callers pass logical screen coordinates where negative X is left and
    // positive Y is top. On the tested Android/Vulkan surface, the shared world
    // pipeline reaches the display with a 90-degree axis transform. Pre-rotate
    // overlay quads here so labels are upright and bars keep their intended
    // horizontal/vertical direction on device.
    const auto toDeviceNdc = [z, &color](float x, float y) -> DebugVertex {
        return DebugVertex{{ -y, x, z }, { color[0], color[1], color[2] }};
    };

    const uint16_t base = static_cast<uint16_t>(vertices.size());
    vertices.push_back(toDeviceNdc(minX, minY));
    vertices.push_back(toDeviceNdc(maxX, minY));
    vertices.push_back(toDeviceNdc(maxX, maxY));
    vertices.push_back(toDeviceNdc(minX, maxY));
    indices.push_back(base);
    indices.push_back(static_cast<uint16_t>(base + 1));
    indices.push_back(static_cast<uint16_t>(base + 2));
    indices.push_back(static_cast<uint16_t>(base + 2));
    indices.push_back(static_cast<uint16_t>(base + 3));
    indices.push_back(base);
}

void appendOverlayLabel(std::vector<DebugVertex>& vertices,
                        std::vector<uint16_t>& indices,
                        const char* text,
                        float minX,
                        float maxY,
                        float cellSize,
                        const std::array<float, 3>& color) {
    constexpr float kOverlayZ = 0.0F;
    float cursorX = minX;
    for (int glyph = 0; text[glyph] != '\0'; ++glyph) {
        if (text[glyph] == ' ') {
            cursorX += cellSize * 3.0F;
            continue;
        }
        const std::array<uint8_t, 7>& rows = glyphRows(text[glyph]);
        for (int row = 0; row < 7; ++row) {
            for (int col = 0; col < 5; ++col) {
                const bool enabled = ((rows[row] >> (4 - col)) & 0x1U) != 0U;
                if (!enabled) {
                    continue;
                }
                const float x0 = cursorX + (static_cast<float>(col) * cellSize);
                const float y1 = maxY - (static_cast<float>(row) * cellSize);
                appendOverlayQuad(vertices, indices, x0, y1 - (cellSize * 0.82F), x0 + (cellSize * 0.82F), y1, kOverlayZ, color);
            }
        }
        cursorX += cellSize * 6.0F;
    }
}


const char* vkResultName(VkResult result) {
    switch (result) {
        case VK_SUCCESS: return "VK_SUCCESS";
        case VK_NOT_READY: return "VK_NOT_READY";
        case VK_TIMEOUT: return "VK_TIMEOUT";
        case VK_EVENT_SET: return "VK_EVENT_SET";
        case VK_EVENT_RESET: return "VK_EVENT_RESET";
        case VK_INCOMPLETE: return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
        case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
        default: return "VK_UNKNOWN_RESULT";
    }
}

bool checkResult(VkResult result, const char* operation) {
    if (result == VK_SUCCESS) {
        return true;
    }
    RF_LOGE("%s failed: %s (%d)", operation, vkResultName(result), static_cast<int>(result));
    return false;
}

} // namespace

VulkanRenderer::~VulkanRenderer() {
    shutdown();
}

bool VulkanRenderer::initialize(ANativeWindow* window) {
    if (initialized_) {
        shutdown();
    }

    window_ = window;
    requestedWidth_ = window_ != nullptr ? ANativeWindow_getWidth(window_) : 0;
    requestedHeight_ = window_ != nullptr ? ANativeWindow_getHeight(window_) : 0;

    RF_LOGI("Initializing Vulkan renderer, native window=%dx%d", requestedWidth_, requestedHeight_);

    if (!createInstance()
        || !createSurface(window_)
        || !pickPhysicalDevice()
        || !createLogicalDevice()
        || !createDebugMeshResources()
        || !createDebugOverlayResources()
        || !createSwapchain()
        || !createImageViews()
        || !createDepthResources()
        || !createRenderPass()
        || !createGraphicsPipeline()
        || !createFramebuffers()
        || !createCommandPool()
        || !createCommandBuffers()
        || !createSyncObjects()) {
        shutdown();
        return false;
    }

    initialized_ = true;
    RF_LOGI("Vulkan renderer initialized");
    return true;
}

void VulkanRenderer::shutdown() {
    if (device_ != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(device_);
    }

    cleanupSwapchain();
    cleanupDebugOverlayResources();
    cleanupDebugMeshResources();

    for (VkSemaphore semaphore : renderFinishedSemaphores_) {
        if (semaphore != VK_NULL_HANDLE) {
            vkDestroySemaphore(device_, semaphore, nullptr);
        }
    }
    renderFinishedSemaphores_.clear();

    for (VkSemaphore semaphore : imageAvailableSemaphores_) {
        if (semaphore != VK_NULL_HANDLE) {
            vkDestroySemaphore(device_, semaphore, nullptr);
        }
    }
    imageAvailableSemaphores_.clear();

    for (VkFence fence : inFlightFences_) {
        if (fence != VK_NULL_HANDLE) {
            vkDestroyFence(device_, fence, nullptr);
        }
    }
    inFlightFences_.clear();

    if (commandPool_ != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device_, commandPool_, nullptr);
        commandPool_ = VK_NULL_HANDLE;
    }
    commandBuffers_.clear();

    if (device_ != VK_NULL_HANDLE) {
        vkDestroyDevice(device_, nullptr);
        device_ = VK_NULL_HANDLE;
    }

    if (surface_ != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        surface_ = VK_NULL_HANDLE;
    }

    if (instance_ != VK_NULL_HANDLE) {
        vkDestroyInstance(instance_, nullptr);
        instance_ = VK_NULL_HANDLE;
    }

    physicalDevice_ = VK_NULL_HANDLE;
    graphicsQueue_ = VK_NULL_HANDLE;
    presentQueue_ = VK_NULL_HANDLE;
    window_ = nullptr;
    currentFrame_ = 0;
    framebufferResized_ = false;
    initialized_ = false;
}

void VulkanRenderer::onSurfaceChanged(int32_t width, int32_t height) {
    requestedWidth_ = width;
    requestedHeight_ = height;
    framebufferResized_ = true;
    RF_LOGI("Surface changed: %dx%d", requestedWidth_, requestedHeight_);
}

void VulkanRenderer::setTouchPulse(float seconds) {
    touchPulseSeconds_ = std::max(touchPulseSeconds_, seconds);
}

void VulkanRenderer::setInputDebug(float steering, float throttle, float brake, bool touchActive) {
    debugTouchActive_ = touchActive;

    if (touchActive) {
        debugSteering_ = std::clamp(steering, -1.0F, 1.0F);
        debugThrottle_ = std::clamp(throttle, 0.0F, 1.0F);
        debugBrake_ = std::clamp(brake, 0.0F, 1.0F);
        debugInputHoldSeconds_ = 1.25F;
    }
}

void VulkanRenderer::setSimulationTiming(double appTimeSeconds, uint64_t simulationTick, double interpolationAlpha) {
    appTimeSeconds_ = appTimeSeconds;
    simulationTick_ = simulationTick;
    interpolationAlpha_ = std::clamp(interpolationAlpha, 0.0, 1.0);
}

void VulkanRenderer::setDebugRoadTransform(const math::Transform& transform) {
    debugRoadTransform_ = transform;
}

void VulkanRenderer::setDebugCamera(const math::Vec3& eye, const math::Vec3& target, const math::Vec3& up, float fovYRadians) {
    debugCameraEye_ = eye;
    debugCameraTarget_ = target;
    debugCameraUp_ = up;
    debugCameraFovYRadians_ = fovYRadians;
}

void VulkanRenderer::setFrameStats(const core::FrameStatsSnapshot& stats) {
    frameStats_ = stats;
}

void VulkanRenderer::setDebugRenderProxies(const std::vector<DebugRenderProxy>& proxies) {
    debugRenderProxies_ = proxies;
}

void VulkanRenderer::tick(float deltaSeconds) {
    touchPulseSeconds_ = std::max(0.0F, touchPulseSeconds_ - deltaSeconds);
    debugInputHoldSeconds_ = std::max(0.0F, debugInputHoldSeconds_ - deltaSeconds);
}

void VulkanRenderer::drawFrame() {
    if (!initialized_ || device_ == VK_NULL_HANDLE || swapchain_ == VK_NULL_HANDLE || framebuffers_.empty()) {
        return;
    }

    if (framebufferResized_) {
        framebufferResized_ = false;
        if (!recreateSwapchain()) {
            return;
        }
    }

    VkFence fence = inFlightFences_[currentFrame_];
    vkWaitForFences(device_, 1, &fence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = 0;
    const VkResult acquireResult = vkAcquireNextImageKHR(
        device_,
        swapchain_,
        UINT64_MAX,
        imageAvailableSemaphores_[currentFrame_],
        VK_NULL_HANDLE,
        &imageIndex);

    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
        (void)recreateSwapchain();
        return;
    }
    if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR) {
        RF_LOGE("vkAcquireNextImageKHR failed: %s", vkResultName(acquireResult));
        return;
    }

    vkResetFences(device_, 1, &fence);

    VkCommandBuffer commandBuffer = commandBuffers_[currentFrame_];
    vkResetCommandBuffer(commandBuffer, 0);
    recordCommandBuffer(commandBuffer, imageIndex);

    const VkSemaphore waitSemaphores[] = { imageAvailableSemaphores_[currentFrame_] };
    const VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    const VkSemaphore signalSemaphores[] = { renderFinishedSemaphores_[currentFrame_] };

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    const VkResult submitResult = vkQueueSubmit(graphicsQueue_, 1, &submitInfo, fence);
    if (submitResult != VK_SUCCESS) {
        RF_LOGE("vkQueueSubmit failed: %s", vkResultName(submitResult));
        vkDeviceWaitIdle(device_);
        return;
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain_;
    presentInfo.pImageIndices = &imageIndex;

    const VkResult presentResult = vkQueuePresentKHR(presentQueue_, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || framebufferResized_) {
        framebufferResized_ = false;
        (void)recreateSwapchain();
    } else if (presentResult != VK_SUCCESS) {
        RF_LOGE("vkQueuePresentKHR failed: %s", vkResultName(presentResult));
    }

    currentFrame_ = (currentFrame_ + 1U) % kMaxFramesInFlight;
}

bool VulkanRenderer::createInstance() {
    uint32_t extensionCount = 0;
    if (!checkResult(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr), "vkEnumerateInstanceExtensionProperties(count)")) {
        return false;
    }

    std::vector<VkExtensionProperties> extensions(extensionCount);
    if (!checkResult(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()), "vkEnumerateInstanceExtensionProperties(list)")) {
        return false;
    }

    for (const char* required : kInstanceExtensions) {
        const bool available = std::any_of(extensions.begin(), extensions.end(), [required](const VkExtensionProperties& extension) {
            return std::strcmp(required, extension.extensionName) == 0;
        });
        if (!available) {
            RF_LOGE("Required Vulkan instance extension not available: %s", required);
            return false;
        }
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "RoadForge Bus Sim";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pEngineName = "RoadForge Native Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(kInstanceExtensions.size());
    createInfo.ppEnabledExtensionNames = kInstanceExtensions.data();
    createInfo.enabledLayerCount = 0;

    return checkResult(vkCreateInstance(&createInfo, nullptr, &instance_), "vkCreateInstance");
}

bool VulkanRenderer::createSurface(ANativeWindow* window) {
    if (window == nullptr) {
        RF_LOGE("Cannot create Vulkan surface without ANativeWindow");
        return false;
    }

    VkAndroidSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    createInfo.window = window;

    return checkResult(vkCreateAndroidSurfaceKHR(instance_, &createInfo, nullptr, &surface_), "vkCreateAndroidSurfaceKHR");
}

bool VulkanRenderer::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    if (!checkResult(vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr), "vkEnumeratePhysicalDevices(count)") || deviceCount == 0) {
        RF_LOGE("No Vulkan physical devices found");
        return false;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    if (!checkResult(vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data()), "vkEnumeratePhysicalDevices(list)")) {
        return false;
    }

    for (VkPhysicalDevice device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice_ = device;
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(device, &properties);
            RF_LOGI("Selected GPU: %s Vulkan %u.%u.%u", properties.deviceName,
                    VK_VERSION_MAJOR(properties.apiVersion),
                    VK_VERSION_MINOR(properties.apiVersion),
                    VK_VERSION_PATCH(properties.apiVersion));
            return true;
        }
    }

    RF_LOGE("No suitable Vulkan 1.1 device with swapchain/present support found");
    return false;
}

bool VulkanRenderer::createLogicalDevice() {
    const QueueFamilyIndices indices = findQueueFamilies(physicalDevice_);
    if (!indices.complete()) {
        RF_LOGE("Queue family indices incomplete");
        return false;
    }

    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.reserve(uniqueQueueFamilies.size());
    const float queuePriority = 1.0F;

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(kDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = kDeviceExtensions.data();
    createInfo.enabledLayerCount = 0;

    if (!checkResult(vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_), "vkCreateDevice")) {
        return false;
    }

    vkGetDeviceQueue(device_, indices.graphicsFamily.value(), 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, indices.presentFamily.value(), 0, &presentQueue_);
    return true;
}

bool VulkanRenderer::createDebugMeshResources() {
    constexpr VkDeviceSize kSceneVertexBufferSize = sizeof(DebugVertex) * 4096U;
    constexpr VkDeviceSize kSceneIndexBufferSize = sizeof(uint16_t) * 6144U;

    for (uint32_t frame = 0; frame < kMaxFramesInFlight; ++frame) {
        if (!createBuffer(kSceneVertexBufferSize,
                          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          debugSceneVertexBuffers_[frame])) {
            return false;
        }
        if (!createBuffer(kSceneIndexBufferSize,
                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          debugSceneIndexBuffers_[frame])) {
            return false;
        }
        debugSceneIndexCounts_[frame] = 0;
    }

    RF_LOGI("Dynamic debug scene resources created for %u frames", kMaxFramesInFlight);
    return true;
}

bool VulkanRenderer::createDebugOverlayResources() {
    constexpr VkDeviceSize kOverlayVertexBufferSize = sizeof(DebugVertex) * 2048U;
    constexpr VkDeviceSize kOverlayIndexBufferSize = sizeof(uint16_t) * 3072U;

    for (uint32_t frame = 0; frame < kMaxFramesInFlight; ++frame) {
        if (!createBuffer(kOverlayVertexBufferSize,
                          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          overlayVertexBuffers_[frame])) {
            return false;
        }
        if (!createBuffer(kOverlayIndexBufferSize,
                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          overlayIndexBuffers_[frame])) {
            return false;
        }
        overlayIndexCounts_[frame] = 0;
    }

    RF_LOGI("Debug overlay resources created for %u frames", kMaxFramesInFlight);
    return true;
}

bool VulkanRenderer::createSwapchain() {
    const SwapchainSupport support = querySwapchainSupport(physicalDevice_);
    if (support.formats.empty() || support.presentModes.empty()) {
        RF_LOGE("Swapchain support is incomplete");
        return false;
    }

    const VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(support.formats);
    const VkPresentModeKHR presentMode = choosePresentMode(support.presentModes);
    const VkExtent2D extent = chooseSwapExtent(support.capabilities);
    if (extent.width == 0 || extent.height == 0) {
        RF_LOGW("Swapchain extent is zero; surface may not be ready yet");
        return false;
    }

    uint32_t imageCount = support.capabilities.minImageCount + 1U;
    if (support.capabilities.maxImageCount > 0 && imageCount > support.capabilities.maxImageCount) {
        imageCount = support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface_;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const QueueFamilyIndices indices = findQueueFamilies(physicalDevice_);
    const uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = support.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (!checkResult(vkCreateSwapchainKHR(device_, &createInfo, nullptr, &swapchain_), "vkCreateSwapchainKHR")) {
        return false;
    }

    if (!checkResult(vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, nullptr), "vkGetSwapchainImagesKHR(count)")) {
        return false;
    }
    swapchainImages_.resize(imageCount);
    if (!checkResult(vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, swapchainImages_.data()), "vkGetSwapchainImagesKHR(list)")) {
        return false;
    }

    swapchainImageFormat_ = surfaceFormat.format;
    swapchainExtent_ = extent;
    RF_LOGI("Swapchain created: %ux%u images=%u format=%d", extent.width, extent.height, imageCount, static_cast<int>(surfaceFormat.format));
    return true;
}

bool VulkanRenderer::createImageViews() {
    swapchainImageViews_.resize(swapchainImages_.size(), VK_NULL_HANDLE);

    for (size_t i = 0; i < swapchainImages_.size(); ++i) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapchainImages_[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapchainImageFormat_;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (!checkResult(vkCreateImageView(device_, &createInfo, nullptr, &swapchainImageViews_[i]), "vkCreateImageView")) {
            return false;
        }
    }
    return true;
}

bool VulkanRenderer::createDepthResources() {
    depthFormat_ = chooseDepthFormat();
    if (depthFormat_ == VK_FORMAT_UNDEFINED) {
        RF_LOGE("No supported depth format found");
        return false;
    }

    if (!createImage(swapchainExtent_.width,
                     swapchainExtent_.height,
                     depthFormat_,
                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     depthImage_)) {
        return false;
    }

    depthImage_.view = createImageView(depthImage_.image, depthFormat_, VK_IMAGE_ASPECT_DEPTH_BIT);
    return depthImage_.view != VK_NULL_HANDLE;
}

bool VulkanRenderer::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchainImageFormat_;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = depthFormat_;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    const std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    return checkResult(vkCreateRenderPass(device_, &renderPassInfo, nullptr, &renderPass_), "vkCreateRenderPass");
}

bool VulkanRenderer::createGraphicsPipeline() {
    const VkShaderModule vertexShader = createShaderModule(
        shaders::kDebugTriangleVertSpv.data(),
        shaders::kDebugTriangleVertSpv.size());
    const VkShaderModule fragmentShader = createShaderModule(
        shaders::kDebugTriangleFragSpv.data(),
        shaders::kDebugTriangleFragSpv.size());

    if (vertexShader == VK_NULL_HANDLE || fragmentShader == VK_NULL_HANDLE) {
        if (vertexShader != VK_NULL_HANDLE) {
            vkDestroyShaderModule(device_, vertexShader, nullptr);
        }
        if (fragmentShader != VK_NULL_HANDLE) {
            vkDestroyShaderModule(device_, fragmentShader, nullptr);
        }
        return false;
    }

    VkPipelineShaderStageCreateInfo vertexStage{};
    vertexStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStage.module = vertexShader;
    vertexStage.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentStage{};
    fragmentStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStage.module = fragmentShader;
    fragmentStage.pName = "main";

    const VkPipelineShaderStageCreateInfo shaderStages[] = { vertexStage, fragmentStage };

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(DebugVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(DebugVertex, position);
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(DebugVertex, color);

    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 1;
    vertexInput.pVertexBindingDescriptions = &bindingDescription;
    vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInput.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0F;
    viewport.y = 0.0F;
    viewport.width = static_cast<float>(swapchainExtent_.width);
    viewport.height = static_cast<float>(swapchainExtent_.height);
    viewport.minDepth = 0.0F;
    viewport.maxDepth = 1.0F;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapchainExtent_;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0F;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
        | VK_COLOR_COMPONENT_G_BIT
        | VK_COLOR_COMPONENT_B_BIT
        | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstants);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    bool success = checkResult(vkCreatePipelineLayout(device_, &pipelineLayoutInfo, nullptr, &pipelineLayout_), "vkCreatePipelineLayout");
    if (success) {
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = nullptr;
        pipelineInfo.layout = pipelineLayout_;
        pipelineInfo.renderPass = renderPass_;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        success = checkResult(vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline_), "vkCreateGraphicsPipelines");
    }

    vkDestroyShaderModule(device_, fragmentShader, nullptr);
    vkDestroyShaderModule(device_, vertexShader, nullptr);

    if (!success) {
        if (graphicsPipeline_ != VK_NULL_HANDLE) {
            vkDestroyPipeline(device_, graphicsPipeline_, nullptr);
            graphicsPipeline_ = VK_NULL_HANDLE;
        }
        if (pipelineLayout_ != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr);
            pipelineLayout_ = VK_NULL_HANDLE;
        }
    }

    return success;
}

bool VulkanRenderer::createFramebuffers() {
    framebuffers_.resize(swapchainImageViews_.size(), VK_NULL_HANDLE);

    for (size_t i = 0; i < swapchainImageViews_.size(); ++i) {
        const VkImageView attachments[] = { swapchainImageViews_[i], depthImage_.view };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass_;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchainExtent_.width;
        framebufferInfo.height = swapchainExtent_.height;
        framebufferInfo.layers = 1;

        if (!checkResult(vkCreateFramebuffer(device_, &framebufferInfo, nullptr, &framebuffers_[i]), "vkCreateFramebuffer")) {
            return false;
        }
    }
    return true;
}

bool VulkanRenderer::createCommandPool() {
    const QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice_);
    if (!queueFamilyIndices.graphicsFamily.has_value()) {
        RF_LOGE("Cannot create command pool without graphics queue family");
        return false;
    }

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    return checkResult(vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool_), "vkCreateCommandPool");
}

bool VulkanRenderer::createCommandBuffers() {
    commandBuffers_.resize(kMaxFramesInFlight, VK_NULL_HANDLE);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool_;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers_.size());

    return checkResult(vkAllocateCommandBuffers(device_, &allocInfo, commandBuffers_.data()), "vkAllocateCommandBuffers");
}

bool VulkanRenderer::createSyncObjects() {
    imageAvailableSemaphores_.resize(kMaxFramesInFlight, VK_NULL_HANDLE);
    renderFinishedSemaphores_.resize(kMaxFramesInFlight, VK_NULL_HANDLE);
    inFlightFences_.resize(kMaxFramesInFlight, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < kMaxFramesInFlight; ++i) {
        if (!checkResult(vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &imageAvailableSemaphores_[i]), "vkCreateSemaphore(imageAvailable)")
            || !checkResult(vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &renderFinishedSemaphores_[i]), "vkCreateSemaphore(renderFinished)")
            || !checkResult(vkCreateFence(device_, &fenceInfo, nullptr, &inFlightFences_[i]), "vkCreateFence")) {
            return false;
        }
    }
    return true;
}

void VulkanRenderer::cleanupDebugMeshResources() {
    for (uint32_t frame = 0; frame < kMaxFramesInFlight; ++frame) {
        debugSceneIndexBuffers_[frame].destroy();
        debugSceneVertexBuffers_[frame].destroy();
        debugSceneIndexCounts_[frame] = 0;
    }
}


void VulkanRenderer::cleanupDebugOverlayResources() {
    for (uint32_t frame = 0; frame < kMaxFramesInFlight; ++frame) {
        overlayIndexBuffers_[frame].destroy();
        overlayVertexBuffers_[frame].destroy();
        overlayIndexCounts_[frame] = 0;
    }
}

void VulkanRenderer::cleanupDepthResources() {
    depthImage_.destroy();
    depthFormat_ = VK_FORMAT_UNDEFINED;
}

void VulkanRenderer::cleanupSwapchain() {
    for (VkFramebuffer framebuffer : framebuffers_) {
        if (framebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(device_, framebuffer, nullptr);
        }
    }
    framebuffers_.clear();

    if (graphicsPipeline_ != VK_NULL_HANDLE) {
        vkDestroyPipeline(device_, graphicsPipeline_, nullptr);
        graphicsPipeline_ = VK_NULL_HANDLE;
    }

    if (pipelineLayout_ != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr);
        pipelineLayout_ = VK_NULL_HANDLE;
    }

    if (renderPass_ != VK_NULL_HANDLE) {
        vkDestroyRenderPass(device_, renderPass_, nullptr);
        renderPass_ = VK_NULL_HANDLE;
    }

    cleanupDepthResources();

    for (VkImageView imageView : swapchainImageViews_) {
        if (imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(device_, imageView, nullptr);
        }
    }
    swapchainImageViews_.clear();
    swapchainImages_.clear();

    if (swapchain_ != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device_, swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
    }

    swapchainImageFormat_ = VK_FORMAT_UNDEFINED;
    swapchainExtent_ = {};
}

bool VulkanRenderer::recreateSwapchain() {
    if (device_ == VK_NULL_HANDLE || physicalDevice_ == VK_NULL_HANDLE || surface_ == VK_NULL_HANDLE) {
        return false;
    }

    const int32_t width = window_ != nullptr ? ANativeWindow_getWidth(window_) : requestedWidth_;
    const int32_t height = window_ != nullptr ? ANativeWindow_getHeight(window_) : requestedHeight_;
    if (width <= 0 || height <= 0) {
        RF_LOGW("Skipping swapchain recreate for invalid extent %dx%d", width, height);
        return false;
    }
    requestedWidth_ = width;
    requestedHeight_ = height;

    vkDeviceWaitIdle(device_);
    cleanupSwapchain();

    return createSwapchain()
        && createImageViews()
        && createDepthResources()
        && createRenderPass()
        && createGraphicsPipeline()
        && createFramebuffers();
}

bool VulkanRenderer::updateDebugSceneBuffers(uint32_t frameIndex) {
    if (frameIndex >= kMaxFramesInFlight || !debugSceneVertexBuffers_[frameIndex].valid() || !debugSceneIndexBuffers_[frameIndex].valid()) {
        return false;
    }

    std::vector<DebugVertex> vertices;
    std::vector<uint16_t> indices;
    buildDebugSceneFromProxies(debugRenderProxies_, vertices, indices);

    const VkDeviceSize vertexBytes = sizeof(DebugVertex) * vertices.size();
    const VkDeviceSize indexBytes = sizeof(uint16_t) * indices.size();
    if (vertexBytes == 0 || indexBytes == 0) {
        debugSceneIndexCounts_[frameIndex] = 0;
        return true;
    }

    if (vertexBytes > debugSceneVertexBuffers_[frameIndex].size || indexBytes > debugSceneIndexBuffers_[frameIndex].size) {
        RF_LOGE("Debug scene buffer overflow: vertexBytes=%llu indexBytes=%llu", static_cast<unsigned long long>(vertexBytes), static_cast<unsigned long long>(indexBytes));
        return false;
    }

    void* vertexData = nullptr;
    if (!checkResult(vkMapMemory(device_, debugSceneVertexBuffers_[frameIndex].memory, 0, vertexBytes, 0, &vertexData), "vkMapMemory(debug scene vertex)")) {
        return false;
    }
    std::memcpy(vertexData, vertices.data(), static_cast<size_t>(vertexBytes));
    vkUnmapMemory(device_, debugSceneVertexBuffers_[frameIndex].memory);

    void* indexData = nullptr;
    if (!checkResult(vkMapMemory(device_, debugSceneIndexBuffers_[frameIndex].memory, 0, indexBytes, 0, &indexData), "vkMapMemory(debug scene index)")) {
        return false;
    }
    std::memcpy(indexData, indices.data(), static_cast<size_t>(indexBytes));
    vkUnmapMemory(device_, debugSceneIndexBuffers_[frameIndex].memory);

    debugSceneIndexCounts_[frameIndex] = static_cast<uint32_t>(indices.size());
    return true;
}

bool VulkanRenderer::updateDebugOverlayBuffers(uint32_t frameIndex) {
    if (frameIndex >= kMaxFramesInFlight || !overlayVertexBuffers_[frameIndex].valid() || !overlayIndexBuffers_[frameIndex].valid()) {
        return false;
    }

    std::vector<DebugVertex> vertices;
    std::vector<uint16_t> indices;
    vertices.reserve(256);
    indices.reserve(384);

    constexpr float z = 0.0F;
    // Because appendOverlayQuad compensates the current Android/Vulkan surface
    // transform, logical +X maps to device-top. Keep the panel's logical X
    // positive so it lands in the physical top-left corner after conversion.
    appendOverlayQuad(vertices, indices, 0.305F, 0.615F, 0.965F, 0.955F, z, { 0.015F, 0.018F, 0.024F });
    appendOverlayQuad(vertices, indices, 0.315F, 0.925F, 0.955F, 0.945F, z, { 0.10F, 0.12F, 0.16F });
    appendOverlayLabel(vertices, indices, "FPS", 0.335F, 0.900F, 0.0085F, { 0.78F, 0.95F, 0.82F });
    appendOverlayLabel(vertices, indices, "MS", 0.335F, 0.820F, 0.0085F, { 0.96F, 0.84F, 0.45F });
    appendOverlayLabel(vertices, indices, "SIM", 0.335F, 0.740F, 0.0085F, { 0.66F, 0.82F, 1.00F });
    appendOverlayLabel(vertices, indices, "DRP", 0.335F, 0.660F, 0.0085F, { 1.00F, 0.50F, 0.50F });

    const float fpsRatio = std::clamp(static_cast<float>(frameStats_.estimatedFps / 60.0), 0.0F, 1.0F);
    const float frameRatio = std::clamp(static_cast<float>(1.0 - (frameStats_.averageFrameMs / 33.333)), 0.0F, 1.0F);
    const float simRatio = std::clamp(static_cast<float>(frameStats_.averageFixedSteps / 2.0), 0.0F, 1.0F);
    const float droppedRatio = frameStats_.droppedTimeEvents > 0 ? 1.0F : 0.05F;

    const auto appendBar = [&vertices, &indices](float y, float ratio, const std::array<float, 3>& color) {
        constexpr float zBar = 0.0F;
        constexpr float x0 = 0.505F;
        constexpr float x1 = 0.935F;
        constexpr float h = 0.035F;
        appendOverlayQuad(vertices, indices, x0, y, x1, y + h, zBar, { 0.055F, 0.062F, 0.075F });
        appendOverlayQuad(vertices, indices, x0, y, x0 + ((x1 - x0) * ratio), y + h, zBar, color);
    };

    appendBar(0.865F, fpsRatio, { 0.10F, 0.90F, 0.22F });
    appendBar(0.785F, frameRatio, { 0.95F, 0.70F, 0.12F });
    appendBar(0.705F, simRatio, { 0.18F, 0.50F, 1.00F });
    appendBar(0.625F, droppedRatio, frameStats_.droppedTimeEvents > 0 ? std::array<float, 3>{ 1.0F, 0.05F, 0.02F } : std::array<float, 3>{ 0.16F, 0.22F, 0.18F });

    const VkDeviceSize vertexBytes = sizeof(DebugVertex) * vertices.size();
    const VkDeviceSize indexBytes = sizeof(uint16_t) * indices.size();
    if (vertexBytes > overlayVertexBuffers_[frameIndex].size || indexBytes > overlayIndexBuffers_[frameIndex].size) {
        RF_LOGE("Debug overlay buffer overflow: vertexBytes=%llu indexBytes=%llu", static_cast<unsigned long long>(vertexBytes), static_cast<unsigned long long>(indexBytes));
        return false;
    }

    void* vertexData = nullptr;
    if (!checkResult(vkMapMemory(device_, overlayVertexBuffers_[frameIndex].memory, 0, vertexBytes, 0, &vertexData), "vkMapMemory(overlay vertex)")) {
        return false;
    }
    std::memcpy(vertexData, vertices.data(), static_cast<size_t>(vertexBytes));
    vkUnmapMemory(device_, overlayVertexBuffers_[frameIndex].memory);

    void* indexData = nullptr;
    if (!checkResult(vkMapMemory(device_, overlayIndexBuffers_[frameIndex].memory, 0, indexBytes, 0, &indexData), "vkMapMemory(overlay index)")) {
        return false;
    }
    std::memcpy(indexData, indices.data(), static_cast<size_t>(indexBytes));
    vkUnmapMemory(device_, overlayIndexBuffers_[frameIndex].memory);

    overlayIndexCounts_[frameIndex] = static_cast<uint32_t>(indices.size());
    return true;
}

void VulkanRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (!checkResult(vkBeginCommandBuffer(commandBuffer, &beginInfo), "vkBeginCommandBuffer")) {
        return;
    }

    const float pulse = std::clamp(touchPulseSeconds_ / 0.22F, 0.0F, 1.0F);
    const float breathe = 0.5F + (0.5F * std::sin(static_cast<float>(appTimeSeconds_) * 1.8F));
    const float fixedTickPhase = static_cast<float>(simulationTick_ % 60U) / 60.0F;
    const float interpolation = static_cast<float>(interpolationAlpha_);
    const bool showInputDebug = debugTouchActive_ || debugInputHoldSeconds_ > 0.0F;

    math::Vec4 base = {
        0.016F + (0.006F * fixedTickPhase),
        0.044F + (0.020F * breathe),
        0.082F + (0.018F * interpolation),
        1.0F,
    };

    if (showInputDebug) {
        if (debugBrake_ > 0.5F) {
            // Alt sol: fren. Testte kaçmasın diye tüm arka plan parlak kırmızı olur.
            base = { 0.95F, 0.00F, 0.00F, 1.0F };
        } else if (debugThrottle_ > 0.5F) {
            // Alt sağ: gaz. Testte kaçmasın diye tüm arka plan parlak yeşil olur.
            base = { 0.00F, 0.85F, 0.08F, 1.0F };
        } else if (debugSteering_ < -0.25F) {
            // Sol taraf: direksiyon sol. Parlak mavi.
            base = { 0.00F, 0.18F, 0.95F, 1.0F };
        } else if (debugSteering_ > 0.25F) {
            // Sağ taraf: direksiyon sağ. Parlak sarı/turuncu.
            base = { 0.95F, 0.72F, 0.00F, 1.0F };
        } else {
            base = { 0.55F, 0.00F, 0.85F, 1.0F };
        }
    }

    const math::Vec4 touch = { 0.95F, 0.38F, 0.06F, 1.0F };
    const float touchMix = showInputDebug ? 0.0F : pulse;
    const math::Vec4 finalColor = math::lerp(base, touch, touchMix);

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color.float32[0] = finalColor.x;
    clearValues[0].color.float32[1] = finalColor.y;
    clearValues[0].color.float32[2] = finalColor.z;
    clearValues[0].color.float32[3] = finalColor.w;
    clearValues[1].depthStencil.depth = 1.0F;
    clearValues[1].depthStencil.stencil = 0;

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass_;
    renderPassInfo.framebuffer = framebuffers_[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapchainExtent_;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    if (graphicsPipeline_ != VK_NULL_HANDLE
        && updateDebugSceneBuffers(currentFrame_)
        && debugSceneIndexCounts_[currentFrame_] > 0) {
        const VkBuffer vertexBuffers[] = { debugSceneVertexBuffers_[currentFrame_].buffer };
        const VkDeviceSize offsets[] = { 0 };
        const float aspect = swapchainExtent_.height > 0
            ? static_cast<float>(swapchainExtent_.width) / static_cast<float>(swapchainExtent_.height)
            : 1.0F;
        const math::Mat4 projection = math::perspectiveVulkanLH(debugCameraFovYRadians_, aspect, 0.1F, 100.0F);
        const math::Mat4 view = math::lookAtLH(debugCameraEye_, debugCameraTarget_, debugCameraUp_);
        const math::Mat4 viewProjection = math::multiply(projection, view);
        const math::Frustum frustum = math::extractFrustum(viewProjection);
        const bool sceneVisible = math::sphereInsideFrustum(frustum, {0.0F, 0.0F, 4.0F}, 1000.0F);

        PushConstants pushConstants{};
        std::memcpy(pushConstants.mvp, viewProjection.data(), sizeof(pushConstants.mvp));

        if (!sceneVisible) {
            vkCmdEndRenderPass(commandBuffer);
            (void)checkResult(vkEndCommandBuffer(commandBuffer), "vkEndCommandBuffer");
            return;
        }

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_);
        vkCmdPushConstants(commandBuffer, pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &pushConstants);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, debugSceneIndexBuffers_[currentFrame_].buffer, 0, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(commandBuffer, debugSceneIndexCounts_[currentFrame_], 1, 0, 0, 0);
    }

    // Temporarily disabled: the shared 3D pipeline is not a reliable place for
    // screen-space UI on all Android surface transforms. We will bring the stats
    // panel back with a dedicated 2D overlay pipeline instead of spending more
    // time compensating this path.

    vkCmdEndRenderPass(commandBuffer);

    (void)checkResult(vkEndCommandBuffer(commandBuffer), "vkEndCommandBuffer");
}

VkShaderModule VulkanRenderer::createShaderModule(const uint8_t* code, size_t size) const {
    if (code == nullptr || size == 0 || (size % sizeof(uint32_t)) != 0) {
        RF_LOGE("Invalid shader bytecode size: %zu", size);
        return VK_NULL_HANDLE;
    }

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code);

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    if (!checkResult(vkCreateShaderModule(device_, &createInfo, nullptr, &shaderModule), "vkCreateShaderModule")) {
        return VK_NULL_HANDLE;
    }
    return shaderModule;
}

bool VulkanRenderer::createBuffer(VkDeviceSize size,
                                  VkBufferUsageFlags usage,
                                  VkMemoryPropertyFlags properties,
                                  DeviceBuffer& output) const {
    output.destroy();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer buffer = VK_NULL_HANDLE;
    if (!checkResult(vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer), "vkCreateBuffer")) {
        return false;
    }

    VkMemoryRequirements memoryRequirements{};
    vkGetBufferMemoryRequirements(device_, buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocationInfo{};
    allocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocationInfo.allocationSize = memoryRequirements.size;
    allocationInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties);

    VkDeviceMemory memory = VK_NULL_HANDLE;
    if (!checkResult(vkAllocateMemory(device_, &allocationInfo, nullptr, &memory), "vkAllocateMemory(buffer)")) {
        vkDestroyBuffer(device_, buffer, nullptr);
        return false;
    }

    if (!checkResult(vkBindBufferMemory(device_, buffer, memory, 0), "vkBindBufferMemory")) {
        vkFreeMemory(device_, memory, nullptr);
        vkDestroyBuffer(device_, buffer, nullptr);
        return false;
    }

    output.device = device_;
    output.buffer = buffer;
    output.memory = memory;
    output.size = size;
    return true;
}

bool VulkanRenderer::createImage(uint32_t width,
                                 uint32_t height,
                                 VkFormat format,
                                 VkImageUsageFlags usage,
                                 VkMemoryPropertyFlags properties,
                                 DeviceImage& output) const {
    output.destroy();

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImage image = VK_NULL_HANDLE;
    if (!checkResult(vkCreateImage(device_, &imageInfo, nullptr, &image), "vkCreateImage")) {
        return false;
    }

    VkMemoryRequirements memoryRequirements{};
    vkGetImageMemoryRequirements(device_, image, &memoryRequirements);

    VkMemoryAllocateInfo allocationInfo{};
    allocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocationInfo.allocationSize = memoryRequirements.size;
    allocationInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties);

    VkDeviceMemory memory = VK_NULL_HANDLE;
    if (!checkResult(vkAllocateMemory(device_, &allocationInfo, nullptr, &memory), "vkAllocateMemory(image)")) {
        vkDestroyImage(device_, image, nullptr);
        return false;
    }

    if (!checkResult(vkBindImageMemory(device_, image, memory, 0), "vkBindImageMemory")) {
        vkFreeMemory(device_, memory, nullptr);
        vkDestroyImage(device_, image, nullptr);
        return false;
    }

    output.device = device_;
    output.image = image;
    output.memory = memory;
    output.format = format;
    output.width = width;
    output.height = height;
    return true;
}

VkImageView VulkanRenderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView = VK_NULL_HANDLE;
    if (!checkResult(vkCreateImageView(device_, &viewInfo, nullptr, &imageView), "vkCreateImageView(generic)")) {
        return VK_NULL_HANDLE;
    }
    return imageView;
}

uint32_t VulkanRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties memoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        const bool typeMatches = (typeFilter & (1U << i)) != 0U;
        const bool propertiesMatch = (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties;
        if (typeMatches && propertiesMatch) {
            return i;
        }
    }

    RF_LOGE("Failed to find suitable Vulkan memory type, typeFilter=%u properties=%u", typeFilter, properties);
    return 0;
}

VkFormat VulkanRenderer::chooseDepthFormat() const {
    constexpr std::array<VkFormat, 2> candidates = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D16_UNORM,
    };

    for (VkFormat format : candidates) {
        VkFormatProperties properties{};
        vkGetPhysicalDeviceFormatProperties(physicalDevice_, format, &properties);
        if ((properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0U) {
            return format;
        }
    }

    return VK_FORMAT_UNDEFINED;
}

bool VulkanRenderer::isDeviceSuitable(VkPhysicalDevice device) const {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(device, &properties);
    if (properties.apiVersion < VK_API_VERSION_1_1) {
        return false;
    }

    const QueueFamilyIndices indices = findQueueFamilies(device);
    const bool extensionsSupported = checkDeviceExtensionSupport(device);
    bool swapchainAdequate = false;
    if (extensionsSupported) {
        const SwapchainSupport support = querySwapchainSupport(device);
        swapchainAdequate = !support.formats.empty() && !support.presentModes.empty();
    }

    return indices.complete() && extensionsSupported && swapchainAdequate;
}

VulkanRenderer::QueueFamilyIndices VulkanRenderer::findQueueFamilies(VkPhysicalDevice device) const {
    QueueFamilyIndices indices{};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        if ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0U) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = VK_FALSE;
        const VkResult presentResult = vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
        if (presentResult == VK_SUCCESS && presentSupport == VK_TRUE) {
            indices.presentFamily = i;
        }

        if (indices.complete()) {
            break;
        }
    }

    return indices;
}

bool VulkanRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device) const {
    uint32_t extensionCount = 0;
    if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr) != VK_SUCCESS) {
        return false;
    }

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()) != VK_SUCCESS) {
        return false;
    }

    for (const char* required : kDeviceExtensions) {
        const bool available = std::any_of(availableExtensions.begin(), availableExtensions.end(), [required](const VkExtensionProperties& extension) {
            return std::strcmp(required, extension.extensionName) == 0;
        });
        if (!available) {
            return false;
        }
    }

    return true;
}

VulkanRenderer::SwapchainSupport VulkanRenderer::querySwapchainSupport(VkPhysicalDevice device) const {
    SwapchainSupport support{};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &support.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);
    if (formatCount > 0) {
        support.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, support.formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);
    if (presentModeCount > 0) {
        support.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, support.presentModes.data());
    }

    return support;
}

VkSurfaceFormatKHR VulkanRenderer::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) const {
    for (const VkSurfaceFormatKHR& availableFormat : formats) {
        if (availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return formats.front();
}

VkPresentModeKHR VulkanRenderer::choosePresentMode(const std::vector<VkPresentModeKHR>& presentModes) const {
    // Mobilde ilk hedef kararlı frame pacing olduğu için FIFO seçilir; Android'de her zaman desteklenir.
    for (const VkPresentModeKHR mode : presentModes) {
        if (mode == VK_PRESENT_MODE_FIFO_KHR) {
            return mode;
        }
    }
    return presentModes.front();
}

VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    uint32_t width = 0;
    uint32_t height = 0;
    if (window_ != nullptr) {
        width = static_cast<uint32_t>(std::max(0, ANativeWindow_getWidth(window_)));
        height = static_cast<uint32_t>(std::max(0, ANativeWindow_getHeight(window_)));
    } else {
        width = static_cast<uint32_t>(std::max(0, requestedWidth_));
        height = static_cast<uint32_t>(std::max(0, requestedHeight_));
    }

    VkExtent2D actualExtent{ width, height };
    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return actualExtent;
}

} // namespace roadforge::renderer
