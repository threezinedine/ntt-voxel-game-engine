#if MEED_USE_VULKAN

#include "MEEDEngine/core/core.h"
#include "MEEDEngine/modules/release_stack/release_stack.h"
#include "MEEDEngine/modules/render/renderer.h"
#include "MEEDEngine/platforms/platforms.h"
#include "vulkan_common.h"
#include <vulkan/vulkan.h>

b8 s_isInitialized = MEED_FALSE;

static const char* instanceExtensions[] = {
	"VK_KHR_surface",
	"VK_KHR_xcb_surface",
	"VK_KHR_xlib_surface",
	"VK_EXT_debug_utils",
};

static const char* deviceExtensions[] = {
	"VK_KHR_swapchain",
};

static const char* layers[] = {
#if MEED_DEBUG
	"VK_LAYER_KHRONOS_validation",
#endif
};

#define NULL_GRAPHICS_FAMILY ((i32)(-1))

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
};

struct MEEDVulkan* g_vulkan = MEED_NULL; // Global Vulkan instance

static struct MEEDReleaseStack* s_releaseStack = MEED_NULL; // Global release stack instance

static void createVulkanInstance();
#if MEED_DEBUG
static void createValidationLayers();
#endif
static void choosePhysicalDevice();
static void createSurface();
static void getQueueFamilyIndices();
static void createDevice();
static void getQueues();
static void chooseSwapchainSettings();
static void createSwapchain();
static void getSwapchainImages();
static void createSwapchainImageViews();

static void deleteGlobalVulkanInstance(void*);

void meedRenderInitialize(struct MEEDWindowData* pWindowData)
{
	MEED_ASSERT_MSG(!s_isInitialized, "Rendering module is already initialized.");
	MEED_ASSERT(pWindowData != MEED_NULL);
	MEED_ASSERT(g_vulkan == MEED_NULL);
	MEED_ASSERT(s_releaseStack == MEED_NULL);
	// Implementation of rendering module initialization
	s_releaseStack = meedReleaseStackCreate();

	g_vulkan = MEED_MALLOC(struct MEEDVulkan);
	meedPlatformMemorySet(g_vulkan, 0, sizeof(struct MEEDVulkan));
	meedReleaseStackPush(s_releaseStack, MEED_NULL, deleteGlobalVulkanInstance);

	// Setup initial values for Vulkan context
	g_vulkan->pWindowData = pWindowData;

	g_vulkan->queueFamilies.graphicsFamily = NULL_GRAPHICS_FAMILY;
	g_vulkan->queueFamilies.presentFamily  = NULL_GRAPHICS_FAMILY;
	g_vulkan->queueFamilies.computeFamily  = NULL_GRAPHICS_FAMILY;
	g_vulkan->queueFamilies.transferFamily = NULL_GRAPHICS_FAMILY;

	createVulkanInstance();
#if MEED_DEBUG
	createValidationLayers();
#endif
	choosePhysicalDevice();
	createSurface();
	getQueueFamilyIndices();
	createDevice();
	getQueues();
	chooseSwapchainSettings();
	createSwapchain();
	getSwapchainImages();
	createSwapchainImageViews();

	s_isInitialized = MEED_TRUE;
}

static void deleteGlobalVulkanInstance(void* pData)
{
	MEED_UNUSED(pData);

	MEED_ASSERT(g_vulkan != MEED_NULL);

	MEED_FREE(g_vulkan, struct MEEDVulkan);
}

void meedRenderShutdown()
{
	MEED_ASSERT_MSG(s_isInitialized, "Rendering module is not initialized.");
	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(s_releaseStack != MEED_NULL);
	// Implementation of rendering module shutdown

	meedReleaseStackDestroy(s_releaseStack);
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

	VkInstanceCreateInfo instanceCreateInfo	   = {};
	instanceCreateInfo.sType				   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo		   = &appInfo;
	instanceCreateInfo.enabledExtensionCount   = MEED_ARRAY_SIZE(instanceExtensions);
	instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions;
	instanceCreateInfo.enabledLayerCount	   = MEED_ARRAY_SIZE(layers);
	instanceCreateInfo.ppEnabledLayerNames	   = layers;

	VK_ASSERT(vkCreateInstance(&instanceCreateInfo, MEED_NULL, &g_vulkan->instance));
	meedReleaseStackPush(s_releaseStack, MEED_NULL, deleteVulkanInstance);
}

