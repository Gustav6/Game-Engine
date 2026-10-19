#include "Common.hlsli"

// This shades takes in a Vertex Index which is given 
// by the Draw command. It will be looped for each 
// vertex so given Draw(4) this shader will execute 
// 4 times , creating a quad of 2 triangles covering
// 100% of what ever texture we are rendering to.

FullscreenVertex main(uint aVertexIndex : SV_VertexID)
{
    const float4 pos[4] =
    {
        float4(-1, -1, 0, 1),
        float4(-1, 1, 0, 1),
        float4(1, -1, 0, 1),
        float4(1, 1, 0, 1)
    };
    
    const float2 uv[4] =
    {
        float2(0.0f, 1.0f),
        float2(0.0f, 0.0f),
        float2(1.0f, 1.0f),
        float2(1.0f, 0.0f),
    };
    
    FullscreenVertex returnValue;
    
    returnValue.Position = pos[aVertexIndex];
    returnValue.UV = uv[aVertexIndex];
    
    return returnValue;
}