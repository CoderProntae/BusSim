#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>

namespace roadforge::renderer {

/** Lightweight RAII wrapper for a Vulkan buffer and its device memory.
 *
 * The wrapper does not allocate by itself; VulkanRenderer creates handles and
 * assigns ownership. This keeps allocation policy inside the renderer while
 * making cleanup deterministic and less error-prone as resource count grows.
 */
struct DeviceBuffer final {
    VkDevice device = VK_NULL_HANDLE;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDeviceSize size = 0;

    DeviceBuffer() = default;
    ~DeviceBuffer();

    DeviceBuffer(const DeviceBuffer&) = delete;
    DeviceBuffer& operator=(const DeviceBuffer&) = delete;
    DeviceBuffer(DeviceBuffer&& other) noexcept;
    DeviceBuffer& operator=(DeviceBuffer&& other) noexcept;

    void destroy();
    [[nodiscard]] bool valid() const { return device != VK_NULL_HANDLE && buffer != VK_NULL_HANDLE && memory != VK_NULL_HANDLE; }
};

/** Lightweight RAII wrapper for a Vulkan image, memory and optional image view. */
struct DeviceImage final {
    VkDevice device = VK_NULL_HANDLE;
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_UNDEFINED;
    uint32_t width = 0;
    uint32_t height = 0;

    DeviceImage() = default;
    ~DeviceImage();

    DeviceImage(const DeviceImage&) = delete;
    DeviceImage& operator=(const DeviceImage&) = delete;
    DeviceImage(DeviceImage&& other) noexcept;
    DeviceImage& operator=(DeviceImage&& other) noexcept;

    void destroy();
    [[nodiscard]] bool valid() const { return device != VK_NULL_HANDLE && image != VK_NULL_HANDLE && memory != VK_NULL_HANDLE; }
    [[nodiscard]] bool hasView() const { return valid() && view != VK_NULL_HANDLE; }
};

} // namespace roadforge::renderer