static void deleteVulkanInstance(void* pData)
{
	MEED_UNUSED(pData);

	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->instance != MEED_NULL);

	vkDestroyInstance(g_vulkan->instance, MEED_NULL);
}

VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT	   messageSeverity,
												   VkDebugUtilsMessageTypeFlagsEXT			   messageType,
												   const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
												   void*									   pUserData)
{
	struct MEEDPlatformConsoleConfig config;
	char							 severityString[10];

	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		config.color = MEED_CONSOLE_COLOR_WHITE;
		meedPlatformBufferedPrint(severityString, sizeof(severityString), "%7s", "VERBOSE");
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		config.color = MEED_CONSOLE_COLOR_GREEN;
		meedPlatformBufferedPrint(severityString, sizeof(severityString), "%7s", "INFO");
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		config.color = MEED_CONSOLE_COLOR_YELLOW;
		meedPlatformBufferedPrint(severityString, sizeof(severityString), "%7s", "WARNING");
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		config.color = MEED_CONSOLE_COLOR_RED;
		meedPlatformBufferedPrint(severityString, sizeof(severityString), "%7s", "ERROR");
		break;
	default:
		config.color = MEED_CONSOLE_COLOR_RESET;
	}

	meedPlatformSetConsoleConfig(config);
	meedPlatformFPrint("[%7s] - [%s] - %s\n", "VULKAN", severityString, pCallbackData->pMessage);

	return VK_FALSE;
}

#if MEED_DEBUG
static void deleteDebugMessenger(void*);

static void createValidationLayers()
{
	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->instance != MEED_NULL);
	MEED_ASSERT(s_releaseStack != MEED_NULL);

	VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};

	messengerCreateInfo.sType			= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
										  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
										  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
									  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
									  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	messengerCreateInfo.pfnUserCallback = vulkanDebugCallback;

	PFN_vkCreateDebugUtilsMessengerEXT createDebugUtilsMessengerEXT =
		(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_vulkan->instance, "vkCreateDebugUtilsMessengerEXT");

	MEED_ASSERT(createDebugUtilsMessengerEXT != MEED_NULL);
	VK_ASSERT(
		createDebugUtilsMessengerEXT(g_vulkan->instance, &messengerCreateInfo, MEED_NULL, &g_vulkan->debugMessenger));
	meedReleaseStackPush(s_releaseStack, MEED_NULL, deleteDebugMessenger);
}

static void deleteDebugMessenger(void* pData)
{
	MEED_UNUSED(pData);

	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->instance != MEED_NULL);
	MEED_ASSERT(g_vulkan->debugMessenger != MEED_NULL);

	PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugUtilsMessengerEXT =
		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_vulkan->instance,
																   "vkDestroyDebugUtilsMessengerEXT");

	MEED_ASSERT(destroyDebugUtilsMessengerEXT != MEED_NULL);
	destroyDebugUtilsMessengerEXT(g_vulkan->instance, g_vulkan->debugMessenger, MEED_NULL);
}
#endif

static u32 isDeviceSuitable(VkPhysicalDevice device);

