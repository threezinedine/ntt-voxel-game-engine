#if MD_USE_VULKAN

#include "MEEDEngine/core/core.h"
#include "MEEDEngine/modules/release_stack/release_stack.h"
#include "MEEDEngine/modules/render/renderer.h"
#include "MEEDEngine/platforms/platforms.h"
#include "vulkan_common.h"

b8 s_isInitialized = MD_FALSE;

static const char* instanceExtensions[] = {
	"VK_KHR_surface",
	"VK_KHR_xcb_surface",
	"VK_KHR_xlib_surface",
	"VK_EXT_debug_utils",
};

static const char* deviceExtensions[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
};

static const char* layers[] = {
#if MD_DEBUG
	"VK_LAYER_KHRONOS_validation",
#endif
};

struct MEEDVulkan* g_vulkan = MD_NULL; // Global Vulkan instance

static struct MdReleaseStack* s_releaseStack = MD_NULL; // Global release stack instance

static void createVulkanInstance();
#if MD_DEBUG
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
static void createRenderPass();
static void createCommandPools();
static void allocateCommandBuffers();
static void createSyncObjects();

static void deleteGlobalVulkanInstance(void*);

void mdRenderInitialize(struct MdWindowData* pWindowData)
{
	MD_ASSERT_MSG(!s_isInitialized, "Rendering module is already initialized.");
	MD_ASSERT(pWindowData != MD_NULL);
	MD_ASSERT(g_vulkan == MD_NULL);
	MD_ASSERT(s_releaseStack == MD_NULL);
	// Implementation of rendering module initialization
	s_releaseStack = mdReleaseStackCreate();

	g_vulkan = MD_MALLOC(struct MEEDVulkan);
	mdMemorySet(g_vulkan, 0, sizeof(struct MEEDVulkan));
	mdReleaseStackPush(s_releaseStack, MD_NULL, deleteGlobalVulkanInstance);

	// Setup initial values for Vulkan context
	g_vulkan->pWindowData = pWindowData;

	g_vulkan->queueFamilies.graphicsFamily = NULL_GRAPHICS_FAMILY;
	g_vulkan->queueFamilies.presentFamily  = NULL_GRAPHICS_FAMILY;
	g_vulkan->queueFamilies.computeFamily  = NULL_GRAPHICS_FAMILY;
	g_vulkan->queueFamilies.transferFamily = NULL_GRAPHICS_FAMILY;

	createVulkanInstance();
#if MD_DEBUG
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
	// createRenderPass();
	createCommandPools();
	allocateCommandBuffers();
	createSyncObjects();

	s_isInitialized = MD_TRUE;
}

static void deleteGlobalVulkanInstance(void* pData)
{
	MD_UNUSED(pData);

	MD_ASSERT(g_vulkan != MD_NULL);

	MD_FREE(g_vulkan, struct MEEDVulkan);
}

void mdRenderShutdown()
{
	MD_ASSERT_MSG(s_isInitialized, "Rendering module is not initialized.");
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(s_releaseStack != MD_NULL);
	// Implementation of rendering module shutdown

	mdReleaseStackDestroy(s_releaseStack);
	s_isInitialized = MD_FALSE;
}

static void deleteVulkanInstance(void*);

static void createVulkanInstance()
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(s_releaseStack != MD_NULL);

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
	instanceCreateInfo.enabledExtensionCount   = MD_ARRAY_SIZE(instanceExtensions);
	instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions;
	instanceCreateInfo.enabledLayerCount	   = MD_ARRAY_SIZE(layers);
	instanceCreateInfo.ppEnabledLayerNames	   = layers;

	VK_ASSERT(vkCreateInstance(&instanceCreateInfo, MD_NULL, &g_vulkan->instance));
	mdReleaseStackPush(s_releaseStack, MD_NULL, deleteVulkanInstance);
}

static void deleteVulkanInstance(void* pData)
{
	MD_UNUSED(pData);

	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->instance != MD_NULL);

	vkDestroyInstance(g_vulkan->instance, MD_NULL);
}

VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT	   messageSeverity,
												   VkDebugUtilsMessageTypeFlagsEXT			   messageType,
												   const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
												   void*									   pUserData)
{
	struct MdConsoleConfig config;
	char				   severityString[10];

	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		config.color = MD_CONSOLE_COLOR_WHITE;
		mdFormatString(severityString, sizeof(severityString), "%7s", "VERBOSE");
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		config.color = MD_CONSOLE_COLOR_GREEN;
		mdFormatString(severityString, sizeof(severityString), "%7s", "INFO");
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		config.color = MD_CONSOLE_COLOR_YELLOW;
		mdFormatString(severityString, sizeof(severityString), "%7s", "WARNING");
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		config.color = MD_CONSOLE_COLOR_RED;
		mdFormatString(severityString, sizeof(severityString), "%7s", "ERROR");
		break;
	default:
		config.color = MD_CONSOLE_COLOR_RESET;
	}

	mdSetConsoleConfig(config);
	mdFormatPrint("[%7s] - [%s] - %s\n", "VULKAN", severityString, pCallbackData->pMessage);

	return VK_FALSE;
}

