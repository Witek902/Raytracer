#include "PCH.h"
#include "../Core/Containers/ArrayView.h"

using namespace rt;


TEST(ArrayView, Empty)
{
    ArrayView<int> array;

    EXPECT_EQ(0u, array.Size());
    EXPECT_TRUE(array.Empty());
    EXPECT_EQ(array.Begin(), array.End());
    EXPECT_EQ(array.End(), array.Find(0));
}

TEST(ArrayView, Access)
{
    int elements[5] = { 0, 1, 2, 3, 4 };
    ArrayView<int> array(elements, 5);

    ASSERT_EQ(5u, array.Size());
    ASSERT_FALSE(array.Empty());

    EXPECT_EQ(0, array.Front());
    EXPECT_EQ(4, array.Back());

    EXPECT_EQ(0, array[0]);
    EXPECT_EQ(1, array[1]);
    EXPECT_EQ(2, array[2]);
    EXPECT_EQ(3, array[3]);
    EXPECT_EQ(4, array[4]);

    array[2] = 123;
    EXPECT_EQ(123, array[2]);
}

TEST(ArrayView, ConstAccess)
{
    const int elements[5] = { 0, 1, 2, 3, 4 };
    const ArrayView<const int> array(elements, 5);

    ASSERT_EQ(5u, array.Size());
    ASSERT_FALSE(array.Empty());

    EXPECT_EQ(0, array.Front());
    EXPECT_EQ(4, array.Back());

    EXPECT_EQ(0, array[0]);
    EXPECT_EQ(1, array[1]);
    EXPECT_EQ(2, array[2]);
    EXPECT_EQ(3, array[3]);
    EXPECT_EQ(4, array[4]);
}

TEST(ArrayView, ConstructConstFromNonConst)
{
    int elements[5] = { 0, 1, 2, 3, 4 };
    const ArrayView<int> array(elements, 5);
    const ArrayView<const int> array2(array);

    ASSERT_EQ(5u, array2.Size());
    ASSERT_FALSE(array2.Empty());

    EXPECT_EQ(0, array2.Front());
    EXPECT_EQ(4, array2.Back());

    EXPECT_EQ(0, array2[0]);
    EXPECT_EQ(1, array2[1]);
    EXPECT_EQ(2, array2[2]);
    EXPECT_EQ(3, array2[3]);
    EXPECT_EQ(4, array2[4]);
}

TEST(ArrayView, AssignConstToNonConst)
{
    int elements[5] = { 0, 1, 2, 3, 4 };
    const ArrayView<int> array(elements, 5);
    const ArrayView<const int> array2 = array;

    ASSERT_EQ(5u, array2.Size());
    ASSERT_FALSE(array2.Empty());

    EXPECT_EQ(0, array2.Front());
    EXPECT_EQ(4, array2.Back());

    EXPECT_EQ(0, array2[0]);
    EXPECT_EQ(1, array2[1]);
    EXPECT_EQ(2, array2[2]);
    EXPECT_EQ(3, array2[3]);
    EXPECT_EQ(4, array2[4]);
}

TEST(ArrayView, Iterator)
{
    int elements[5] = { 0, 10, 20, 30, 40 };
    ArrayView<int> array(elements, 5);
    ASSERT_EQ(5u, array.Size());

    ArrayView<int>::Iterator iter = array.Begin();
    *iter = 123;
    EXPECT_EQ(123, *iter);

    ++iter;
    ASSERT_NE(array.Begin(), iter);
    ASSERT_NE(array.End(), iter);
    EXPECT_EQ(10, *iter);

    iter += 2;
    ASSERT_NE(array.Begin(), iter);
    ASSERT_NE(array.End(), iter);
    EXPECT_EQ(30, *iter);

    iter -= 1;
    ASSERT_NE(array.Begin(), iter);
    ASSERT_NE(array.End(), iter);
    EXPECT_EQ(20, *iter);

    iter += 2;
    ASSERT_NE(array.Begin(), iter);
    ASSERT_NE(array.End(), iter);
    EXPECT_EQ(40, *iter);

    ++iter;
    ASSERT_NE(array.Begin(), iter);
    ASSERT_EQ(array.End(), iter);
}

TEST(ArrayView, ConstIterator)
{
    int elements[5] = { 0, 10, 20, 30, 40 };
    const ArrayView<int> array(elements, 5);
    ASSERT_EQ(5u, array.Size());

    ArrayView<int>::ConstIterator iter = array.Begin();
    EXPECT_EQ(0, *iter);

    ++iter;
    ASSERT_NE(array.Begin(), iter);
    ASSERT_NE(array.End(), iter);
    EXPECT_EQ(10, *iter);

    iter += 2;
    ASSERT_NE(array.Begin(), iter);
    ASSERT_NE(array.End(), iter);
    EXPECT_EQ(30, *iter);

    iter -= 1;
    ASSERT_NE(array.Begin(), iter);
    ASSERT_NE(array.End(), iter);
    EXPECT_EQ(20, *iter);

    iter += 2;
    ASSERT_NE(array.Begin(), iter);
    ASSERT_NE(array.End(), iter);
    EXPECT_EQ(40, *iter);

    ++iter;
    ASSERT_NE(array.Begin(), iter);
    ASSERT_EQ(array.End(), iter);
}

TEST(ArrayView, IteratorConversion)
{
    int elements[5] = { 0, 10, 20, 30, 40 };
    ArrayView<int> array(elements, 5);
    ASSERT_EQ(5u, array.Size());

    const ArrayView<int>::Iterator iter = array.Begin();
    const ArrayView<int>::ConstIterator iter2 = iter;

    ASSERT_NE(array.End(), iter2);
    ASSERT_EQ(0, *iter2);
}

