#pragma once

#define VK_ASSERT(call)                                                                                                \
	do                                                                                                                 \
	{                                                                                                                  \
		VkResult result = call;                                                                                        \
		MEED_ASSERT_MSG(result == VK_SUCCESS, "Vulkan call failed with error code: %d", result);                       \
	} while (0)
