#pragma once

#if __cplusplus
extern "C" {
#endif
#include "MEEDEngine/core/core.h"
#include "MEEDEngine/modules/release_stack/release_stack.h"
#include "MEEDEngine/platforms/common.h"

/**
 * @brief Structure representing a render pipeline.
 */
struct MEEDPipeline
{
	void* pInternal; ///< Pointer to internal implementation-specific data (Vulkan, WebGL, etc.).

	const char* vertexShaderPath;	///< Path to the vertex shader file.
	const char* fragmentShaderPath; ///< Path to the fragment shader file.

	struct MEEDReleaseStack* pReleaseStack; ///< Release stack for managing resources.
};

/**
 * @brief Creates a render pipeline with the specified shaders.
 *
 * @param vertexShaderPath Path to the vertex shader file.
 * @param fragmentShaderPath Path to the fragment shader file.
 * @return A pointer to the created MEEDPipeline.
 */
struct MEEDPipeline* meedPipelineCreate(const char* vertexShaderPath, const char* fragmentShaderPath);

/**
 * @brief Destroys the specified render pipeline and releases its resources.
 *
 * @param pPipeline Pointer to the MEEDPipeline to destroy.
 */
void meedPipelineDestroy(struct MEEDPipeline* pPipeline);

#if __cplusplus
}
#endif