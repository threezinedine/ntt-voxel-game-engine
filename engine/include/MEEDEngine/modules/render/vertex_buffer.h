#pragma once

#if __cplusplus
extern "C" {
#endif
#include "MEEDEngine/core/core.h"
#include "MEEDEngine/modules/release_stack/release_stack.h"
#include "MEEDEngine/platforms/common.h"

#define MEED_VERTEX_BUFFER_MAX_ATTRIBUTES 16

/**
 * List of all vertex buffer attribute types. The `layout` will use these types to
 * define the structure of each vertex in the buffer.
 */
enum MdVertexBufferAttributeType
{
	MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_FLOAT,
	MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_UNSIGNED_INT,
	MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_FLOAT2,
	MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_UNSIGNED_INT2,
	MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_FLOAT3,
	MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_UNSIGNED_INT3,
	MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_FLOAT4,
	MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_UNSIGNED_INT4,
};

/**
 * List of all vertex buffer types.
 */
enum MdVertexBufferType
{
	MD_VERTEX_BUFFER_TYPE_STATIC,  ///< The vertex buffer is static and will not change often.
	MD_VERTEX_BUFFER_TYPE_DYNAMIC, ///< Should be used for vertex buffers that change frequently (every frame).
};

/**
 * The method which is used for receiving the vertex data and write it into the buffer which will
 * 	be sent to the GPU, the offset will be updated automatically after each write. The write data must be
 * 	exactly equal to the size of the vertex defined by the layout else errors may occur (no checks are done).
 */
typedef void (*MdVertexBufferWriteCallback)(u8* pDest, const void* pSrc);

/**
 * @brief Structure representing a vertex buffer.
 */
struct MdVertexBuffer
{
	void* pInternal; ///< Pointer to internal implementation-specific data (Vulkan, WebGL, etc.).

	u32							verticesCount;	 ///< Number of vertices in the buffer.
	u32							attributesCount; ///< Number of attributes per vertex.
	u32							stride;			 ///< The stride (in bytes) between consecutive vertices in the buffer.
	u32							bufferSize;		 ///< Total size of the vertex buffer in bytes.
	MdVertexBufferWriteCallback writeCallback;	 ///< Callback function for writing vertex data.
	enum MdVertexBufferType		bufferType;		 ///< Type of the vertex buffer (static or dynamic).

	// Caching
	u32 currentMemoryOffset; ///< Current memory offset for writing vertex data. If exceeded, return to 0.

	/**
	 * Array defining the layout of vertex attributes. Each entry corresponds to a type from the
	 */
	enum MdVertexBufferAttributeType layout[MEED_VERTEX_BUFFER_MAX_ATTRIBUTES];

	struct MdReleaseStack* pReleaseStack; ///< Resource release stack.
};

/**
 * @brief Creates a vertex buffer with the specified layout.
 *
 * @param layout An array defining the types of each attribute in the vertex.
 * @param attributesCount The number of attributes in the vertex.
 * @param verticesCount The number of vertices the buffer will hold.
 * @param writeCallback A callback function to write vertex data into the buffer, cannot be NULL.
 * @param bufferType The type of the vertex buffer (static or dynamic).
 * @return A pointer to the created MdVertexBuffer.
 *
 * @note The data of the vertex buffer will be write later
 */
struct MdVertexBuffer* mdVertexBufferCreate(enum MdVertexBufferAttributeType* layout,
											u32								  attributesCount,
											u32								  verticesCount,
											MdVertexBufferWriteCallback		  writeCallback,
											enum MdVertexBufferType			  bufferType);

/**
 * Trigger that binds the specified vertex buffer for rendering.
 * @param pVertexBuffer A pointer to the MdVertexBuffer to bind. if NULL, unbind the current buffer.
 */
void mdVertexBufferBind(struct MdVertexBuffer* pVertexBuffer);

/**
 * @brief Writes vertex data into the specified vertex buffer at the current offset.
 *
 * @param pVertexBuffer A pointer to the MdVertexBuffer to write to. if NULL raise an exception.
 * @param pData A pointer to the vertex data to write.
 *
 * @return The current offset after writing the data.
 *
 * @note With the static buffer type, if the buffer is full, the data will be transferred to the GPU and the offset
 * reset to 0. With the dynamic buffer type, if the buffer is full, the offset will simply reset to 0.
 */
u32 mdVertexBufferWrite(struct MdVertexBuffer* pVertexBuffer, const void* pData);

/**
 * @brief Destroys the specified vertex buffer and releases its resources.
 *
 * @param pVertexBuffer A pointer to the MdVertexBuffer to destroy. if NULL raise an exception.
 */
void mdVertexBufferDestroy(struct MdVertexBuffer* pVertexBuffer);

#if __cplusplus
}
#endif
