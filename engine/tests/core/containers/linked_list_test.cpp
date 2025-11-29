#include "common.hpp"

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

class LinkedListTest : public Test
{
protected:
	void SetUp() override
	{
		s_pList			  = meedLinkedListCreate(NULL);
		s_deleteCallCount = 0;
	}

	void TearDown() override
	{
		meedLinkedListDestroy(s_pList);
	}

protected:
	struct MEEDLinkedList* s_pList;

	int a = 10;
	int b = 20;
	int c = 30;
};

TEST_F(LinkedListTest, PushBackAndSize)
{
	meedLinkedListPush(s_pList, &a);
	meedLinkedListPush(s_pList, &b);
	meedLinkedListPush(s_pList, &c);

	EXPECT_EQ(meedLinkedListGetCount(s_pList), 3u);
}

TEST_F(LinkedListTest, GetElement)
{
	meedLinkedListPush(s_pList, &a);
	meedLinkedListPush(s_pList, &b);
	meedLinkedListPush(s_pList, &c);

	EXPECT_EQ(*(int*)(meedLinkedListAt(s_pList, 0)), 10);
	EXPECT_EQ(*(int*)(meedLinkedListAt(s_pList, 1)), 20);
	EXPECT_EQ(*(int*)(meedLinkedListAt(s_pList, 2)), 30);
}

TEST_F(LinkedListTest, OutOfBoundsAccess)
{
	meedLinkedListPush(s_pList, &a);
	meedLinkedListPush(s_pList, &b);

	EXPECT_EXIT(
		{
			meedLinkedListAt(s_pList, 2);
			std::exit(MEED_EXCEPTION_TYPE_OUT_OF_INDEX);
		},
		testing::ExitedWithCode(MEED_EXCEPTION_TYPE_OUT_OF_INDEX),
		"");
}

TEST_F(LinkedListTest, InsertWithIndex0)
{
	meedLinkedListInsert(s_pList, 0, &a); // List: [10]
	meedLinkedListInsert(s_pList, 0, &c); // List: [30, 10]

	EXPECT_EQ(meedLinkedListGetCount(s_pList), 2u);
	EXPECT_EQ(*(int*)(meedLinkedListAt(s_pList, 0)), 30);
	EXPECT_EQ(*(int*)(meedLinkedListAt(s_pList, 1)), 10);
}

TEST_F(LinkedListTest, InsertWithMiddleIndex)
{
	meedLinkedListPush(s_pList, &a);	  // List: [10]
	meedLinkedListPush(s_pList, &c);	  // List: [10, 30]
	meedLinkedListInsert(s_pList, 1, &b); // List: [10, 20, 30]

	EXPECT_EQ(meedLinkedListGetCount(s_pList), 3u);
	EXPECT_EQ(*(int*)(meedLinkedListAt(s_pList, 0)), 10);
	EXPECT_EQ(*(int*)(meedLinkedListAt(s_pList, 1)), 20);
	EXPECT_EQ(*(int*)(meedLinkedListAt(s_pList, 2)), 30);
}

TEST_F(LinkedListTest, InsertWithEndIndex)
{
	meedLinkedListPush(s_pList, &a);	  // List: [10]
	meedLinkedListPush(s_pList, &b);	  // List: [10, 20]
	meedLinkedListInsert(s_pList, 2, &c); // List: [10, 20, 30]

	EXPECT_EQ(meedLinkedListGetCount(s_pList), 3u);
	EXPECT_EQ(*(int*)(meedLinkedListAt(s_pList, 0)), 10);
	EXPECT_EQ(*(int*)(meedLinkedListAt(s_pList, 1)), 20);
	EXPECT_EQ(*(int*)(meedLinkedListAt(s_pList, 2)), 30);
}

TEST_F(LinkedListTest, InsertOutOfBounds)
{
	meedLinkedListPush(s_pList, &a); // List: [10]

	EXPECT_EXIT(
		{
			meedLinkedListInsert(s_pList, 2, &b);
			std::exit(MEED_EXCEPTION_TYPE_OUT_OF_INDEX);
		},
		testing::ExitedWithCode(MEED_EXCEPTION_TYPE_OUT_OF_INDEX),
		"");
}

TEST_F(LinkedListTest, EraseByIndex0)
{
	meedLinkedListPush(s_pList, &a); // List: [10]
	meedLinkedListPush(s_pList, &b); // List: [10, 20]
	meedLinkedListPush(s_pList, &c); // List: [10, 20, 30]

	meedLinkedListErase(s_pList, 0); // List: [20, 30]

	EXPECT_EQ(meedLinkedListGetCount(s_pList), 2u);
	EXPECT_EQ(*(int*)(meedLinkedListAt(s_pList, 0)), 20);
	EXPECT_EQ(*(int*)(meedLinkedListAt(s_pList, 1)), 30);
}

