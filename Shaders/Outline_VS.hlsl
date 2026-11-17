#include "Include/Core/Common.hlsli"
#include "Include/Core/Material.hlsli"

StructuredBuffer<InstanceData> InstanceBuffer : register(t20);

VStoPS main(Vertex aVertex, uint instanceID : SV_InstanceID)
{
    VStoPS result;
     
    float4x4 world = OB_World;
    float4x4 worldInvT = OB_WorldInvT;

    if (OB_IsInstanced)
    {
        InstanceData instance = InstanceBuffer[instanceID];
        
        world = mul(OB_World, instance.World);
        worldInvT = instance.WorldInvT;
    }
    
    const float3 vertexNormal = aVertex.Normal;
    const float3x3 worldNormalRot = (float3x3)worldInvT;
    
    result.Normal = normalize(mul(worldNormalRot, vertexNormal));
    
    const float4 worldPos = mul(world, aVertex.Position);
    const float4 viewPos = mul(FB_View, worldPos);
    
    float4 clipPos = mul(FB_Projection, viewPos);
    float4 clipNormal = mul(FB_Projection, mul(FB_View, float4(result.Normal, 0.0f)));
    
    float2 offset = normalize(clipNormal.xy) * (10 / float2(FB_ViewportSize)) * clipPos.w * 2.0f;
    clipPos.xy += offset;

    result.Position = clipPos;
    
    result.Color = 0;
    result.BiNormal = 0;
    result.Tangent = 0;
    result.UV0 = 0;
    result.UV1 = 0;
    result.ViewDepth = 0;
    result.WorldPosition = 0;
    
    return result;
}