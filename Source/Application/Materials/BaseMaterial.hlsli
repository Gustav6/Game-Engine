#include "Core/MaterialParameters.hlsli"
#include "Core/Common.hlsli"

cbuffer MaterialBuffer : register(b5)
{
    float3 Color_Tint = float3(1, 1, 1);
    float Tiling_U = 1;
    float Tiling_V = 1;
    float Color_Alpha = 1;
}

void Material_Vertex(inout MaterialVertexParameters aParameters)
{
}

void Material_Pixel(inout MaterialPixelParameters aParameters)
{
    aParameters.UV0 *= float2(Tiling_U, Tiling_V);
    aParameters.PixelColor.rgb *= Color_Tint;
    aParameters.PixelColor.a = Color_Alpha;
}