#if MD_DEBUG
static void deleteDebugMessenger(void*);

static void createValidationLayers()
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->instance != MD_NULL);
	MD_ASSERT(s_releaseStack != MD_NULL);

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

	MD_ASSERT(createDebugUtilsMessengerEXT != MD_NULL);
	VK_ASSERT(
		createDebugUtilsMessengerEXT(g_vulkan->instance, &messengerCreateInfo, MD_NULL, &g_vulkan->debugMessenger));
	mdReleaseStackPush(s_releaseStack, MD_NULL, deleteDebugMessenger);
}

static void deleteDebugMessenger(void* pData)
{
	MD_UNUSED(pData);

	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->instance != MD_NULL);
	MD_ASSERT(g_vulkan->debugMessenger != MD_NULL);

	PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugUtilsMessengerEXT =
		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_vulkan->instance,
																   "vkDestroyDebugUtilsMessengerEXT");

	MD_ASSERT(destroyDebugUtilsMessengerEXT != MD_NULL);
	destroyDebugUtilsMessengerEXT(g_vulkan->instance, g_vulkan->debugMessenger, MD_NULL);
}
#endif

static u32 isDeviceSuitable(VkPhysicalDevice device);

static void choosePhysicalDevice()
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->instance != MD_NULL);
	MD_ASSERT(s_releaseStack != MD_NULL);

	u32 devicesCount = 0;
	VK_ASSERT(vkEnumeratePhysicalDevices(g_vulkan->instance, &devicesCount, MD_NULL));
	MD_ASSERT_MSG(devicesCount > 0, "Failed to find GPUs with Vulkan support.");

	VkPhysicalDevice* pDevices = MD_MALLOC_ARRAY(VkPhysicalDevice, devicesCount);
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

	MD_ASSERT_MSG(bestDeviceIndex != -1, "Failed to find a suitable GPU.");
	g_vulkan->physicalDevice = pDevices[bestDeviceIndex];
	MD_FREE_ARRAY(pDevices, VkPhysicalDevice, devicesCount);
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
	vkEnumerateDeviceExtensionProperties(device, MD_NULL, &deviceExtensionsCount, MD_NULL);
	MD_ASSERT_MSG(deviceExtensionsCount > 0, "Failed to get device extension properties.");

	VkExtensionProperties* pAvailableExtensions = MD_MALLOC_ARRAY(VkExtensionProperties, deviceExtensionsCount);
	vkEnumerateDeviceExtensionProperties(device, MD_NULL, &deviceExtensionsCount, pAvailableExtensions);

	for (u32 requiredEXTIndex = 0u; requiredEXTIndex < MD_ARRAY_SIZE(deviceExtensions); ++requiredEXTIndex)
	{
		b8 extensionFound = MD_FALSE;
		for (u32 availableEXTIndex = 0u; availableEXTIndex < deviceExtensionsCount; ++availableEXTIndex)
		{
			if (mdStringCompare(deviceExtensions[requiredEXTIndex],
								pAvailableExtensions[availableEXTIndex].extensionName) == 0)
			{
				extensionFound = MD_TRUE;
				break;
			}
		}

		if (!extensionFound)
		{
			MD_FREE_ARRAY(pAvailableExtensions, VkExtensionProperties, deviceExtensionsCount);
			return MD_FALSE;
		}
	}

	MD_FREE_ARRAY(pAvailableExtensions, VkExtensionProperties, deviceExtensionsCount);
	return MD_TRUE;
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
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->pWindowData != MD_NULL);
	MD_ASSERT(g_vulkan->instance != MD_NULL);
	MD_ASSERT(g_vulkan->physicalDevice != MD_NULL);
	MD_ASSERT(g_vulkan->surface == MD_NULL);
	MD_ASSERT(s_releaseStack != MD_NULL);

	VK_ASSERT(mdWindowCreateVulkanSurface(g_vulkan->pWindowData, g_vulkan->instance, &g_vulkan->surface));
	mdReleaseStackPush(s_releaseStack, MD_NULL, deleteSurface);
};

static void deleteSurface(void* pData)
{
	MD_UNUSED(pData);

	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->pWindowData != MD_NULL);
	MD_ASSERT(g_vulkan->instance != MD_NULL);
	MD_ASSERT(g_vulkan->surface != MD_NULL);

	mdWindowDestroyVulkanSurface(g_vulkan->pWindowData, g_vulkan->instance, g_vulkan->surface);
}

