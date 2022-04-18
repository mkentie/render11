#pragma once

#include "DynamicBuffer.h"

class GouraudRenderer
{
public:
    struct Vertex
    {
        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT3 Color;
        DirectX::XMFLOAT2 TexCoords;
        unsigned int PolyFlags;
    };

    explicit GouraudRenderer(ID3D11Device& Device, ID3D11DeviceContext& DeviceContext);
    GouraudRenderer(const GouraudRenderer&) = delete;
    GouraudRenderer& operator=(const GouraudRenderer&) = delete;

    void NewFrame();
    void Map() { m_VertexBuffer.Map(); m_IndexBuffer.Map(); }
    void Unmap() { m_VertexBuffer.Unmap(); m_IndexBuffer.Unmap(); }
    bool IsMapped() const { return m_VertexBuffer.IsMapped() || m_IndexBuffer.IsMapped(); }

    void Bind();
    void Draw();

    Vertex* GetTriangleFan(const size_t iSize) { return DynamicGPUBufferHelpers::GetTriangleFan(m_VertexBuffer, m_IndexBuffer, iSize); }

    //Diagnostics
    size_t GetNumIndices() const { return m_IndexBuffer.GetSize(); }
    size_t GetNumDraws() const { return m_iNumDraws; }
    size_t GetMaxIndices() const { return m_IndexBuffer.GetReserved(); }

protected:
    ID3D11Device& m_Device;
    ID3D11DeviceContext& m_DeviceContext;

    ComPtr<ID3D11InputLayout> m_pInputLayout;
    ComPtr<ID3D11VertexShader> m_pVertexShader;
    ComPtr<ID3D11PixelShader> m_pPixelShader;

    DynamicGPUBuffer<Vertex, D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER> m_VertexBuffer;
    DynamicGPUBuffer<unsigned short, D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER> m_IndexBuffer;

    size_t m_iNumDraws = 0; //Number of draw calls this frame, for stats
};