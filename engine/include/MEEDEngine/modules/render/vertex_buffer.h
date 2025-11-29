#pragma once

#if __cplusplus
extern "C" {
#endif
#include "MEEDEngine/platforms/common.h"

#define MEED_VERTEX_BUFFER_MAX_ATTRIBUTES 16

/**
 * List of all vertex buffer attribute types. The `layout` will use these types to
 * define the structure of each vertex in the buffer.
 */
enum MEEDVertexBufferAttributeType
{
	MEED_VERTEX_BUFFER_ATTRIBUTE_TYPE_FLOAT,
	MEED_VERTEX_BUFFER_ATTRIBUTE_TYPE_UNSIGNED_INT,
	MEED_VERTEX_BUFFER_ATTRIBUTE_TYPE_FLOAT2,
	MEED_VERTEX_BUFFER_ATTRIBUTE_TYPE_UNSIGNED_INT2,
	MEED_VERTEX_BUFFER_ATTRIBUTE_TYPE_FLOAT3,
	MEED_VERTEX_BUFFER_ATTRIBUTE_TYPE_UNSIGNED_INT3,
	MEED_VERTEX_BUFFER_ATTRIBUTE_TYPE_FLOAT4,
	MEED_VERTEX_BUFFER_ATTRIBUTE_TYPE_UNSIGNED_INT4,
};

/**
 * @brief Structure representing a vertex buffer.
 */
struct MEEDVertexBuffer
{
	void* pInternal; ///< Pointer to internal implementation-specific data (Vulkan, WebGL, etc.).

	u32 verticesCount;	 ///< Number of vertices in the buffer.
	u32 attributesCount; ///< Number of attributes per vertex.
	/**
	 * Array defining the layout of vertex attributes. Each entry corresponds to a type from the
	 */
	enum MEEDVertexBufferAttributeType layout[MEED_VERTEX_BUFFER_MAX_ATTRIBUTES];

	/**
	 * The stride (in bytes) between consecutive vertices in the buffer.
	 */
	u32 stride;
};

/**
 * @brief Creates a vertex buffer with the specified layout.
 *
 * @param layout An array defining the types of each attribute in the vertex.
 * @param attributesCount The number of attributes in the vertex.
 * @param verticesCount The number of vertices the buffer will hold.
 * @return A pointer to the created MEEDVertexBuffer.
 *
 * @note The data of the vertex buffer will be write later
 */
struct MEEDVertexBuffer*
meedVertexBufferCreate(enum MEEDVertexBufferAttributeType* layout, u32 attributesCount, u32 verticesCount);

/**
 * @brief Destroys the specified vertex buffer and releases its resources.
 *
 * @param pVertexBuffer A pointer to the MEEDVertexBuffer to destroy. if NULL raise an exception.
 */
void meedVertexBufferDestroy(struct MEEDVertexBuffer* pVertexBuffer);

#if __cplusplus
}
#endif
