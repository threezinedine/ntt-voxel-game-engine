#include "MEEDEngine/modules/release_stack/release_stack.h"
#include "MEEDEngine/modules/render/renderer.h"
#include "MEEDEngine/platforms/platforms.h"
#include "vulkan_common.h"
#include <vulkan/vulkan.h>

b8 s_isInitialized = MEED_FALSE;

static const char* instanceExtensions[] = {
	"VK_KHR_surface",
	"VK_KHR_xcb_surface",
	"VK_EXT_debug_utils",
};

static const char* layers[] = {
	"VK_LAYER_KHRONOS_validation",
};

/// @brief The context of the Vulkan renderer.
struct MEEDVulkan
{
	VkInstance instance;
};

struct MEEDVulkan* g_vulkan = MEED_NULL; // Global Vulkan instance

static struct MEEDReleaseStack* s_releaseStack = MEED_NULL; // Global release stack instance

static void createVulkanInstance();

void meedRenderInitialize()
{
	MEED_ASSERT_MSG(!s_isInitialized, "Rendering module is already initialized.");
	MEED_ASSERT(g_vulkan == MEED_NULL);
	MEED_ASSERT(s_releaseStack == MEED_NULL);
	// Implementation of rendering module initialization
	meedWindowInitialize();
	s_releaseStack = meedReleaseStackCreate();

	g_vulkan = MEED_MALLOC(struct MEEDVulkan);

	createVulkanInstance();

	s_isInitialized = MEED_TRUE;
}

void meedRenderShutdown()
{
	MEED_ASSERT_MSG(s_isInitialized, "Rendering module is not initialized.");
	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(s_releaseStack != MEED_NULL);
	// Implementation of rendering module shutdown

	MEED_FREE(g_vulkan, struct MEEDVulkan);

	meedReleaseStackDestroy(s_releaseStack);
	meedWindowShutdown();
	s_isInitialized = MEED_FALSE;
}

static void deleteVulkanInstance(void*);

static void createVulkanInstance()
{
	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(s_releaseStack != MEED_NULL);

	VkApplicationInfo appInfo  = {};
	appInfo.sType			   = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName   = "MEED Engine Application";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName		   = "MEED Engine";
	appInfo.engineVersion	   = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion		   = VK_API_VERSION_1_3;

	VkInstanceCreateInfo createInfo	   = {};
	createInfo.sType				   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo		   = &appInfo;
	createInfo.enabledExtensionCount   = MEED_ARRAY_SIZE(instanceExtensions);
	createInfo.ppEnabledExtensionNames = instanceExtensions;
	createInfo.enabledLayerCount	   = MEED_ARRAY_SIZE(layers);
	createInfo.ppEnabledLayerNames	   = layers;

#if 0
	VK_ASSERT(vkCreateInstance(&createInfo, MEED_NULL, &g_vulkan->instance));
	meedReleaseStackPush(s_releaseStack, MEED_NULL, deleteVulkanInstance);
#else
	VkResult result = vkCreateInstance(&createInfo, MEED_NULL, &g_vulkan->instance);
	if (result != VK_SUCCESS)
	{
		struct MEEDPlatformConsoleConfig config = {};
		config.color							= MEED_CONSOLE_COLOR_RED;
		meedPlatformSetConsoleConfig(config);

		if (result == VK_ERROR_INCOMPATIBLE_DRIVER)
		{
			meedPlatformPrint("Cannot find a compatible Vulkan ICD (Install the latest drivers from your GPU vendor).");
		}
		else if (result == VK_ERROR_EXTENSION_NOT_PRESENT)
		{
			meedPlatformPrint(
				"Cannot find a specified Vulkan extension (Make sure required extensions are available).");
		}
		else
		{
			meedPlatformFPrint("Failed to create Vulkan instance (VkResult: %d).", result);
		}

		MEED_UNTOUCHABLE();
	}
#endif
}

static void deleteVulkanInstance(void* pData)
{
	MEED_UNUSED(pData);

	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->instance != MEED_NULL);

	vkDestroyInstance(g_vulkan->instance, MEED_NULL);
}
