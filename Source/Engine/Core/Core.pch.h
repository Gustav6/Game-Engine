// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here

#define WIN32_LEAN_AND_MEAN 
#define NOMINMAX 
#include <Windows.h>

#include <bitset>
#include <chrono>
#include <iostream>
#include <vector>
#include <random>
#include <type_traits>
#include <cassert>
#include <cmath>
#include <initializer_list>

#include "ensure.h"

#include "Wrappers/Logger/LoggerWrapper.h"
#include "Wrappers/Json/JsonWrapper.h"
#include "tracy/core/Tracy.hpp"

#include "tgafbx/Importer.h"
#include "tgafbx/TgaFbxStructs.h"

#include "CommonUtilities/Math/Vector.hpp"
#include "CommonUtilities/Math/Matrix.hpp"
#include "CommonUtilities/Math/MathUtils.hpp"
#include "CommonUtilities/Math/Quaternion.hpp"
#include "CommonUtilities/Collision/AABB3D.hpp"
#include "CommonUtilities/Collision/Sphere.hpp"
#include "CommonUtilities/Collision/Intersection.hpp"

#endif PCH_H
