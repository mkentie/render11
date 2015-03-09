#include "Render11.hlsli"

Texture2D TexDiffuse : register(t0);

struct SPoly
{
    float3 Pos : Position0; //Left, right, top, bottom in pixel coordinates
    float3 Color : Color0;
    float2 TexCoord : TexCoord0;
    uint PolyFlags : BlendIndices0;
};

struct VSOut
{
    float4 Pos : SV_Position;
    float3 Color : Color0;
    float2 TexCoord : TexCoord0;
    uint PolyFlags : BlendIndices0;
};

VSOut VSMain(const SPoly Input)
{
    VSOut Output;
    Output.Pos = mul(float4(Input.Pos, 1.0f), ProjectionMatrix);
    Output.Color = Input.Color;
    Output.TexCoord = Input.TexCoord;
    Output.PolyFlags = Input.PolyFlags;
    return Output;
}

float4 PSMain(const VSOut Input) : SV_Target
{
    float4 Color = float4(Input.Color, 1.0f);

    if (Input.PolyFlags & PF_Masked)
    {
        clip(TexDiffuse.Sample(SamPoint, Input.TexCoord).a - 0.5f);
    }

    const float3 Diffuse = TexDiffuse.Sample(SamLinear, Input.TexCoord).rgb;
    Color.rgb *= Diffuse;

    return Color;
}
