#include "Include/Common.hlsli"
#include "Include/Samplers.hlsli"

Texture2D SpriteTexture : register(t0);

float4 main(VStoPS aPixel) : SV_TARGET
{
    float4 textureColor = SpriteTexture.Sample(TrilinearWrap, aPixel.UV0);

    return textureColor * aPixel.Color;
}

