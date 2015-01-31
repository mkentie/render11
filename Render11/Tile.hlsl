#include "Render11.hlsli"

Texture2D TexDiffuse : register(t0);

struct STile
{
    float4 XYPos : Position0; //Left, right, top, bottom in pixel coordinates
    float4 TexCoord : TexCoord0; //Left, right, top, bottom
    float3 Color : TexCoord1;
    uint PolyFlags : BlendIndices0;
};

struct VSOut
{
    float4 Pos : SV_Position;
    float2 TexCoord : TexCoord0;
    float3 Color : TexCoord1;
    uint PolyFlags : BlendIndices0;
};

VSOut VSMain(const STile Tile, const uint VertexID : SV_VertexID)
{
    VSOut Output;
    const uint IndexX = VertexID / 2;
    const uint IndexY = 3 - VertexID % 2;
    Output.Pos = float4(-1.0f + 2.0f * (Tile.XYPos[IndexX] / fRes.x), 1.0f - 2.0f * (Tile.XYPos[IndexY] / fRes.y), 1.0f, 1.0f);
    Output.TexCoord = float2(Tile.TexCoord[IndexX], Tile.TexCoord[IndexY]);
    Output.Color = Tile.Color;
    Output.PolyFlags = Tile.PolyFlags;
    return Output;
}

float3 PSMain(const VSOut Input) : SV_Target
{
    const float3 Diffuse = TexDiffuse.Sample(SamLinear, Input.TexCoord).rgb;

    if (Input.PolyFlags & PF_Masked)
    {
        clip(TexDiffuse.Sample(SamPoint, Input.TexCoord).a - 0.5f);
    }

    float3 Color = Diffuse * Input.Color.rgb;

    return Color;
}
