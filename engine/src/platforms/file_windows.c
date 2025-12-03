#if PLATFORM_IS_WINDOWS
#include "MEEDEngine/platforms/common.h"
#include "MEEDEngine/platforms/file.h"
#include <windows.h>

struct WindowsFileData
{
	HANDLE file;
};

struct MdFileData* mdFileOpen(const char* filePath, enum MdFileMode mode)
{
	struct MdFileData* pFileData = MD_MALLOC(struct MdFileData);
	MD_ASSERT(pFileData != MD_NULL);
	mdMemorySet(pFileData, 0, sizeof(struct MdFileData));

	pFileData->isOpen	= MD_FALSE;
	pFileData->filePath = filePath;
	pFileData->mode		= mode;

	pFileData->pInternal = MD_MALLOC(struct WindowsFileData);
	MD_ASSERT(pFileData->pInternal != MD_NULL);
	struct WindowsFileData* pWindowsData = (struct WindowsFileData*)pFileData->pInternal;

	pWindowsData->file = CreateFileA(filePath,
									 (mode == MD_FILE_MODE_READ) ? GENERIC_READ : GENERIC_WRITE,
									 0,
									 NULL,
									 (mode == MD_FILE_MODE_READ) ? OPEN_EXISTING : OPEN_ALWAYS,
									 FILE_ATTRIBUTE_NORMAL,
									 NULL);

	if (pWindowsData->file == INVALID_HANDLE_VALUE)
	{
		return pFileData;
	}

	DWORD size		= GetFileSize(pWindowsData->file, NULL);
	pFileData->size = (u32)size;

	pFileData->content = MD_MALLOC_ARRAY(i8, pFileData->size + 1);
	MD_ASSERT(pFileData->content != MD_NULL);
	DWORD bytesRead = 0;
	ReadFile(pWindowsData->file, pFileData->content, size, &bytesRead, NULL);
	MD_ASSERT_MSG(bytesRead == size,
				  "Failed to read entire file: Expected %u "
				  "bytes, but read %u bytes.",
				  size,
				  bytesRead);
	pFileData->isOpen		 = MD_TRUE;
	pFileData->content[size] = '\0'; // Null-terminate the content buffer

	return pFileData;
}

void mdFileWrite(struct MdFileData* pFileData, const char* data, mdSize size)
{
	MD_ASSERT(pFileData != MD_NULL);
	MD_ASSERT(pFileData->isOpen == MD_TRUE);
	MD_ASSERT(pFileData->mode == MD_FILE_MODE_WRITE || pFileData->mode == MD_FILE_MODE_APPEND);

	struct WindowsFileData* pWindowsData = (struct WindowsFileData*)pFileData->pInternal;
	DWORD					bytesWritten = 0;
	WriteFile(pWindowsData->file, data, size, &bytesWritten, NULL);
	MD_ASSERT_MSG(bytesWritten == size,
				  "Failed to write entire file: Expected %u "
				  "bytes, but wrote %u bytes.",
				  size,
				  bytesWritten);
}

void mdFileClose(struct MdFileData* pFileData)
{
	MD_ASSERT(pFileData != MD_NULL);
	MD_ASSERT(pFileData->isOpen == MD_TRUE);

	struct WindowsFileData* pWindowsData = (struct WindowsFileData*)pFileData->pInternal;
	CloseHandle(pWindowsData->file);

	pFileData->isOpen = MD_FALSE;

	MD_FREE_ARRAY(pFileData->content, i8, pFileData->size + 1);

	MD_FREE(pWindowsData, struct WindowsFileData);
	MD_FREE(pFileData, struct MdFileData);
}

#endif // PLATFORM_IS_WINDOWS