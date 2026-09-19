#include "Include/Common.hlsli"

VStoPS main(Vertex aVertex)
{
    VStoPS result;
	
    float4 vertexLocalPosition = aVertex.Position;
	
    const float4 localPosition = float4(vertexLocalPosition.xyz, 1);
    const float4 worldPosition = mul(OB_World, localPosition);
	
    result.Position = worldPosition;
    result.Color = aVertex.Color;
    result.UV0 = aVertex.UV0;
    result.UV1 = aVertex.UV1;
    
    result.WorldPosition = 0;
    result.Normal = 0;
    result.Tangent = 0;
    result.BiNormal = 0;
    
    return result;
}