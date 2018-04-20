#pragma once

#include <stdlib.h>
#include <malloc.h>

template <typename T, std::size_t N = alignof(T)>
class AlignmentAllocator
{
public:
    typedef T value_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    typedef T * pointer;
    typedef const T * const_pointer;

    typedef T & reference;
    typedef const T & const_reference;

public:
    AlignmentAllocator() throw () { }

    template <typename T2>
    AlignmentAllocator(const AlignmentAllocator<T2, N> &) throw () { }

    ~AlignmentAllocator() throw () { }

    pointer adress(reference r)
    {
        return &r;
    }

    const_pointer adress(const_reference r) const
    {
        return &r;
    }

    pointer allocate(size_type n)
    {
        return (pointer)_aligned_malloc(n * sizeof(value_type), N);
    }

    void deallocate(pointer p, size_type)
    {
        _aligned_free(p);
    }

    void construct(pointer p, const value_type & wert)
    {
        new (p) value_type(wert);
    }

    void destroy(pointer p)
    {
        p->~value_type();
    }

    size_type max_size() const throw ()
    {
        return size_type(-1) / sizeof(value_type);
    }

    template <typename T2>
    struct rebind
    {
        typedef AlignmentAllocator<T2, N> other;
    };

    bool operator!=(const AlignmentAllocator<T, N>& other) const
    {
        return !(*this == other);
    }

    bool operator==(const AlignmentAllocator<T, N>& other) const
    {
        return true;
    }
};
