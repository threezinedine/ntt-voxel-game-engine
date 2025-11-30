#pragma once
#include <vulkan/vulkan.h>

/**
 * @file vulkan_common.h
 * Stores all the shared structures, enums, definitions, and macros used across the Vulkan renderer implementation.
 * @note this file is internal to the Vulkan renderer and should not be included directly by other modules.
 */
struct VulkanShader
{
	VkShaderModule module;
};

#define NULL_GRAPHICS_FAMILY ((i32)(-1))

/**
 * @brief The queue family indices for the selected physical device.
 */
struct QueueFamilyIndices
{
	i32 graphicsFamily;
	i32 presentFamily;
	i32 computeFamily;
	i32 transferFamily;
};

/// @brief The context of the Vulkan renderer.
struct MEEDVulkan
{
	struct MEEDWindowData* pWindowData;

	VkInstance instance;
#if MEED_DEBUG
	VkDebugUtilsMessengerEXT debugMessenger;
#endif
	VkPhysicalDevice		  physicalDevice;
	VkSurfaceKHR			  surface;
	struct QueueFamilyIndices queueFamilies; ///< The queue family indices for the selected physical device

	VkDevice device;

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkSurfaceFormatKHR			  surfaceFormat;
	VkPresentModeKHR			  presentMode;
	VkExtent2D					  extent;
	VkSurfaceTransformFlagBitsKHR preTransform;
	u32							  imagesCount;
	VkSwapchainKHR				  swapchain;

	VkImage*	 pSwapchainImages;
	VkImageView* pSwapchainImageViews;

	VkRenderPass renderPass;
};

#define VK_ASSERT(call)                                                                                                \
	do                                                                                                                 \
	{                                                                                                                  \
		VkResult result = call;                                                                                        \
		MEED_ASSERT_MSG(result == VK_SUCCESS, "Vulkan call failed with error code: %d", result);                       \
	} while (0)
