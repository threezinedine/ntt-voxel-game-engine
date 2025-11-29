#include "container_common.hpp"

class DynamicArrayTest : public Test
{
protected:
	void SetUp() override
	{
		s_pArray = meedDynamicArrayCreate(0, nullptr);
	}

	void TearDown() override
	{
		meedDynamicArrayDestroy(s_pArray);
	}

protected:
	struct MEEDDynamicArray* s_pArray;
	int						 a = 10;
	int						 b = 20;
	int						 c = 30;
};

TEST_F(DynamicArrayTest, CreateAndDestroy)
{
	EXPECT_NE(s_pArray, nullptr);
	EXPECT_EQ(s_pArray->count, 0u);
	EXPECT_EQ(s_pArray->capacity, MEED_DEFAULT_DYNAMIC_ARRAY_CAPACITY);
}

TEST_F(DynamicArrayTest, InitialCapacity)
{
	meedDynamicArrayDestroy(s_pArray);

	s_pArray = meedDynamicArrayCreate(5, nullptr);
	EXPECT_NE(s_pArray, nullptr);
	EXPECT_EQ(s_pArray->count, 0u);
	EXPECT_EQ(s_pArray->capacity, 5u);
}

TEST_F(DynamicArrayTest, PushBackAndSize)
{
	meedDynamicArrayPush(s_pArray, &a);
	EXPECT_EQ(s_pArray->count, 1u);
	EXPECT_EQ(*(int*)meedDynamicArrayAt(s_pArray, 0), 10);

	meedDynamicArrayPush(s_pArray, &b);
	EXPECT_EQ(s_pArray->count, 2u);
	EXPECT_EQ(*(int*)meedDynamicArrayAt(s_pArray, 1), 20);

	meedDynamicArrayPush(s_pArray, &c);
	EXPECT_EQ(s_pArray->count, 3u);
	EXPECT_EQ(*(int*)meedDynamicArrayAt(s_pArray, 2), 30);

	EXPECT_GE(s_pArray->capacity, s_pArray->count);
}

TEST_F(DynamicArrayTest, AccessOutOfBounds)
{
	meedDynamicArrayPush(s_pArray, &a);
	meedDynamicArrayPush(s_pArray, &b);

	EXPECT_EXIT(
		{
			meedDynamicArrayAt(s_pArray, 2);
			std::exit(MEED_EXCEPTION_TYPE_OUT_OF_INDEX);
		},
		testing::ExitedWithCode(MEED_EXCEPTION_TYPE_OUT_OF_INDEX),
		"");
}

TEST_F(DynamicArrayTest, Resize)
{
	meedDynamicArrayPush(s_pArray, &a);
	meedDynamicArrayPush(s_pArray, &b);

	u32 oldCapacity = s_pArray->capacity;
	meedDynamicArrayResize(s_pArray, oldCapacity * 2);
	EXPECT_EQ(s_pArray->capacity, oldCapacity * 2);
	EXPECT_EQ(s_pArray->count, 2u);
	EXPECT_EQ(*(int*)meedDynamicArrayAt(s_pArray, 0), 10);
	EXPECT_EQ(*(int*)meedDynamicArrayAt(s_pArray, 1), 20);
}

TEST_F(DynamicArrayTest, ResizeInvalidOperation)
{
	meedDynamicArrayPush(s_pArray, &a);
	meedDynamicArrayPush(s_pArray, &b);

	EXPECT_EXIT(
		{
			meedDynamicArrayResize(s_pArray, 1);
			std::exit(MEED_EXCEPTION_TYPE_INVALID_OPERATION);
		},
		testing::ExitedWithCode(MEED_EXCEPTION_TYPE_INVALID_OPERATION),
		"");
}

TEST_F(DynamicArrayTest, WithDeleteCallback)
{
	struct MEEDDynamicArray* pArrayWithCallback = meedDynamicArrayCreate(0, deleteTestNode);

	TestNode* pNode1 = MEED_MALLOC(TestNode);
	pNode1->value	 = 100;

	TestNode* pNode2 = MEED_MALLOC(TestNode);
	pNode2->value	 = 200;

	meedDynamicArrayPush(pArrayWithCallback, pNode1);
	meedDynamicArrayPush(pArrayWithCallback, pNode2);

	EXPECT_EQ(pArrayWithCallback->count, 2u);
	EXPECT_EQ(((TestNode*)meedDynamicArrayAt(pArrayWithCallback, 0))->value, 100);
	EXPECT_EQ(((TestNode*)meedDynamicArrayAt(pArrayWithCallback, 1))->value, 200);

	meedDynamicArrayDestroy(pArrayWithCallback);
}