#include "PCH.h"
#include "../Core/Containers/DynArray.h"

#include "gtest/gtest.h"

using namespace rt;

#include "TestClasses.h"

namespace {

struct TestClass
{
    int a;
    int b;
    TestClass(int a, int b) : a(a), b(b) { }
};

} // namespace

TEST(DynArray, Empty)
{
    DynArray<int> array;

    EXPECT_EQ(0u, array.Size());
    EXPECT_TRUE(array.Empty());
    EXPECT_EQ(array.Begin(), array.End());
    EXPECT_EQ(array.End(), array.Find(0));
}

TEST(DynArray, CreateWithSize_Default)
{
    const Uint32 size = 123;
    DynArray<int> array(size);

    EXPECT_EQ(size, array.Size());

    for (Uint32 i = 0; i < size; ++i)
    {
        EXPECT_EQ(0, array[i]) << "i=" << i;
    }
}

TEST(DynArray, CreateWithSize_Custom)
{
    const Uint32 size = 123;
    DynArray<int> array(size, 567);

    EXPECT_EQ(size, array.Size());

    for (Uint32 i = 0; i < size; ++i)
    {
        EXPECT_EQ(567, array[i]) << "i=" << i;
    }
}

TEST(DynArray, PushBack)
{
    DynArray<int> array;

    {
        const auto iter1 = array.PushBack(1);
        ASSERT_NE(array.End(), iter1);
        ASSERT_EQ(1u, array.Size());
    }

    const auto iter2 = array.PushBack(2);
    ASSERT_NE(array.End(), iter2);
    ASSERT_EQ(2u, array.Size());

    EXPECT_EQ(1, array[0]);
    EXPECT_EQ(2, array[1]);
}

TEST(DynArray, EmplaceBack_Simple)
{
    DynArray<int> array;

    const auto iter = array.EmplaceBack(123);
    ASSERT_NE(array.End(), iter);
    ASSERT_EQ(1u, array.Size());
    EXPECT_EQ(123, array[0]);
}

TEST(DynArray, EmplaceBack_Class)
{
    DynArray<TestClass> array;

    const auto iter = array.EmplaceBack(123, 456);
    ASSERT_NE(array.End(), iter);
    ASSERT_EQ(1u, array.Size());
    EXPECT_EQ(123, array[0].a);
    EXPECT_EQ(456, array[0].b);
}

TEST(DynArray, InitializeFromArray)
{
    const int elements[] = { 10, 20, 30 };
    DynArray<int> array(elements, 3);

    ASSERT_EQ(3u, array.Size());
    EXPECT_EQ(10, array[0]);
    EXPECT_EQ(20, array[1]);
    EXPECT_EQ(30, array[2]);
}

TEST(DynArray, Clear)
{
    const Uint32 numElements = 20;
    DynArray<int> array;

    for (Uint32 i = 0; i < numElements; ++i)
    {
        array.PushBack(i);
    }

    ASSERT_EQ(numElements, array.Size());

    array.Clear();
    EXPECT_EQ(0u, array.Size());
    EXPECT_TRUE(array.Empty());
}

TEST(DynArray, Resize)
{
    struct Type
    {
        int a;
        Type() : a(1234) { }

        bool operator==(const Type& other) const
        {
            return a == other.a;
        }
    };

    DynArray<Type> array;
    ASSERT_TRUE(array.Resize(5));

    const Type defaultVal;
    EXPECT_EQ(5u, array.Size());
    EXPECT_EQ(defaultVal, array[0]);
    EXPECT_EQ(defaultVal, array[1]);
    EXPECT_EQ(defaultVal, array[2]);
    EXPECT_EQ(defaultVal, array[3]);
    EXPECT_EQ(defaultVal, array[4]);
}

TEST(DynArray, Resize_ReduceSize)
{
    DynArray<Int32> array;
    array.PushBack(1);
    array.PushBack(2);
    array.PushBack(3);
    array.PushBack(4);

    ASSERT_TRUE(array.Resize(2));

    EXPECT_EQ(2u, array.Size());
    EXPECT_EQ(1, array[0]);
    EXPECT_EQ(2, array[1]);
}

TEST(DynArray, Resize_CustomElement)
{
    DynArray<Int32> array;
    array.PushBack(1);
    array.PushBack(2);

    ASSERT_TRUE(array.Resize(5, 1234));

    EXPECT_EQ(5u, array.Size());
    EXPECT_EQ(1, array[0]);
    EXPECT_EQ(2, array[1]);
    EXPECT_EQ(1234, array[2]);
    EXPECT_EQ(1234, array[3]);
    EXPECT_EQ(1234, array[4]);
}

