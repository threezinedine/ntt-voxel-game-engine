#pragma once

#if __cplusplus
extern "C" {
#endif

#include "MEEDEngine/platforms/common.h"

/**
 * Enumeration of log levels used for categorizing log messages.
 */
enum MdLogLevel
{
	MD_LOG_LEVEL_VERBOSE, ///< Detailed information, typically of interest only when diagnosing problems.
	MD_LOG_LEVEL_DEBUG,	  ///< Information useful to developers for debugging the application.
	MD_LOG_LEVEL_INFO,	  ///< General operational information about the application.
	MD_LOG_LEVEL_WARNING, ///< Indication of potential issues or important situations that are not errors.
	MD_LOG_LEVEL_ERROR,	  ///< Errors that might still allow the application to continue running.
	MD_LOG_LEVEL_FATAL,	  ///< Severe errors that will presumably lead the application to abort.
};

#if __cplusplus
}
#endif