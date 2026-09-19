#include "Include/Common.hlsli"
#include "Include/Samplers.hlsli"

Texture2D FontTexture : register(t0);

float Median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

float ScreenPixelRange(float2 uv, uint aWidth, uint aHeight)
{
    float2 uintRange = float2(6, 6) / float2(aWidth, aHeight);

    float2 texSize = float2(1, 1) / fwidth(uv);

    return max(0.5f * dot(uintRange, texSize), 1.0f);
}

float4 main(VStoPS aPixel) : SV_TARGET
{
    uint textureWidth, textureHeight, textureMips;
    FontTexture.GetDimensions(0, textureWidth, textureHeight, textureMips);
    
    float4 texel = FontTexture.Sample(TrilinearWrap, aPixel.UV0);
    float distance = Median(texel.r, texel.b, texel.g);
    float pxDist = ScreenPixelRange(aPixel.UV0, textureWidth, textureHeight);
    float glyphPxDist = pxDist * (distance - 0.5f);
    
    float opacity = saturate(glyphPxDist + 0.5f);
    
    if (opacity <= 0.01f)
    {
        discard;
    }
    
    return float4(aPixel.Color.rgb, aPixel.Color.a * opacity);
}