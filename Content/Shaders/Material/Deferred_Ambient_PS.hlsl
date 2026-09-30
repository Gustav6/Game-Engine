#include "Include/Common.hlsli"
#include "Include/Samplers.hlsli"
#include "Include/PBL_Functions.hlsli"

Texture2D GBufferAlbedo : register(t0);
Texture2D GBufferNormal : register(t1);
Texture2D GBufferMaterial : register(t2);
Texture2D GBufferEmission : register(t3);
Texture2D GBufferWorldPosition : register(t4);

float4 main(FullscreenVertex aInput) : SV_TARGET
{
    const float4 albedoPixel = GBufferAlbedo.Sample(TrilinearClamp, aInput.UV);
    
    // If this is 0 we've read Infinity from the GBuffer. We cannot light Infinity so discard.
    
    if (albedoPixel.a == 0)
    {
        discard;
    }
    
    if (FB_TypeOfRenderPass != 0 && FB_TypeOfRenderPass != 7)
    {
        return albedoPixel;
    }
    
    const float4 normalPixel = GBufferNormal.Sample(TrilinearClamp, aInput.UV);
    const float4 materialPixel = GBufferMaterial.Sample(TrilinearClamp, aInput.UV);
    const float4 worldPositionPixel = GBufferWorldPosition.Sample(TrilinearClamp, aInput.UV);
    const float4 emissionPixel = GBufferEmission.Sample(TrilinearClamp, aInput.UV);
    
    float occlusion = materialPixel.r;
    float roughness = materialPixel.g;
    float metalness = materialPixel.b;
    
    // This is used later in the course: 
    // const float4 vertexNormalPixel = GBufferVertexNormal.Sample ( TrilinearClamp , input.UV ); 
    
    const float3 specularColor = lerp((float3) 0.04f, albedoPixel.rgb, metalness);
    const float3 diffuseColor = lerp((float3) 0.00f, albedoPixel.rgb, 1 - metalness);
    const float3 pixelView = normalize(FB_CameraPosition - worldPositionPixel.xyz);

    const float3 emission = emissionPixel.rgb;
    
    const int envNumMips = GetNumMips(EnvCubeTexture) - 1;
  
    float3 radiance = CalculateAmbientContribution(
    	normalPixel.xyz, 
    	pixelView, 
    	specularColor, 
    	diffuseColor,
		envNumMips, 
		roughness, 
		occlusion
	);
         
    radiance += emission;

    return float4(radiance, 1);
}