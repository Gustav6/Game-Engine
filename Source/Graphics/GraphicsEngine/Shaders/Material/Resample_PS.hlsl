#include "Include/Common.hlsli"
#include "Include/Samplers.hlsli"

Texture2D Resource : register(t0); // Some input texture.

float4 main(FullscreenVertex aVertex) : SV_Target
{
    return Resource.Sample(TrilinearClamp, aVertex.UV);
}