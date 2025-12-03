#if MD_USE_VULKAN

#include "MEEDEngine/modules/render/vertex_buffer.h"
#include "vulkan_common.h"

static void freeVulkanVertexBuffer(void*);

static void createBuffer(VkBuffer* pBuffer, u32 bufferSize, i32 queueFamily, VkBufferUsageFlags usages);
static void deleteBuffer(void*);

static void
allocateMemory(VkDeviceMemory* pMemory, VkMemoryRequirements* pMemoryRequirements, VkMemoryPropertyFlags properties);
static void freeMemory(void*);

static VkFormat getVulkanFormatFromAttributeType(enum MdVertexBufferAttributeType type);

static void createStaticVertexBuffer(struct MdVertexBuffer* pVertexBuffer);
static void createDynamicVertexBuffer(struct MdVertexBuffer* pVertexBuffer);

struct MdVertexBuffer* mdVertexBufferCreate(enum MdVertexBufferAttributeType* layout,
											u32								  attributesCount,
											u32								  verticesCount,
											MdVertexBufferWriteCallback		  writeCallback,
											enum MdVertexBufferType			  bufferType)
{
	struct MdVertexBuffer* pVertexBuffer = MD_MALLOC(struct MdVertexBuffer);
	mdMemorySet(pVertexBuffer, 0, sizeof(struct MdVertexBuffer));
	MD_ASSERT_MSG(pVertexBuffer != NULL, "Failed to allocate memory for MdVertexBuffer");

	pVertexBuffer->verticesCount   = verticesCount;
	pVertexBuffer->attributesCount = attributesCount;
	pVertexBuffer->bufferType	   = bufferType;

	MD_ASSERT_MSG(writeCallback != NULL, "Vertex buffer write callback cannot be NULL");
	pVertexBuffer->writeCallback = writeCallback;

	pVertexBuffer->pReleaseStack = mdReleaseStackCreate();

	pVertexBuffer->pInternal = MD_MALLOC(struct VulkanVertexBuffer);
	mdMemorySet(pVertexBuffer->pInternal, 0, sizeof(struct VulkanVertexBuffer));
	MD_ASSERT_MSG(pVertexBuffer->pInternal != NULL, "Failed to allocate memory for VulkanVertexBuffer");
	mdReleaseStackPush(pVertexBuffer->pReleaseStack, pVertexBuffer, freeVulkanVertexBuffer);

	struct VulkanVertexBuffer* pVulkanVertexBuffer = (struct VulkanVertexBuffer*)pVertexBuffer->pInternal;

	// Copy the layout
	u32 stride = 0u;
	for (u32 attributeIndex = 0u; attributeIndex < attributesCount; ++attributeIndex)
	{
		enum MdVertexBufferAttributeType attributeType = layout[attributeIndex];
		pVertexBuffer->layout[attributeIndex]		   = attributeType;

		// Setup attribute descriptions
		VkVertexInputAttributeDescription* pAttrDesc = &pVulkanVertexBuffer->attributeDescriptions[attributeIndex];

		pAttrDesc->binding	= 0;
		pAttrDesc->location = attributeIndex;
		pAttrDesc->format	= getVulkanFormatFromAttributeType(attributeType);
		pAttrDesc->offset	= stride;

		stride += mdGetVertexAttributeTypeSize(attributeType);
	}
	pVertexBuffer->stride	  = stride;
	pVertexBuffer->bufferSize = stride * verticesCount;

	pVulkanVertexBuffer->bindingDescription.binding	  = 0;
	pVulkanVertexBuffer->bindingDescription.stride	  = pVertexBuffer->stride;
	pVulkanVertexBuffer->bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	MD_ASSERT(g_vulkan != MD_NULL);

	switch (pVertexBuffer->bufferType)
	{
	case MD_VERTEX_BUFFER_TYPE_STATIC:
		createStaticVertexBuffer(pVertexBuffer);
		break;

	case MD_VERTEX_BUFFER_TYPE_DYNAMIC:
		createDynamicVertexBuffer(pVertexBuffer);
		break;

	default:
		MD_UNTOUCHABLE();
		break;
	}

	return pVertexBuffer;
}

