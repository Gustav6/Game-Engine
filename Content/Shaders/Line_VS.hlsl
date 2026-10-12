#include "Include/Core/Common.hlsli"

VStoPS main(LineVertex aVertex)
{
    VStoPS result;
    
    const float4 worldPos = float4(aVertex.Position.xyz, 1.0f);
    const float4 viewPos = mul(FB_View, worldPos);
    
    result.Position = mul(FB_Projection, viewPos);
    result.WorldPosition = worldPos;
    result.Color = aVertex.Color;
    
    result.Normal = 0;
    result.Tangent = 0;
    result.BiNormal = 0;
    result.UV0 = 0;
    result.UV1 = 0;
    result.ViewDepth = 0;
    
    return result;
}
