#include "../Include/Core/Common.hlsli"
#include "../Include/Core/Samplers.hlsli"

Texture2D Resource : register(t0); // Some input texture.

float4 main(FullscreenVertex aVertex) : SV_Target
{
    return Resource.Sample(TrilinearClamp, aVertex.UV);
}