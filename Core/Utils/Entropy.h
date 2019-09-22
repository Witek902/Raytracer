#pragma once

#include "../RayLib.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <Wincrypt.h>
#endif // WIN32

namespace rt {

// True random number generator (slow)
class RAYLIB_API Entropy : public NoCopyable
{
public:
    Entropy();
    ~Entropy();

    uint32 GetInt();

private:
#if defined(WIN32)
    HCRYPTPROV mCryptProv;
#elif defined(__LINUX__) | defined(__linux__)
    int mRandomSourceFD;
#endif // defined(WIN32)
};

} // namespace rt