static void choosePhysicalDevice()
{
	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->instance != MEED_NULL);
	MEED_ASSERT(s_releaseStack != MEED_NULL);

	u32 devicesCount = 0;
	VK_ASSERT(vkEnumeratePhysicalDevices(g_vulkan->instance, &devicesCount, MEED_NULL));
	MEED_ASSERT_MSG(devicesCount > 0, "Failed to find GPUs with Vulkan support.");

	VkPhysicalDevice* pDevices = MEED_MALLOC_ARRAY(VkPhysicalDevice, devicesCount);
	VK_ASSERT(vkEnumeratePhysicalDevices(g_vulkan->instance, &devicesCount, pDevices));

	u32 highestScore	= 0;
	i32 bestDeviceIndex = -1;

	for (u32 deviceIndex = 0u; deviceIndex < devicesCount; ++deviceIndex)
	{
		u32 score = isDeviceSuitable(pDevices[deviceIndex]);
		if (score > highestScore)
		{
			highestScore	= score;
			bestDeviceIndex = deviceIndex;
		}
	}

	MEED_ASSERT_MSG(bestDeviceIndex != -1, "Failed to find a suitable GPU.");
	g_vulkan->physicalDevice = pDevices[bestDeviceIndex];
	MEED_FREE_ARRAY(pDevices, VkPhysicalDevice, devicesCount);
}

static b8  checkDeviceExtensionsSupport(VkPhysicalDevice device);
static u32 rateDevice(VkPhysicalDevice device);

static u32 isDeviceSuitable(VkPhysicalDevice device)
{
	if (!checkDeviceExtensionsSupport(device))
	{
		return 0;
	}

	return rateDevice(device);
}

static b8 checkDeviceExtensionsSupport(VkPhysicalDevice device)
{
	u32 deviceExtensionsCount = 0;
	vkEnumerateDeviceExtensionProperties(device, MEED_NULL, &deviceExtensionsCount, MEED_NULL);
	MEED_ASSERT_MSG(deviceExtensionsCount > 0, "Failed to get device extension properties.");

	VkExtensionProperties* pAvailableExtensions = MEED_MALLOC_ARRAY(VkExtensionProperties, deviceExtensionsCount);
	vkEnumerateDeviceExtensionProperties(device, MEED_NULL, &deviceExtensionsCount, pAvailableExtensions);

	for (u32 requiredEXTIndex = 0u; requiredEXTIndex < MEED_ARRAY_SIZE(deviceExtensions); ++requiredEXTIndex)
	{
		b8 extensionFound = MEED_FALSE;
		for (u32 availableEXTIndex = 0u; availableEXTIndex < deviceExtensionsCount; ++availableEXTIndex)
		{
			if (meedPlatformStringCompare(deviceExtensions[requiredEXTIndex],
										  pAvailableExtensions[availableEXTIndex].extensionName) == 0)
			{
				extensionFound = MEED_TRUE;
				break;
			}
		}

		if (!extensionFound)
		{
			MEED_FREE_ARRAY(pAvailableExtensions, VkExtensionProperties, deviceExtensionsCount);
			return MEED_FALSE;
		}
	}

	MEED_FREE_ARRAY(pAvailableExtensions, VkExtensionProperties, deviceExtensionsCount);
	return MEED_TRUE;
}

static u32 rateDevice(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(device, &properties);

	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(device, &features);

	u32 score = 0;

	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		score += 1000;
	}
	else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
	{
		score += 500;
	}

	if (features.geometryShader)
	{
		score += 100;
	}

	score += properties.limits.maxImageDimension2D;

	return score;
}

static void deleteSurface(void*);
static void createSurface()
{
	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->pWindowData != MEED_NULL);
	MEED_ASSERT(g_vulkan->instance != MEED_NULL);
	MEED_ASSERT(g_vulkan->physicalDevice != MEED_NULL);
	MEED_ASSERT(g_vulkan->surface == MEED_NULL);
	MEED_ASSERT(s_releaseStack != MEED_NULL);

	VK_ASSERT(meedWindowCreateVulkanSurface(g_vulkan->pWindowData, g_vulkan->instance, &g_vulkan->surface));
	meedReleaseStackPush(s_releaseStack, MEED_NULL, deleteSurface);
};

static void deleteSurface(void* pData)
{
	MEED_UNUSED(pData);

	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->pWindowData != MEED_NULL);
	MEED_ASSERT(g_vulkan->instance != MEED_NULL);
	MEED_ASSERT(g_vulkan->surface != MEED_NULL);

	meedWindowDestroyVulkanSurface(g_vulkan->pWindowData, g_vulkan->instance, g_vulkan->surface);
}

