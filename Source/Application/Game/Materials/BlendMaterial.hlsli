#include "MaterialParameters.hlsli"
#include "Common.hlsli"

cbuffer MaterialBuffer : register(b5)
{
    float4 MB_Tint = float4(1, 1, 1, 1);
    float MB_Alpha = 0.5f;
}

void Material_Vertex(inout MaterialVertexParameters aParameters)
{
}

void Material_Pixel(inout MaterialPixelParameters aParameters)
{
    aParameters.PixelColor = aParameters.PixelColor * MB_Tint;
    aParameters.PixelColor.a = MB_Alpha;
}