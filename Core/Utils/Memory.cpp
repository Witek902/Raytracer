#include "PCH.h"
#include "Memory.h"
#include "Logger.h"
#include "../Math/Math.h"

#include <stdlib.h>
#include <malloc.h>
#include <Windows.h>

namespace rt {

#if defined(WIN32)

static bool TogglePrivilege(TCHAR* pszPrivilege, BOOL bEnable)
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;

    // open process token
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        RT_LOG_ERROR("OpenProcessToken failed, error code: %u", GetLastError());
        return false;
    }

    // get the luid
    if (!LookupPrivilegeValue(NULL, pszPrivilege, &tp.Privileges[0].Luid))
    {
        RT_LOG_ERROR("LookupPrivilegeValue failed, error code: %u", GetLastError());
        return false;
    }

    tp.PrivilegeCount = 1;

    // enable or disable privilege
    if (bEnable)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    // enable or disable privilege
    BOOL status = AdjustTokenPrivileges(hToken, FALSE, &tp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

    // It is possible for AdjustTokenPrivileges to return TRUE and still not succeed.
    // So always check for the last error value.
    DWORD error = GetLastError();
    if (!status || (error != ERROR_SUCCESS))
    {
        RT_LOG_WARNING("AdjustTokenPrivileges failed, error code: %u", error);
        return false;
    }

    // close the handle
    if (!CloseHandle(hToken))
    {
        RT_LOG_ERROR("CloseHandle failed, error code: %u", GetLastError());
        return false;
    }

    return true;
}

static void EnableLargePagesSupport()
{
    if (TogglePrivilege(TEXT("SeLockMemoryPrivilege"), TRUE))
    {
        RT_LOG_INFO("Large page support enabled. Minimum large page size: %zu bytes", GetLargePageMinimum());
    }
    else
    {
        RT_LOG_WARNING("Failed to enable large page support");
    }
}

#else

static void EnableLargePagesSupport()
{
    // TODO
}

#endif // WIN32

void InitMemory()
{
    EnableLargePagesSupport();
}

void* DefaultAllocator::Allocate(size_t size, size_t alignment)
{
    void* ptr = nullptr;
#if defined(WIN32)
    ptr = _aligned_malloc(size, alignment);
#elif defined(__LINUX__) | defined(__linux__)
    alignment = std::max(alignment, sizeof(void*));
    int ret = posix_memalign(&ptr, alignment, size);
    if (ret != 0)
    {
        RT_ASSERT("posix_memalign() returned %i", ret);
        ptr = nullptr;
    }
#endif // defined(WIN32)
    return ptr;
}

void DefaultAllocator::Free(void* ptr)
{
#if defined(WIN32)
    _aligned_free(ptr);
#elif defined(__LINUX__) | defined(__linux__)
    free(ptr);
#endif // defined(WIN32)
}


void* SystemAllocator::Allocate(size_t size, size_t alignment)
{
    if (size == 0)
    {
        return nullptr;
    }

    void* ptr = nullptr;

#if defined(WIN32)

    if (size < 64u * 1024u)
    {
        RT_LOG_WARNING("SystemAllocator: Allocating less than 64KB is not optimal (requested %zu bytes)", size);
    }

    RT_UNUSED(alignment);

    // try large pages first
    const size_t largePageMinNumpages = 4;
    const size_t minLargePageSize = largePageMinNumpages * ::GetLargePageMinimum();
    if (size >= minLargePageSize)
    {
        const size_t roundedSize = math::RoundUp(size, minLargePageSize);
        ptr = ::VirtualAlloc(NULL, roundedSize, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE);
        if (ptr)
        {
            RT_LOG_DEBUG("SystemAllocator: Allocated %.2f KB using (via large pages)", size / 1024.0);
        }
    }

    if (!ptr)
    {
        ptr = ::VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (ptr)
        {
            RT_LOG_DEBUG("SystemAllocator: Allocated %.2f KB", size / 1024.0);
        }
        else
        {
            RT_LOG_ERROR("SystemAllocator: Failed to allocate %.2f KB, error code: %u", size / 1024.0, GetLastError());
        }
    }

#elif defined(__LINUX__) | defined(__linux__)

    // TODO
    DefaultAllocator::Allocate(size, alignment);

#endif // 
    
    return ptr;
}

void SystemAllocator::Free(void* ptr)
{
#if defined(WIN32)
    ::VirtualFree(ptr, 0, MEM_RELEASE);
#elif defined(__LINUX__) | defined(__linux__)
    // TODO
    DefaultAllocator::Free(ptr);
#endif // 
}

} // namespace rt