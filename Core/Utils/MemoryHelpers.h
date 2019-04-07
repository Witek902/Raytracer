#pragma once

#include <type_traits>
#include <string.h>

namespace rt {

/**
 * Collection of various classes for low-level C++ objects memory manipulation.
 */
class MemoryHelpers
{
public:
    template<typename T>
    static constexpr bool isMoveConstructible = std::is_move_constructible<T>::value;

    template<typename T>
    static constexpr bool isCopyConstructible = std::is_copy_constructible<T>::value;

    /**
     * Move an object from 'source' to 'target' using move constructor & destructor.
     * This is a preferred way if move constructor is available.
     */
    template<typename T>
    RT_FORCE_INLINE static
        typename std::enable_if<isMoveConstructible<T>, void>::type
        Move(T* target, T* source)
    {
        new (target) T(std::move(*source));
        source->~T();
    }

    /**
     * Move an object from 'source' to 'target' using move constructor & destructor.
     * This is a preferred way if move constructor is not available.
     */
    template<typename T>
    RT_FORCE_INLINE static
        typename std::enable_if<isCopyConstructible<T> && !isMoveConstructible<T>, void>::type
        Move(T* target, T* source)
    {
        new (target) T(*source);
        source->~T();
    }

    /**
     * Move an object from 'source' to 'target' using move memory copy.
     * This is only valid for POD types.
     */
    template<typename T>
    RT_FORCE_INLINE static
        typename std::enable_if<!isMoveConstructible<T> && !isCopyConstructible<T>, void>::type
        Move(T* target, T* source)
    {
        memcpy(target, source, sizeof(T));
    }

    /**
     * Move an array of objects (will call move constructor or copy constructor if possible).
     * @note    Source and target memory blocks can overlap.
     */
    template<typename T>
    static void MoveArray(T* target, T* source, size_t numElements)
    {
        if (target == source || numElements == 0)
        {
            // nothing to do
            return;
        }

        if (reinterpret_cast<std::uintptr_t>(target) > reinterpret_cast<std::uintptr_t>(source))
        {
            // move starting from the end in this scenario:
            // source: .....XXXXXXXX.........
            // target: .........XXXXXXXXX....

            for (size_t i = numElements; i-- > 0; )
            {
                Move(target + i, source + i);
            }
        }
        else
        {
            for (size_t i = 0; i < numElements; ++i)
            {
                Move(target + i, source + i);
            }
        }
    }
};

} // rt