TEST(ArrayView, IteratorArithmetics)
{
    int elements[5] = { 0, 10, 20, 30, 40 };
    ArrayView<int> array(elements, 5);
    ASSERT_EQ(5u, array.Size());

    const ArrayView<int>::Iterator iter = array.Begin();
    const ArrayView<int>::Iterator iter2 = iter + 1;

    ASSERT_NE(array.End(), iter2);
    ASSERT_EQ(10, *iter2);
}

TEST(ArrayView, ConstIteratorArithmetics)
{
    int elements[5] = { 0, 10, 20, 30, 40 };
    ArrayView<int> array(elements, 5);
    ASSERT_EQ(5u, array.Size());

    const ArrayView<int>::ConstIterator iter = array.Begin();
    const ArrayView<int>::ConstIterator iter2 = iter + 1;

    ASSERT_NE(array.End(), iter2);
    ASSERT_EQ(10, *iter2);
}

TEST(ArrayView, RangeBasedFor)
{
    int elements[5] = { 0, 1, 2, 3, 4 };
    ArrayView<int> array(elements, 5);

    int i = 0;
    for (auto val : array)
    {
        EXPECT_EQ(i, val) << "i = " << i;
        i++;
    }
    ASSERT_EQ(5, i);
}

TEST(ArrayView, Range)
{
    int elements[5] = { 0, 1, 2, 3, 4 };
    ArrayView<int> array(elements, 5);

    ArrayView<int> array2 = array.Range(2, 3);
    ASSERT_EQ(3u, array2.Size());
    ASSERT_FALSE(array2.Empty());

    EXPECT_EQ(2, array2[0]);
    EXPECT_EQ(3, array2[1]);
    EXPECT_EQ(4, array2[2]);
}

TEST(ArrayView, Equal)
{
    int elements[5] = { 0, 1, 2, 3, 4 };
    ArrayView<int> array(elements, 5);

    int elements2[5] = { 0, 1, 2, 3, 4 };
    ArrayView<int> array2(elements2, 5);

    ASSERT_TRUE(array == array2);
    ASSERT_TRUE(array2 == array);
    ASSERT_FALSE(array != array2);
    ASSERT_FALSE(array2 != array);
}

TEST(ArrayView, NotEqual_DifferentSize)
{
    int elements[5] = { 0, 1, 2, 3, 4 };
    ArrayView<int> array(elements, 5);

    int elements2[5] = { 0, 1, 2, 3 };
    ArrayView<int> array2(elements2, 5);

    ASSERT_TRUE(array != array2);
    ASSERT_TRUE(array2 != array);
    ASSERT_FALSE(array == array2);
    ASSERT_FALSE(array2 == array);
}

TEST(ArrayView, NotEqual_DifferentValues)
{
    int elements[5] = { 0, 1, 2, 3, 4 };
    ArrayView<int> array(elements, 5);

    int elements2[5] = { 0, 1, 2, 3, 5 };
    ArrayView<int> array2(elements2, 5);

    ASSERT_TRUE(array != array2);
    ASSERT_TRUE(array2 != array);
    ASSERT_FALSE(array == array2);
    ASSERT_FALSE(array2 == array);
}

TEST(ArrayView, Find)
{
    int elements[5] = { 16, 357, 24, 123, 754 };
    ArrayView<int> array(elements, 5);

    EXPECT_EQ(array.End(), array.Find(42));
    ASSERT_NE(array.End(), array.Find(123));
    EXPECT_EQ(3, array.Find(123).GetIndex());
}

TEST(ArrayView, CompareConstAndNonConst)
{
    int elements[5] = { 0, 1, 2, 3, 4 };
    ArrayView<int> array(elements, 5);

    const int elements2[5] = { 0, 1, 2, 3, 4 };
    ArrayView<const int> array2(elements2, 5);

    ASSERT_TRUE(array == array2);
    ASSERT_TRUE(array2 == array);
    ASSERT_FALSE(array != array2);
    ASSERT_FALSE(array2 != array);
}

TEST(ArrayView, Stl_FindIf)
{
    int elements[5] = { 0, 1, 2, 3, 4 };
    const ArrayView<int> array(elements, 5);

    // find existing element
    {
        const auto it = std::find_if(array.begin(), array.end(), [](int x) { return x == 1; });
        ASSERT_NE(array.End(), it);
        ASSERT_EQ(1, *it);
    }

    // find non-existing element
    {
        const auto it = std::find_if(array.begin(), array.end(), [](int x) { return x == 123; });
        ASSERT_EQ(array.End(), it);
    }
}

TEST(ArrayView, Stl_Count)
{
    int elements[5] = { 0, 0, 1, 1, 1 };
    const ArrayView<int> array(elements, 5);

    const auto result = std::count(array.begin(), array.end(), 1);
    ASSERT_EQ(3, result);
}

TEST(ArrayView, Stl_Sort)
{
    int elements[5] = { 3, 2, 0, 4, 1 };
    ArrayView<int> array(elements, 5);

    std::sort(array.Begin(), array.End());

    EXPECT_EQ(0, array[0]);
    EXPECT_EQ(1, array[1]);
    EXPECT_EQ(2, array[2]);
    EXPECT_EQ(3, array[3]);
    EXPECT_EQ(4, array[4]);
}

TEST(ArrayView, Stl_StableSort)
{
    int elements[5] = { 3, 2, 0, 4, 1 };
    ArrayView<int> array(elements, 5);

    std::stable_sort(array.Begin(), array.End());

    EXPECT_EQ(0, array[0]);
    EXPECT_EQ(1, array[1]);
    EXPECT_EQ(2, array[2]);
    EXPECT_EQ(3, array[3]);
    EXPECT_EQ(4, array[4]);
}
