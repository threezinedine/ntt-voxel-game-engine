#if PLATFORM_IS_WEB
#include "MEEDEngine/platforms/file.h"
#include <fcntl.h>
#include <unistd.h>

struct LinuxFileData
{
	i32 fd;
};

struct MEEDFileData* meedPlatformOpenFile(const char* filePath, enum MEEDFileMode mode)
{
	struct MEEDFileData* pFileData = MEED_MALLOC(struct MEEDFileData);
	meedPlatformMemorySet(pFileData, 0, sizeof(struct MEEDFileData));

	pFileData->isOpen	= MEED_FALSE;
	pFileData->filePath = filePath;
	pFileData->mode		= mode;

	pFileData->pInternal = MEED_MALLOC(struct LinuxFileData);
	MEED_ASSERT(pFileData->pInternal != MEED_NULL);

	struct LinuxFileData* pLinuxData = (struct LinuxFileData*)pFileData->pInternal;

	switch (mode)
	{
	case MEED_FILE_MODE_READ:
		pLinuxData->fd = open(filePath, O_RDONLY);
		break;
	case MEED_FILE_MODE_WRITE:
		pLinuxData->fd = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		break;
	case MEED_FILE_MODE_APPEND:
		pLinuxData->fd = open(filePath, O_WRONLY | O_CREAT | O_APPEND, 066);
		break;

	default:
		MEED_UNTOUCHABLE();
	}

	if (pLinuxData->fd != -1)
	{
		pFileData->isOpen = MEED_TRUE;

		if (mode == MEED_FILE_MODE_READ)
		{
			// Get file size
			off_t currentPos = lseek(pLinuxData->fd, 0, SEEK_CUR);
			off_t fileSize	 = lseek(pLinuxData->fd, 0, SEEK_END);
			lseek(pLinuxData->fd, currentPos, SEEK_SET);

			pFileData->size = (u32)fileSize;

			// Read file content into memory
			pFileData->content = MEED_MALLOC_ARRAY(char, pFileData->size + 1);
			MEED_ASSERT(pFileData->content != MEED_NULL);
			ssize_t bytesRead = read(pLinuxData->fd, pFileData->content, pFileData->size);
			MEED_ASSERT_MSG(bytesRead == (ssize_t)pFileData->size,
							"Failed to read entire file: Expected %u bytes, but read %zd bytes.",
							pFileData->size,
							bytesRead);
			pFileData->content[pFileData->size] = '\0'; // Null-terminate the content
		}
	}

	return pFileData;
}

b8 meedPlatformIsOpen(struct MEEDFileData* pFileData)
{
	MEED_ASSERT(pFileData != MEED_NULL);
	return pFileData->isOpen;
}

void meedPlatformWrite(struct MEEDFileData* pFileData, const char* data, meedSize size)
{
	MEED_ASSERT(pFileData != MEED_NULL);
	MEED_ASSERT(pFileData->isOpen == MEED_TRUE);

	struct LinuxFileData* pLinuxData = (struct LinuxFileData*)pFileData->pInternal;
	MEED_ASSERT(pLinuxData != MEED_NULL);

	ssize_t bytesWritten = write(pLinuxData->fd, data, size);
	MEED_ASSERT_MSG(bytesWritten == (ssize_t)size,
					"Failed to write all data to file: Expected to write %zu bytes, but wrote %zd bytes.",
					size,
					bytesWritten);
}

void meedPlatformCloseFile(struct MEEDFileData* pFileData)
{
	MEED_ASSERT(pFileData != MEED_NULL);

	struct LinuxFileData* pLinuxData = (struct LinuxFileData*)pFileData->pInternal;
	MEED_ASSERT(pLinuxData != MEED_NULL);

	if (pFileData->isOpen)
	{
		close(pLinuxData->fd);

		if (pFileData->mode == MEED_FILE_MODE_READ && pFileData->content != MEED_NULL)
		{
			MEED_FREE_ARRAY(pFileData->content, char, pFileData->size + 1);
		}
	}

	MEED_FREE(pLinuxData, struct LinuxFileData);
	MEED_FREE(pFileData, struct MEEDFileData);
}

#endif // PLATFORM_IS_WEB
