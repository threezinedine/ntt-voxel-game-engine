#if MEED_USE_OPENGL

#include "MEEDEngine/modules/render/shader.h"
#include "opengl_common.h"

struct MEEDShader* meedShaderCreate(enum MEEDShaderType type, const char* shaderSource)
{
	struct MEEDShader* pShader = MEED_MALLOC(struct MEEDShader);
	MEED_ASSERT(pShader != MEED_NULL);
	meedPlatformMemorySet(pShader, 0, sizeof(struct MEEDShader));

	pShader->type = type;

	pShader->pInternal = MEED_MALLOC(struct OpenGLShader);
	MEED_ASSERT(pShader->pInternal != MEED_NULL);
	meedPlatformMemorySet(pShader->pInternal, 0, sizeof(struct OpenGLShader));

	struct OpenGLShader* pOpenGLShader = (struct OpenGLShader*)pShader->pInternal;

	struct MEEDFileData* pFile = meedPlatformOpenFile(shaderSource, MEED_FILE_MODE_READ);
	MEED_ASSERT_MSG(pFile != MEED_NULL && pFile->isOpen, "Failed to open shader file \"%s\".", shaderSource);

	GLenum shaderType;

	switch (type)
	{
	case MEED_SHADER_TYPE_VERTEX:
		shaderType = GL_VERTEX_SHADER;
		break;
	case MEED_SHADER_TYPE_FRAGMENT:
		shaderType = GL_FRAGMENT_SHADER;
		break;
	case MEED_SHADER_TYPE_COMPUTE:
		shaderType = GL_COMPUTE_SHADER;
		break;
	case MEED_SHADER_TYPE_GEOMETRY:
		shaderType = GL_GEOMETRY_SHADER;
		break;
	case MEED_SHADER_TYPE_TESSELLATION:
		shaderType = GL_TESS_CONTROL_SHADER; // or GL_TESS_EVALUATION_SHADER based on specific use
		break;
	default:
		MEED_UNTOUCHABLE();
	}

	GL_ASSERT(pOpenGLShader->shaderID = glCreateShader(shaderType));
	GL_ASSERT(glShaderSource(pOpenGLShader->shaderID, 1, (const char**)&pFile->content, (const GLint*)&pFile->size));
	GL_ASSERT(glCompileShader(pOpenGLShader->shaderID));

	glGetShaderiv(pOpenGLShader->shaderID, GL_COMPILE_STATUS, (GLint*)&shaderType);
	if (shaderType != GL_TRUE)
	{
		char infoLog[512];
		glGetShaderInfoLog(pOpenGLShader->shaderID, 512, NULL, infoLog);
		MEED_ASSERT_MSG(MEED_FALSE, "Shader \"%s\" compilation failed: %s", shaderSource, infoLog);
	}

	meedPlatformCloseFile(pFile);

	return pShader;
}

void meedShaderDestroy(struct MEEDShader* pShader)
{
	MEED_ASSERT(pShader != MEED_NULL);

	struct OpenGLShader* pOpenGLShader = (struct OpenGLShader*)pShader->pInternal;
	GL_ASSERT(glDeleteShader(pOpenGLShader->shaderID));

	MEED_FREE(pShader->pInternal, struct OpenGLShader);
	MEED_FREE(pShader, struct MEEDShader);
	pShader = MEED_NULL;
}

#endif // MEED_USE_OPENGL