static void getQueueFamilyIndices()
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->instance != MD_NULL);
	MD_ASSERT(g_vulkan->physicalDevice != MD_NULL);
	MD_ASSERT(g_vulkan->queueFamilies.graphicsFamily == NULL_GRAPHICS_FAMILY);
	MD_ASSERT(g_vulkan->queueFamilies.presentFamily == NULL_GRAPHICS_FAMILY);
	MD_ASSERT(g_vulkan->queueFamilies.computeFamily == NULL_GRAPHICS_FAMILY);
	MD_ASSERT(g_vulkan->queueFamilies.transferFamily == NULL_GRAPHICS_FAMILY);

	u32 queueFamiliesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(g_vulkan->physicalDevice, &queueFamiliesCount, MD_NULL);
	MD_ASSERT_MSG(queueFamiliesCount > 0, "Failed to get queue family properties.");

	VkQueueFamilyProperties* pQueueFamilies = MD_MALLOC_ARRAY(VkQueueFamilyProperties, queueFamiliesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(g_vulkan->physicalDevice, &queueFamiliesCount, pQueueFamilies);

	for (i32 queueFamilyIndex = 0; queueFamilyIndex < queueFamiliesCount; ++queueFamilyIndex)
	{
		VkQueueFamilyProperties currentFamily  = pQueueFamilies[queueFamilyIndex];
		VkBool32				presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(
			g_vulkan->physicalDevice, queueFamilyIndex, g_vulkan->surface, &presentSupport);

		if (g_vulkan->queueFamilies.presentFamily == NULL_GRAPHICS_FAMILY)
		{
			if (presentSupport)
			{
				g_vulkan->queueFamilies.presentFamily = queueFamilyIndex;
			}
		}

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

		if (currentFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			g_vulkan->queueFamilies.transferFamily = queueFamilyIndex;
		}
	}

	MD_ASSERT_MSG(g_vulkan->queueFamilies.graphicsFamily != NULL_GRAPHICS_FAMILY,
				  "Failed to find a graphics queue family.");

	MD_ASSERT_MSG(g_vulkan->queueFamilies.presentFamily != NULL_GRAPHICS_FAMILY,
				  "Failed to find a present queue family.");

	MD_ASSERT_MSG(g_vulkan->queueFamilies.transferFamily != NULL_GRAPHICS_FAMILY,
				  "Failed to find a unique transfer queue family.");

	MD_FREE_ARRAY(pQueueFamilies, VkQueueFamilyProperties, queueFamiliesCount);
}

static void deleteDevice(void*);
static i32	queueFamilyCompare(const void*, const void*);
static void createDevice()
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->instance != MD_NULL);
	MD_ASSERT(g_vulkan->physicalDevice != MD_NULL);

	float queuePriority = 1.0f;

	i32 familyIndices[] = {
		g_vulkan->queueFamilies.graphicsFamily,
		g_vulkan->queueFamilies.presentFamily,
		g_vulkan->queueFamilies.transferFamily,
	};

	struct MdSet* pSet				 = mdSetCreate(queueFamilyCompare);
	u32			  familyIndicesCount = MD_ARRAY_SIZE(familyIndices);
	for (u32 i = 0u; i < familyIndicesCount; ++i)
	{
		mdSetPush(pSet, &familyIndices[i]);
	}

	u32 uniqueQueueFamiliesCount = mdSetCount(pSet);

	VkDeviceQueueCreateInfo* pQueueCreateInfos = MD_MALLOC_ARRAY(VkDeviceQueueCreateInfo, uniqueQueueFamiliesCount);

	for (u32 i = 0u; i < uniqueQueueFamiliesCount; ++i)
	{
		u32*					pFamilyIndex	= (u32*)mdSetAt(pSet, i);
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType					= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex		= *pFamilyIndex;
		queueCreateInfo.pQueuePriorities		= &queuePriority;
		queueCreateInfo.queueCount				= 1;

		pQueueCreateInfos[i] = queueCreateInfo;
	}

	VkPhysicalDeviceFeatures2 feature2 = {};
	feature2.sType					   = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

	VkPhysicalDeviceVulkan13Features vulkan13Features = {};
	vulkan13Features.sType							  = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	vulkan13Features.dynamicRendering				  = VK_TRUE;
	vulkan13Features.synchronization2				  = VK_TRUE;
	vulkan13Features.pNext							  = &feature2;

	VkDeviceCreateInfo deviceCreateInfo		 = {};
	deviceCreateInfo.sType					 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext					 = &vulkan13Features;
	deviceCreateInfo.enabledExtensionCount	 = MD_ARRAY_SIZE(deviceExtensions);
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
	deviceCreateInfo.queueCreateInfoCount	 = uniqueQueueFamiliesCount;
	deviceCreateInfo.pQueueCreateInfos		 = pQueueCreateInfos;

	VK_ASSERT(vkCreateDevice(g_vulkan->physicalDevice, &deviceCreateInfo, MD_NULL, &g_vulkan->device));
	mdReleaseStackPush(s_releaseStack, MD_NULL, deleteDevice);

	MD_FREE_ARRAY(pQueueCreateInfos, VkDeviceQueueCreateInfo, uniqueQueueFamiliesCount);
	mdSetDestroy(pSet);
}

