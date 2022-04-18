#pragma once

#include "ConstantBuffer.h"

class GlobalShaderConstants : public XMMAligned
{
public:
    explicit GlobalShaderConstants(ID3D11Device& Device, ID3D11DeviceContext& m_DeviceContext);
    GlobalShaderConstants(const GlobalShaderConstants&) = delete;
    GlobalShaderConstants& operator=(const GlobalShaderConstants&) = delete;

    void Init();
    void SetSceneNode(const FSceneNode& SceneNode);
    void Bind();

protected:
    struct PerFrame
    {
        float fRes[2];
        float padding[2];
        DirectX::XMMATRIX ProjectionMatrix;
    };
    ConstantBuffer<PerFrame> m_CBufPerFrame;

    //Vars for projection change check
    float m_fFov = 0.0f;
    int m_iViewPortX = 0;
    int m_iViewPortY = 0;
};

