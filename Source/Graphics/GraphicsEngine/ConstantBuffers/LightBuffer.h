#pragma once

#include "CommonUtilities/Math/Vector3.hpp"

#include "Objects/Light.h"

static constexpr unsigned int MAX_LIGHTS = 8;

struct LightBuffer
{
    Light Lights[MAX_LIGHTS];
    unsigned int NumLights = 0;
    CommonUtilities::Vector3f _padding;
};