static void getQueueFamilyIndices()
{
	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->instance != MEED_NULL);
	MEED_ASSERT(g_vulkan->physicalDevice != MEED_NULL);
	MEED_ASSERT(g_vulkan->queueFamilies.graphicsFamily == NULL_GRAPHICS_FAMILY);
	MEED_ASSERT(g_vulkan->queueFamilies.presentFamily == NULL_GRAPHICS_FAMILY);
	MEED_ASSERT(g_vulkan->queueFamilies.computeFamily == NULL_GRAPHICS_FAMILY);
	MEED_ASSERT(g_vulkan->queueFamilies.transferFamily == NULL_GRAPHICS_FAMILY);

	u32 queueFamiliesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(g_vulkan->physicalDevice, &queueFamiliesCount, MEED_NULL);
	MEED_ASSERT_MSG(queueFamiliesCount > 0, "Failed to get queue family properties.");

	VkQueueFamilyProperties* pQueueFamilies = MEED_MALLOC_ARRAY(VkQueueFamilyProperties, queueFamiliesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(g_vulkan->physicalDevice, &queueFamiliesCount, pQueueFamilies);

	for (i32 queueFamilyIndex = 0; queueFamilyIndex < queueFamiliesCount; ++queueFamilyIndex)
	{
		VkQueueFamilyProperties currentFamily = pQueueFamilies[queueFamilyIndex];

		if (g_vulkan->queueFamilies.graphicsFamily == NULL_GRAPHICS_FAMILY &&
			(currentFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			g_vulkan->queueFamilies.graphicsFamily = queueFamilyIndex;
		}

		if (g_vulkan->queueFamilies.computeFamily == NULL_GRAPHICS_FAMILY &&
			(currentFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
		{
			g_vulkan->queueFamilies.computeFamily = queueFamilyIndex;
		}

		if (g_vulkan->queueFamilies.transferFamily == NULL_GRAPHICS_FAMILY &&
			(currentFamily.queueFlags & VK_QUEUE_TRANSFER_BIT))
		{
			g_vulkan->queueFamilies.transferFamily = queueFamilyIndex;
		}

		if (g_vulkan->queueFamilies.presentFamily == NULL_GRAPHICS_FAMILY)
		{
			VkBool32 presentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(
				g_vulkan->physicalDevice, queueFamilyIndex, g_vulkan->surface, &presentSupport);

			if (presentSupport)
			{
				g_vulkan->queueFamilies.presentFamily = queueFamilyIndex;
			}
		}
	}

	MEED_ASSERT_MSG(g_vulkan->queueFamilies.graphicsFamily != NULL_GRAPHICS_FAMILY,
					"Failed to find a graphics queue family.");

	MEED_ASSERT_MSG(g_vulkan->queueFamilies.presentFamily != NULL_GRAPHICS_FAMILY,
					"Failed to find a present queue family.");

	MEED_FREE_ARRAY(pQueueFamilies, VkQueueFamilyProperties, queueFamiliesCount);
}

static void deleteDevice(void*);
static i32	queueFamilyCompare(const void*, const void*);
static void createDevice()
{
	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->instance != MEED_NULL);
	MEED_ASSERT(g_vulkan->physicalDevice != MEED_NULL);

	float queuePriority = 1.0f;

	i32 familyIndices[] = {
		g_vulkan->queueFamilies.graphicsFamily,
		g_vulkan->queueFamilies.presentFamily,
	};

	struct MEEDSet* pSet			   = meedSetCreate(queueFamilyCompare);
	u32				familyIndicesCount = MEED_ARRAY_SIZE(familyIndices);
	for (u32 i = 0u; i < familyIndicesCount; ++i)
	{
		meedSetPush(pSet, &familyIndices[i]);
	}

	u32 uniqueQueueFamiliesCount = meedSetCount(pSet);

	VkDeviceQueueCreateInfo* pQueueCreateInfos = MEED_MALLOC_ARRAY(VkDeviceQueueCreateInfo, uniqueQueueFamiliesCount);

	for (u32 i = 0u; i < uniqueQueueFamiliesCount; ++i)
	{
		u32*					pFamilyIndex	= (u32*)meedSetAt(pSet, i);
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType					= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex		= *pFamilyIndex;
		queueCreateInfo.pQueuePriorities		= &queuePriority;
		queueCreateInfo.queueCount				= 1;

		pQueueCreateInfos[i] = queueCreateInfo;
	}

	VkDeviceCreateInfo deviceCreateInfo		 = {};
	deviceCreateInfo.sType					 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.enabledExtensionCount	 = MEED_ARRAY_SIZE(deviceExtensions);
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
	deviceCreateInfo.queueCreateInfoCount	 = uniqueQueueFamiliesCount;
	deviceCreateInfo.pQueueCreateInfos		 = pQueueCreateInfos;

	VK_ASSERT(vkCreateDevice(g_vulkan->physicalDevice, &deviceCreateInfo, MEED_NULL, &g_vulkan->device));
	meedReleaseStackPush(s_releaseStack, MEED_NULL, deleteDevice);

	MEED_FREE_ARRAY(pQueueCreateInfos, VkDeviceQueueCreateInfo, uniqueQueueFamiliesCount);
	meedSetDestroy(pSet);
}

static void deleteDevice(void* pData)
{
	MEED_UNUSED(pData);

	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->device != MEED_NULL);

	vkDestroyDevice(g_vulkan->device, MEED_NULL);
}

static i32 queueFamilyCompare(const void* pA, const void* pB)
{
	return *(u32*)(pA) - *(u32*)(pB);
}

static void getQueues()
{
	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->device != MEED_NULL);
	MEED_ASSERT(g_vulkan->queueFamilies.graphicsFamily != NULL_GRAPHICS_FAMILY);
	MEED_ASSERT(g_vulkan->queueFamilies.presentFamily != NULL_GRAPHICS_FAMILY);

	vkGetDeviceQueue(g_vulkan->device, g_vulkan->queueFamilies.graphicsFamily, 0, &g_vulkan->graphicsQueue);
	vkGetDeviceQueue(g_vulkan->device, g_vulkan->queueFamilies.presentFamily, 0, &g_vulkan->presentQueue);
}

