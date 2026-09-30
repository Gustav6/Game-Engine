#ifndef GaussianBlur_HLSLI
#define GaussianBlur_HLSLI

#include "Samplers.hlsli"

static const float GaussianKernel[5] = { 0.06136f, 0.24477f, 0.38774f, 0.24477f, 0.06136 };
static const uint kernelSize = 5;

float4 GetHorizontalGaussianBlur(in Texture2D aTexture, in float2 aPixelUV)
{
    uint width, height, mips;
    aTexture.GetDimensions(0, width, height, mips);

    float texelSize = 1.0f / width;
    float step = (((float) kernelSize - 1.0f) / 2.0f) * -1.0f;
    float3 result = 0;

    for (uint s = 0; s < kernelSize; ++s)
    {
        float2 uv = aPixelUV + float2(texelSize * (step + (float) s), 0.0f);
        float3 color = aTexture.Sample(TrilinearClamp, uv).rgb;
        result += color * GaussianKernel[s];
    }

    return float4(result, 1.0f);
}

float4 GetVerticalGaussianBlur(in Texture2D aTexture, in float2 aPixelUV)
{
    uint width, height, mips;
    aTexture.GetDimensions(0, width, height, mips);

    float texelSize = 1.0f / height;
    float step = (((float) kernelSize - 1.0f) / 2.0f) * -1.0f;
    float3 result = 0;

    for (uint s = 0; s < kernelSize; ++s)
    {
        float2 uv = aPixelUV + float2(0.0f, texelSize * (step + (float) s));
        float3 color = aTexture.Sample(TrilinearClamp, uv).rgb;
        result += color * GaussianKernel[s];
    }

    return float4(result, 1.0f);
}

#endif