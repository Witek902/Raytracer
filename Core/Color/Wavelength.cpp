#include "PCH.h"
#include "Wavelength.h"

namespace rt {

using namespace math;

#ifdef RT_ENABLE_SPECTRAL_RENDERING

void Wavelength::Randomize(Random& rng)
{
    constexpr float offset = 1.0f / static_cast<float>(NumComponents);

    value = Vector8(rng.GetFloat()); // "hero" wavelength
    value += Vector8(0.0f, 1.0f * offset, 2.0f * offset, 3.0f * offset, 4.0f * offset, 5.0f * offset, 6.0f * offset, 7.0f * offset);
    value = Vector8::Fmod1(value);
    value *= 0.99999f; // make sure the value does not exceed 1.0f

    // make multi wavelength
    isSingle = false;
}

#else // !RT_ENABLE_SPECTRAL_RENDERING

void Wavelength::Randomize(Random& rng)
{
    RT_UNUSED(rng);
}

#endif // RT_ENABLE_SPECTRAL_RENDERING

} // namespace rt