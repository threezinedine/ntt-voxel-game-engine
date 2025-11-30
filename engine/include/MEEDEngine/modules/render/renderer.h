#pragma once

#if __cplusplus
extern "C" {
#endif
#include "MEEDEngine/platforms/platforms.h"

/**
 * @brief Initializes the rendering module.
 * This function sets up all necessary resources for rendering.
 */
void meedRenderInitialize(struct MEEDWindowData* pWindowData);

/**
 * @brief Shuts down the rendering module.
 * This function releases all resources allocated for rendering.
 */
void meedRenderShutdown();

#if __cplusplus
}
#endif