static void chooseExtent();
static void choosePresentMode();
static void chooseFormat();
static void chooseImagesCount();

static void chooseSwapchainSettings()
{
	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->physicalDevice != MEED_NULL);
	MEED_ASSERT(g_vulkan->device != MEED_NULL);
	MEED_ASSERT(g_vulkan->surface != MEED_NULL);

	chooseExtent();
	choosePresentMode();
	chooseFormat();
	chooseImagesCount();
}

static void clamp(u32* value, u32 min, u32 max);

static void chooseExtent()
{
	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->physicalDevice != MEED_NULL);
	MEED_ASSERT(g_vulkan->device != MEED_NULL);
	MEED_ASSERT(g_vulkan->surface != MEED_NULL);

	VkSurfaceCapabilitiesKHR capabilities;
	VK_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_vulkan->physicalDevice, g_vulkan->surface, &capabilities));

	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		g_vulkan->extent = capabilities.currentExtent;
	}
	else
	{
		u32 width  = g_vulkan->pWindowData->width;
		u32 height = g_vulkan->pWindowData->height;

		clamp(&width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		clamp(&height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		meedPlatformMemorySet(&g_vulkan->extent, 0, sizeof(VkExtent2D));
		g_vulkan->extent.width	= width;
		g_vulkan->extent.height = height;
	}
}

static void clamp(u32* value, u32 min, u32 max)
{
	if (*value < min)
	{
		*value = min;
	}
	else if (*value > max)
	{
		*value = max;
	}
}

static void choosePresentMode()
{
	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->physicalDevice != MEED_NULL);

	u32 presentModesCount = 0;
	VK_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(
		g_vulkan->physicalDevice, g_vulkan->surface, &presentModesCount, MEED_NULL));
	MEED_ASSERT_MSG(presentModesCount > 0, "Failed to get present modes.");

	VkPresentModeKHR* pPresentModes = MEED_MALLOC_ARRAY(VkPresentModeKHR, presentModesCount);
	VK_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(
		g_vulkan->physicalDevice, g_vulkan->surface, &presentModesCount, pPresentModes));

	for (u32 i = 0u; i < presentModesCount; ++i)
	{
		if (pPresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			g_vulkan->presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			MEED_FREE_ARRAY(pPresentModes, VkPresentModeKHR, presentModesCount);
			return;
		}
	}

	g_vulkan->presentMode = VK_PRESENT_MODE_FIFO_KHR;
	MEED_FREE_ARRAY(pPresentModes, VkPresentModeKHR, presentModesCount);
	return;
}

