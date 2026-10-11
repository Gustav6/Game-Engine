#include "Include/Common.hlsli"

#include "Include/MaterialParameters.hlsli"
#include "Include/Material.hlsli"

VStoPS main(Vertex aVertex)
{
    VStoPS result;
	
    result.Position = mul(FB_Projection, aVertex.Position);
    result.Color = aVertex.Color;
    result.UV0 = aVertex.UV0;
    result.UV1 = aVertex.UV1;
    
    MaterialVertexParameters parameters;
    parameters.WorldPosition = result.Position;
    parameters.VertexColor = result.Color;
    parameters.Normal = 0;
    
    Material_Vertex(parameters);
    
    result.WorldPosition = 0;
    result.Normal = 0;
    result.Tangent = 0;
    result.BiNormal = 0;
    result.ViewDepth = 0;
    
    return result;
}
