#include "common.hpp"

class StackTest : public Test
{
protected:
	void SetUp() override
	{
		s_pStack = meedStackCreate(NULL);
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