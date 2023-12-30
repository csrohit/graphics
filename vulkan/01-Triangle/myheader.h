#pragma once

#include <X11/Xlib.h>
#include <optional>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xlib.h>

extern Display *dpy;
extern Window   w;

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

int initVulkan();

void cleanUp();
