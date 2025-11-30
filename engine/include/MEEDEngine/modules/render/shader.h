#pragma once

#if __cplusplus
extern "C" {
#endif

#include "MEEDEngine/platforms/platforms.h"

/**
 * @file shader.h
 * The utilities for managing shaders inside the `MEEDEngine` rendering module.
 */

enum MEEDShaderType
{
	MEED_SHADER_TYPE_VERTEX = 0,   ///< Vertex shader type.
	MEED_SHADER_TYPE_FRAGMENT,	   ///< Fragment shader type.
	MEED_SHADER_TYPE_COMPUTE,	   ///< Compute shader type.
	MEED_SHADER_TYPE_GEOMETRY,	   ///< Geometry shader type.
	MEED_SHADER_TYPE_TESSELLATION, ///< Tessellation shader type.
	MEED_SHADER_TYPE_COUNT
};

/**
 * Represents a shader used in the rendering pipeline.
 */
struct MEEDShader
{
	void*				pInternal; ///< Pointer to internal shader data (implementation-specific).
	enum MEEDShaderType type;	   ///< The type of the shader (e.g., vertex, fragment).
};

/**
 * Creates a shader from the specified file.
 * @param type The type of the shader to create.
 * @param filePath The file path to the shader source code.
 * @return A pointer to the created `MEEDShader` structure.
 */
struct MEEDShader* meedShaderCreate(enum MEEDShaderType type, const char* filePath);

/**
 * Destroys a previously created shader.
 * @param pShader A pointer to the `MEEDShader` structure to destroy.
 */
void meedShaderDestroy(struct MEEDShader* pShader);

#if __cplusplus
}
#endif