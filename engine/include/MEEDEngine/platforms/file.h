#pragma once

#if __cplusplus
extern "C" {
#endif

#include "common.h"

/**
 * @file file.h
 * The utilities for managing file operations inside the `MEEDEngine`.
 */

enum MEED_BINDING MEEDFileMode
{
	MEED_FILE_MODE_READ,  ///< Open the file in read mode.
	MEED_FILE_MODE_WRITE, ///< Open the file in write mode.
	MEED_FILE_MODE_APPEND ///< Open the file in append mode.
};

/**
 * Needed information for working with files.
 */
struct MEED_BINDING MEEDFileData
{
	void* pInternal MEED_HIDDEN; ///< Used for storing custom file system data (e.g., file handles).

	b8				  isOpen;	///< Flag indicating whether the file is currently open.
	const char*		  filePath; ///< The path of the file.
	u32				  size;		///< The size of the file in bytes.
	char*			  content;	///< Pointer to the file content in memory.
	enum MEEDFileMode mode;		///< The mode in which the file was opened.
};

/**
 * Opens a file at the specified path with the given mode.
 * @param filePath The path of the file to open.
 * @param mode The mode in which to open the file.
 * @return Pointer to the MEEDFileData representing the opened file.
 */
struct MEEDFileData* meedPlatformOpenFile(const char* filePath, enum MEEDFileMode mode);

/**
 * Checks if the specified file is open.
 * @param pFileData Pointer to the MEEDFileData representing the file.
 * @return MEED_TRUE if the file is open, MEED_FALSE otherwise.
 */
b8 meedPlatformIsOpen(struct MEEDFileData* pFileData);

/**
 * Writes data to the specified file.
 * @param pFileData Pointer to the MEEDFileData representing the file.
 * @param data Pointer to the data to write.
 * @param size The size of the data to write in bytes.
 */
void meedPlatformWrite(struct MEEDFileData* pFileData, const char* data, meedSize size);

/**
 * Closes the specified file.
 * @param pFileData Pointer to the MEEDFileData representing the file to close.
 */
void meedPlatformCloseFile(struct MEEDFileData* pFileData);

#if __cplusplus
}
#endif