void mdVertexBufferBind(struct MdVertexBuffer* pVertexBuffer)
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(pVertexBuffer != MD_NULL);
	MD_ASSERT(pVertexBuffer->pInternal != MD_NULL);

	VkCommandBuffer commandBuffer = g_vulkan->graphicsCommandBuffers[g_vulkan->currentFrame];

	struct VulkanVertexBuffer* pVulkanVertexBuffer = (struct VulkanVertexBuffer*)pVertexBuffer->pInternal;

	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &pVulkanVertexBuffer->buffer, &offset);
}

static void writeDataStatic(struct MdVertexBuffer* pVertexBuffer, const void* pData);
static void writeDataDynamic(struct MdVertexBuffer* pVertexBuffer, const void* pData);

u32 mdVertexBufferWrite(struct MdVertexBuffer* pVertexBuffer, const void* pData)
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);
	MD_ASSERT(pVertexBuffer != MD_NULL);

	switch (pVertexBuffer->bufferType)
	{
	case MD_VERTEX_BUFFER_TYPE_STATIC:
		writeDataStatic(pVertexBuffer, pData);
		break;
	case MD_VERTEX_BUFFER_TYPE_DYNAMIC:
		writeDataDynamic(pVertexBuffer, pData);
		break;
	default:
		MD_UNTOUCHABLE();
		break;
	}

	pVertexBuffer->currentMemoryOffset += pVertexBuffer->stride;
	pVertexBuffer->currentMemoryOffset %= pVertexBuffer->bufferSize;

	return pVertexBuffer->currentMemoryOffset;
}

static void writeDataStatic(struct MdVertexBuffer* pVertexBuffer, const void* pData)
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);
	MD_ASSERT(g_vulkan->transferCommandBuffer != MD_NULL);
	MD_ASSERT(pVertexBuffer != MD_NULL);
	MD_ASSERT(pVertexBuffer->pInternal != MD_NULL);

	struct VulkanVertexBuffer* pVulkanVertexBuffer = (struct VulkanVertexBuffer*)pVertexBuffer->pInternal;

	u8* bufferAddr = MD_NULL;

	VK_ASSERT(vkMapMemory(g_vulkan->device,
						  pVulkanVertexBuffer->stagingBufferMemory,
						  pVertexBuffer->currentMemoryOffset,
						  pVertexBuffer->stride,
						  0,
						  (void**)&bufferAddr));

	MD_ASSERT(bufferAddr != MD_NULL);

	pVertexBuffer->writeCallback(bufferAddr, pData);

	vkUnmapMemory(g_vulkan->device, pVulkanVertexBuffer->stagingBufferMemory);

	// Copy from staging buffer to vertex buffer
	if (pVertexBuffer->currentMemoryOffset + pVertexBuffer->stride >= pVertexBuffer->bufferSize)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType					   = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags					   = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VK_ASSERT(vkBeginCommandBuffer(g_vulkan->transferCommandBuffer, &beginInfo));

		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset	= 0;
		copyRegion.dstOffset	= 0;
		copyRegion.size			= pVertexBuffer->bufferSize;
		vkCmdCopyBuffer(g_vulkan->transferCommandBuffer,
						pVulkanVertexBuffer->stagingBuffer,
						pVulkanVertexBuffer->buffer,
						1,
						&copyRegion);

		VK_ASSERT(vkEndCommandBuffer(g_vulkan->transferCommandBuffer));

		VkSubmitInfo submitInfo		  = {};
		submitInfo.sType			  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers	  = &g_vulkan->transferCommandBuffer;
		VK_ASSERT(vkQueueSubmit(g_vulkan->transferQueue, 1, &submitInfo, VK_NULL_HANDLE));
		vkQueueWaitIdle(g_vulkan->transferQueue);
	}
}

static void writeDataDynamic(struct MdVertexBuffer* pVertexBuffer, const void* pData)
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);
	MD_ASSERT(pVertexBuffer != MD_NULL);
	MD_ASSERT(pVertexBuffer->pInternal != MD_NULL);
	struct VulkanVertexBuffer* pVulkanVertexBuffer = (struct VulkanVertexBuffer*)pVertexBuffer->pInternal;

	u8* bufferAddr = MD_NULL;

	VK_ASSERT(vkMapMemory(g_vulkan->device,
						  pVulkanVertexBuffer->bufferMemory,
						  pVertexBuffer->currentMemoryOffset,
						  pVertexBuffer->stride,
						  0,
						  (void**)&bufferAddr));

	MD_ASSERT(bufferAddr != MD_NULL);

	pVertexBuffer->writeCallback(bufferAddr, pData);

	vkUnmapMemory(g_vulkan->device, pVulkanVertexBuffer->bufferMemory);
}