static void deleteDevice(void* pData)
{
	MD_UNUSED(pData);

	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);

	vkDestroyDevice(g_vulkan->device, MD_NULL);
}

static i32 queueFamilyCompare(const void* pA, const void* pB)
{
	return *(u32*)(pA) - *(u32*)(pB);
}

static void getQueues()
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);
	MD_ASSERT(g_vulkan->queueFamilies.graphicsFamily != NULL_GRAPHICS_FAMILY);
	MD_ASSERT(g_vulkan->queueFamilies.presentFamily != NULL_GRAPHICS_FAMILY);
	MD_ASSERT(g_vulkan->queueFamilies.transferFamily != NULL_GRAPHICS_FAMILY);

	vkGetDeviceQueue(g_vulkan->device, g_vulkan->queueFamilies.graphicsFamily, 0, &g_vulkan->graphicsQueue);
	vkGetDeviceQueue(g_vulkan->device, g_vulkan->queueFamilies.presentFamily, 0, &g_vulkan->presentQueue);
	vkGetDeviceQueue(g_vulkan->device, g_vulkan->queueFamilies.transferFamily, 0, &g_vulkan->transferQueue);
}

static void chooseExtent();
static void choosePresentMode();
static void chooseFormat();
static void chooseImagesCount();

static void chooseSwapchainSettings()
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->physicalDevice != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);
	MD_ASSERT(g_vulkan->surface != MD_NULL);

	chooseExtent();
	choosePresentMode();
	chooseFormat();
	chooseImagesCount();
}

static void clamp(u32* value, u32 min, u32 max);

static void chooseExtent()
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->physicalDevice != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);
	MD_ASSERT(g_vulkan->surface != MD_NULL);

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

		mdMemorySet(&g_vulkan->extent, 0, sizeof(VkExtent2D));
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
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->physicalDevice != MD_NULL);

	u32 presentModesCount = 0;
	VK_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(
		g_vulkan->physicalDevice, g_vulkan->surface, &presentModesCount, MD_NULL));
	MD_ASSERT_MSG(presentModesCount > 0, "Failed to get present modes.");

	VkPresentModeKHR* pPresentModes = MD_MALLOC_ARRAY(VkPresentModeKHR, presentModesCount);
	VK_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(
		g_vulkan->physicalDevice, g_vulkan->surface, &presentModesCount, pPresentModes));

	g_vulkan->presentMode = VK_PRESENT_MODE_FIFO_KHR; // Default present mode

	for (u32 i = 0u; i < presentModesCount; ++i)
	{
		if (pPresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			g_vulkan->presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if (pPresentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			g_vulkan->presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	g_vulkan->presentMode = VK_PRESENT_MODE_FIFO_KHR;
	MD_FREE_ARRAY(pPresentModes, VkPresentModeKHR, presentModesCount);
	return;
}

static void chooseFormat()
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->physicalDevice != MD_NULL);

	u32 formatsCount = 0;
	VK_ASSERT(
		vkGetPhysicalDeviceSurfaceFormatsKHR(g_vulkan->physicalDevice, g_vulkan->surface, &formatsCount, MD_NULL));
	MD_ASSERT_MSG(formatsCount > 0, "Failed to get surface formats.");

	VkSurfaceFormatKHR* pFormats = MD_MALLOC_ARRAY(VkSurfaceFormatKHR, formatsCount);

	VK_ASSERT(
		vkGetPhysicalDeviceSurfaceFormatsKHR(g_vulkan->physicalDevice, g_vulkan->surface, &formatsCount, pFormats));

	for (u32 i = 0u; i < formatsCount; ++i)
	{
		if (pFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
			pFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			g_vulkan->surfaceFormat = pFormats[i];
			MD_FREE_ARRAY(pFormats, VkSurfaceFormatKHR, formatsCount);
			return;
		}
	}

	g_vulkan->surfaceFormat = pFormats[0]; // Fallback to the first format
	MD_FREE_ARRAY(pFormats, VkSurfaceFormatKHR, formatsCount);
}

static void chooseImagesCount()
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->physicalDevice != MD_NULL);

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
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);
	MD_ASSERT(g_vulkan->surface != MD_NULL);

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
		swapchainCreateInfo.queueFamilyIndexCount = MD_ARRAY_SIZE(queueFamilyIndices);
		swapchainCreateInfo.pQueueFamilyIndices	  = queueFamilyIndices;
	}
	else
	{
		swapchainCreateInfo.imageSharingMode	  = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices	  = MD_NULL;
	}

	VK_ASSERT(vkCreateSwapchainKHR(g_vulkan->device, &swapchainCreateInfo, MD_NULL, &g_vulkan->swapchain));
	mdReleaseStackPush(s_releaseStack, MD_NULL, deleteSwapchain);
}

