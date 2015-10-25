#include "Render11.hlsli"

Texture2D TexDiffuse : register(t0);
Texture2D TexLight : register(t1);

struct SPoly
{
    float4 Pos : Position0;
    float2 TexCoord : TexCoord0;
    float2 TexCoord1 : TexCoord1;
    uint PolyFlags : BlendIndices0;
    uint TexFlags : BlendIndices1;
};

struct VSOut
{
    float4 Pos : SV_Position;
    float2 TexCoord : TexCoord0;
    float2 TexCoord1 : TexCoord1;
    uint PolyFlags : BlendIndices0;
    uint TexFlags : BlendIndices1;
};

VSOut VSMain(const SPoly Input)
{
    VSOut Output;
    Output.Pos = mul(Input.Pos, ProjectionMatrix);
    Output.TexCoord = Input.TexCoord;
    Output.TexCoord1 = Input.TexCoord1;
    Output.PolyFlags = Input.PolyFlags;
    Output.TexFlags = Input.TexFlags;
    return Output;
}

float4 PSMain(const VSOut Input) : SV_Target
{
    if (Input.PolyFlags & PF_Masked)
    {
        clip(TexDiffuse.Sample(SamPoint, Input.TexCoord).a - 0.5f);
    }

    float4 Color = float4(1.0f, 1.0f, 1.0f, 1.0f);

    if (Input.TexFlags & 0x00000001)
    {
        const float3 Diffuse = TexDiffuse.Sample(SamLinear, Input.TexCoord).rgb;
        Color.rgb *= Diffuse;
    }
    if (Input.TexFlags & 0x00000002)
    {
        const float3 Light = TexLight.Sample(SamLinear, Input.TexCoord1).bgr * 2.0f;
        Color.rgb *= Light;
    }
    

    return Color;
}