TEST_F(LinkedListTest, EraseByMiddleIndex)
{
	meedLinkedListPush(s_pList, &a); // List: [10]
	meedLinkedListPush(s_pList, &b); // List: [10, 20]
	meedLinkedListPush(s_pList, &c); // List: [10, 20, 30]

	meedLinkedListErase(s_pList, 1); // List: [10, 30]

	EXPECT_EQ(meedLinkedListGetCount(s_pList), 2u);
	EXPECT_EQ(*(int*)(meedLinkedListAt(s_pList, 0)), 10);
	EXPECT_EQ(*(int*)(meedLinkedListAt(s_pList, 1)), 30);
}

TEST_F(LinkedListTest, EraseByEndIndex)
{
	meedLinkedListPush(s_pList, &a); // List: [10]
	meedLinkedListPush(s_pList, &b); // List: [10, 20]
	meedLinkedListPush(s_pList, &c); // List: [10, 20, 30]

	meedLinkedListErase(s_pList, 2); // List: [10, 20]

	EXPECT_EQ(meedLinkedListGetCount(s_pList), 2u);
	EXPECT_EQ(*(int*)(meedLinkedListAt(s_pList, 0)), 10);
	EXPECT_EQ(*(int*)(meedLinkedListAt(s_pList, 1)), 20);
}

TEST_F(LinkedListTest, EraseOutOfBounds)
{
	meedLinkedListPush(s_pList, &a); // List: [10]

	EXPECT_EXIT(
		{
			meedLinkedListErase(s_pList, 1);
			std::exit(MEED_EXCEPTION_TYPE_OUT_OF_INDEX);
		},
		testing::ExitedWithCode(MEED_EXCEPTION_TYPE_OUT_OF_INDEX),
		"");
}

TEST_F(LinkedListTest, EraseWithCallback)
{
	struct MEEDLinkedList* pListWithCallback = meedLinkedListCreate(deleteTestNode);

	TestNode* pNode1 = MEED_MALLOC(TestNode);
	pNode1->value	 = 100;

	TestNode* pNode2 = MEED_MALLOC(TestNode);
	pNode2->value	 = 200;

	meedLinkedListPush(pListWithCallback, pNode1);
	meedLinkedListPush(pListWithCallback, pNode2);

	EXPECT_EQ(meedLinkedListGetCount(pListWithCallback), 2u);

	EXPECT_EQ(s_deleteCallCount, 0);
	meedLinkedListErase(pListWithCallback, 0);
	EXPECT_EQ(s_deleteCallCount, 1);

	EXPECT_EQ(meedLinkedListGetCount(pListWithCallback), 1u);
	EXPECT_EQ(((TestNode*)meedLinkedListAt(pListWithCallback, 0))->value, 200);

	meedLinkedListDestroy(pListWithCallback);
}

TEST_F(LinkedListTest, EmptyListSize)
{
	EXPECT_EQ(meedLinkedListGetCount(s_pList), 0u);
	EXPECT_EQ(meedLinkedListEmpty(s_pList), MEED_TRUE);
}

TEST_F(LinkedListTest, ListWithDeleteCallback)
{
	struct MEEDLinkedList* pListWithCallback = meedLinkedListCreate(deleteTestNode);

	TestNode* pNode1 = MEED_MALLOC(TestNode);
	pNode1->value	 = 100;

	TestNode* pNode2 = MEED_MALLOC(TestNode);
	pNode2->value	 = 200;

	meedLinkedListPush(pListWithCallback, pNode1);
	meedLinkedListPush(pListWithCallback, pNode2);

	EXPECT_EQ(meedLinkedListGetCount(pListWithCallback), 2u);
	EXPECT_EQ(((TestNode*)meedLinkedListAt(pListWithCallback, 0))->value, 100);
	EXPECT_EQ(((TestNode*)meedLinkedListAt(pListWithCallback, 1))->value, 200);

	meedLinkedListDestroy(pListWithCallback);
}

TEST_F(LinkedListTest, ClearWithDeleteCallback)
{
	struct MEEDLinkedList* pListWithCallback = meedLinkedListCreate(deleteTestNode);

	TestNode* pNode1 = MEED_MALLOC(TestNode);
	pNode1->value	 = 100;

	TestNode* pNode2 = MEED_MALLOC(TestNode);
	pNode2->value	 = 200;

	meedLinkedListPush(pListWithCallback, pNode1);
	meedLinkedListPush(pListWithCallback, pNode2);

	EXPECT_EQ(meedLinkedListGetCount(pListWithCallback), 2u);

	meedLinkedListClear(pListWithCallback);

	EXPECT_EQ(meedLinkedListGetCount(pListWithCallback), 0u);

	meedLinkedListDestroy(pListWithCallback);
}

TEST_F(LinkedListTest, Clear)
{
	meedLinkedListPush(s_pList, &a);
	meedLinkedListPush(s_pList, &b);
	meedLinkedListPush(s_pList, &c);

	EXPECT_EQ(meedLinkedListGetCount(s_pList), 3u);

	meedLinkedListClear(s_pList);

	EXPECT_EQ(meedLinkedListGetCount(s_pList), 0u);
}