#include "Include/Common.hlsli"
#include "Include/Samplers.hlsli"

Texture2D Resource : register(t0);

float4 main(FullscreenVertex aVertex) : SV_Target
{
    float3 p = Resource.Sample(TrilinearClamp, aVertex.UV).rgb;
	// Rec. 709 Luma coefficients.
    float luminance = dot(p, float3(0.2126, 0.7152, 0.0722));

    if (luminance > 1.0f)
    {
        return float4(p, 1.0f);
    }

    return 0;
}