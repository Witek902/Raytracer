#pragma once

#include "ArrayIterator.h"


namespace rt {

// this makes sure that sizeof(DynArray) == 16 on x64
#pragma pack(push, 4)

/**
 * A view of contiguous array of elements.
 * Access type (const vs. non-const) depends on ElementType.
 */
template<typename ElementType>
class ArrayView
{
    template<typename T, typename Allocator> friend class DynArray;

public:

    // maximum size of array view (1 element is reserved for iterator to the end)
    static constexpr Uint32 MaxSize = std::numeric_limits<Uint32>::max() - 1;

    using Iterator = ArrayIterator<ElementType>;
    using ConstIterator = ConstArrayIterator<ElementType>;

    // create empty view
    RT_FORCE_INLINE ArrayView();

    // create view of raw array
    RT_FORCE_INLINE ArrayView(ElementType* elements, Uint32 numElements);

    // copy/move constructor/assignment
    ArrayView(const ArrayView& other) = default;
    ArrayView& operator = (const ArrayView& other) = default;
    ArrayView(ArrayView&& other) = default;
    ArrayView& operator = (ArrayView&& other) = default;

    // copy constructor from read-write-typed to const-typed
    template<typename ElementType2>
    ArrayView(const ArrayView<ElementType2>& other);

    // copy assignment from read-write-typed to const-typed
    template<typename ElementType2>
    ArrayView& operator = (const ArrayView<ElementType2>& other);


    /**
     * Get number of elements.
     */
    RT_FORCE_INLINE Uint32 Size() const;

    /**
     * Get raw data pointed by the view.
     */
    RT_FORCE_INLINE const ElementType* Data() const;
    RT_FORCE_INLINE ElementType* Data();

    /**
     * Check if the array is empty.
     */
    RT_FORCE_INLINE bool Empty() const;

    /**
     * Get first element.
     * @note Array must not be empty. Otherwise it will cause an assertion.
     */
    RT_FORCE_INLINE const ElementType& Front() const;
    RT_FORCE_INLINE ElementType& Front();

    /**
     * Get last element.
     * @note Array must not be empty. Otherwise it will cause an assertion.
     */
    RT_FORCE_INLINE const ElementType& Back() const;
    RT_FORCE_INLINE ElementType& Back();

    /**
     * Get iterator to the first element.
     */
    RT_FORCE_INLINE ConstIterator Begin() const;
    RT_FORCE_INLINE Iterator Begin();

    /**
     * Get iterator to the end.
     */
    RT_FORCE_INLINE ConstIterator End() const;
    RT_FORCE_INLINE Iterator End();

    /**
     * Element access operators.
     * @note The index must be valid. Otherwise it will cause an assertion.
     */
    RT_FORCE_INLINE ElementType& operator[](Uint32 index);
    RT_FORCE_INLINE const ElementType& operator[](Uint32 index) const;

    /**
     * Create view of a sub-range.
     * @param index Starting index.
     * @param size  Number of elements in range.
     */
    RT_FORCE_INLINE ArrayView Range(Uint32 index, Uint32 size) const;

    /**
     * Find element by value.
     * @note    This operation takes O(N) time.
     * @return  Iterator to the found element, or iterator to the end if the element does not exist.
     */
    ConstIterator Find(const ElementType& element) const;
    Iterator Find(const ElementType& element);

    // lower-case aliases for Begin()/End(), required by C++
    RT_FORCE_INLINE ConstIterator begin() const { return Begin(); }
    RT_FORCE_INLINE ConstIterator end() const { return End(); }
    RT_FORCE_INLINE ConstIterator cbegin() const { return Begin(); }
    RT_FORCE_INLINE ConstIterator cend() const { return End(); }
    RT_FORCE_INLINE Iterator begin() { return Begin(); }
    RT_FORCE_INLINE Iterator end() { return End(); }

protected:
    alignas(void*) ElementType* mElements;
    Uint32 mSize;
};

#pragma pack(pop)


/**
 * Comparison operators.
 */
template<typename ElementTypeA, typename ElementTypeB>
bool operator == (const ArrayView<ElementTypeA>& lhs, const ArrayView<ElementTypeB>& rhs);
template<typename ElementTypeA, typename ElementTypeB>
bool operator != (const ArrayView<ElementTypeA>& lhs, const ArrayView<ElementTypeB>& rhs);
template<typename ElementTypeA, typename ElementTypeB>
bool operator < (const ArrayView<ElementTypeA>& lhs, const ArrayView<ElementTypeB>& rhs);
template<typename ElementTypeA, typename ElementTypeB>
bool operator > (const ArrayView<ElementTypeA>& lhs, const ArrayView<ElementTypeB>& rhs);
template<typename ElementTypeA, typename ElementTypeB>
bool operator <= (const ArrayView<ElementTypeA>& lhs, const ArrayView<ElementTypeB>& rhs);
template<typename ElementTypeA, typename ElementTypeB>
bool operator >= (const ArrayView<ElementTypeA>& lhs, const ArrayView<ElementTypeB>& rhs);


/**
 * Calculate hash of an array view.
 * @note This function is meant to be fast (it's used in hash tables), not to be cryptographically secure.
 */
template<typename ElementType>
Uint32 GetHash(const ArrayView<ElementType>& arrayView);


} // namespace rt

// ArrayView class definitions go here:
#include "ArrayViewImpl.h"
