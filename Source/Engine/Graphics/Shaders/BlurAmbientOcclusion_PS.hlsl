#include "Include/Core/Common.hlsli"
#include "Include/Core/Samplers.hlsli"

// Depth aware blurring acordding to the book - Foundations of Game Engine Development, Volume 2 Rendering

Texture2D OcclusionTexture : register(t0);
Texture2D StructureTexture : register(t1);

float4 main(FullscreenVertex aVertex) : SV_TARGET
{
    const float kDepthDelta = 0.0078125;

    float2 pixelCoord = aVertex.Position.xy;

    float2 size;
    OcclusionTexture.GetDimensions(size.x, size.y);
    float2 invSize = 1.0f / size;

    float4 structure = StructureTexture.SampleLevel(PointClamp, pixelCoord * invSize, 0);

    float range = (max(abs(structure.x), abs(structure.y)) + kDepthDelta) * 1.5f;
    float z0 = structure.z + structure.w;

    float2 value = float2(0.0f, 1.0f);
    float3 occlusion = float3(0.0f, 0.0f, 0.0f);
    
    float2 uv = float2(0.0f, 0.0f);
    float2 depth = float2(0.0f, 0.0f);
    
    {
        uv = (pixelCoord + float2(-0.5f, -0.5f)) * invSize;

        value.x = OcclusionTexture.SampleLevel(LinearClamp, uv, 0).x;
        occlusion.z += value.x;

        depth = StructureTexture.SampleLevel(LinearClamp, uv, 0).zw;

        if (abs(depth.x + depth.y - z0) < range)
        {
            occlusion.xy += value;
        }
    }
    
    {
        uv = (pixelCoord + float2(1.5f, -0.5f)) * invSize;

        value.x = OcclusionTexture.SampleLevel(LinearClamp, uv, 0).x;
        occlusion.z += value.x;

        depth = StructureTexture.SampleLevel(LinearClamp, uv, 0).zw;
        
        if (abs(depth.x + depth.y - z0) < range)
        {
            occlusion.xy += value;
        }
    }
    
    {
        uv = (pixelCoord + float2(-0.5f, 1.5f)) * invSize;

        value.x = OcclusionTexture.SampleLevel(LinearClamp, uv, 0).x;
        occlusion.z += value.x;

        depth = StructureTexture.SampleLevel(LinearClamp, uv, 0).zw;
        
        if (abs(depth.x + depth.y - z0) < range)
        {
            occlusion.xy += value;
        }
    }
    
    {
        uv = (pixelCoord + float2(1.5f, 1.5f)) * invSize;

        value.x = OcclusionTexture.SampleLevel(LinearClamp, uv, 0).x;
        occlusion.z += value.x;

        depth = StructureTexture.SampleLevel(LinearClamp, uv, 0).zw;
        
        if (abs(depth.x + depth.y - z0) < range)
        {
            occlusion.xy += value;
        }
    }

    return ((occlusion.y > 0.0) ? occlusion.x / occlusion.y : occlusion.z * 0.25);
}