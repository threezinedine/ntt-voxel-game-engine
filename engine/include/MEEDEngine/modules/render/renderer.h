#pragma once

#if __cplusplus
extern "C" {
#endif
#include "MEEDEngine/platforms/platforms.h"
#include "pipeline.h"

/**
 * @brief Initializes the rendering module.
 * This function sets up all necessary resources for rendering.
 */
void meedRenderInitialize(struct MEEDWindowData* pWindowData);

/**
 * @brief Starts a new frame for rendering.
 * This function prepares the renderer to accept drawing commands for the new frame.
 */
void meedRenderStartFrame();

/**
 * @brief Ends the current frame for rendering.
 * End of the rendering stages for the current frame.
 */
void meedRenderEndFrame();

/**
 * @brief Clears the screen with the specified color.
 * @param color The color to clear the screen with.
 */
void meedRenderClearScreen(struct MEEDColor color);

/**
 * @brief Issues a draw call to render primitives.
 * @param vertexCount The number of vertices to draw.
 * @param instanceCount The number of instances to draw.
 * @param firstVertex The index of the first vertex to draw.
 * @param firstInstance The instance ID of the first instance to draw.
 */
void meedRenderDraw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance);

/**
 * @brief Presents the rendered frame to the display.
 * This function submits the rendered frame to the swapchain for presentation.
 */
void meedRenderPresent();

/**
 * @brief Waits until the device is idle.
 * This function blocks until all submitted commands to the GPU have finished executing.
 */
void meedWaitIdle();

/**
 * @brief Shuts down the rendering module.
 * This function releases all resources allocated for rendering.
 */
void meedRenderShutdown();

#if __cplusplus
}
#endif
