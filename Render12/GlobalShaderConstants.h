#pragma once

#include "ConstantBuffer.h"

class GlobalShaderConstants : public XMMAligned
{
public:
    explicit GlobalShaderConstants(ID3D12Device& Device, ID3D12GraphicsCommandList& CommandList);
    GlobalShaderConstants(const GlobalShaderConstants&) = delete;
    GlobalShaderConstants& operator=(const GlobalShaderConstants&) = delete;

    void Init();
    void SetSceneNode(const FSceneNode& SceneNode);
    void Bind();

    ID3D12RootSignature& GetRootSignature() const { assert(m_pRootSignature); return *m_pRootSignature.Get(); }

protected:
    struct PerFrame
    {
        float fRes[2];
        float padding[2];
        DirectX::XMMATRIX ProjectionMatrix;
    };
    PerFrame m_CBufPerFrame;

    //Vars for projection change check
    float m_fFov = 0.0f;
    int m_iViewPortX = 0;
    int m_iViewPortY = 0;
    bool m_bDirty = false;

    ComPtr<ID3D12RootSignature> m_pRootSignature;
    ID3D12GraphicsCommandList& m_CommandList;
};

