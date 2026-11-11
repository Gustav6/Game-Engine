#include "../Include/Core/Common.hlsli"
#include "../Include/Core/MaterialParameters.hlsli"
#include "../Include/Core/Material.hlsli"

VStoPS main(SpriteVertex aVertex)
{
    VStoPS result;
	
    result.Position = mul(FB_Projection, aVertex.Position);
    result.Color = aVertex.Color;
    result.UV0 = aVertex.UV;
    
    MaterialVertexParameters parameters;
    parameters.WorldPosition = result.Position;
    parameters.VertexColor = result.Color;
    parameters.Normal = 0;
    
    Material_Vertex(parameters);
    
    result.UV1 = 0;
    result.WorldPosition = 0;
    result.Normal = 0;
    result.Tangent = 0;
    result.BiNormal = 0;
    result.ViewDepth = 0;
    
    return result;
}