static void chooseFormat()
{
	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->physicalDevice != MEED_NULL);

	u32 formatsCount = 0;
	VK_ASSERT(
		vkGetPhysicalDeviceSurfaceFormatsKHR(g_vulkan->physicalDevice, g_vulkan->surface, &formatsCount, MEED_NULL));
	MEED_ASSERT_MSG(formatsCount > 0, "Failed to get surface formats.");

	VkSurfaceFormatKHR* pFormats = MEED_MALLOC_ARRAY(VkSurfaceFormatKHR, formatsCount);

	VK_ASSERT(
		vkGetPhysicalDeviceSurfaceFormatsKHR(g_vulkan->physicalDevice, g_vulkan->surface, &formatsCount, pFormats));

	for (u32 i = 0u; i < formatsCount; ++i)
	{
		if (pFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
			pFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			g_vulkan->surfaceFormat = pFormats[i];
			MEED_FREE_ARRAY(pFormats, VkSurfaceFormatKHR, formatsCount);
			return;
		}
	}

	g_vulkan->surfaceFormat = pFormats[0]; // Fallback to the first format
	MEED_FREE_ARRAY(pFormats, VkSurfaceFormatKHR, formatsCount);
}

static void chooseImagesCount()
{
	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->physicalDevice != MEED_NULL);

	VkSurfaceCapabilitiesKHR capabilities;
	VK_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_vulkan->physicalDevice, g_vulkan->surface, &capabilities));

	g_vulkan->imagesCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && g_vulkan->imagesCount > capabilities.maxImageCount)
	{
		g_vulkan->imagesCount = capabilities.maxImageCount;
	}

	g_vulkan->preTransform = capabilities.currentTransform;
}

static void deleteSwapchain(void*);
static void createSwapchain()
{
	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->device != MEED_NULL);
	MEED_ASSERT(g_vulkan->surface != MEED_NULL);

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType					 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface					 = g_vulkan->surface;
	swapchainCreateInfo.minImageCount			 = g_vulkan->imagesCount;
	swapchainCreateInfo.imageFormat				 = g_vulkan->surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace			 = g_vulkan->surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent				 = g_vulkan->extent;
	swapchainCreateInfo.presentMode				 = g_vulkan->presentMode;
	swapchainCreateInfo.imageArrayLayers		 = 1;
	swapchainCreateInfo.imageUsage				 = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.preTransform			 = g_vulkan->preTransform;
	swapchainCreateInfo.compositeAlpha			 = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.clipped					 = VK_TRUE;
	swapchainCreateInfo.oldSwapchain			 = VK_NULL_HANDLE;

	if (g_vulkan->queueFamilies.graphicsFamily != g_vulkan->queueFamilies.presentFamily)
	{
		u32 queueFamilyIndices[] = {
			(u32)g_vulkan->queueFamilies.graphicsFamily,
			(u32)g_vulkan->queueFamilies.presentFamily,
		};

		swapchainCreateInfo.imageSharingMode	  = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices	  = queueFamilyIndices;
	}
	else
	{
		swapchainCreateInfo.imageSharingMode	  = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices	  = MEED_NULL;
	}

	VK_ASSERT(vkCreateSwapchainKHR(g_vulkan->device, &swapchainCreateInfo, MEED_NULL, &g_vulkan->swapchain));
	meedReleaseStackPush(s_releaseStack, MEED_NULL, deleteSwapchain);
}

