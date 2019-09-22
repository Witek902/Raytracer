#pragma once

#include "DynArray.h"
#include "../Math/Math.h"
#include "../Utils/MemoryHelpers.h"
#include "../Utils/Memory.h"

namespace rt {

template<typename ElementType, typename Allocator>
DynArray<ElementType, Allocator>::DynArray()
    : mAllocSize(0)
{
    static_assert(sizeof(DynArray<ElementType, Allocator>) == sizeof(void*) + 2 * sizeof(uint32), "Invalid DynArray size");
}

template<typename ElementType, typename Allocator>
DynArray<ElementType, Allocator>::~DynArray()
{
    Clear(true);
}

template<typename ElementType, typename Allocator>
DynArray<ElementType, Allocator>::DynArray(const DynArray& other)
    : ArrayView<ElementType>()
{
    mAllocSize = 0;
    this->mElements = nullptr;
    this->mSize = 0;

    if (!Reserve(other.mSize))
    {
        RT_FATAL("Failed to reserve memory for DynArray");
        return;
    }

    this->mSize = other.mSize;
    for (uint32 i = 0; i < other.mSize; ++i)
    {
        new (this->mElements + i) ElementType(other.mElements[i]);
    }
}

template<typename ElementType, typename Allocator>
DynArray<ElementType, Allocator>::DynArray(DynArray&& other)
    : ArrayView<ElementType>()
{
    // don't free memory if not needed
    Clear(true);

    this->mElements = other.mElements;
    this->mSize = other.mSize;
    mAllocSize = other.mAllocSize;

    other.mElements = nullptr;
    other.mSize = 0;
    other.mAllocSize = 0;
}

template<typename ElementType, typename Allocator>
DynArray<ElementType, Allocator>& DynArray<ElementType, Allocator>::operator = (const DynArray& other)
{
    if (&other == this)
        return *this;

    Clear();

    if (!Reserve(other.mSize))
    {
        RT_ASSERT("Failed to reserve memory for DynArray");
        return *this;
    }

    this->mSize = other.mSize;
    for (uint32 i = 0; i < other.mSize; ++i)
    {
        new (this->mElements + i) ElementType(other.mElements[i]);
    }

    return *this;
}

template<typename ElementType, typename Allocator>
DynArray<ElementType, Allocator>& DynArray<ElementType, Allocator>::operator = (DynArray&& other)
{
    // don't free memory if not needed
    Clear(true);

    this->mElements = other.mElements;
    this->mSize = other.mSize;
    mAllocSize = other.mAllocSize;

    other.mElements = nullptr;
    other.mSize = 0;
    other.mAllocSize = 0;

    return *this;
}

template<typename ElementType, typename Allocator>
DynArray<ElementType, Allocator>::DynArray(const std::initializer_list<ElementType>& list)
    : DynArray()
{
    if (!Reserve(static_cast<uint32>(list.size())))
    {
        RT_FATAL("Failed to reserve memory for DynArray");
        return;
    }

    for (const ElementType element : list)
    {
        PushBack(element);
    }
}

template<typename ElementType, typename Allocator>
DynArray<ElementType, Allocator>::DynArray(const ElementType* elements, uint32 count)
    : DynArray()
{
    if (!Reserve(count))
    {
        RT_FATAL("Failed to reserve memory for DynArray");
        return;
    }

    this->mSize = count;
    for (uint32 i = 0; i < count; ++i)
    {
        new (this->mElements + i) ElementType(elements[i]);
    }
}

template<typename ElementType, typename Allocator>
DynArray<ElementType, Allocator>::DynArray(uint32 size)
    : DynArray()
{
    static_assert(std::is_trivially_constructible<ElementType>::value, "Element type is not trivially constructible");

    if (!Reserve(size))
    {
        RT_FATAL("Failed to reserve memory for DynArray");
        return;
    }

    this->mSize = size;
    for (uint32 i = 0; i < size; ++i)
    {
        new (this->mElements + i) ElementType();
    }
}

template<typename ElementType, typename Allocator>
DynArray<ElementType, Allocator>::DynArray(uint32 size, const ElementType& value)
    : DynArray()
{
    if (!Reserve(size))
    {
        RT_FATAL("Failed to reserve memory for DynArray");
        return;
    }

    this->mSize = size;
    for (uint32 i = 0; i < size; ++i)
    {
        new (this->mElements + i) ElementType(value);
    }
}

//////////////////////////////////////////////////////////////////////////

template<typename ElementType, typename Allocator>
void DynArray<ElementType, Allocator>::Clear(bool freeMemory)
{
    if (!this->mElements)
    {
        // already empty
        return;
    }

    // call destructors
    for (uint32 i = 0; i < this->mSize; ++i)
    {
        this->mElements[i].~ElementType();
    }

    this->mSize = 0;

    if (freeMemory)
    {
        Allocator::Free(this->mElements);
        this->mElements = nullptr;
        mAllocSize = 0;
    }
}

template<typename ElementType, typename Allocator>
bool DynArray<ElementType, Allocator>::ContainsElement(const ElementType& element) const
{
    return (&element - this->mElements >= 0) && (&element < this->mElements + this->mSize);
}

template<typename ElementType, typename Allocator>
typename DynArray<ElementType, Allocator>::IteratorType DynArray<ElementType, Allocator>::PushBack(const ElementType& element)
{
    RT_ASSERT(!ContainsElement(element), "Adding element to a DynArray that is already contained by the array is not supported");

    if (!Reserve(this->mSize + 1))
    {
        return this->End();
    }

    new (this->mElements + this->mSize) ElementType(element);
    return IteratorType(this->mElements, this->mSize++);
}

template<typename ElementType, typename Allocator>
typename DynArray<ElementType, Allocator>::IteratorType DynArray<ElementType, Allocator>::PushBack(ElementType&& element)
{
    RT_ASSERT(!ContainsElement(element), "Adding element to a DynArray that is already contained by the array is not supported");

    if (!Reserve(this->mSize + 1))
    {
        // memory allocation failed
        return this->End();
    }

    new (this->mElements + this->mSize) ElementType(std::move(element));
    return IteratorType(this->mElements, this->mSize++);
}

template<typename ElementType, typename Allocator>
template<typename ... Args>
typename DynArray<ElementType, Allocator>::IteratorType DynArray<ElementType, Allocator>::EmplaceBack(Args&& ... args)
{
    if (!Reserve(this->mSize + 1))
    {
        // memory allocation failed
        return this->End();
    }

    new (this->mElements + this->mSize) ElementType(std::forward<Args>(args) ...);
    return IteratorType(this->mElements, this->mSize++);
}

template<typename ElementType, typename Allocator>
template<typename ElementType2>
bool DynArray<ElementType, Allocator>::PushBackArray(const ArrayView<ElementType2>& arrayView)
{
    static_assert(std::is_same<typename std::remove_cv<ElementType>::type, typename std::remove_cv<ElementType2>::type>::value,
                  "Incompatible element types");

    if (arrayView.Empty())
    {
        // empty array - do nothing
        return true;
    }

    if (!Reserve(this->mSize + arrayView.mSize))
    {
        // memory allocation failed
        return false;
    }

    // copy elements
    for (uint32 i = 0; i < arrayView.mSize; ++i)
    {
        new (this->mElements + this->mSize + i) ElementType(arrayView.mElements[i]);
    }

    this->mSize += arrayView.mSize;
    return true;
}

template<typename ElementType, typename Allocator>
bool DynArray<ElementType, Allocator>::PopBack()
{
    if (this->Empty())
        return false;

    this->mElements[--this->mSize].~ElementType();
    return true;
}

template<typename ElementType, typename Allocator>
typename DynArray<ElementType, Allocator>::IteratorType DynArray<ElementType, Allocator>::InsertAt(uint32 index, const ElementType& element)
{
    if (!Reserve(this->mSize + 1))
    {
        // memory allocation failed
        return this->End();
    }

    ElementType* base = this->mElements + index;
    MemoryHelpers::MoveArray<ElementType>(base + 1, base, this->mSize - index);
    new (base) ElementType(element);
    this->mSize++;
    return IteratorType(this->mElements, index);
}

template<typename ElementType, typename Allocator>
typename DynArray<ElementType, Allocator>::IteratorType DynArray<ElementType, Allocator>::InsertAt(uint32 index, ElementType&& element)
{
    if (!Reserve(this->mSize + 1))
    {
        // memory allocation failed
        return this->End();
    }

    ElementType* base = this->mElements + index;
    MemoryHelpers::MoveArray<ElementType>(base + 1, base, this->mSize - index);
    new (base) ElementType(std::move(element));
    this->mSize++;
    return IteratorType(this->mElements, index);
}

template<typename ElementType, typename Allocator>
typename DynArray<ElementType, Allocator>::IteratorType DynArray<ElementType, Allocator>::InsertAt(uint32 index, const ElementType& element, uint32 count)
{
    if (count == 0)
    {
        // nothing to DO
        return this->End();
    }

    if (!Reserve(this->mSize + count))
    {
        // memory allocation failed
        return this->End();
    }

    ElementType* base = this->mElements + index;
    MemoryHelpers::MoveArray<ElementType>(base + count, base, this->mSize - index);

    for (uint32 i = 0; i < count; ++i)
    {
        new (base + i) ElementType(element);
    }

    this->mSize += count;
    return IteratorType(this->mElements, index);
}

template<typename ElementType, typename Allocator>
template<typename ElementType2>
typename DynArray<ElementType, Allocator>::IteratorType DynArray<ElementType, Allocator>::InsertArrayAt(uint32 index, const ArrayView<ElementType2>& arrayView)
{
    static_assert(std::is_same<typename std::remove_cv<ElementType>::type, typename std::remove_cv<ElementType2>::type>::value,
                  "Incompatible element types");

    if (arrayView.Empty())
    {
        // trying to insert empty array - do nothing
        return this->End();
    }

    if (!Reserve(this->mSize + arrayView.mSize))
    {
        // memory allocation failed
        return this->End();
    }

    ElementType* base = this->mElements + index;
    MemoryHelpers::MoveArray<ElementType>(base + arrayView.mSize, base, this->mSize - index);

    for (uint32 i = 0; i < arrayView.mSize; ++i)
    {
        new (base + i) ElementType(arrayView.mElements[i]);
    }

    this->mSize += arrayView.mSize;
    return IteratorType(this->mElements, index);
}

template<typename ElementType, typename Allocator>
bool DynArray<ElementType, Allocator>::Erase(const ConstIteratorType& iterator)
{
    if (iterator == this->End())
    {
        return false;
    }

    this->mElements[iterator.GetIndex()].~ElementType();

    ElementType* base = this->mElements + iterator.GetIndex();
    MemoryHelpers::MoveArray<ElementType>(base, base + 1, this->mSize - iterator.GetIndex() - 1);
    this->mSize--;
    return true;
}

template<typename ElementType, typename Allocator>
bool DynArray<ElementType, Allocator>::Erase(const ConstIteratorType& first, const ConstIteratorType& last)
{
    if (first.GetIndex() >= last.GetIndex())
    {
        // nothing to do
        return false;
    }

    // call destructors
    for (int32 i = first.GetIndex(); i < last.GetIndex(); ++i)
    {
        this->mElements[i].~ElementType();
    }

    const int32 num = last.GetIndex() - first.GetIndex();
    ElementType* base = this->mElements + first.GetIndex();
    MemoryHelpers::MoveArray<ElementType>(base, base + num, this->mSize - last.GetIndex());
    this->mSize -= num;
    return true;
}

template<typename ElementType, typename Allocator>
bool DynArray<ElementType, Allocator>::Reserve(uint32 size)
{
    if (size <= mAllocSize)
    {
        // smaller that allocated - ignore
        return true;
    }

    uint32 newAllocSize = mAllocSize;
    while (size > newAllocSize)
    {
        // grow by 50%
        newAllocSize += math::Max<uint32>(1, newAllocSize / 2);
    }

    ElementType* newBuffer = static_cast<ElementType*>(Allocator::Allocate(newAllocSize * sizeof(ElementType), alignof(ElementType)));
    if (!newBuffer)
    {
        // memory allocation failed
        return false;
    }

    // move elements
    MemoryHelpers::MoveArray<ElementType>(newBuffer, this->mElements, this->mSize);

    // replace buffer
    Allocator::Free(this->mElements);
    this->mElements = newBuffer;
    mAllocSize = newAllocSize;
    return true;
}

template<typename ElementType, typename Allocator>
bool DynArray<ElementType, Allocator>::Resize_SkipConstructor(uint32 size)
{
    const uint32 oldSize = this->mSize;

    // call destructors
    for (uint32 i = size; i < oldSize; ++i)
    {
        this->mElements[i].~ElementType();
    }

    if (!Reserve(size))
    {
        return false;
    }

    this->mSize = size;
    return true;
}

template<typename ElementType, typename Allocator>
bool DynArray<ElementType, Allocator>::Resize(uint32 size)
{
    const uint32 oldSize = this->mSize;

    // call destructors
    for (uint32 i = size; i < oldSize; ++i)
    {
        this->mElements[i].~ElementType();
    }

    if (!Reserve(size))
    {
        return false;
    }

    // initialize new elements
    for (uint32 i = oldSize; i < size; ++i)
    {
        new (this->mElements + i) ElementType;
    }

    this->mSize = size;
    return true;
}

template<typename ElementType, typename Allocator>
bool DynArray<ElementType, Allocator>::Resize(uint32 size, const ElementType& defaultElement)
{
    const uint32 oldSize = this->mSize;

    // call destructors
    for (uint32 i = size; i < oldSize; ++i)
    {
        this->mElements[i].~ElementType();
    }

    if (!Reserve(size))
    {
        return false;
    }

    // initialize new elements
    for (uint32 i = oldSize; i < size; ++i)
    {
        new (this->mElements + i) ElementType(defaultElement);
    }

    this->mSize = size;
    return true;
}

template<typename ElementType, typename Allocator>
void DynArray<ElementType, Allocator>::Swap(DynArray& other)
{
    std::swap(this->mElements, other->mElements);
    std::swap(this->mSize, other->mSize);
    std::swap(mAllocSize, other.mAllocSize);
}


} // namespace rt