static void deleteSwapchain(void* pData)
{
	MD_UNUSED(pData);

	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);
	MD_ASSERT(g_vulkan->swapchain != MD_NULL);

	vkDestroySwapchainKHR(g_vulkan->device, g_vulkan->swapchain, MD_NULL);
}

static void freeSwapchainImages(void*);
static void getSwapchainImages()
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);
	MD_ASSERT(g_vulkan->swapchain != MD_NULL);
	MD_ASSERT(g_vulkan->pSwapchainImages == MD_NULL);

	g_vulkan->pSwapchainImages = MD_MALLOC_ARRAY(VkImage, g_vulkan->imagesCount);
	mdReleaseStackPush(s_releaseStack, MD_NULL, freeSwapchainImages);

	VK_ASSERT(vkGetSwapchainImagesKHR(
		g_vulkan->device, g_vulkan->swapchain, &g_vulkan->imagesCount, g_vulkan->pSwapchainImages));
}

static void freeSwapchainImages(void* pData)
{
	MD_UNUSED(pData);

	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->pSwapchainImages != MD_NULL);

	MD_FREE_ARRAY(g_vulkan->pSwapchainImages, VkImage, g_vulkan->imagesCount);
}

static void freeSwapchainImageViews(void*);
static void deleteSwapchainImageViews(void*);
static void createSwapchainImageViews()
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->pSwapchainImages != MD_NULL);
	MD_ASSERT(g_vulkan->pSwapchainImageViews == MD_NULL);

	g_vulkan->pSwapchainImageViews = MD_MALLOC_ARRAY(VkImageView, g_vulkan->imagesCount);
	mdReleaseStackPush(s_releaseStack, MD_NULL, freeSwapchainImageViews);

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
			g_vulkan->device, &imageViewCreateInfo, MD_NULL, &g_vulkan->pSwapchainImageViews[imageIndex]));
	}
	mdReleaseStackPush(s_releaseStack, MD_NULL, deleteSwapchainImageViews);
}

static void freeSwapchainImageViews(void* pData)
{
	MD_UNUSED(pData);

	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->pSwapchainImageViews != MD_NULL);

	MD_FREE_ARRAY(g_vulkan->pSwapchainImageViews, VkImageView, g_vulkan->imagesCount);
}

static void deleteSwapchainImageViews(void* pData)
{
	MD_UNUSED(pData);

	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);
	MD_ASSERT(g_vulkan->pSwapchainImageViews != MD_NULL);

	for (u32 imageIndex = 0u; imageIndex < g_vulkan->imagesCount; ++imageIndex)
	{
		vkDestroyImageView(g_vulkan->device, g_vulkan->pSwapchainImageViews[imageIndex], MD_NULL);
	}
}

static void deleteRenderPass(void*);
static void createRenderPass()
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);
	MD_ASSERT(g_vulkan->renderPass == MD_NULL);

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format					= g_vulkan->surfaceFormat.format;
	colorAttachment.samples					= VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp					= VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp					= VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp			= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout			= VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format					= VK_FORMAT_D32_SFLOAT;
	depthAttachment.samples					= VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp					= VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp					= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp			= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout			= VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout				= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment			 = 0;
	colorAttachmentRef.layout				 = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment			 = 1;
	depthAttachmentRef.layout				 = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass	= {};
	subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount	= 1;
	subpass.pColorAttachments		= &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass		   = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass		   = 0;
	dependency.srcStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription attachments[] = {colorAttachment, depthAttachment};

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType					= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount		= MD_ARRAY_SIZE(attachments);
	renderPassCreateInfo.pAttachments			= attachments;
	renderPassCreateInfo.subpassCount			= 1;
	renderPassCreateInfo.pSubpasses				= &subpass;
	renderPassCreateInfo.dependencyCount		= 1;
	renderPassCreateInfo.pDependencies			= &dependency;

	VK_ASSERT(vkCreateRenderPass(g_vulkan->device, &renderPassCreateInfo, MD_NULL, &g_vulkan->renderPass));
	mdReleaseStackPush(s_releaseStack, MD_NULL, deleteRenderPass);
}

static void deleteRenderPass(void* pData)
{
	MD_UNUSED(pData);

	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);
	MD_ASSERT(g_vulkan->renderPass != MD_NULL);

	vkDestroyRenderPass(g_vulkan->device, g_vulkan->renderPass, MD_NULL);
}

