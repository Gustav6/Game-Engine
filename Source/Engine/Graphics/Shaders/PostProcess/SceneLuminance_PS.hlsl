#include "../Include/Core/Common.hlsli"
#include "../Include/Core/Samplers.hlsli"

// Thresholds added from:
// https://salivity.github.io/glsl/article/hdr-bloom-extraction-with-thresholds-in-glsl

Texture2D Resource : register(t0);

float4 main(FullscreenVertex aVertex) : SV_Target
{
    const float threshold = 1.0; // Point where bloom starts
    const float softness = 0.5; // Controls the width of the transition knee
    
    float3 p = Resource.Sample(LinearClamp, aVertex.UV).rgb;
    float luminance = dot(p, float3(0.2126, 0.7152, 0.0722));

    // Compute knee parameters
    float knee = threshold * softness;
    float soft = luminance - (threshold - knee);
    soft = clamp(soft, 0.0, 2.0 * knee);
    soft = (soft * soft) / (4.0 * knee + 1e-5);

    // Calculate dynamic scaling factor
    float weight = max(soft, luminance - threshold);
    weight = max(weight, 0.0) / max(luminance, 1e-5);

    return  float4(p * weight, 1.0);
}