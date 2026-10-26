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
    uint width, height, mips;
    Resource.GetDimensions(0, width, height, mips);

    float2 srcTexelSize = 1.0 / float2(width, height);
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = Resource.Sample(LinearClamp, float2(aVertex.UV.x - 2 * x, aVertex.UV.y + 2 * y)).rgb;
    float3 b = Resource.Sample(LinearClamp, float2(aVertex.UV.x, aVertex.UV.y + 2 * y)).rgb;
    float3 c = Resource.Sample(LinearClamp, float2(aVertex.UV.x + 2 * x, aVertex.UV.y + 2 * y)).rgb;

    float3 d = Resource.Sample(LinearClamp, float2(aVertex.UV.x - 2 * x, aVertex.UV.y)).rgb;
    float3 e = Resource.Sample(LinearClamp, float2(aVertex.UV.x, aVertex.UV.y)).rgb;
    float3 f = Resource.Sample(LinearClamp, float2(aVertex.UV.x + 2 * x, aVertex.UV.y)).rgb;

    float3 g = Resource.Sample(LinearClamp, float2(aVertex.UV.x - 2 * x, aVertex.UV.y - 2 * y)).rgb;
    float3 h = Resource.Sample(LinearClamp, float2(aVertex.UV.x, aVertex.UV.y - 2 * y)).rgb;
    float3 i = Resource.Sample(LinearClamp, float2(aVertex.UV.x + 2 * x, aVertex.UV.y - 2 * y)).rgb;

    float3 j = Resource.Sample(LinearClamp, float2(aVertex.UV.x - x, aVertex.UV.y + y)).rgb;
    float3 k = Resource.Sample(LinearClamp, float2(aVertex.UV.x + x, aVertex.UV.y + y)).rgb;
    float3 l = Resource.Sample(LinearClamp, float2(aVertex.UV.x - x, aVertex.UV.y - y)).rgb;
    float3 m = Resource.Sample(LinearClamp, float2(aVertex.UV.x + x, aVertex.UV.y - y)).rgb;
    
    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
    // This shows 5 square areas that are being sampled. But some of them overlap,
    // so to have an energy preserving downsample we need to make some adjustments.
    // The weights are the distributed, so that the sum of j,k,l,m (e.g.)
    // contribute 0.5 to the final color output. The code below is written
    // to effectively yield this sum. We get:
    // 0.125*5 + 0.03125*4 + 0.0625*4 = 1
    float3 downsample = e * 0.125;
    downsample += (a + c + g + i) * 0.03125;
    downsample += (b + d + f + h) * 0.0625;
    downsample += (j + k + l + m) * 0.125;

    return float4(downsample, 1.0f);
}