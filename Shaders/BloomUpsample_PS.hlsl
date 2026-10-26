#include "Include/Core/Common.hlsli"
#include "Include/Core/Samplers.hlsli"

// This shader performs downsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.
// This particular method was customly designed to eliminate
// "pulsating artifacts and temporal stability issues".

// Remember to add bilinear minification filter for this texture!
// Remember to use edge clamping for this texture!

Texture2D Resource : register(t0);

float4 main(FullscreenVertex aVertex) : SV_TARGET
{
    float filterRadius = 0.005f;

    // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.
    float x = filterRadius;
    float y = filterRadius;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = Resource.Sample(LinearClamp, float2(aVertex.UV.x - x, aVertex.UV.y + y)).rgb;
    float3 b = Resource.Sample(LinearClamp, float2(aVertex.UV.x, aVertex.UV.y + y)).rgb;
    float3 c = Resource.Sample(LinearClamp, float2(aVertex.UV.x + x, aVertex.UV.y + y)).rgb;

    float3 d = Resource.Sample(LinearClamp, float2(aVertex.UV.x - x, aVertex.UV.y)).rgb;
    float3 e = Resource.Sample(LinearClamp, float2(aVertex.UV.x, aVertex.UV.y)).rgb;
    float3 f = Resource.Sample(LinearClamp, float2(aVertex.UV.x + x, aVertex.UV.y)).rgb;

    float3 g = Resource.Sample(LinearClamp, float2(aVertex.UV.x - x, aVertex.UV.y - y)).rgb;
    float3 h = Resource.Sample(LinearClamp, float2(aVertex.UV.x, aVertex.UV.y - y)).rgb;
    float3 i = Resource.Sample(LinearClamp, float2(aVertex.UV.x + x, aVertex.UV.y - y)).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    float3 upsample = e * 4.0;
    upsample += (b + d + f + h) * 2.0;
    upsample += (a + c + g + i);
    upsample *= 1.0 / 16.0;
    
    return float4(upsample, 1.0f);
}