#include "Include/Core/Common.hlsli"
#include "Include/Core/Samplers.hlsli"

Texture2D SelectedMask : register(t0);

static const float OutlineThickness = 1.0f;

// This shader compares the adjacent and vertical pixels to the current one.
// The mask we take in expects an alpha of either 0 or 1.
// This means that the pixels with an alpha of 0 that are next to the ones with an alpha of 1 are edges.
// These edges are then draw as an outline to the backbuffer.

float4 main(FullscreenVertex aInput) : SV_TARGET
{
    uint width, height;
    SelectedMask.GetDimensions(width, height);
    
    const float2 texelSize = 1.0f / float2(width, height);
    
    const float center = SelectedMask.Sample(TrilinearClamp, aInput.UV).a;
    const float left = SelectedMask.Sample(TrilinearClamp, aInput.UV + float2(-texelSize.x * OutlineThickness, 0)).a;
    const float right = SelectedMask.Sample(TrilinearClamp, aInput.UV + float2(texelSize.x * OutlineThickness, 0)).a;
    const float up = SelectedMask.Sample(TrilinearClamp, aInput.UV + float2(0, -texelSize.y * OutlineThickness)).a;
    const float down = SelectedMask.Sample(TrilinearClamp, aInput.UV + float2(0, texelSize.y * OutlineThickness)).a;
    
    const float topLeft = SelectedMask.Sample(TrilinearClamp, aInput.UV + float2(-texelSize.x * OutlineThickness, texelSize.y * OutlineThickness)).a;
    const float topRight = SelectedMask.Sample(TrilinearClamp, aInput.UV + float2(texelSize.x * OutlineThickness, texelSize.y * OutlineThickness)).a;
    const float bottomLeft = SelectedMask.Sample(TrilinearClamp, aInput.UV + float2(-texelSize.x * OutlineThickness, -texelSize.y * OutlineThickness)).a;
    const float bottomRight = SelectedMask.Sample(TrilinearClamp, aInput.UV + float2(texelSize.x * OutlineThickness, -texelSize.y * OutlineThickness)).a;
    
    const float edge = max(
        max(max(abs(center - left), abs(center - right)),
        max(abs(center - up), abs(center - down))),
        max(max(abs(center - topLeft), abs(center - topRight)),
        max(abs(center - bottomLeft), abs(center - bottomRight)))
    );
    
    if (edge < 0.05f)
    {
        discard;
    }

    return float4(OB_Color.rgb, edge * OB_Color.a);
}