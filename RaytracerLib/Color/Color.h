#pragma once

#include "../RayLib.h"
#include "../Math/Vector4.h"


namespace rt {


// Color represented as spectrum samples
// Used for representing color of lights, materials reflectivity, etc.
class RAYLIB_API Color
{
public:
    Color();
    ~Color();

    // build from sRGB values
    static Color FromRGB(const float r, const float g, const float b, const Uint32 numSamples = 10);
    static Color FromRGB(const math::Vector4& color, const Uint32 numSamples = 10);


private:
    std::unique_ptr<float[]> mSpectrum;
    Uint32 mNumSamples;
};


} // namespace rt
