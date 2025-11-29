#include "common.hpp"

namespace {
static u32 s_deleteCallCount = 0;

struct TestNode
{
	int value;
};

void deleteTestNode(void* pData)
{
	TestNode* pNode = (TestNode*)pData;
	MEED_FREE(pNode, TestNode);
	pData = MEED_NULL;
	s_deleteCallCount++;
}
} // anonymous namespace
