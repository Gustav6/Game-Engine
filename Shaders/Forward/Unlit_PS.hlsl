#include "../Include/Core/Common.hlsli"
#include "../Include/Core/Material.hlsli"
#include "../Include/Core/Samplers.hlsli"

Texture2D AlbedoTexture : register(t0);
Texture2D NormalTexture : register(t1);

float4 main(VStoPS aPixel) : SV_TARGET
{
    float3 albedoPixel = AlbedoTexture.Sample(TrilinearWrap, aPixel.UV0).rgb;
    float2 normalPixel = NormalTexture.Sample(TrilinearWrap, aPixel.UV0).rg;
    
    float2 unpackedNormal = normalPixel * 2.0 - 1.0;
    
    float3 normal = normalize(float3(unpackedNormal, sqrt(saturate(1.0 - dot(unpackedNormal, unpackedNormal)))));

    const float3x3 TBN = float3x3(
        normalize(aPixel.Tangent),
        normalize(aPixel.BiNormal),
        normalize(aPixel.Normal)
    );

    normal = normalize(mul(normal, TBN));
    
    MaterialPixelParameters parameters;
    parameters.PixelColor = float4(albedoPixel, 1);
    parameters.WorldPosition = aPixel.WorldPosition;
    parameters.PixelNormal = normal;
    parameters.UV0 = aPixel.UV0;
    parameters.Occlusion = 0;
    parameters.Roughness = 0;
    parameters.Metalness = 0;
    
    Material_Pixel(parameters);
    
    return parameters.PixelColor;
}