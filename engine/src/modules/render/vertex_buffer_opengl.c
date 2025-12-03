#if MD_USE_OPENGL
#include "MEEDEngine/modules/render/vertex_buffer.h"
#include "opengl_common.h"
#include "render_common.h"

static void	  freeVertexBuffer(void*);
static GLenum getAttributeTypeOpenGLBaseType(enum MdVertexBufferAttributeType attributeType);

struct MdVertexBuffer* mdVertexBufferCreate(enum MdVertexBufferAttributeType* layout,
											u32								  attributesCount,
											u32								  verticesCount,
											MdVertexBufferWriteCallback		  writeCallback)
{
	struct MdVertexBuffer* pVertexBuffer = MD_MALLOC(struct MdVertexBuffer);
	MD_ASSERT(pVertexBuffer != MD_NULL);
	mdMemorySet(pVertexBuffer, 0, sizeof(struct MdVertexBuffer));

	MD_ASSERT_MSG(writeCallback != MD_NULL, "Vertex buffer write callback cannot be NULL");

	pVertexBuffer->pReleaseStack   = mdReleaseStackCreate();
	pVertexBuffer->attributesCount = attributesCount;
	pVertexBuffer->verticesCount   = verticesCount;
	pVertexBuffer->writeCallback   = writeCallback;

	mdMemoryCopy(pVertexBuffer->layout, layout, sizeof(enum MdVertexBufferAttributeType) * attributesCount);

	pVertexBuffer->pInternal = MD_MALLOC(struct OpenGLVertexBuffer);
	MD_ASSERT(pVertexBuffer->pInternal != MD_NULL);
	mdMemorySet(pVertexBuffer->pInternal, 0, sizeof(struct OpenGLVertexBuffer));
	mdReleaseStackPush(pVertexBuffer->pReleaseStack, pVertexBuffer, freeVertexBuffer);

	struct OpenGLVertexBuffer* pOpenGLVertexBuffer = (struct OpenGLVertexBuffer*)pVertexBuffer->pInternal;
	GL_ASSERT(glGenBuffers(1, &pOpenGLVertexBuffer->vboID));
	GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, pOpenGLVertexBuffer->vboID));

	// Calculate stride
	pVertexBuffer->stride = 0;

	for (u32 attributIndex = 0u; attributIndex < attributesCount; ++attributIndex)
	{
		pVertexBuffer->stride += mdGetVertexAttributeTypeSize(layout[attributIndex]);
	}
	pVertexBuffer->bufferSize = pVertexBuffer->stride * verticesCount;

	GL_ASSERT(glBufferData(GL_ARRAY_BUFFER, pVertexBuffer->bufferSize, NULL, GL_DYNAMIC_DRAW));

	GL_ASSERT(glGenVertexArrays(1, &pOpenGLVertexBuffer->vaoID));
	GL_ASSERT(glBindVertexArray(pOpenGLVertexBuffer->vaoID));

	// Setup vertex attributes
	u32 offset = 0;
	for (u32 attributeIndex = 0u; attributeIndex < attributesCount; ++attributeIndex)
	{
		GL_ASSERT(glVertexAttribPointer(attributeIndex,
										mdGetVertexAttributeTypeCount(layout[attributeIndex]),
										getAttributeTypeOpenGLBaseType(layout[attributeIndex]),
										GL_FALSE,
										pVertexBuffer->stride,
										(const void*)(uintptr_t)offset));
		GL_ASSERT(glEnableVertexAttribArray(attributeIndex));
		offset += 0;
	}

	GL_ASSERT(glBindVertexArray(0));

	return pVertexBuffer;
}

static void freeVertexBuffer(void* pData)
{
	MD_ASSERT(pData != MD_NULL);
	struct MdVertexBuffer* pVertexBuffer = (struct MdVertexBuffer*)pData;

	MD_FREE(pVertexBuffer->pInternal, struct OpenGLVertexBuffer);
}

void mdVertexBufferBind(struct MdVertexBuffer* pVertexBuffer)
{
	if (pVertexBuffer == MD_NULL)
	{
		GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, 0));
		return;
	}

	struct OpenGLVertexBuffer* pOpenGLVertexBuffer = (struct OpenGLVertexBuffer*)pVertexBuffer->pInternal;
	GL_ASSERT(glBindVertexArray(pOpenGLVertexBuffer->vaoID));
}

u32 mdVertexBufferWrite(struct MdVertexBuffer* pVertexBuffer, const void* pData)
{
	MD_ASSERT(pVertexBuffer != MD_NULL);
	MD_ASSERT(pData != MD_NULL);

	struct OpenGLVertexBuffer* pOpenGLVertexBuffer = (struct OpenGLVertexBuffer*)pVertexBuffer->pInternal;
	GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, pOpenGLVertexBuffer->vboID));

	u8* pTransferBuffer = (u8*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	MD_ASSERT_MSG(pTransferBuffer != MD_NULL, "Failed to map vertex buffer for writing.");
	pVertexBuffer->writeCallback(pTransferBuffer + pVertexBuffer->currentMemoryOffset, pData);
	GL_ASSERT(glUnmapBuffer(GL_ARRAY_BUFFER));

	pVertexBuffer->currentMemoryOffset += pVertexBuffer->stride;
	return pVertexBuffer->currentMemoryOffset;
}

void mdVertexBufferDestroy(struct MdVertexBuffer* pVertexBuffer)
{
	MD_ASSERT(pVertexBuffer != MD_NULL);
	mdReleaseStackDestroy(pVertexBuffer->pReleaseStack);
	MD_FREE(pVertexBuffer, struct MdVertexBuffer);
}

static GLenum getAttributeTypeOpenGLBaseType(enum MdVertexBufferAttributeType attributeType)
{
	switch (attributeType)
	{
	case MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_FLOAT:
	case MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_FLOAT2:
	case MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_FLOAT3:
	case MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_FLOAT4:
		return GL_FLOAT;
	case MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_UNSIGNED_INT:
	case MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_UNSIGNED_INT2:
	case MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_UNSIGNED_INT3:
	case MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_UNSIGNED_INT4:
		return GL_UNSIGNED_INT;
	default:
		MD_UNTOUCHABLE();
		return GL_INVALID_ENUM;
	}
}

#endif // MD_USE_OPENGL