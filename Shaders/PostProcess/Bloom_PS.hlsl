#include "../Include/Core/Common.hlsli"
#include "../Include/Core/Samplers.hlsli"

Texture2D Resource : register(t0); // Some input texture.

float4 main(FullscreenVertex aVertex) : SV_Target
{
    const float BloomStrength = 0.1f;

    float3 bloom = Resource.Sample(LinearClamp, aVertex.UV).rgb;
    return float4(bloom * BloomStrength, 1.0f);
}