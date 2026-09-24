#include "Include/Common.hlsli"

VStoPS main(Vertex aVertex)
{
    VStoPS result;
	
    result.Position = mul(FB_Projection, aVertex.Position);

    result.Color = aVertex.Color;
    result.UV0 = aVertex.UV0;
    result.UV1 = aVertex.UV1;
    
    result.WorldPosition = 0;
    result.Normal = 0;
    result.Tangent = 0;
    result.BiNormal = 0;
    
    return result;
}