#include "../Include/Core/Common.hlsli"
#include "../Include/Core/Samplers.hlsli"
#include "../Include/PostProcess/Tonemap.hlsli"

Texture2D HDRBuffer: register(t0);

float4 main(FullscreenVertex aPixel) : SV_TARGET
{
    const float4 radiance = HDRBuffer.Sample(TrilinearClamp, aPixel.UV);
    
    switch (FB_TypeOfTonemap)
    {
        case 0: // No tonemap selected!
            return float4(radiance.xyz, 1.0f);
        case 1: // Aces tonemap
            return float4(LinearToGamma(Tonemap_ACES(radiance.xyz)), 1.0f);
        case 2: // Lottes tonemap
            return float4(LinearToGamma(Tonemap_Lottes(radiance.xyz)), 1.0f);
        case 3: // Unreal tonemap
            return float4(Tonemap_UnrealEngine(radiance.xyz), 1.0f);
        default:
            return float4(radiance.xyz, 1.0f);
    }
}