#pragma once

#include "../Common.h"

#include <iterator>

namespace rt {


template<typename ElementType>
class ArrayIterator;

/**
 * Array iterator with read-only access.
 */
template<typename ElementType>
class ConstArrayIterator : public std::iterator<std::bidirectional_iterator_tag, ElementType>
{
    template<typename T>
    friend class ConstArrayIterator;

public:
    // C++ standard iterator traits
    using self_type = ConstArrayIterator;

    ConstArrayIterator() : mElements(nullptr), mIndex(0) { }
    ConstArrayIterator(const ElementType* elements, Int32 index) : mElements(elements), mIndex(index) { }

    RT_FORCE_INLINE ConstArrayIterator(const ArrayIterator<const ElementType>& other);
    RT_FORCE_INLINE ConstArrayIterator(const ConstArrayIterator<const ElementType>& other);

    // comparisons
    RT_FORCE_INLINE bool operator == (const ConstArrayIterator& other) const;
    RT_FORCE_INLINE bool operator != (const ConstArrayIterator& other) const;
    RT_FORCE_INLINE bool operator < (const ConstArrayIterator& rhs) const;
    RT_FORCE_INLINE ptrdiff_t operator - (const ConstArrayIterator& rhs) const;

    // element access
    RT_FORCE_INLINE const ElementType& operator * () const;
    RT_FORCE_INLINE const ElementType* operator -> () const;

    // arithmetics
    RT_FORCE_INLINE ConstArrayIterator& operator ++ ();
    RT_FORCE_INLINE ConstArrayIterator operator ++ (int);
    RT_FORCE_INLINE ConstArrayIterator& operator += (ptrdiff_t offset);
    RT_FORCE_INLINE ConstArrayIterator& operator -- ();
    RT_FORCE_INLINE ConstArrayIterator operator -- (int);
    RT_FORCE_INLINE ConstArrayIterator& operator -= (ptrdiff_t offset);
    RT_FORCE_INLINE ConstArrayIterator operator + (ptrdiff_t offset) const;
    RT_FORCE_INLINE ConstArrayIterator operator - (ptrdiff_t offset) const;

    // get array index
    Int32 GetIndex() const { return mIndex; }

protected:
    const ElementType* mElements;   // array elements
    Int32 mIndex;                   // current index in the array
};

/**
 * Array iterator with read-write access.
 */
template<typename ElementType>
class ArrayIterator : public ConstArrayIterator<ElementType>
{
public:
    // C++ standard iterator traits
    using self_type = ArrayIterator;

    ArrayIterator() = default;
    ArrayIterator(ElementType* elements, Uint32 index)
        : ConstArrayIterator<ElementType>(const_cast<ElementType*>(elements), index)
    { }

    // comparisons
    RT_FORCE_INLINE ptrdiff_t operator - (const ArrayIterator& rhs) const;

    // element access
    RT_FORCE_INLINE ElementType& operator * () const;
    RT_FORCE_INLINE ElementType* operator -> () const;

    // arithmetics
    RT_FORCE_INLINE ArrayIterator& operator ++ ();
    RT_FORCE_INLINE ArrayIterator operator ++ (int);
    RT_FORCE_INLINE ArrayIterator& operator += (ptrdiff_t offset);
    RT_FORCE_INLINE ArrayIterator& operator -- ();
    RT_FORCE_INLINE ArrayIterator operator -- (int);
    RT_FORCE_INLINE ArrayIterator& operator -= (ptrdiff_t offset);
    RT_FORCE_INLINE ArrayIterator operator + (ptrdiff_t offset) const;
    RT_FORCE_INLINE ArrayIterator operator - (ptrdiff_t offset) const;

    RT_FORCE_INLINE ElementType* GetElements() const { return const_cast<ElementType*>(this->mElements); }
};


} // namespace rt

#include "ArrayIteratorImpl.h"