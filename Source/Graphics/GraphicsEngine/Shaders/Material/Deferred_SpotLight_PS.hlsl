#include "Include/Common.hlsli"
#include "Include/Shadow.hlsli"
#include "Include/PBL_Functions.hlsli"
#include "Include/Light.hlsli"

Texture2D GBufferAlbedo : register(t0);
Texture2D GBufferNormal : register(t1);
Texture2D GBufferMaterial : register(t2);
Texture2D GBufferEmission : register(t3);
Texture2D GBufferWorldPosition : register(t4);

float4 main(FullTextureVertex aInput) : SV_TARGET
{
    if (FB_TypeOfRenderPass != 0)
    {
        discard;
    }

    const float4 albedoPixel = GBufferAlbedo.Sample(TrilinearClamp, aInput.UV);
    
    // If this is 0 we've read Infinity from the GBuffer. We cannot light Infinity so discard.
    
    if (albedoPixel.a == 0)
    {
        discard;
    }
    
    const float4 normalPixel = GBufferNormal.Sample(TrilinearClamp, aInput.UV);
    const float4 materialPixel = GBufferMaterial.Sample(TrilinearClamp, aInput.UV);
    const float4 worldPositionPixel = GBufferWorldPosition.Sample(TrilinearClamp, aInput.UV);
    
    Light light = LB_Lights[0];
    
    float3 radiance = float3(0, 0, 0);
    
    float occlusion = materialPixel.r;
    float roughness = materialPixel.g;
    float metalness = materialPixel.b;
    
    const float3 specularColor = lerp((float3) 0.04f, albedoPixel.rgb, metalness);
    const float3 diffuseColor = lerp((float3) 0.00f, albedoPixel.rgb, 1 - metalness);
    const float3 pixelView = normalize(FB_CameraPosition - worldPositionPixel.xyz);
    
    float3 lightContribution = CalculateSpotLightContribution(
        light,
        normalPixel.xyz,
        worldPositionPixel.xyz,
        pixelView,
        specularColor,
        diffuseColor,
        roughness
    );
                
    float shadowFactor = CalculateSpotLightShadowFactor(
        light,
        0,
        worldPositionPixel,
        normalPixel.xyz
    );
               
    radiance += lightContribution * shadowFactor;
    
    
    return float4(radiance, 1);
}