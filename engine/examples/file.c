#include "MEEDEngine/MEEDEngine.h"

int main(void)
{
	meedPlatformMemoryInit();

	struct MEEDFileData* pFile = meedPlatformOpenFile("./example.txt", MEED_FILE_MODE_WRITE);

	const char* content = "Hello, MEED Engine File!\n";
	meedPlatformWrite(pFile, content, meedPlatformGetStringLength(content));
	meedPlatformCloseFile(pFile);

	struct MEEDFileData* pReadFile = meedPlatformOpenFile("./example.txt", MEED_FILE_MODE_READ);
	meedPlatformPrint("File Content:\n");
	meedPlatformPrint(pReadFile->content);
	meedPlatformCloseFile(pReadFile);

	meedPlatformMemoryShutdown();
	return 0;
}