void mdVertexBufferDestroy(struct MdVertexBuffer* pVertexBuffer)
{
	MD_ASSERT(pVertexBuffer != MD_NULL);
	mdReleaseStackDestroy(pVertexBuffer->pReleaseStack);
	MD_FREE(pVertexBuffer, struct MdVertexBuffer);
	pVertexBuffer = MD_NULL;
}

static void freeVulkanVertexBuffer(void* pData)
{
	MD_ASSERT(pData != MD_NULL);
	struct MdVertexBuffer*	   pVertexBuffer	   = (struct MdVertexBuffer*)pData;
	struct VulkanVertexBuffer* pVulkanVertexBuffer = (struct VulkanVertexBuffer*)pVertexBuffer->pInternal;

	MD_FREE(pVulkanVertexBuffer, struct VulkanVertexBuffer);
	pVertexBuffer->pInternal = MD_NULL;
}

static void createBuffer(VkBuffer* pBuffer, u32 bufferSize, i32 queueFamily, VkBufferUsageFlags usages)
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);

	VkBufferCreateInfo bufferInfo	 = {};
	bufferInfo.sType				 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size					 = bufferSize;
	bufferInfo.usage				 = usages;
	bufferInfo.pQueueFamilyIndices	 = &queueFamily;
	bufferInfo.queueFamilyIndexCount = 1;

	VK_ASSERT(vkCreateBuffer(g_vulkan->device, &bufferInfo, MD_NULL, pBuffer));
}

static void deleteBuffer(void* pBuffer)
{
	MD_ASSERT(pBuffer != MD_NULL);
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);

	vkDestroyBuffer(g_vulkan->device, *(VkBuffer*)pBuffer, MD_NULL);
}

static i32 findMemoryTypeIndex(u32 typeFilter, VkMemoryPropertyFlags properties);

static void
allocateMemory(VkDeviceMemory* pMemory, VkMemoryRequirements* pMemoryRequirements, VkMemoryPropertyFlags properties)
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);
	MD_ASSERT(pMemoryRequirements != MD_NULL);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType				   = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize	   = pMemoryRequirements->size;
	allocInfo.memoryTypeIndex	   = findMemoryTypeIndex(pMemoryRequirements->memoryTypeBits, properties);

	VK_ASSERT(vkAllocateMemory(g_vulkan->device, &allocInfo, MD_NULL, pMemory));
}

static i32 findMemoryTypeIndex(u32 typeFilter, VkMemoryPropertyFlags properties)
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->physicalDevice != MD_NULL);

	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(g_vulkan->physicalDevice, &memoryProperties);

	for (u32 propertyIndex = 0u; propertyIndex < memoryProperties.memoryTypeCount; ++propertyIndex)
	{
		if ((typeFilter & (1 << propertyIndex)) &&
			(memoryProperties.memoryTypes[propertyIndex].propertyFlags & properties) == properties)
		{
			return propertyIndex;
		}
	}

	MD_UNTOUCHABLE();
	return -1;
}

static void freeMemory(void* pMemory)
{
	MD_ASSERT(pMemory != MD_NULL);
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);

	vkFreeMemory(g_vulkan->device, *(VkDeviceMemory*)pMemory, MD_NULL);
}

static VkFormat getVulkanFormatFromAttributeType(enum MdVertexBufferAttributeType type)
{
	switch (type)
	{
	case MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_FLOAT:
		return VK_FORMAT_R32_SFLOAT;
	case MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_UNSIGNED_INT:
		return VK_FORMAT_R32_UINT;
	case MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_FLOAT2:
		return VK_FORMAT_R32G32_SFLOAT;
	case MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_UNSIGNED_INT2:
		return VK_FORMAT_R32G32_UINT;
	case MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_FLOAT3:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_UNSIGNED_INT3:
		return VK_FORMAT_R32G32B32_UINT;
	case MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_FLOAT4:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_UNSIGNED_INT4:
		return VK_FORMAT_R32G32B32A32_UINT;
	default:
		MD_UNTOUCHABLE();
		return VK_FORMAT_UNDEFINED;
	}
}

