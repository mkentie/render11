static const uint PF_Masked = 0x00000002;

cbuffer CBufGlobal : register(b0)
{
    float2 fRes;
    float padding[2];
    matrix ProjectionMatrix;
};

sampler SamLinear : register(s0);
sampler SamPoint : register(s1);
