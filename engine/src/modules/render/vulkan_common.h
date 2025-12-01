#pragma once
#if MEED_USE_VULKAN
#include "MEEDEngine/platforms/platforms.h"
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

#define FRAME_IN_FLIGHT_COUNT 3

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

/// @brief The Vulkan-specific implementation of the render pipeline.
struct VulkanPipeline
{
	struct MEEDShader* pVertexShader;
	struct MEEDShader* pFragmentShader;
	VkPipelineLayout   layout;
	VkPipeline		   pipeline;
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

	VkCommandPool graphicsCommandPool;
	VkCommandPool presentCommandPool;

	VkCommandBuffer graphicsCommandBuffers[FRAME_IN_FLIGHT_COUNT];

	u32			currentFrame;
	u32			imageIndex;
	VkFence		inFlightFences[FRAME_IN_FLIGHT_COUNT];
	VkSemaphore imageAvailableSemaphores[FRAME_IN_FLIGHT_COUNT];
	VkSemaphore renderFinishedSemaphores[FRAME_IN_FLIGHT_COUNT];

	VkRenderingAttachmentInfo colorAttachmentInfos[FRAME_IN_FLIGHT_COUNT];
	VkRenderingInfoKHR		  renderingInfos[FRAME_IN_FLIGHT_COUNT];
};

extern struct MEEDVulkan* g_vulkan; // Global Vulkan instance

#if MEED_DEBUG
#define VK_ASSERT(call)                                                                                                \
	do                                                                                                                 \
	{                                                                                                                  \
		VkResult result = call;                                                                                        \
		MEED_ASSERT_MSG(result == VK_SUCCESS, "Vulkan call failed with error code: %d", result);                       \
	} while (0)
#else
#define VK_ASSERT(call) call
#endif

#endif // MEED_USE_VULKAN