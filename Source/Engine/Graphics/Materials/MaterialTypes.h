#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

/**
 * Enum of types used in HLSL during the AGP Course. To make matching easier.
 * @return
 */
enum class MaterialParameterType : uint8_t
{
	Unknown,
	Float, Float2, Float3, Float4,
	Int, Int2, Int3, Int4,
	Uint, Uint2, Uint3, Uint4,
	Bool, Matrix4x4
};

/**
 * Information about a Material Parameter found in the HLSL MaterialParametersBuffer
 * @return
 */
struct MaterialParameterInfo
{
	std::string Name;
	MaterialParameterType Type = MaterialParameterType::Unknown;
	size_t Offset = 0;
	size_t Size = 0;
	size_t Index = 0;
};