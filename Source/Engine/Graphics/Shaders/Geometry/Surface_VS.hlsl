#include "../Include/Core/Common.hlsli"
#include "../Include/Core/Material.hlsli"

struct InstanceData
{
    float4x4 World;
    float4x4 WorldInvT;
};

StructuredBuffer<InstanceData> InstanceBuffer : register(t20);

VStoPS main(Vertex aVertex, uint instanceID : SV_InstanceID)
{
    VStoPS result;
    
    float4x4 world = OB_World;
    float4x4 worldInvT = OB_WorldInvT;

    if (OB_IsInstanced)
    {
        InstanceData instance = InstanceBuffer[instanceID];

        world = instance.World;
        worldInvT = instance.WorldInvT;
    }
    
    float3 vertexNormal = aVertex.Normal;
    float3 vertexTangent = aVertex.Tangent;
    
    if (OB_SkinningFlag)
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
    
    const float3x3 worldNormalRot = (float3x3) worldInvT;
    
    result.Normal = normalize(mul(worldNormalRot, vertexNormal));
    result.Tangent = normalize(mul(worldNormalRot, vertexTangent));
    result.BiNormal = normalize(cross(result.Normal, result.Tangent));
    
    const float4 worldPos = mul(world, aVertex.Position);
    
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
    result.ViewDepth = viewPos.z;
    
    return result;
}