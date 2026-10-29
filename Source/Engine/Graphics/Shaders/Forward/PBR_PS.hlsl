#include "../Include/Core/Common.hlsli"
#include "../Include/Core/Material.hlsli"
#include "../Include/Shadows/Shadow.hlsli"
#include "../Include/Lighting/PBRFunctions.hlsli"
#include "../Include/Lighting/Light.hlsli"

#ifdef MATERIAL_LAYOUT_SPLAT
#include "../Include/SurfaceLayout/SurfaceSplat.hlsli"
#else
#include "../Include/SurfaceLayout/SurfacePBR.hlsli"
#endif

float4 main(VStoPS aPixel) : SV_TARGET
{
    MaterialPixelParameters parameters;
    parameters.PixelColor = 0;
    parameters.WorldPosition = 0;
    parameters.WorldNormal = 0;
    parameters.UV0 = aPixel.UV0;
    parameters.Occlusion = 0;
    parameters.Roughness = 0;
    parameters.Metalness = 0;
    
    Material_Pixel(parameters);
    
#ifdef MATERIAL_LAYOUT_SPLAT
    SurfaceData surface = EvaluateSurface(aPixel, aPixel.UV0);
#else
    SurfaceData surface = EvaluateSurface(aPixel, aPixel.UV0);
#endif
    
    parameters.PixelColor = surface.Albedo;
    parameters.WorldPosition = aPixel.WorldPosition;
    parameters.WorldNormal = surface.WorldNormal;
    parameters.UV0 = aPixel.UV0;
    parameters.Occlusion = surface.Occlusion;
    parameters.Roughness = surface.Roughness;
    parameters.Metalness = surface.Metalness;
    
    Material_Pixel(parameters);
    
    if (parameters.PixelColor.a < 0.05f)
    {
        discard;
    }
    
    switch (FB_TypeOfRenderPass)
    {
        case 0:
            break;
        case 1: // sRGB pass
            return float4(surface.Albedo.rgb, 1.0f);
        case 2: // Roughness pass
            return float4(surface.Roughness, surface.Roughness, surface.Roughness, 1.0f);
        case 3: // Metalness pass
            return float4(surface.Metalness, surface.Metalness, surface.Metalness, 1.0f);
        case 4: // Ambient Occlusion texture pass
            return float4(surface.Occlusion, surface.Occlusion, surface.Occlusion, 1.0f);
        case 5: // Ambient Occlusion screen space pass
            return float4(1.0f, 1.0f, 1.0f, 1.0f); // TODO : FIX
        case 6: // Normals tangent space pass
            return float4(surface.NormalMapXY, 0.0f, 1.0f);
        case 7: // Normals world space pass
            return float4(surface.WorldNormal, 1.0f);
        case 8: // Shadows pass
            parameters.PixelColor = float4(0.5f, 0.5f, 0.5f, 1.0f);
            parameters.Roughness = 1.0f;
            break;
        default:
            break;
    }

    const float3 specularColor = lerp((float3) 0.04f, parameters.PixelColor.rgb, parameters.Metalness);
    const float3 diffuseColor = lerp((float3) 0.00f, parameters.PixelColor.rgb, 1 - parameters.Metalness);
    const float3 pixelView = normalize(FB_CameraPosition - aPixel.WorldPosition.xyz);
    
    const int envNumMips = GetNumMips(EnvCubeTexture) - 1;
    
    float3 radiance = CalculateAmbientContribution(parameters.WorldNormal, pixelView, specularColor, diffuseColor, envNumMips, parameters.Roughness, parameters.Occlusion);
    
    for (uint l = 0; l < LB_NumActiveLights; ++l)
    {
        Light light = LB_Lights[l];
        
        switch (light.Type)
        {
            case 0:
            {
                uint cascadeIndex;
                
                if (aPixel.ViewDepth < CB_CascadeFarPlanes.x)
                {
                    //radiance = float3(1, 0, 0);
                    cascadeIndex = 0;
                }
                else if (aPixel.ViewDepth < CB_CascadeFarPlanes.y)
                {
                    //radiance = float3(0, 1, 0);
                    cascadeIndex = 1;
                }
                else if (aPixel.ViewDepth < CB_CascadeFarPlanes.z)
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
                    parameters.WorldNormal,
                    pixelView,
                    specularColor,
                    diffuseColor,
                    parameters.Roughness
                );
                
                float shadowFactor = CalculateDirectionalLightShadowFactor(
                    cascadeIndex,
                    aPixel.WorldPosition
                );
                
                radiance += lightContribution * shadowFactor;
                
                break;
            }
            case 1:
            {
                float3 lightContribution = CalculatePointLightContribution(
                    light,
                    parameters.WorldNormal,
                    aPixel.WorldPosition.xyz,
                    pixelView,
                    specularColor,
                    diffuseColor,
                    parameters.Roughness
                );
                
                float shadowFactor = CalculatePointLightShadowFactor(
                    light, 
                    light.ShadowIndex,
                    aPixel.WorldPosition.xyz
                );
                
                radiance += lightContribution * shadowFactor;
                
                break;
            }
            case 2:
            {
                float3 lightContribution = CalculateSpotLightContribution(
                    light,
                    parameters.WorldNormal,
                    aPixel.WorldPosition.xyz,
                    pixelView,
                    specularColor,
                    diffuseColor,
                    parameters.Roughness
                );
                
                float shadowFactor = CalculateSpotLightShadowFactor(
                    light,
                    light.ShadowIndex,
                    aPixel.WorldPosition,
                    parameters.WorldNormal
                );
               
                radiance += lightContribution * shadowFactor;
                
                break;
            }
            default:
                break;
        }
    }
    
    radiance += surface.Emission;
   
    return float4(radiance, parameters.PixelColor.a);
}