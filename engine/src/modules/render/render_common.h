#pragma once

#include "MEEDEngine/modules/render/render.h"
#include "MEEDEngine/platforms/platforms.h"

/**
 * @file render_common.h
 * Contains common structures and definitions used across different rendering backends.
 */

/**
 * @brief Gets the size in bytes of the specified vertex attribute type.
 *
 * @param type The vertex buffer attribute type.
 * @return The size in bytes of the attribute type.
 */
u32 mdGetVertexAttributeTypeSize(enum MdVertexBufferAttributeType type);

/**
 * @brief Gets the count of components for the specified vertex attribute type.
 * @param type The vertex buffer attribute type.
 * @return The number of components for the attribute type.
 */
u32 mdGetVertexAttributeTypeCount(enum MdVertexBufferAttributeType type);