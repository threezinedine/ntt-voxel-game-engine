#if MD_USE_OPENGL

#include "MEEDEngine/modules/release_stack/release_stack.h"
#include "MEEDEngine/modules/render/pipeline.h"
#include "MEEDEngine/modules/render/shader.h"
#include "opengl_common.h"

void freeInternalOpenGLPipeline(void* pData)
{
	MD_ASSERT(pData != MD_NULL);
	struct MdPipeline* pPipeline = (struct MdPipeline*)pData;

	MD_FREE(pPipeline->pInternal, struct OpenGLPipeline);
}

static void deleteProgram(void* pData)
{
	MD_ASSERT(pData != MD_NULL);

	struct MdPipeline*	   pPipeline	   = (struct MdPipeline*)pData;
	struct OpenGLPipeline* pOpenGLPipeline = (struct OpenGLPipeline*)pPipeline->pInternal;
	GL_ASSERT(glDeleteProgram(pOpenGLPipeline->shaderProgram));
}

struct MdPipeline*
mdPipelineCreate(const char* vertexShaderPath, const char* fragmentShaderPath, struct MdVertexBuffer* pDesc)
{
	struct MdPipeline* pPipeline = MD_MALLOC(struct MdPipeline);
	MD_ASSERT(pPipeline != MD_NULL);
	mdMemorySet(pPipeline, 0, sizeof(struct MdPipeline));

	pPipeline->pReleaseStack	  = mdReleaseStackCreate();
	pPipeline->vertexShaderPath	  = vertexShaderPath;
	pPipeline->fragmentShaderPath = fragmentShaderPath;

	pPipeline->pInternal = MD_MALLOC(struct OpenGLPipeline);
	MD_ASSERT(pPipeline->pInternal != MD_NULL);
	mdMemorySet(pPipeline->pInternal, 0, sizeof(struct OpenGLPipeline));
	mdReleaseStackPush(pPipeline->pReleaseStack, pPipeline, freeInternalOpenGLPipeline);

	struct OpenGLPipeline* pOpenGLPipeline = (struct OpenGLPipeline*)pPipeline->pInternal;

	GL_ASSERT(pOpenGLPipeline->shaderProgram = glCreateProgram());

	struct MdShader* pVertexShader	 = mdShaderCreate(MD_SHADER_TYPE_VERTEX, vertexShaderPath);
	struct MdShader* pFragmentShader = mdShaderCreate(MD_SHADER_TYPE_FRAGMENT, fragmentShaderPath);

	struct OpenGLShader* pOpenGLVertexShader   = (struct OpenGLShader*)pVertexShader->pInternal;
	struct OpenGLShader* pOpenGLFragmentShader = (struct OpenGLShader*)pFragmentShader->pInternal;

	GL_ASSERT(glAttachShader(pOpenGLPipeline->shaderProgram, pOpenGLVertexShader->shaderID));
	GL_ASSERT(glAttachShader(pOpenGLPipeline->shaderProgram, pOpenGLFragmentShader->shaderID));
	GL_ASSERT(glLinkProgram(pOpenGLPipeline->shaderProgram));

	u32 linkStatus;
	glGetProgramiv(pOpenGLPipeline->shaderProgram, GL_LINK_STATUS, (GLint*)&linkStatus);
	if (linkStatus != GL_TRUE)
	{
		char infoLog[512];
		glGetProgramInfoLog(pOpenGLPipeline->shaderProgram, 512, NULL, infoLog);
		MD_ASSERT_MSG(MD_FALSE, "Shader program linking failed: %s", infoLog);
	}

	mdShaderDestroy(pFragmentShader);
	mdShaderDestroy(pVertexShader);

	mdReleaseStackPush(pPipeline->pReleaseStack, pPipeline, deleteProgram);

	return pPipeline;
}

void mdPipelineUse(struct MdPipeline* pPipeline)
{
	MD_ASSERT(pPipeline != MD_NULL);
	struct OpenGLPipeline* pOpenGLPipeline = (struct OpenGLPipeline*)pPipeline->pInternal;
	GL_ASSERT(glUseProgram(pOpenGLPipeline->shaderProgram));
}

void mdPipelineDestroy(struct MdPipeline* pPipeline)
{
	MD_ASSERT(pPipeline != MD_NULL);

	mdReleaseStackDestroy(pPipeline->pReleaseStack);
	MD_FREE(pPipeline, struct MdPipeline);
	pPipeline = MD_NULL;
}

#endif // MD_USE_OPENGL