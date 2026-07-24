#include "roadforge/renderer/VulkanResources.hpp"

#include <utility>

namespace roadforge::renderer {

DeviceBuffer::~DeviceBuffer() {
    destroy();
}

DeviceBuffer::DeviceBuffer(DeviceBuffer&& other) noexcept {
    *this = std::move(other);
}

DeviceBuffer& DeviceBuffer::operator=(DeviceBuffer&& other) noexcept {
    if (this != &other) {
        destroy();
        device = other.device;
        buffer = other.buffer;
        memory = other.memory;
        size = other.size;

        other.device = VK_NULL_HANDLE;
        other.buffer = VK_NULL_HANDLE;
        other.memory = VK_NULL_HANDLE;
        other.size = 0;
    }
    return *this;
}

void DeviceBuffer::destroy() {
    if (device == VK_NULL_HANDLE) {
        buffer = VK_NULL_HANDLE;
        memory = VK_NULL_HANDLE;
        size = 0;
        return;
    }

    if (buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, buffer, nullptr);
        buffer = VK_NULL_HANDLE;
    }
    if (memory != VK_NULL_HANDLE) {
        vkFreeMemory(device, memory, nullptr);
        memory = VK_NULL_HANDLE;
    }
    device = VK_NULL_HANDLE;
    size = 0;
}

DeviceImage::~DeviceImage() {
    destroy();
}

DeviceImage::DeviceImage(DeviceImage&& other) noexcept {
    *this = std::move(other);
}

DeviceImage& DeviceImage::operator=(DeviceImage&& other) noexcept {
    if (this != &other) {
        destroy();
        device = other.device;
        image = other.image;
        memory = other.memory;
        view = other.view;
        format = other.format;
        width = other.width;
        height = other.height;

        other.device = VK_NULL_HANDLE;
        other.image = VK_NULL_HANDLE;
        other.memory = VK_NULL_HANDLE;
        other.view = VK_NULL_HANDLE;
        other.format = VK_FORMAT_UNDEFINED;
        other.width = 0;
        other.height = 0;
    }
    return *this;
}

void DeviceImage::destroy() {
    if (device == VK_NULL_HANDLE) {
        image = VK_NULL_HANDLE;
        memory = VK_NULL_HANDLE;
        view = VK_NULL_HANDLE;
        format = VK_FORMAT_UNDEFINED;
        width = 0;
        height = 0;
        return;
    }

    if (view != VK_NULL_HANDLE) {
        vkDestroyImageView(device, view, nullptr);
        view = VK_NULL_HANDLE;
    }
    if (image != VK_NULL_HANDLE) {
        vkDestroyImage(device, image, nullptr);
        image = VK_NULL_HANDLE;
    }
    if (memory != VK_NULL_HANDLE) {
        vkFreeMemory(device, memory, nullptr);
        memory = VK_NULL_HANDLE;
    }

    device = VK_NULL_HANDLE;
    format = VK_FORMAT_UNDEFINED;
    width = 0;
    height = 0;
}

} // namespace roadforge::renderer
