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
        case 4: // AGX tonemap
            return float4(LinearToGamma(Tonemap_AGX(radiance.xyz)), 1.0f);
        case 5: // AGX Punchy tonemap
            return float4(LinearToGamma(Tonemap_AGX_Punchy(radiance.xyz)), 1.0f);
        case 6: // Uncharted2 tonemap
            return float4(LinearToGamma(Tonemap_Uncharted2(radiance.xyz)), 1.0f);
        case 7: // Uchimura tonemap
            return float4(LinearToGamma(Tonemap_Uchimura(radiance.xyz)), 1.0f);
        case 8: // Filmic tonemap
            return float4(LinearToGamma(Tonemap_filmic(radiance.xyz)), 1.0f);
        case 9: // Neutral tonemap
            return float4(LinearToGamma(Tonemap_Neutral(radiance.xyz)), 1.0f);
        case 10: // FilmicHejl tonemap
            return float4(LinearToGamma(Tonemap_FilmicHejl(radiance.xyz, 4.0)), 1.0f);
        default:
            return float4(radiance.xyz, 1.0f);
    }
}