#include "Include/Common.hlsli"
#include "Include/Shadow.hlsli"
#include "Include/Samplers.hlsli"
#include "Include/PBL_Functions.hlsli"
#include "Include/Light.hlsli"

Texture2D GBufferAlbedo : register(t0);
Texture2D GBufferNormal : register(t1);
Texture2D GBufferMaterial : register(t2);
Texture2D GBufferEmission : register(t3);
Texture2D GBufferWorldPosition : register(t4);

float4 main(FullTextureVertex aInput) : SV_TARGET
{
    const float4 albedoPixel = GBufferAlbedo.Sample(TrilinearClamp, aInput.UV);
    
    // If this is 0 we've read Infinity from the GBuffer. We cannot light Infinity so discard.
    
    if (albedoPixel.a == 0) 
    {
        discard;
    }
    
    const float4 normalPixel = GBufferNormal.Sample(TrilinearClamp, aInput.UV);
    const float4 materialPixel = GBufferMaterial.Sample(TrilinearClamp, aInput.UV);
    const float4 worldPositionPixel = GBufferWorldPosition.Sample(TrilinearClamp, aInput.UV);
    
    float occlusion = materialPixel.r;
    float roughness = materialPixel.g;
    float metalness = materialPixel.b;
    
    // This is used later in the course: 
    // const float4 vertexNormalPixel = GBufferVertexNormal.Sample ( TrilinearClamp , input.UV ); 
    
    const float3 specularColor = lerp((float3) 0.04f, albedoPixel.rgb, metalness);
    const float3 diffuseColor = lerp((float3) 0.00f, albedoPixel.rgb, 1 - metalness);
    const float3 pixelView = normalize(FB_CameraPosition - worldPositionPixel.xyz);
    
    const int envNumMips = GetNumMips(EnvCubeTexture) - 1;
    
    Light light = LB_Lights[0];
    
    float3 radiance = float3(0, 0, 0);
    float pixelDistance = length(worldPositionPixel.xyz - FB_CameraPosition);

    uint cascadeIndex;
                
    if (pixelDistance < CB_CascadeFarPlanes.x)
    {
        //radiance = float3(1, 0, 0);
        cascadeIndex = 0;
    }
    else if (pixelDistance < CB_CascadeFarPlanes.y)
    {
        //radiance = float3(0, 1, 0);
        cascadeIndex = 1;
    }
    else if (pixelDistance < CB_CascadeFarPlanes.z)
    {
        //radiance = float3(0, 0, 1);
        cascadeIndex = 2;
    }
    else
    {
        //radiance = float3(1, 1, 0);
        cascadeIndex = 3;
    }
    
    float3 lightContribution = CalculateDirectionalLightContribution(
        light,
        normalPixel.xyz,
        pixelView,
        specularColor,
        diffuseColor,
        roughness
    );
    
    float shadowFactor = CalculateDirectionalLightShadowFactor(
        cascadeIndex,
        worldPositionPixel
    );
                
    radiance += lightContribution * shadowFactor;
    
    return float4(radiance, 1);
}