static void createStaticVertexBuffer(struct MdVertexBuffer* pVertexBuffer)
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);
	MD_ASSERT(pVertexBuffer != MD_NULL);
	MD_ASSERT(pVertexBuffer->pInternal != MD_NULL);

	struct VulkanVertexBuffer* pVulkanVertexBuffer = (struct VulkanVertexBuffer*)pVertexBuffer->pInternal;

	createBuffer(&pVulkanVertexBuffer->stagingBuffer,
				 pVertexBuffer->bufferSize,
				 g_vulkan->queueFamilies.graphicsFamily,
				 VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	MD_ASSERT(pVulkanVertexBuffer->stagingBuffer != VK_NULL_HANDLE);
	mdReleaseStackPush(pVertexBuffer->pReleaseStack, &pVulkanVertexBuffer->stagingBuffer, deleteBuffer);

	VkMemoryRequirements stagingMemoryRequirements;
	vkGetBufferMemoryRequirements(g_vulkan->device, pVulkanVertexBuffer->stagingBuffer, &stagingMemoryRequirements);

	allocateMemory(&pVulkanVertexBuffer->stagingBufferMemory,
				   &stagingMemoryRequirements,
				   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	MD_ASSERT(pVulkanVertexBuffer->stagingBufferMemory != VK_NULL_HANDLE);
	mdReleaseStackPush(pVertexBuffer->pReleaseStack, &pVulkanVertexBuffer->stagingBufferMemory, freeMemory);

	VK_ASSERT(vkBindBufferMemory(
		g_vulkan->device, pVulkanVertexBuffer->stagingBuffer, pVulkanVertexBuffer->stagingBufferMemory, 0));

	createBuffer(&pVulkanVertexBuffer->buffer,
				 pVertexBuffer->bufferSize,
				 g_vulkan->queueFamilies.graphicsFamily,
				 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	MD_ASSERT(pVulkanVertexBuffer->buffer != VK_NULL_HANDLE);
	mdReleaseStackPush(pVertexBuffer->pReleaseStack, &pVulkanVertexBuffer->buffer, deleteBuffer);

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(g_vulkan->device, pVulkanVertexBuffer->buffer, &memoryRequirements);
	allocateMemory(&pVulkanVertexBuffer->bufferMemory, &memoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	MD_ASSERT(pVulkanVertexBuffer->bufferMemory != VK_NULL_HANDLE);
	mdReleaseStackPush(pVertexBuffer->pReleaseStack, &pVulkanVertexBuffer->bufferMemory, freeMemory);

	VK_ASSERT(vkBindBufferMemory(g_vulkan->device, pVulkanVertexBuffer->buffer, pVulkanVertexBuffer->bufferMemory, 0));
}

static void createDynamicVertexBuffer(struct MdVertexBuffer* pVertexBuffer)
{
	MD_ASSERT(g_vulkan != MD_NULL);
	MD_ASSERT(g_vulkan->device != MD_NULL);
	MD_ASSERT(pVertexBuffer != MD_NULL);
	MD_ASSERT(pVertexBuffer->pInternal != MD_NULL);

	struct VulkanVertexBuffer* pVulkanVertexBuffer = (struct VulkanVertexBuffer*)pVertexBuffer->pInternal;

	createBuffer(&pVulkanVertexBuffer->buffer,
				 pVertexBuffer->bufferSize,
				 g_vulkan->queueFamilies.graphicsFamily,
				 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	MD_ASSERT(pVulkanVertexBuffer->buffer != VK_NULL_HANDLE);
	mdReleaseStackPush(pVertexBuffer->pReleaseStack, &pVulkanVertexBuffer->buffer, deleteBuffer);

	MD_ASSERT(g_vulkan->device != MD_NULL);

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(g_vulkan->device, pVulkanVertexBuffer->buffer, &memoryRequirements);

	allocateMemory(&pVulkanVertexBuffer->bufferMemory,
				   &memoryRequirements,
				   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	MD_ASSERT(pVulkanVertexBuffer->bufferMemory != VK_NULL_HANDLE);
	mdReleaseStackPush(pVertexBuffer->pReleaseStack, &pVulkanVertexBuffer->bufferMemory, freeMemory);

	VK_ASSERT(vkBindBufferMemory(g_vulkan->device, pVulkanVertexBuffer->buffer, pVulkanVertexBuffer->bufferMemory, 0));
}

#endif // MD_USE_VULKAN