static void deleteCommandPool(void*);
static void createCommandPools()
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);
	MD_ASSERT(g_vulkan->graphicsCommandPool == MD_NULL);
	MD_ASSERT(g_vulkan->presentCommandPool == MD_NULL);
	MD_ASSERT(g_vulkan->transferCommandPool == MD_NULL);

	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType					  = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.flags					  = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex		  = g_vulkan->queueFamilies.graphicsFamily;
	VK_ASSERT(vkCreateCommandPool(g_vulkan->device, &commandPoolCreateInfo, MD_NULL, &g_vulkan->graphicsCommandPool));

	if (g_vulkan->queueFamilies.presentFamily != g_vulkan->queueFamilies.graphicsFamily)
	{
		VkCommandPoolCreateInfo presentCommandPoolCreateInfo = {};
		presentCommandPoolCreateInfo.sType					 = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		presentCommandPoolCreateInfo.flags					 = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		presentCommandPoolCreateInfo.queueFamilyIndex		 = g_vulkan->queueFamilies.presentFamily;
		VK_ASSERT(vkCreateCommandPool(
			g_vulkan->device, &presentCommandPoolCreateInfo, MD_NULL, &g_vulkan->presentCommandPool));
	}
	else
	{
		g_vulkan->presentCommandPool = g_vulkan->graphicsCommandPool;
	}

	if (g_vulkan->queueFamilies.transferFamily != g_vulkan->queueFamilies.graphicsFamily)
	{
		VkCommandPoolCreateInfo transferCommandPoolCreateInfo = {};
		transferCommandPoolCreateInfo.sType					  = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		transferCommandPoolCreateInfo.flags					  = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		transferCommandPoolCreateInfo.queueFamilyIndex		  = g_vulkan->queueFamilies.transferFamily;
		VK_ASSERT(vkCreateCommandPool(
			g_vulkan->device, &transferCommandPoolCreateInfo, MD_NULL, &g_vulkan->transferCommandPool));
	}
	else
	{
		g_vulkan->transferCommandPool = g_vulkan->graphicsCommandPool;
	}

	mdReleaseStackPush(s_releaseStack, MD_NULL, deleteCommandPool);
}

static void deleteCommandPool(void* pData)
{
	MD_UNUSED(pData);

	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);
	MD_ASSERT(g_vulkan->graphicsCommandPool != MD_NULL);

	vkDestroyCommandPool(g_vulkan->device, g_vulkan->graphicsCommandPool, MD_NULL);

	if (g_vulkan->queueFamilies.graphicsFamily != g_vulkan->queueFamilies.presentFamily)
	{
		MD_ASSERT(g_vulkan->presentCommandPool != MD_NULL);
		vkDestroyCommandPool(g_vulkan->device, g_vulkan->presentCommandPool, MD_NULL);
	}

	if (g_vulkan->queueFamilies.graphicsFamily != g_vulkan->queueFamilies.transferFamily)
	{
		MD_ASSERT(g_vulkan->transferCommandPool != MD_NULL);
		vkDestroyCommandPool(g_vulkan->device, g_vulkan->transferCommandPool, MD_NULL);
	}
}

static void freeCommandBuffers(void*);
static void allocateCommandBuffers()
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);
	MD_ASSERT(g_vulkan->graphicsCommandPool != MD_NULL);
	MD_ASSERT(g_vulkan->presentCommandPool != MD_NULL);

	VkCommandBufferAllocateInfo graphicsCommandBufferAllocateInfo = {};
	graphicsCommandBufferAllocateInfo.sType						  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	graphicsCommandBufferAllocateInfo.commandPool				  = g_vulkan->graphicsCommandPool;
	graphicsCommandBufferAllocateInfo.level						  = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	graphicsCommandBufferAllocateInfo.commandBufferCount		  = FRAME_IN_FLIGHT_COUNT;
	VK_ASSERT(vkAllocateCommandBuffers(
		g_vulkan->device, &graphicsCommandBufferAllocateInfo, g_vulkan->graphicsCommandBuffers));

	VkCommandBufferAllocateInfo transferCommandBufferAllocateInfo = {};
	transferCommandBufferAllocateInfo.sType						  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	transferCommandBufferAllocateInfo.commandPool				  = g_vulkan->transferCommandPool;
	transferCommandBufferAllocateInfo.level						  = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	transferCommandBufferAllocateInfo.commandBufferCount		  = 1;
	VK_ASSERT(vkAllocateCommandBuffers(
		g_vulkan->device, &transferCommandBufferAllocateInfo, &g_vulkan->transferCommandBuffer));
	mdReleaseStackPush(s_releaseStack, MD_NULL, freeCommandBuffers);
}

static void freeCommandBuffers(void* pData)
{
	MD_UNUSED(pData);
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);
	MD_ASSERT(g_vulkan->graphicsCommandPool != MD_NULL);
	MD_ASSERT(g_vulkan->graphicsCommandBuffers != MD_NULL);

	vkFreeCommandBuffers(
		g_vulkan->device, g_vulkan->graphicsCommandPool, FRAME_IN_FLIGHT_COUNT, g_vulkan->graphicsCommandBuffers);

	vkFreeCommandBuffers(g_vulkan->device, g_vulkan->transferCommandPool, 1, &g_vulkan->transferCommandBuffer);
}

