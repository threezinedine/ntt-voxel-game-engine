#include "container_common.hpp"

class StackTest : public Test
{
protected:
	void SetUp() override
	{
		s_pStack		  = meedStackCreate(NULL);
		s_deleteCallCount = 0;
	}

	void TearDown() override
	{
		meedStackDestroy(s_pStack);
	}

protected:
	struct MEEDStack* s_pStack;
	int				  a = 10;
	int				  b = 20;
	int				  c = 30;
};

TEST_F(StackTest, PushAndTop)
{
	meedStackPush(s_pStack, &a);
	EXPECT_EQ(*(int*)meedStackTop(s_pStack), 10);

	meedStackPush(s_pStack, &b);
	EXPECT_EQ(*(int*)meedStackTop(s_pStack), 20);

	meedStackPush(s_pStack, &c);
	EXPECT_EQ(*(int*)meedStackTop(s_pStack), 30);
}

TEST_F(StackTest, PopWithEmptyStack)
{
	meedStackPush(s_pStack, &a);
	meedStackPush(s_pStack, &b);
	meedStackPush(s_pStack, &c);

	meedStackPop(s_pStack);
	EXPECT_EQ(*(int*)meedStackTop(s_pStack), 20);

	meedStackPop(s_pStack);
	EXPECT_EQ(*(int*)meedStackTop(s_pStack), 10);

	meedStackPop(s_pStack);
	EXPECT_TRUE(meedStackEmpty(s_pStack) == MEED_TRUE);

	EXPECT_EXIT(
		{
			meedStackPop(s_pStack);
			std::exit(MEED_EXCEPTION_TYPE_EMPTY_CONTAINER);
		},
		testing::ExitedWithCode(MEED_EXCEPTION_TYPE_EMPTY_CONTAINER),
		"");
}

TEST_F(StackTest, EmptyStack)
{
	EXPECT_TRUE(meedStackEmpty(s_pStack) == MEED_TRUE);

	meedStackPush(s_pStack, &a);
	EXPECT_TRUE(meedStackEmpty(s_pStack) == MEED_FALSE);

	meedStackPop(s_pStack);
	EXPECT_TRUE(meedStackEmpty(s_pStack) == MEED_TRUE);
}

TEST_F(StackTest, GetCount)
{
	EXPECT_EQ(meedStackGetCount(s_pStack), 0u);

	meedStackPush(s_pStack, &a);
	EXPECT_EQ(meedStackGetCount(s_pStack), 1u);

	meedStackPush(s_pStack, &b);
	EXPECT_EQ(meedStackGetCount(s_pStack), 2u);

	meedStackPush(s_pStack, &c);
	EXPECT_EQ(meedStackGetCount(s_pStack), 3u);

	meedStackPop(s_pStack);
	EXPECT_EQ(meedStackGetCount(s_pStack), 2u);

	meedStackPop(s_pStack);
	EXPECT_EQ(meedStackGetCount(s_pStack), 1u);

	meedStackPop(s_pStack);
	EXPECT_EQ(meedStackGetCount(s_pStack), 0u);
}

TEST_F(StackTest, Clear)
{
	meedStackPush(s_pStack, &a);
	meedStackPush(s_pStack, &b);
	meedStackPush(s_pStack, &c);

	EXPECT_EQ(meedStackGetCount(s_pStack), 3u);

	meedStackClear(s_pStack);
	EXPECT_EQ(meedStackGetCount(s_pStack), 0u);
	EXPECT_TRUE(meedStackEmpty(s_pStack) == MEED_TRUE);
}

TEST_F(StackTest, TestWithCallback)
{
	struct MEEDStack* pStackWithCallback = meedStackCreate(deleteTestNode);

	TestNode* pNode1 = MEED_MALLOC(TestNode);
	pNode1->value	 = 100;

	TestNode* pNode2 = MEED_MALLOC(TestNode);
	pNode2->value	 = 200;

	meedStackPush(pStackWithCallback, pNode1);
	meedStackPush(pStackWithCallback, pNode2);

	EXPECT_EQ(meedStackGetCount(pStackWithCallback), 2u);

	EXPECT_EQ(s_deleteCallCount, 0);
	meedStackPop(pStackWithCallback);
	EXPECT_EQ(s_deleteCallCount, 1);

	EXPECT_EQ(meedStackGetCount(pStackWithCallback), 1u);
	EXPECT_EQ(((TestNode*)meedStackTop(pStackWithCallback))->value, 100);

	meedStackClear(pStackWithCallback);
	EXPECT_EQ(s_deleteCallCount, 2);

	TestNode* pNode3 = MEED_MALLOC(TestNode);
	pNode3->value	 = 300;

	meedStackPush(pStackWithCallback, pNode3);

	meedStackDestroy(pStackWithCallback);
}