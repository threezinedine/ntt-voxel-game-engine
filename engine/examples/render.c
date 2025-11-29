#include "MEEDEngine/MEEDEngine.h"

int main(void)
{
	meedPlatformMemoryInitialize();
	meedRenderInitialize();

	meedRenderShutdown();
	meedPlatformMemoryShutdown();
	return 0;
}