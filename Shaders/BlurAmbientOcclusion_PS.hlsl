#include "Include/Core/Common.hlsli"
#include "Include/Core/Samplers.hlsli"

// Depth aware blurring acordding to the book - Foundations of Game Engine Development, Volume 2 Rendering

Texture2D OcclusionTexture : register(t0);
Texture2D StructureTexture : register(t1);

float4 main(FullscreenVertex aVertex) : SV_TARGET
{
    const float kDepthDelta = 0.0078125;

    // Use depth and gradient to calculate a valid range for the blur samples.
    float2 pixelCoord = aVertex.Position.xy;

    float2 size;
    OcclusionTexture.GetDimensions(size.x, size.y);
    float2 invSize = 1.0f / size;
    
    // Center pixel: point sample (use your point-clamp sampler).
    float4 structure = StructureTexture.SampleLevel(PointClamp, pixelCoord * invSize, 0);

    float range = (max(abs(structure.x), abs(structure.y)) + kDepthDelta) * 1.5f;
    float z0 = structure.z + structure.w;

    float2 value = float2(0.0f, 1.0f);
    float3 occlusion = float3(0.0f, 0.0f, 0.0f);
    
    [unroll]
    for (int j = 0; j < 2; j++)
    {
        float y = float(j * 2) - 0.5f;
        
        [unroll]
        for (int i = 0; i < 2; i++)
        {
            float x = float(i * 2) - 0.5f;

            // Offsets land on texel corners, so LINEAR filtering gives a 2x2 average per tap.
            float2 uv = (pixelCoord + float2(x, y)) * invSize;

            value.x = OcclusionTexture.SampleLevel(LinearClamp, uv, 0).x;
            occlusion.z += value.x;

            float2 depth = StructureTexture.SampleLevel(LinearClamp, uv, 0).zw;
            if (abs(depth.x + depth.y - z0) < range)
            {
                occlusion.xy += value;
            }
        }
    }
    
    // Divide the accumulated occlusion value by the number of samples that passed.
    return ((occlusion.y > 0.0) ? occlusion.x / occlusion.y : occlusion.z * 0.25);
}