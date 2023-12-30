#pragma once

#include <vulkan/vulkan.h>
#include <optional>

 
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
};

int initVulkan();

void cleanUp();
