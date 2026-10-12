#include "../Include/Core/Common.hlsli"
#include "../Include/Core/Samplers.hlsli"

Texture2D Resource : register(t0); // Some input texture.

static const float GaussianKernel[5] = { 0.06136f, 0.24477f, 0.38774f, 0.24477f, 0.06136 };
static const uint kernelSize = 5;

float4 main(FullscreenVertex aVertex) : SV_Target
{
    uint width, height, mips;
    Resource.GetDimensions(0, width, height, mips);

    float texelSize = 1.0f / width;
    float step = (((float) kernelSize - 1.0f) / 2.0f) * -1.0f;

    float4 result = 0;

    for (uint s = 0; s < kernelSize; ++s)
    {
        float2 uv = aVertex.UV + float2(texelSize * (step + (float) s), 0.0f);
        float4 color = Resource.Sample(TrilinearClamp, uv);
        result += color * GaussianKernel[s];
    }

    return result;
}