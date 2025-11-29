#pragma once

#if __cplusplus
extern "C" {
#endif
#include "MEEDEngine/platforms/common.h"

/**
 * @brief Structure representing a render pipeline.
 */
struct MEEDRenderPipeline
{
	void* pInternal; ///< Pointer to internal implementation-specific data (Vulkan, WebGL, etc.).

	const char* vertexShaderPath;	///< Path to the vertex shader file.
	const char* fragmentShaderPath; ///< Path to the fragment shader file.
};

/**
 * @brief Creates a render pipeline with the specified shaders.
 *
 * @param vertexShaderPath Path to the vertex shader file.
 * @param fragmentShaderPath Path to the fragment shader file.
 * @return A pointer to the created MEEDRenderPipeline.
 */
struct MEEDRenderPipeline* meedRenderPipelineCreate(const char* vertexShaderPath, const char* fragmentShaderPath);

/**
 * @brief Destroys the specified render pipeline and releases its resources.
 *
 * @param pPipeline Pointer to the MEEDRenderPipeline to destroy.
 */
void meedRenderPipelineDestroy(struct MEEDRenderPipeline* pPipeline);

#if __cplusplus
}
#endif