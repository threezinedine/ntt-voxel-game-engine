#pragma once

#if __cplusplus
extern "C" {
#endif

#include "MEEDEngine/platforms/common.h"

/**
 * @file callback.h
 *
 * Commonly used callback function types.
 */
typedef void (*MEEDNodeDataDeleteCallback)(void* pData);

#if __cplusplus
}
#endif