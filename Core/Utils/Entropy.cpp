#include "PCH.h"
#include "Entropy.h"
#include "Utils/Logger.h"

#if defined(__LINUX__) | defined(__linux__)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif // __LINUX__

namespace rt {

Entropy::Entropy()
{
#if defined(WIN32)

    mCryptProv = NULL;
    const LPCSTR userName = "MyKeyContainer";
    if (!::CryptAcquireContextA(&mCryptProv, userName, NULL, PROV_RSA_FULL, 0))
    {
        uint32 errorCode = GetLastError();
        if (::GetLastError() == NTE_BAD_KEYSET)
        {
            if (!::CryptAcquireContextA(&mCryptProv, userName, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET))
            {
                errorCode = ::GetLastError();
                RT_LOG_ERROR("Failed to create key container. Error code: %u", errorCode);
            }
        }
        else
        {
            RT_LOG_ERROR("A cryptographic service handle could not be acquired. Error code: %u", errorCode);
        }

    }

#elif defined(__LINUX__) | defined(__linux__)

    mRandomSourceFD = ::open("/dev/urandom", O_RDONLY);
    if (mRandomSourceFD == -1)
    {
        RT_LOG_ERROR("Failed to open /dev/urandom, error code: %u", errno);
    }

#endif // WIN32
}

Entropy::~Entropy()
{
#if defined(WIN32)

    ::CryptReleaseContext(mCryptProv, 0);

#elif defined(__LINUX__) | defined(__linux__)

    if (mRandomSourceFD != -1)
    {
        ::close(mRandomSourceFD);
    }

#endif // WIN32
}

uint32 Entropy::GetInt()
{
    uint32 result = 0;

#if defined(WIN32)

    ::CryptGenRandom(mCryptProv, sizeof(result), (BYTE*)&result);

#elif defined(__LINUX__) | defined(__linux__)

    if (mRandomSourceFD != -1)
    {
        ::read(mRandomSourceFD, &result, sizeof(result));
    }

#endif // WIN32

    // TODO combine with RTRND?

    return result;
}

} // namespace rt
