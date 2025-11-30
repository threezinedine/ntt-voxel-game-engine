#if MEED_USE_OPENGL

// clang-format off
#include <GL/glew.h>
#include <GLFW/glfw3.h>
// clang-format on

struct OpenGLPipeline
{
	u32 shaderProgram; ///< OpenGL shader program ID.
};

struct OpenGLRenderData
{
	struct MEEDWindowData* pWindowData;
};

struct OpenGLShader
{
	u32 shaderID; ///< OpenGL shader ID.
};

extern struct OpenGLRenderData* s_pRenderData;

#define GL_ASSERT(exp)                                                                                                 \
	do                                                                                                                 \
	{                                                                                                                  \
		while (glGetError());                                                                                          \
		exp;                                                                                                           \
		GLenum err = glGetError();                                                                                     \
		MEED_ASSERT_MSG(err == GL_NO_ERROR, "OpenGL error occurred with code %u", err);                                \
	} while (MEED_FALSE)

#endif // MEED_USE_OPENGL
