#include "Include/Common.hlsli"
#include "Include/Samplers.hlsli"

#include "Include/MaterialParameters.hlsli"
#include "Include/Material.hlsli"

Texture2D SpriteTexture : register(t0);

float4 main(VStoPS aPixel) : SV_TARGET
{
    MaterialPixelParameters parameters;

    parameters.PixelColor = aPixel.Color;
    parameters.WorldPosition = 0;
    parameters.PixelNormal = 0;
    parameters.UV0 = aPixel.UV0;
    parameters.Occlusion = 0;
    parameters.Roughness = 0;
    parameters.Metalness = 0;
    
    Material_Pixel(parameters);

    float4 textureColor = SpriteTexture.Sample(TrilinearWrap, parameters.UV0);
    
    return textureColor * parameters.PixelColor;
}

