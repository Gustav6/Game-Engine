#include "Core/MaterialParameters.hlsli"
#include "Core/Common.hlsli"

cbuffer MaterialBuffer : register(b5)
{
    float4 MB_Alpha = 0.5f;
}

void Material_Vertex(inout MaterialVertexParameters aParameters)
{
}

void Material_Pixel(inout MaterialPixelParameters aParameters)
{
    aParameters.PixelColor.a = MB_Alpha;
}