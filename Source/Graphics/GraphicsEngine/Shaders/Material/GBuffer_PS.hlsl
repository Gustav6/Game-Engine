#include "Include/Common.hlsli"
#include "Include/Samplers.hlsli"
#include "Include/Material.hlsli"

struct GBufferOutput
{
    float4 AlbedoTexture    : SV_TARGET0;
    float4 NormalTexture    : SV_TARGET1;
    float4 MaterialTexture  : SV_TARGET2;
    float4 Emission         : SV_TARGET3;
    float4 WorldPosition    : SV_TARGET4;
};

Texture2D AlbedoTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D MaterialTexture : register(t2);

GBufferOutput main(VStoPS aPixel)
{
    const float4 albedoMap = AlbedoTexture.Sample(TrilinearWrap, aPixel.UV0);
    const float2 normalMap = NormalTexture.Sample(TrilinearWrap, aPixel.UV0).rg;
    const float3 materialMap = MaterialTexture.Sample(TrilinearWrap, aPixel.UV0).rgb;
    
    float2 unpackedNormal = normalMap * 2.0 - 1.0;
    
    float3 normal = normalize(float3(unpackedNormal, sqrt(saturate(1.0 - dot(unpackedNormal, unpackedNormal)))));

    const float3x3 TBN = float3x3(
        normalize(aPixel.Tangent),
        normalize(aPixel.BiNormal),
        normalize(aPixel.Normal)
    );
    
    normal = normalize(mul(normal, TBN));
    
    MaterialPixelParameters parameters;
    parameters.PixelColor = albedoMap;
    parameters.WorldPosition = aPixel.WorldPosition;
    parameters.Normal = normal;
    parameters.UV0 = aPixel.UV0;
    parameters.Occlusion = materialMap.r;
    parameters.Roughness = materialMap.g;
    parameters.Metalness = materialMap.b;
    
    GBufferOutput result;
    
    result.AlbedoTexture = float4(parameters.PixelColor.rgb, 1);
    result.NormalTexture = float4(parameters.Normal, 0);
    result.MaterialTexture = float4(parameters.Occlusion, parameters.Roughness, parameters.Metalness, 0);
    result.WorldPosition = parameters.WorldPosition;
    
    return result;
}