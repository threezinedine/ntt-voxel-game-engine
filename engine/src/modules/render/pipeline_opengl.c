#if MEED_USE_OPENGL

#include "MEEDEngine/modules/release_stack/release_stack.h"
#include "MEEDEngine/modules/render/pipeline.h"
#include "MEEDEngine/modules/render/shader.h"
#include "opengl_common.h"

void freeInternalOpenGLPipeline(void* pData)
{
	MEED_ASSERT(pData != MEED_NULL);
	struct MEEDPipeline* pPipeline = (struct MEEDPipeline*)pData;

	MEED_FREE(pPipeline->pInternal, struct OpenGLPipeline);
}

static void deleteProgram(void* pData)
{
	MEED_ASSERT(pData != MEED_NULL);

	struct MEEDPipeline*   pPipeline	   = (struct MEEDPipeline*)pData;
	struct OpenGLPipeline* pOpenGLPipeline = (struct OpenGLPipeline*)pPipeline->pInternal;
	GL_ASSERT(glDeleteProgram(pOpenGLPipeline->shaderProgram));
}

struct MEEDPipeline* meedPipelineCreate(const char* vertexShaderPath, const char* fragmentShaderPath)
{
	struct MEEDPipeline* pPipeline = MEED_MALLOC(struct MEEDPipeline);
	MEED_ASSERT(pPipeline != MEED_NULL);
	meedPlatformMemorySet(pPipeline, 0, sizeof(struct MEEDPipeline));

	pPipeline->pReleaseStack	  = meedReleaseStackCreate();
	pPipeline->vertexShaderPath	  = vertexShaderPath;
	pPipeline->fragmentShaderPath = fragmentShaderPath;

	pPipeline->pInternal = MEED_MALLOC(struct OpenGLPipeline);
	MEED_ASSERT(pPipeline->pInternal != MEED_NULL);
	meedPlatformMemorySet(pPipeline->pInternal, 0, sizeof(struct OpenGLPipeline));
	meedReleaseStackPush(pPipeline->pReleaseStack, pPipeline, freeInternalOpenGLPipeline);

	struct OpenGLPipeline* pOpenGLPipeline = (struct OpenGLPipeline*)pPipeline->pInternal;

	GL_ASSERT(pOpenGLPipeline->shaderProgram = glCreateProgram());

	struct MEEDShader* pVertexShader   = meedShaderCreate(MEED_SHADER_TYPE_VERTEX, vertexShaderPath);
	struct MEEDShader* pFragmentShader = meedShaderCreate(MEED_SHADER_TYPE_FRAGMENT, fragmentShaderPath);

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
		MEED_ASSERT_MSG(MEED_FALSE, "Shader program linking failed: %s", infoLog);
	}

	meedShaderDestroy(pFragmentShader);
	meedShaderDestroy(pVertexShader);

	meedReleaseStackPush(pPipeline->pReleaseStack, pPipeline, deleteProgram);

	return pPipeline;
}

void meedPipelineUse(struct MEEDPipeline* pPipeline)
{
	MEED_ASSERT(pPipeline != MEED_NULL);
	struct OpenGLPipeline* pOpenGLPipeline = (struct OpenGLPipeline*)pPipeline->pInternal;
	GL_ASSERT(glUseProgram(pOpenGLPipeline->shaderProgram));
}

void meedPipelineDestroy(struct MEEDPipeline* pPipeline)
{
	MEED_ASSERT(pPipeline != MEED_NULL);

	meedReleaseStackDestroy(pPipeline->pReleaseStack);
	MEED_FREE(pPipeline, struct MEEDPipeline);
	pPipeline = MEED_NULL;
}

#endif // MEED_USE_OPENGL