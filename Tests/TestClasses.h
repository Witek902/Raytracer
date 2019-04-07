#pragma once

struct ClassMethodCallCounters
{
    int constructor;
    int copyConstructor;
    int assignment;
    int moveConstructor;
    int moveAssignment;
    int destructor;

    ClassMethodCallCounters()
        : constructor(0)
        , copyConstructor(0)
        , assignment(0)
        , moveConstructor(0)
        , moveAssignment(0)
        , destructor(0)
    {}
};


/**
 * a helper class that can be only copied
 */
template<typename T>
class CopyOnlyTestClass
{
public:
    static_assert(std::is_copy_assignable<T>::value, "Payload type must be copy assignable");
    static_assert(std::is_copy_constructible<T>::value, "Payload type must be copy constructible");

    struct Comparator
    {
        RT_FORCE_INLINE bool Less(const CopyOnlyTestClass& left, const CopyOnlyTestClass& right) const
        {
            return left.mPayload < right.mPayload;
        }

        RT_FORCE_INLINE bool Equal(const CopyOnlyTestClass& left, const CopyOnlyTestClass& right) const
        {
            return left.mPayload == right.mPayload;
        }
    };

    CopyOnlyTestClass(ClassMethodCallCounters* counters, const T& payload = T())
        : mCounters(counters)
        , mPayload(payload)
    {
        if (mCounters)
        {
            mCounters->constructor++;
        }
    }

    CopyOnlyTestClass(const CopyOnlyTestClass& rhs)
        : mCounters(rhs.mCounters)
        , mPayload(rhs.mPayload)
    {
        if (mCounters)
        {
            mCounters->copyConstructor++;
        }
    }

    CopyOnlyTestClass& operator=(const CopyOnlyTestClass& rhs)
    {
        mPayload = rhs.mPayload;
        mCounters = rhs.mCounters;

        if (mCounters)
        {
            mCounters->assignment++;
        }

        return *this;
    }

    ~CopyOnlyTestClass()
    {
        if (mCounters)
        {
            mCounters->destructor++;
        }
    }

    RT_FORCE_INLINE bool operator==(const CopyOnlyTestClass& other) const
    {
        return mPayload == other.mPayload;
    }

    RT_FORCE_INLINE bool operator!=(const CopyOnlyTestClass& other) const
    {
        return mPayload != other.mPayload;
    }

private:
    CopyOnlyTestClass(CopyOnlyTestClass&& rhs) = delete;
    CopyOnlyTestClass& operator=(CopyOnlyTestClass&& rhs) = delete;

    ClassMethodCallCounters* mCounters;
    T mPayload;
};


/**
 * A helper class that can be only moved
 */
template<typename T>
class MoveOnlyTestClass
{
public:
    static_assert(std::is_move_assignable<T>::value, "Payload type must be move assignable");
    static_assert(std::is_move_constructible<T>::value, "Payload type must be move constructible");

    struct Comparator
    {
        RT_FORCE_INLINE bool Less(const MoveOnlyTestClass& left, const MoveOnlyTestClass& right) const
        {
            return left.mPayload < right.mPayload;
        }

        RT_FORCE_INLINE bool Equal(const MoveOnlyTestClass& left, const MoveOnlyTestClass& right) const
        {
            return left.mPayload == right.mPayload;
        }
    };

    MoveOnlyTestClass(ClassMethodCallCounters* counters, const T& payload = T())
        : mCounters(counters)
        , mPayload(payload)
    {
        if (mCounters)
        {
            mCounters->constructor++;
        }
    }

    MoveOnlyTestClass(MoveOnlyTestClass&& rhs)
        : mCounters(rhs.mCounters)
        , mPayload(std::move(rhs.mPayload))
    {
        if (mCounters)
        {
            mCounters->moveConstructor++;
        }

        rhs.mCounters = nullptr;
    }

    MoveOnlyTestClass& operator=(MoveOnlyTestClass&& rhs)
    {
        mPayload = std::move(rhs.mPayload);
        mCounters = rhs.mCounters;
        rhs.mCounters = nullptr;

        if (mCounters)
        {
            mCounters->moveAssignment++;
        }

        return *this;
    }

    ~MoveOnlyTestClass()
    {
        if (mCounters)
        {
            mCounters->destructor++;
        }
    }

    RT_FORCE_INLINE bool operator==(const MoveOnlyTestClass& other) const
    {
        return mPayload == other.mPayload;
    }

    RT_FORCE_INLINE bool operator!=(const MoveOnlyTestClass& other) const
    {
        return mPayload != other.mPayload;
    }

private:
    MoveOnlyTestClass(const MoveOnlyTestClass& rhs) = delete;
    MoveOnlyTestClass& operator=(const MoveOnlyTestClass& rhs) = delete;

    ClassMethodCallCounters* mCounters;
    T mPayload;
};