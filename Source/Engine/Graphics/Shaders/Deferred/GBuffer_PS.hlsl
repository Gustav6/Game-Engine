#include "../Include/Core/Common.hlsli"
#include "../Include/Core/Material.hlsli"

#ifdef MATERIAL_LAYOUT_SPLAT
#include "../Include/SurfaceLayout/SurfaceSplat.hlsli"
#else
#include "../Include/SurfaceLayout/SurfacePBR.hlsli"
#endif

GBufferOutput main(VStoPS aPixel)
{
    MaterialPixelParameters parameters;
    parameters.PixelColor = 0;
    parameters.WorldPosition = 0;
    parameters.PixelNormal = 0;
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

    if (surface.Albedo.a < 0.05f)
    {
        discard;
    }
    
    GBufferOutput result;
    
    switch (FB_TypeOfRenderPass)
    {
        case 0:
            parameters.PixelColor = surface.Albedo;
            parameters.WorldPosition = aPixel.WorldPosition;
            parameters.PixelNormal = surface.WorldNormal;
            parameters.UV0 = aPixel.UV0;
            parameters.Occlusion = surface.Occlusion;
            parameters.Roughness = surface.Roughness;
            parameters.Metalness = surface.Metalness;
    
            Material_Pixel(parameters);

            result.Albedo = float4(surface.Albedo.rgb, 1);
            break;
        case 1: // sRGB pass
            result.Albedo = float4(surface.Albedo.rgb, 1);
            break;
        case 2: // Roughness pass
            result.Albedo = float4(surface.Roughness, surface.Roughness, surface.Roughness, 1.0f);
            break;
        case 3: // Metalness pass
            result.Albedo = float4(surface.Metalness, surface.Metalness, surface.Metalness, 1.0f);
            break;
        case 4: // Ambient Occlusion texture pass
            result.Albedo = float4(surface.Occlusion, surface.Occlusion, surface.Occlusion, 1.0f);
            break;
        case 5: // Ambient Occlusion screen space pass
            result.Albedo = float4(0, 0, 0, 1);
            break;
        case 6: // Normals tangent space pass
            result.Albedo = float4(surface.NormalMapXY, 0.0f, 1.0f);
            break;
        case 7: // Normals world space pass
            result.Albedo = float4(surface.WorldNormal, 1.0f);
            break;
        case 8: // Shadows pass
            result.Albedo = float4(0.5f, 0.5f, 0.5f, 1.0f);
            surface.Roughness = 1.0f;
            break;
        default:
            result.Albedo = 0;
            break;
    }
    
    result.WolrdNormalDepth = float4(surface.WorldNormal, aPixel.ViewDepth);
    result.Material = float4(surface.Occlusion, surface.Roughness, surface.Metalness, 0);
    result.Emission = float4(surface.Emission, 0);
    result.WorldPosition = aPixel.WorldPosition;
    
    return result;
}