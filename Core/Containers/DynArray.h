#pragma once

#include "ArrayView.h"
#include "../Utils/Memory.h"

namespace rt {

/**
 * Dynamic array (like std::vector).
 */
template<typename ElementType, typename Allocator = DefaultAllocator>
class DynArray : public ArrayView<ElementType>
{
public:
    using IteratorType = typename ArrayView<ElementType>::Iterator;
    using ConstIteratorType = typename ArrayView<ElementType>::ConstIterator;

    // basic constructors and assignment operators
    RT_FORCE_INLINE DynArray();
    RT_FORCE_INLINE ~DynArray();
    RT_FORCE_INLINE DynArray(const DynArray& other);
    RT_FORCE_INLINE DynArray(DynArray&& other);
    RT_FORCE_INLINE DynArray& operator = (const DynArray& other);
    RT_FORCE_INLINE DynArray& operator = (DynArray&& other);

    // initialize using initializer list
    DynArray(const std::initializer_list<ElementType>& list);

    // initialize using C-style array
    DynArray(const ElementType* elements, uint32 count);

    // initialize with given size (creates default objects)
    // NOTE: ElementType must be trivially constructible
    explicit DynArray(uint32 size);

    // initialize with given size
    DynArray(uint32 size, const ElementType& value);

    /**
     * Remove all the elements.
     * @param freeMemory    Release memory?
     */
    void Clear(bool freeMemory = false);

    /**
     * Insert a new element at the end.
     * @return  Iterator to the inserted element, or iterator to the end if the insertion failed.
     */
    IteratorType PushBack(const ElementType& element);
    IteratorType PushBack(ElementType&& element);

    /**
     * In-place construct a new element at the end.
     * @return  Iterator to the inserted element, or iterator to the end if the insertion failed.
     */
    template<typename ... Args>
    IteratorType EmplaceBack(Args&& ... args);

    /**
     * Insert elements from a view at the end.
     * @return  'True' on success, 'false' on memory allocation failure.
     */
    template<typename ElementType2>
    bool PushBackArray(const ArrayView<ElementType2>& arrayView);

    /**
     * Insert a new element at given index.
     * @return  Iterator to the inserted element, or iterator to the end if the insertion failed.
     */
    IteratorType InsertAt(uint32 index, const ElementType& element);
    IteratorType InsertAt(uint32 index, ElementType&& element);

    /**
     * Insert a new element multiple times, at given index.
     * @return  Iterator to the first inserted element, or iterator to the end if the insertion failed.
     */
    IteratorType InsertAt(uint32 index, const ElementType& element, uint32 count);

    /**
     * Insert elements from a view at given index.
     * @param   index   Place, where elements will be inserted. First element in the view will have
     *                  this index after the operation.
     * @param   arrayView Array view to insert.
     * @return  Iterator to the inserted element, or iterator to the end if the insertion failed.
     */
    template<typename ElementType2>
    IteratorType InsertArrayAt(uint32 index, const ArrayView<ElementType2>& arrayView);

    /**
     * Remove an element by iterator.
     * @return  True if the element has been removed.
     */
    bool Erase(const ConstIteratorType& iterator);

    /**
     * Remove range of elements.
     * @return  True if the element has been removed.
     */
    bool Erase(const ConstIteratorType& first, const ConstIteratorType& last);

    /**
     * Remove last element if exists.
     * @return 'false' if there is nothing to pop (container is empty).
     */
    bool PopBack();

    /**
     * Reserve space.
     * @return  'false' if memory allocation failed.
     */
    bool Reserve(uint32 size);

    /**
     * Resize the array.
     * Element type must have default constructor.
     * @return 'false' if memory allocation failed.
     */
    bool Resize(uint32 size);
    bool Resize_SkipConstructor(uint32 size);
    bool Resize(uint32 size, const ElementType& defaultElement);

    /**
     * Replace contents of two arrays.
     * @note Does not call any constructor or destructor, just pointers are swapped.
     */
    void Swap(DynArray& other);

    // lower-case aliases for Begin()/End(), required by C++ for range-based 'for' to work
    RT_FORCE_INLINE ConstIteratorType begin() const { return this->Begin(); }
    RT_FORCE_INLINE ConstIteratorType end() const { return this->End(); }
    RT_FORCE_INLINE IteratorType begin() { return this->Begin(); }
    RT_FORCE_INLINE IteratorType end() { return this->End(); }

private:

    bool ContainsElement(const ElementType& element) const;

    // allocated size
    uint32 mAllocSize;
};

/**
 * Calculate hash of an dynamic array (simple wrapper for ArrayView's GetHash).
 */
template<typename ElementType>
RT_FORCE_INLINE uint32 GetHash(const DynArray<ElementType>& array)
{
    return GetHash(static_cast<const ArrayView<ElementType>&>(array));
}


} // namespace rt


#include "DynArrayImpl.h"
