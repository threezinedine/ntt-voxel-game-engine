#include "common.hpp"

static i32 compareIntCallback(const void* pA, const void* pB)
{
	return (*(int*)pA) - (*(int*)pB);
}

class SetTest : public Test
{
protected:
	void SetUp() override
	{
		s_pSet = meedSetCreate(compareIntCallback);
	}
	void TearDown() override
	{
		meedSetDestroy(s_pSet);
	}

protected:
	struct MEEDSet* s_pSet;
	int				a = 10;
	int				b = 20;
	int				c = 30;
};

TEST_F(SetTest, CreateAndDestroy)
{
	EXPECT_NE(s_pSet, MEED_NULL);
	EXPECT_EQ(meedSetCount(s_pSet), 0u);
}

TEST_F(SetTest, Count)
{
	meedSetPush(s_pSet, &a);
	EXPECT_EQ(meedSetCount(s_pSet), 1u);
	EXPECT_EQ(*(int*)meedSetAt(s_pSet, 0), 10);
}

TEST_F(SetTest, PushAndAt)
{
	meedSetPush(s_pSet, &b);
	meedSetPush(s_pSet, &a);
	meedSetPush(s_pSet, &c);

	EXPECT_EQ(meedSetCount(s_pSet), 3u);
	EXPECT_EQ(*(int*)meedSetAt(s_pSet, 0), 10);
	EXPECT_EQ(*(int*)meedSetAt(s_pSet, 1), 20);
	EXPECT_EQ(*(int*)meedSetAt(s_pSet, 2), 30);
}

TEST_F(SetTest, PushDuplicate)
{
	meedSetPush(s_pSet, &b);
	meedSetPush(s_pSet, &a);
	meedSetPush(s_pSet, &c);

	EXPECT_EQ(meedSetCount(s_pSet), 3u);

	// Attempt to push duplicate
	meedSetPush(s_pSet, &b);
	EXPECT_EQ(meedSetCount(s_pSet), 3u);
}

TEST_F(SetTest, AtOutOfBounds)
{
	meedSetPush(s_pSet, &a);
	meedSetPush(s_pSet, &b);

	EXPECT_EXIT(
		{
			meedSetAt(s_pSet, 2);
			std::exit(MEED_EXCEPTION_TYPE_OUT_OF_INDEX);
		},
		testing::ExitedWithCode(MEED_EXCEPTION_TYPE_OUT_OF_INDEX),
		"");
}

TEST_F(SetTest, PushNullData)
{
	EXPECT_EXIT(
		{
			meedSetPush(s_pSet, MEED_NULL);
			std::exit(MEED_EXCEPTION_TYPE_INVALID_OPERATION);
		},
		testing::ExitedWithCode(MEED_EXCEPTION_TYPE_INVALID_OPERATION),
		"");
}

TEST_F(SetTest, EraseElement)
{
	meedSetPush(s_pSet, &a);
	meedSetPush(s_pSet, &b);
	meedSetPush(s_pSet, &c);

	EXPECT_EQ(meedSetCount(s_pSet), 3u);

	meedSetErase(s_pSet, 1);

	EXPECT_EQ(meedSetCount(s_pSet), 2u);
	EXPECT_EQ(*(int*)meedSetAt(s_pSet, 0), 10);
	EXPECT_EQ(*(int*)meedSetAt(s_pSet, 1), 30);
}

TEST_F(SetTest, EraseOutOfBounds)
{
	meedSetPush(s_pSet, &a);

	EXPECT_EXIT(
		{
			meedSetErase(s_pSet, 1);
			std::exit(MEED_EXCEPTION_TYPE_OUT_OF_INDEX);
		},
		testing::ExitedWithCode(MEED_EXCEPTION_TYPE_OUT_OF_INDEX),
		"");
}

TEST_F(SetTest, EraseAllElements)
{
	meedSetPush(s_pSet, &a);
	meedSetPush(s_pSet, &b);
	meedSetPush(s_pSet, &c);

	EXPECT_EQ(meedSetCount(s_pSet), 3u);

	meedSetClear(s_pSet);

	EXPECT_EQ(meedSetCount(s_pSet), 0u);
}

TEST_F(SetTest, FindElement)
{
	meedSetPush(s_pSet, &a);
	meedSetPush(s_pSet, &c);

	u32 aIndex = meedSetFind(s_pSet, &a);
	u32 bIndex = meedSetFind(s_pSet, &b);

	EXPECT_EQ(aIndex, 0u);
	EXPECT_EQ(bIndex, MEED_SET_NOT_FOUND_INDEX);
}