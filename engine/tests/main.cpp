#include <gtest/gtest.h>
#include <iostream>

extern "C" {
#include "MEEDEngine/MEEDEngine.h"
}

int main(int argc, char** argv)
{
	meedPlatformMemoryInitialize();
	::testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();
	meedPlatformMemoryShutdown();
	return result;
}