static void deleteSyncObjects(void*);
static void createSyncObjects()
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);

	for (u32 frameIndex = 0u; frameIndex < FRAME_IN_FLIGHT_COUNT; ++frameIndex)
	{
		VkSemaphoreCreateInfo renderFinishedSemaphore = {};
		renderFinishedSemaphore.sType				  = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VK_ASSERT(vkCreateSemaphore(
			g_vulkan->device, &renderFinishedSemaphore, MD_NULL, &g_vulkan->renderFinishedSemaphores[frameIndex]));

		VkSemaphoreCreateInfo imageAvailableSemaphore = {};
		imageAvailableSemaphore.sType				  = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VK_ASSERT(vkCreateSemaphore(
			g_vulkan->device, &imageAvailableSemaphore, MD_NULL, &g_vulkan->imageAvailableSemaphores[frameIndex]));

		VkFenceCreateInfo inFlightFenceCreateInfo = {};
		inFlightFenceCreateInfo.sType			  = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		inFlightFenceCreateInfo.flags			  = VK_FENCE_CREATE_SIGNALED_BIT;

		VK_ASSERT(
			vkCreateFence(g_vulkan->device, &inFlightFenceCreateInfo, MD_NULL, &g_vulkan->inFlightFences[frameIndex]));
	}

	mdReleaseStackPush(s_releaseStack, MD_NULL, deleteSyncObjects);
}

static void deleteSyncObjects(void* pData)
{
	MD_UNUSED(pData);
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);

	for (u32 frameIndex = 0u; frameIndex < FRAME_IN_FLIGHT_COUNT; ++frameIndex)
	{
		MD_ASSERT(g_vulkan->inFlightFences[frameIndex] != MD_NULL);
		MD_ASSERT(g_vulkan->renderFinishedSemaphores[frameIndex] != MD_NULL);
		MD_ASSERT(g_vulkan->imageAvailableSemaphores[frameIndex] != MD_NULL);

		vkDestroyFence(g_vulkan->device, g_vulkan->inFlightFences[frameIndex], MD_NULL);
		vkDestroySemaphore(g_vulkan->device, g_vulkan->renderFinishedSemaphores[frameIndex], MD_NULL);
		vkDestroySemaphore(g_vulkan->device, g_vulkan->imageAvailableSemaphores[frameIndex], MD_NULL);
	}
}

static void transitionImageLayout(VkImage			   image,
								  VkImageLayout		   oldLayout,
								  VkImageLayout		   newLayout,
								  VkAccessFlags		   srcAccessMask,
								  VkAccessFlags		   dstAccessMask,
								  VkPipelineStageFlags srcStage,
								  VkPipelineStageFlags dstStage);

void mdRenderStartFrame()
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->graphicsCommandBuffers != MD_NULL);

	vkWaitForFences(g_vulkan->device, 1, &g_vulkan->inFlightFences[g_vulkan->currentFrame], VK_TRUE, UINT64_MAX);
	VK_ASSERT(vkResetFences(g_vulkan->device, 1, &g_vulkan->inFlightFences[g_vulkan->currentFrame]));

	vkAcquireNextImageKHR(g_vulkan->device,
						  g_vulkan->swapchain,
						  UINT64_MAX,
						  g_vulkan->imageAvailableSemaphores[g_vulkan->currentFrame],
						  VK_NULL_HANDLE,
						  &g_vulkan->imageIndex);

	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VK_ASSERT(vkBeginCommandBuffer(g_vulkan->graphicsCommandBuffers[g_vulkan->currentFrame], &commandBufferBeginInfo));

	transitionImageLayout(g_vulkan->pSwapchainImages[g_vulkan->imageIndex],
						  VK_IMAGE_LAYOUT_UNDEFINED,
						  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						  VK_ACCESS_NONE,
						  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
						  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
						  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

	VkRenderingInfo* pRenderingInfo		 = &g_vulkan->renderingInfos[g_vulkan->currentFrame];
	pRenderingInfo->sType				 = VK_STRUCTURE_TYPE_RENDERING_INFO;
	pRenderingInfo->renderArea.offset.x	 = 0;
	pRenderingInfo->renderArea.offset.y	 = 0;
	pRenderingInfo->renderArea.extent	 = g_vulkan->extent;
	pRenderingInfo->layerCount			 = 1;
	pRenderingInfo->pColorAttachments	 = &g_vulkan->colorAttachmentInfos[g_vulkan->currentFrame];
	pRenderingInfo->colorAttachmentCount = 1;

	vkCmdBeginRendering(g_vulkan->graphicsCommandBuffers[g_vulkan->currentFrame], pRenderingInfo);

	VkViewport viewport = {};
	viewport.x			= 0.0f;
	viewport.y			= 0.0f;
	viewport.width		= (f32)g_vulkan->extent.width;
	viewport.height		= (f32)g_vulkan->extent.height;
	viewport.minDepth	= 0.0f;
	viewport.maxDepth	= 1.0f;

	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent	 = g_vulkan->extent;

	vkCmdSetViewport(g_vulkan->graphicsCommandBuffers[g_vulkan->currentFrame], 0, 1, &viewport);
	vkCmdSetScissor(g_vulkan->graphicsCommandBuffers[g_vulkan->currentFrame], 0, 1, &scissor);
}