static void deleteSwapchain(void* pData)
{
	MEED_UNUSED(pData);

	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->device != MEED_NULL);
	MEED_ASSERT(g_vulkan->swapchain != MEED_NULL);

	vkDestroySwapchainKHR(g_vulkan->device, g_vulkan->swapchain, MEED_NULL);
}

static void freeSwapchainImages(void*);
static void getSwapchainImages()
{
	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->device != MEED_NULL);
	MEED_ASSERT(g_vulkan->swapchain != MEED_NULL);
	MEED_ASSERT(g_vulkan->pSwapchainImages == MEED_NULL);

	g_vulkan->pSwapchainImages = MEED_MALLOC_ARRAY(VkImage, g_vulkan->imagesCount);
	meedReleaseStackPush(s_releaseStack, MEED_NULL, freeSwapchainImages);

	VK_ASSERT(vkGetSwapchainImagesKHR(
		g_vulkan->device, g_vulkan->swapchain, &g_vulkan->imagesCount, g_vulkan->pSwapchainImages));
}

static void freeSwapchainImages(void* pData)
{
	MEED_UNUSED(pData);

	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->pSwapchainImages != MEED_NULL);

	MEED_FREE_ARRAY(g_vulkan->pSwapchainImages, VkImage, g_vulkan->imagesCount);
}

static void freeSwapchainImageViews(void*);
static void deleteSwapchainImageViews(void*);
static void createSwapchainImageViews()
{
	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->pSwapchainImages != MEED_NULL);
	MEED_ASSERT(g_vulkan->pSwapchainImageViews == MEED_NULL);

	g_vulkan->pSwapchainImageViews = MEED_MALLOC_ARRAY(VkImageView, g_vulkan->imagesCount);
	meedReleaseStackPush(s_releaseStack, MEED_NULL, freeSwapchainImageViews);

	for (u32 imageIndex = 0u; imageIndex < g_vulkan->imagesCount; ++imageIndex)
	{
		VkImageViewCreateInfo imageViewCreateInfo			= {};
		imageViewCreateInfo.sType							= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image							= g_vulkan->pSwapchainImages[imageIndex];
		imageViewCreateInfo.viewType						= VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format							= g_vulkan->surfaceFormat.format;
		imageViewCreateInfo.components.r					= VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g					= VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b					= VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a					= VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel	= 0;
		imageViewCreateInfo.subresourceRange.levelCount		= 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount		= 1;

		VK_ASSERT(vkCreateImageView(
			g_vulkan->device, &imageViewCreateInfo, MEED_NULL, &g_vulkan->pSwapchainImageViews[imageIndex]));
	}
	meedReleaseStackPush(s_releaseStack, MEED_NULL, deleteSwapchainImageViews);
}

static void freeSwapchainImageViews(void* pData)
{
	MEED_UNUSED(pData);

	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->pSwapchainImageViews != MEED_NULL);

	MEED_FREE_ARRAY(g_vulkan->pSwapchainImageViews, VkImageView, g_vulkan->imagesCount);
}

static void deleteSwapchainImageViews(void* pData)
{
	MEED_UNUSED(pData);

	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->device != MEED_NULL);
	MEED_ASSERT(g_vulkan->pSwapchainImageViews != MEED_NULL);

	for (u32 imageIndex = 0u; imageIndex < g_vulkan->imagesCount; ++imageIndex)
	{
		vkDestroyImageView(g_vulkan->device, g_vulkan->pSwapchainImageViews[imageIndex], MEED_NULL);
	}
}

#endif // MEED_USE_VULKAN
