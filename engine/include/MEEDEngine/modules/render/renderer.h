#pragma once

#if __cplusplus
extern "C" {
#endif
#include "MEEDEngine/platforms/common.h"

/**
 * @brief Initializes the rendering module.
 * This function sets up all necessary resources for rendering.
 */
void meedRenderInitialize();

/**
 * @brief Shuts down the rendering module.
 * This function releases all resources allocated for rendering.
 */
void meedRenderShutdown();

#if __cplusplus
}
#endif