TEST(DynArray, InitializerList)
{
    const DynArray<int> array({ 10, 20, 30, 40, 50 });

    EXPECT_EQ(5u, array.Size());
    EXPECT_FALSE(array.Empty());

    EXPECT_EQ(10, array[0]);
    EXPECT_EQ(20, array[1]);
    EXPECT_EQ(30, array[2]);
    EXPECT_EQ(40, array[3]);
    EXPECT_EQ(50, array[4]);
}

TEST(DynArray, PopBack)
{
    int initialElements[] = { 10, 20, 30, 40 };
    DynArray<int> array(initialElements, 4);

    ASSERT_TRUE(array.PopBack());
    ASSERT_EQ(3u, array.Size());
    EXPECT_EQ(ArrayView<int>(initialElements, 3), array);

    ASSERT_TRUE(array.PopBack());
    ASSERT_EQ(2u, array.Size());
    EXPECT_EQ(ArrayView<int>(initialElements, 2), array);

    ASSERT_TRUE(array.PopBack());
    ASSERT_EQ(1u, array.Size());
    EXPECT_EQ(ArrayView<int>(initialElements, 1), array);

    ASSERT_TRUE(array.PopBack());
    ASSERT_EQ(0u, array.Size());

    ASSERT_FALSE(array.PopBack());
}

