#pragma once

#include "../RayLib.h"

#include <stdlib.h>
#include <malloc.h>

namespace rt {

RAYLIB_API void InitMemory();

class DefaultAllocator
{
public:
    RAYLIB_API static void* Allocate(size_t size, size_t alignment = 1);
    RAYLIB_API static void Free(void* ptr);
};

class SystemAllocator
{
public:
    RAYLIB_API static void* Allocate(size_t size, size_t alignment = 1);
    RAYLIB_API static void Free(void* ptr);
};

// Override this class to align children objects.
template <size_t Alignment, typename Allocator = DefaultAllocator>
class Aligned
{
public:
    RT_FORCE_INLINE void* operator new(size_t size)
    {
        return Allocator::Allocate(size, Alignment);
    }

    RT_FORCE_INLINE void operator delete(void* ptr)
    {
        Allocator::Free(ptr);
    }

    RT_FORCE_INLINE void* operator new[](size_t size)
    {
        return Allocator::Allocate(size, Alignment);
    }

    RT_FORCE_INLINE void operator delete[](void* ptr)
    {
        Allocator::Free(ptr);
    }

        RT_FORCE_INLINE void* operator new(size_t size, void* ptr)
    {
        RT_UNUSED(size);
        return ptr;
    }

    RT_FORCE_INLINE void* operator new[](size_t size, void* ptr)
    {
        RT_UNUSED(size);
        return ptr;
    }
};

template <typename T, std::size_t N = alignof(T), typename Allocator = DefaultAllocator>
class AlignedAllocator
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
    AlignedAllocator() throw () { }

    template <typename T2>
    AlignedAllocator(const AlignedAllocator<T2, N> &) throw () { }

    ~AlignedAllocator() throw () { }

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
        return (pointer)Allocator::Allocate(n * sizeof(value_type), N);
    }

    void deallocate(pointer p, size_type)
    {
        Allocator::Free(p);
    }

    void construct(pointer p, const value_type & wert)
    {
        new (p) value_type(wert);
    }

    void construct(pointer p, value_type&& wert)
    {
        new (p) value_type(std::move(wert));
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
        typedef AlignedAllocator<T2, N> other;
    };

    bool operator!=(const AlignedAllocator<T, N>& other) const
    {
        return !(*this == other);
    }

    bool operator==(const AlignedAllocator<T, N>&) const
    {
        return true;
    }
};

} // namespace rt