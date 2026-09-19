#include "Include/Common.hlsli"
#include "Include/Material.hlsli"

VStoPS main(Vertex aVertex)
{
    VStoPS result;
    
    float3 vertexNormal = aVertex.Normal;
    float3 vertexTangent = aVertex.Tangent;
    
    if (OB_HasSkinning)
    {
        float4x4 skinMatrix = 0;
        
        skinMatrix += AB_JointTransforms[aVertex.BoneIDs[0]] * aVertex.SkinWeights[0];
        skinMatrix += AB_JointTransforms[aVertex.BoneIDs[1]] * aVertex.SkinWeights[1];
        skinMatrix += AB_JointTransforms[aVertex.BoneIDs[2]] * aVertex.SkinWeights[2];
        skinMatrix += AB_JointTransforms[aVertex.BoneIDs[3]] * aVertex.SkinWeights[3];
        
        aVertex.Position = mul(skinMatrix, aVertex.Position);
        
        const float3x3 skinRotation = (float3x3) skinMatrix;
        vertexNormal = mul(vertexNormal, skinRotation);
        vertexTangent = mul(vertexTangent, skinRotation);
    }
    
    const float3x3 worldNormalRot = (float3x3) OB_WorldInvT;
    
    result.Normal = normalize(mul(worldNormalRot, vertexNormal));
    result.Tangent = normalize(mul(worldNormalRot, vertexTangent));
    result.BiNormal = normalize(cross(result.Normal, result.Tangent));
    
    const float4 worldPos = mul(OB_World, aVertex.Position);
    
    MaterialVertexParameters parameters;
    parameters.WorldPosition = worldPos;
    parameters.VertexColor = aVertex.Color;
    parameters.Normal = result.Normal;
    
    Material_Vertex(parameters);
    
    const float4 viewPos = mul(FB_View, parameters.WorldPosition);
    const float4 clipPos = mul(FB_Projection, viewPos);
    
    result.Position = clipPos;
    result.WorldPosition = parameters.WorldPosition;
    result.Color = parameters.VertexColor;
    result.UV0 = aVertex.UV0;
    result.UV1 = aVertex.UV1;
    
    return result;
}