void mdRenderClearScreen(struct MdColor color)
{
	MD_ASSERT(g_vulkan != MD_NULL);

	VkClearValue clearValue		= {};
	clearValue.color.float32[0] = color.r;
	clearValue.color.float32[1] = color.g;
	clearValue.color.float32[2] = color.b;
	clearValue.color.float32[3] = color.a;

	VkRenderingAttachmentInfo* attachmentInfo = &g_vulkan->colorAttachmentInfos[g_vulkan->currentFrame];
	attachmentInfo->sType					  = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	attachmentInfo->clearValue				  = clearValue;
	attachmentInfo->loadOp					  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentInfo->storeOp					  = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentInfo->imageView				  = g_vulkan->pSwapchainImageViews[g_vulkan->imageIndex];
	attachmentInfo->imageLayout				  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}

void mdRenderDraw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->graphicsCommandBuffers != MD_NULL);

	vkCmdDraw(g_vulkan->graphicsCommandBuffers[g_vulkan->currentFrame],
			  vertexCount,
			  instanceCount,
			  firstVertex,
			  firstInstance);
}

void mdRenderEndFrame()
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->graphicsCommandBuffers != MD_NULL);
	MD_ASSERT(g_vulkan->graphicsQueue != MD_NULL);

	vkCmdEndRendering(g_vulkan->graphicsCommandBuffers[g_vulkan->currentFrame]);

	transitionImageLayout(g_vulkan->pSwapchainImages[g_vulkan->imageIndex],
						  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						  VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
						  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
						  VK_ACCESS_NONE,
						  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
						  VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

	VK_ASSERT(vkEndCommandBuffer(g_vulkan->graphicsCommandBuffers[g_vulkan->currentFrame]));

	// Submit command buffer
	VkSemaphore waitSemaphores[] = {
		g_vulkan->imageAvailableSemaphores[g_vulkan->currentFrame],
	};

	VkSemaphore signalSemaphores[] = {
		g_vulkan->renderFinishedSemaphores[g_vulkan->currentFrame],
	};

	VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	};

	VkSubmitInfo submitInfo			= {};
	submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount	= MD_ARRAY_SIZE(waitSemaphores);
	submitInfo.pWaitSemaphores		= waitSemaphores;
	submitInfo.pWaitDstStageMask	= waitStages;
	submitInfo.commandBufferCount	= 1;
	submitInfo.pCommandBuffers		= &g_vulkan->graphicsCommandBuffers[g_vulkan->currentFrame];
	submitInfo.signalSemaphoreCount = MD_ARRAY_SIZE(signalSemaphores);
	submitInfo.pSignalSemaphores	= signalSemaphores;

	VK_ASSERT(vkQueueSubmit(g_vulkan->graphicsQueue, 1, &submitInfo, g_vulkan->inFlightFences[g_vulkan->currentFrame]));
}

void mdRenderPresent()
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->presentQueue != MD_NULL);

	VkSemaphore waitSemaphores[] = {
		g_vulkan->renderFinishedSemaphores[g_vulkan->currentFrame],
	};

	VkPresentInfoKHR presentInfo   = {};
	presentInfo.sType			   = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = MD_ARRAY_SIZE(waitSemaphores);
	presentInfo.pWaitSemaphores	   = waitSemaphores;
	presentInfo.swapchainCount	   = 1;
	presentInfo.pSwapchains		   = &g_vulkan->swapchain;
	presentInfo.pImageIndices	   = &g_vulkan->imageIndex;

	VK_ASSERT(vkQueuePresentKHR(g_vulkan->presentQueue, &presentInfo));

	g_vulkan->currentFrame = (g_vulkan->currentFrame + 1) % FRAME_IN_FLIGHT_COUNT;
}

static void transitionImageLayout(VkImage			   image,
								  VkImageLayout		   oldLayout,
								  VkImageLayout		   newLayout,
								  VkAccessFlags		   srcAccessMask,
								  VkAccessFlags		   dstAccessMask,
								  VkPipelineStageFlags srcStage,
								  VkPipelineStageFlags dstStage)
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->graphicsCommandBuffers != MD_NULL);

	VkCommandBuffer commandBuffer = g_vulkan->graphicsCommandBuffers[g_vulkan->currentFrame];

	VkImageMemoryBarrier barrier			= {};
	barrier.sType							= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout						= oldLayout;
	barrier.newLayout						= newLayout;
	barrier.srcAccessMask					= srcAccessMask;
	barrier.dstAccessMask					= dstAccessMask;
	barrier.srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
	barrier.image							= image;
	barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel	= 0;
	barrier.subresourceRange.levelCount		= 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount		= 1;

	vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, MD_NULL, 0, MD_NULL, 1, &barrier);
}

void mdRenderWaitIdle()
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);

	vkDeviceWaitIdle(g_vulkan->device);
}

#endif // MD_USE_VULKAN
