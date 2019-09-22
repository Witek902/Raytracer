#pragma once

#include "ArrayView.h"

namespace rt {

template<typename ElementType>
ArrayView<ElementType>::ArrayView()
    : mElements(nullptr)
    , mSize(0)
{
}

template<typename ElementType>
ArrayView<ElementType>::ArrayView(ElementType* elements, uint32 numElements)
    : mElements(elements)
    , mSize(numElements)
{
}

template<typename ElementType>
template<typename ElementType2>
ArrayView<ElementType>::ArrayView(const ArrayView<ElementType2>& other)
{
    static_assert(std::is_same<typename std::remove_cv<ElementType>::type, ElementType2>::value,
                  "Only (non-const -> const) ArrayView element type conversion is supported");

    mElements = other.Data();
    mSize = other.Size();
}

template<typename ElementType>
template<typename ElementType2>
ArrayView<ElementType>& ArrayView<ElementType>::operator = (const ArrayView<ElementType2>& other)
{
    static_assert(std::is_same<typename std::remove_cv<ElementType>::type, ElementType2>::value,
                  "Only (non-const -> const) ArrayView element type conversion is supported");

    mElements = other.Data();
    mSize = other.Size();
    return *this;
}

//////////////////////////////////////////////////////////////////////////

template<typename ElementType>
uint32 ArrayView<ElementType>::Size() const
{
    return mSize;
}

template<typename ElementType>
bool ArrayView<ElementType>::Empty() const
{
    return mSize == 0;
}

template<typename ElementType>
const ElementType* ArrayView<ElementType>::Data() const
{
    return mElements;
}

template<typename ElementType>
ElementType* ArrayView<ElementType>::Data()
{
    return mElements;
}

template<typename ElementType>
const ElementType& ArrayView<ElementType>::Front() const
{
    RT_ASSERT(mSize > 0, "Array is empty");
    return mElements[0];
}

template<typename ElementType>
ElementType& ArrayView<ElementType>::Front()
{
    RT_ASSERT(mSize > 0, "Array is empty");
    return mElements[0];
}

template<typename ElementType>
const ElementType& ArrayView<ElementType>::Back() const
{
    RT_ASSERT(mSize > 0, "Array is empty");
    return mElements[mSize - 1];
}

template<typename ElementType>
ElementType& ArrayView<ElementType>::Back()
{
    static_assert(!std::is_const<ElementType>::value, "You can only use ConstIterator for const-typed ArrayView");
    RT_ASSERT(mSize > 0, "Array is empty");
    return mElements[mSize - 1];
}

template<typename ElementType>
typename ArrayView<ElementType>::ConstIterator ArrayView<ElementType>::Begin() const
{
    return ConstIterator(mElements, 0);
}

template<typename ElementType>
typename ArrayView<ElementType>::Iterator ArrayView<ElementType>::Begin()
{
    return Iterator(mElements, 0);
}

template<typename ElementType>
typename ArrayView<ElementType>::ConstIterator ArrayView<ElementType>::End() const
{
    return ConstIterator(mElements, mSize);
}

template<typename ElementType>
typename ArrayView<ElementType>::Iterator ArrayView<ElementType>::End()
{
    return Iterator(mElements, mSize);
}

template<typename ElementType>
ElementType& ArrayView<ElementType>::operator[](uint32 index)
{
    static_assert(!std::is_const<ElementType>::value, "You can only use const-reference to access const-typed ArrayView elements");
    RT_ASSERT(index < mSize, "Invalid array index %u (size is %u)", index, mSize);
    return mElements[index];
}

template<typename ElementType>
const ElementType& ArrayView<ElementType>::operator[](uint32 index) const
{
    RT_ASSERT(index < mSize, "Invalid array index %u (size is %u)", index, mSize);
    return mElements[index];
}

template<typename ElementType>
ArrayView<ElementType> ArrayView<ElementType>::Range(uint32 index, uint32 size) const
{
    RT_ASSERT(index < mSize, "Invalid array index %u (size is %u)", index, mSize);
    RT_ASSERT(index + size < mSize + 1, "Subrange exceedes array size (last index is %u, size is %u)", index + size, mSize);
    return ArrayView(mElements + index, size);
}

template<typename ElementType>
typename ArrayView<ElementType>::ConstIterator ArrayView<ElementType>::Find(const ElementType& element) const
{
    for (uint32 i = 0; i < mSize; ++i)
    {
        if (mElements[i] == element)
        {
            return ConstIterator(mElements, i);
        }
    }

    return End();
}

template<typename ElementType>
typename ArrayView<ElementType>::Iterator ArrayView<ElementType>::Find(const ElementType& element)
{
    for (uint32 i = 0; i < mSize; ++i)
    {
        if (mElements[i] == element)
        {
            return Iterator(mElements, i);
        }
    }

    return End();
}

template<typename ElementTypeA, typename ElementTypeB>
bool operator == (const ArrayView<ElementTypeA>& lhs, const ArrayView<ElementTypeB>& rhs)
{
    static_assert(std::is_same<typename std::remove_cv<ElementTypeA>::type,
                               typename std::remove_cv<ElementTypeB>::type>::value,
                  "ArrayView types are incompatible");

    if (lhs.Size() != rhs.Size())
        return false;

    for (uint32 i = 0; i < lhs.Size(); ++i)
    {
        if (lhs[i] != rhs[i])
        {
            return false;
        }
    }

    return true;
}

template<typename ElementTypeA, typename ElementTypeB>
bool operator != (const ArrayView<ElementTypeA>& lhs, const ArrayView<ElementTypeB>& rhs)
{
    static_assert(std::is_same<typename std::remove_cv<ElementTypeA>::type,
                               typename std::remove_cv<ElementTypeB>::type>::value,
                  "ArrayView types are incompatible");

    if (lhs.Size() != rhs.Size())
        return true;

    for (uint32 i = 0; i < lhs.Size(); ++i)
    {
        if (lhs[i] != rhs[i])
        {
            return true;
        }
    }

    return false;
}

template<typename ElementTypeA, typename ElementTypeB>
bool operator < (const ArrayView<ElementTypeA>& lhs, const ArrayView<ElementTypeB>& rhs)
{
    static_assert(std::is_same<typename std::remove_cv<ElementTypeA>::type,
                               typename std::remove_cv<ElementTypeB>::type>::value,
                  "ArrayView types are incompatible");

    if (lhs.Size() < rhs.Size())
        return true;

    if (lhs.Size() > rhs.Size())
        return false;

    for (uint32 i = 0; i < lhs.Size(); ++i)
    {
        if (lhs[i] < rhs[i])
            return true;

        if (lhs[i] > rhs[i])
            return false;
    }

    // arrays are equal
    return false;
}

template<typename ElementTypeA, typename ElementTypeB>
bool operator > (const ArrayView<ElementTypeA>& lhs, const ArrayView<ElementTypeB>& rhs)
{
    return rhs < lhs;
}

template<typename ElementTypeA, typename ElementTypeB>
bool operator <= (const ArrayView<ElementTypeA>& lhs, const ArrayView<ElementTypeB>& rhs)
{
    static_assert(std::is_same<typename std::remove_cv<ElementTypeA>::type,
                               typename std::remove_cv<ElementTypeB>::type>::value,
                  "ArrayView types are incompatible");

    if (lhs.Size() < rhs.Size())
        return true;

    if (lhs.Size() > rhs.Size())
        return false;

    for (uint32 i = 0; i < lhs.Size(); ++i)
    {
        if (lhs[i] < rhs[i])
            return true;

        if (lhs[i] > rhs[i])
            return false;
    }

    // arrays are equal
    return true;
}

template<typename ElementTypeA, typename ElementTypeB>
bool operator >= (const ArrayView<ElementTypeA>& lhs, const ArrayView<ElementTypeB>& rhs)
{
    return rhs <= lhs;
}

template<typename ElementType>
uint32 GetHash(const ArrayView<ElementType>& arrayView)
{
    // hashing algorithm based on boost's hash_combine

    uint32 seed = 0;
    for (const ElementType& element : arrayView)
    {
        seed ^= GetHash(element) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}


} // namespace rt