TEST(DynArray, CopyConstructor)
{
    using Type = CopyOnlyTestClass<int>;
    using ArrayType = DynArray<Type>;

    ClassMethodCallCounters counters;
    const Type value1(&counters, 1);
    const Type value2(&counters, 2);

    EXPECT_EQ(2, counters.constructor);
    EXPECT_EQ(0, counters.moveConstructor);
    EXPECT_EQ(0, counters.copyConstructor);
    EXPECT_EQ(0, counters.moveAssignment);
    EXPECT_EQ(0, counters.assignment);
    EXPECT_EQ(0, counters.destructor);

    {
        ArrayType array;
        array.Reserve(2);
        {
            auto iter = array.PushBack(value1);
            ASSERT_NE(array.End(), iter);
            iter = array.PushBack(value2);
            ASSERT_NE(array.End(), iter);
        }
        // array = [1, 2]
        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(0, counters.moveConstructor);
        EXPECT_EQ(2, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // copy constructor
        const ArrayType array2(array);
        // array = []
        // array2 = [1, 2]

        ArrayType::ConstIterator iter;
        const Type* ptr;
        const Type* ptr2;

        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(0, counters.moveConstructor);
        EXPECT_EQ(4, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        ASSERT_EQ(2u, array.Size());
        ASSERT_EQ(2u, array2.Size());

        // convert "array" to const to force use of ConstIterator
        const ArrayType& constSet = array;
        iter = constSet.Find(value1);
        ASSERT_NE(constSet.End(), iter);
        ptr = &(*iter);

        iter = array2.Find(value1);
        ASSERT_NE(array2.End(), iter);
        ptr2 = &(*iter);

        // compare pointer to the same value in both sets - they should be different
        EXPECT_NE(ptr, ptr2);
    }

    EXPECT_EQ(2, counters.constructor);
    EXPECT_EQ(0, counters.moveConstructor);
    EXPECT_EQ(4, counters.copyConstructor);
    EXPECT_EQ(0, counters.moveAssignment);
    EXPECT_EQ(0, counters.assignment);
    EXPECT_EQ(4, counters.destructor);
}

TEST(DynArray, Assign)
{
    using Type = CopyOnlyTestClass<int>;
    using ArrayType = DynArray<Type>;

    ClassMethodCallCounters counters;
    const Type value1(&counters, 1);
    const Type value2(&counters, 2);
    const Type value3(&counters, 3);
    const Type value4(&counters, 4);

    EXPECT_EQ(4, counters.constructor);
    EXPECT_EQ(0, counters.moveConstructor);
    EXPECT_EQ(0, counters.copyConstructor);
    EXPECT_EQ(0, counters.moveAssignment);
    EXPECT_EQ(0, counters.assignment);
    EXPECT_EQ(0, counters.destructor);

    {
        ArrayType array;
        array.Reserve(2);
        {
            auto iter = array.PushBack(value1);
            ASSERT_NE(array.End(), iter);
            iter = array.PushBack(value2);
            ASSERT_NE(array.End(), iter);
        }
        // array = [1, 2]
        EXPECT_EQ(4, counters.constructor);
        EXPECT_EQ(0, counters.moveConstructor);
        EXPECT_EQ(2, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        ArrayType array2;
        array2.Reserve(2);
        {
            auto iter = array2.PushBack(value3);
            ASSERT_NE(array2.End(), iter);
            iter = array2.PushBack(value4);
            ASSERT_NE(array2.End(), iter);
        }
        // array = [1, 2]
        // array2 = [3, 4]
        EXPECT_EQ(4, counters.constructor);
        EXPECT_EQ(0, counters.moveConstructor);
        EXPECT_EQ(4, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // assignment operator
        array2 = array;
        // array = [1, 2]
        // array2 = [1, 2]
        EXPECT_EQ(4, counters.constructor);
        EXPECT_EQ(0, counters.moveConstructor);
        EXPECT_EQ(6, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(2, counters.destructor);

        ASSERT_EQ(2u, array.Size());
        ASSERT_EQ(2u, array2.Size());

        // convert "array" to const to force use of ConstIterator
        auto iter = array.Find(value1);
        ASSERT_NE(array.End(), iter);
        const Type* ptr = &(*iter);

        iter = array2.Find(value1);
        ASSERT_NE(array2.End(), iter);
        const Type* ptr2 = &(*iter);

        // compare pointer to the same value in both sets - they should be different
        EXPECT_NE(ptr, ptr2);
    }

    EXPECT_EQ(4, counters.constructor);
    EXPECT_EQ(0, counters.moveConstructor);
    EXPECT_EQ(6, counters.copyConstructor);
    EXPECT_EQ(0, counters.moveAssignment);
    EXPECT_EQ(0, counters.assignment);
    EXPECT_EQ(6, counters.destructor);
}

TEST(DynArray, MoveConstructor)
{
    using Type = MoveOnlyTestClass<int>;
    using ArrayType = DynArray<Type>;

    ClassMethodCallCounters counters;
    {
        // initialize source DynArray
        ArrayType array;
        array.Reserve(2);
        {
            auto iter = array.PushBack(Type(&counters, 1));
            ASSERT_NE(array.End(), iter);
            iter = array.PushBack(Type(&counters, 2));
            ASSERT_NE(array.End(), iter);
        }
        // array = [1, 2]
        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(2, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // call the move constructor
        ArrayType array2(std::move(array));
        // array = []
        // array2 = [1, 2]
        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(2, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // "array" should be now empty
        EXPECT_EQ(0u, array.Size());

        // check if elements were moved
        EXPECT_EQ(2u, array2.Size());
        EXPECT_NE(array2.End(), array2.Find(Type(nullptr, 1)));
        EXPECT_NE(array2.End(), array2.Find(Type(nullptr, 2)));
    }

    EXPECT_EQ(2, counters.constructor);
    EXPECT_EQ(2, counters.moveConstructor);
    EXPECT_EQ(0, counters.copyConstructor);
    EXPECT_EQ(0, counters.moveAssignment);
    EXPECT_EQ(0, counters.assignment);
    EXPECT_EQ(2, counters.destructor);
}

TEST(DynArray, MoveAssignment)
{
    using Type = MoveOnlyTestClass<int>;
    using ArrayType = DynArray<Type>;

    ClassMethodCallCounters counters;
    {
        // initialize target array
        ArrayType array;
        array.Reserve(2);
        {
            auto iter = array.PushBack(Type(&counters, 1));
            ASSERT_NE(array.End(), iter);
            iter = array.PushBack(Type(&counters, 2));
            ASSERT_NE(array.End(), iter);
        }
        // array = [1, 2]
        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(2, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // initialize source array
        ArrayType array2;
        array2.Reserve(2);
        {
            auto iter = array2.PushBack(Type(&counters, 3));
            ASSERT_NE(array2.End(), iter);
            iter = array2.PushBack(Type(&counters, 4));
            ASSERT_NE(array2.End(), iter);
        }
        // array = [1, 2]
        // array2 = [3, 4]
        EXPECT_EQ(4, counters.constructor);
        EXPECT_EQ(4, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // move assign
        array2 = std::move(array);
        // array = []
        // array2 = [1, 2]
        EXPECT_EQ(4, counters.constructor);
        EXPECT_EQ(4, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(2, counters.destructor);

        // "array" should be now empty
        EXPECT_EQ(0u, array.Size());

        // check if elements were moved
        EXPECT_EQ(2u, array2.Size());
        EXPECT_NE(array2.End(), array2.Find(Type(nullptr, 1)));
        EXPECT_NE(array2.End(), array2.Find(Type(nullptr, 2)));
    }

    EXPECT_EQ(4, counters.constructor);
    EXPECT_EQ(4, counters.moveConstructor);
    EXPECT_EQ(0, counters.copyConstructor);
    EXPECT_EQ(0, counters.moveAssignment);
    EXPECT_EQ(0, counters.assignment);
    EXPECT_EQ(4, counters.destructor);
}

TEST(DynArray, PushBackArray)
{
    DynArray<int> array;
    array.PushBack(10);
    array.PushBack(20);
    array.PushBack(30);
    array.PushBack(40);

    int elements[] = { 100, 200, 300, 400, 500 };
    const ArrayView<int> toInsert(elements, 5);

    ASSERT_TRUE(array.PushBackArray(toInsert));
    ASSERT_EQ(9u, array.Size());

    int expectedElements[] = { 10, 20, 30, 40, 100, 200, 300, 400, 500 };
    const ArrayView<int> expected(expectedElements, 9);
    EXPECT_EQ(expected, array);
}

TEST(DynArray, PushBackConstArray)
{
    DynArray<int> array;
    array.PushBack(10);
    array.PushBack(20);
    array.PushBack(30);
    array.PushBack(40);

    const int elements[] = { 100, 200, 300, 400, 500 };
    const ArrayView<const int> toInsert(elements, 5);

    ASSERT_TRUE(array.PushBackArray(toInsert));
    ASSERT_EQ(9u, array.Size());

    int expectedElements[] = { 10, 20, 30, 40, 100, 200, 300, 400, 500 };
    const ArrayView<int> expected(expectedElements, 9);
    EXPECT_EQ(expected, array);
}

TEST(DynArray, InsertAt_Front)
{
    DynArray<int> array({ 10, 20, 30, 40, 50 });
    auto iter = array.InsertAt(0, 111);
    ASSERT_NE(array.End(), iter);

    int expectedElements[] = { 111, 10, 20, 30, 40, 50 };
    const ArrayView<int> expected(expectedElements, 6);
    EXPECT_EQ(expected, array);
}

TEST(DynArray, InsertAt_Back)
{
    DynArray<int> array({ 10, 20, 30, 40, 50 });
    auto iter = array.InsertAt(5, 111);
    ASSERT_NE(array.End(), iter);

    int expectedElements[] = { 10, 20, 30, 40, 50, 111 };
    const ArrayView<int> expected(expectedElements, 6);
    EXPECT_EQ(expected, array);
}

TEST(DynArray, InsertAt_Middle)
{
    DynArray<int> array({ 10, 20, 30, 40, 50 });
    auto iter = array.InsertAt(2, 111);
    ASSERT_NE(array.End(), iter);

    int expectedElements[] = { 10, 20, 111, 30, 40, 50 };
    const ArrayView<int> expected(expectedElements, 6);
    EXPECT_EQ(expected, array);
}

TEST(DynArray, InsertArrayAt_Empty)
{
    DynArray<int> array({ 10, 20, 30, 40, 50 });
    const ArrayView<int> toInsert;

    auto iter = array.InsertArrayAt(0, toInsert);
    ASSERT_EQ(array.End(), iter);

    iter = array.InsertArrayAt(2, toInsert);
    ASSERT_EQ(array.End(), iter);

    iter = array.InsertArrayAt(5, toInsert);
    ASSERT_EQ(array.End(), iter);
}

TEST(DynArray, InsertArrayAt_Front)
{
    int elementsToInsert[] = { 111, 222, 333 };
    const ArrayView<int> toInsert(elementsToInsert, 3);

    DynArray<int> array({ 10, 20, 30, 40, 50 });
    auto iter = array.InsertArrayAt(0, toInsert);
    ASSERT_NE(array.End(), iter);

    int expectedElements[] = { 111, 222, 333, 10, 20, 30, 40, 50 };
    const ArrayView<int> expected(expectedElements, 8);
    EXPECT_EQ(expected, array);
}

TEST(DynArray, InsertAt_Multiple)
{
    DynArray<int> array({ 10, 20, 30, 40, 50 });
    auto iter = array.InsertAt(1, 111, 3);
    ASSERT_NE(array.End(), iter);

    int expectedElements[] = { 10, 111, 111, 111, 20, 30, 40, 50 };
    const ArrayView<int> expected(expectedElements, 8);
    EXPECT_EQ(expected, array);
}

TEST(DynArray, InsertArrayAt_Back)
{
    int elementsToInsert[] = { 111, 222, 333 };
    const ArrayView<int> toInsert(elementsToInsert, 3);

    DynArray<int> array({ 10, 20, 30, 40, 50 });
    auto iter = array.InsertArrayAt(5, toInsert);
    ASSERT_NE(array.End(), iter);

    int expectedElements[] = { 10, 20, 30, 40, 50, 111, 222, 333 };
    const ArrayView<int> expected(expectedElements, 8);
    EXPECT_EQ(expected, array);
}

TEST(DynArray, InsertArrayAt_Middle)
{
    int elementsToInsert[] = { 111, 222, 333 };
    const ArrayView<int> toInsert(elementsToInsert, 3);

    DynArray<int> array({ 10, 20, 30, 40, 50 });
    auto iter = array.InsertArrayAt(2, toInsert);
    ASSERT_NE(array.End(), iter);

    int expectedElements[] = { 10, 20, 111, 222, 333, 30, 40, 50 };
    const ArrayView<int> expected(expectedElements, 8);
    EXPECT_EQ(expected, array);
}

TEST(DynArray, Erase_Front)
{
    DynArray<int> array({ 10, 20, 30, 40, 50 });

    EXPECT_TRUE(array.Erase(array.Begin()));

    int expectedElements[] = { 20, 30, 40, 50 };
    const ArrayView<int> expected(expectedElements, 4);
    EXPECT_EQ(expected, array);
}

TEST(DynArray, Erase_Middle)
{
    DynArray<int> array({ 10, 20, 30, 40, 50 });

    EXPECT_TRUE(array.Erase(array.Begin() + 2));

    int expectedElements[] = { 10, 20, 40, 50 };
    const ArrayView<int> expected(expectedElements, 4);
    EXPECT_EQ(expected, array);
}

TEST(DynArray, Erase_Back)
{
    DynArray<int> array({ 10, 20, 30, 40, 50 });

    EXPECT_TRUE(array.Erase(array.End() - 1));

    int expectedElements[] = { 10, 20, 30, 40 };
    const ArrayView<int> expected(expectedElements, 4);
    EXPECT_EQ(expected, array);
}

TEST(DynArray, Erase_Range_Front)
{
    DynArray<int> array({ 10, 20, 30, 40, 50 });

    EXPECT_TRUE(array.Erase(array.Begin(), array.Begin() + 2));

    int expectedElements[] = { 30, 40, 50 };
    const ArrayView<int> expected(expectedElements, 3);
    EXPECT_EQ(expected, array);
}

TEST(DynArray, Erase_Range_Middle)
{
    DynArray<int> array({ 10, 20, 30, 40, 50 });

    EXPECT_TRUE(array.Erase(array.Begin() + 1, array.Begin() + 3));

    int expectedElements[] = { 10, 40, 50 };
    const ArrayView<int> expected(expectedElements, 3);
    EXPECT_EQ(expected, array);
}

TEST(DynArray, Erase_Range_End)
{
    DynArray<int> array({ 10, 20, 30, 40, 50 });

    EXPECT_TRUE(array.Erase(array.Begin() + 3, array.End()));

    int expectedElements[] = { 10, 20, 30 };
    const ArrayView<int> expected(expectedElements, 3);
    EXPECT_EQ(expected, array);
}

TEST(DynArray, Erase_Range_Everything)
{
    DynArray<int> array({ 10, 20, 30, 40, 50 });

    EXPECT_TRUE(array.Erase(array.Begin(), array.End()));
    ASSERT_TRUE(array.Empty());
}

TEST(DynArray, Erase_Range_Empty)
{
    DynArray<int> array({ 10, 20, 30, 40, 50 });

    EXPECT_FALSE(array.Erase(array.Begin(), array.Begin()));
    ASSERT_EQ(5u, array.Size());

    EXPECT_FALSE(array.Erase(array.Begin() + 1, array.Begin()));
    ASSERT_EQ(5u, array.Size());

    EXPECT_FALSE(array.Erase(array.Begin() + 1, array.Begin() + 1));
    ASSERT_EQ(5u, array.Size());

    EXPECT_FALSE(array.Erase(array.End(), array.End()));
    ASSERT_EQ(5u, array.Size());
}