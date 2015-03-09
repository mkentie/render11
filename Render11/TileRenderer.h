#pragma once

#include "DynamicBuffer.h"

class TileRenderer
{
public:
    struct Tile
    {
        DirectX::XMFLOAT4 XYPos;
        DirectX::XMFLOAT4 TexCoord;
        DirectX::XMFLOAT3 Color;
        unsigned int PolyFlags;
    };

    explicit TileRenderer(ID3D11Device& Device, ID3D11DeviceContext& DeviceContext);
    TileRenderer(const TileRenderer&) = delete;
    TileRenderer& operator=(const TileRenderer&) = delete;

    void NewFrame();
    void Map() { m_InstanceBuffer.Map(); }
    void Unmap() { m_InstanceBuffer.Unmap(); }
    bool IsMapped() const { return m_InstanceBuffer.IsMapped(); }

    void Bind();
    void Draw();

    Tile& GetTile();

    //Diagnostics
    size_t GetNumTiles() const { return m_InstanceBuffer.GetSize(); }
    size_t GetNumDraws() const { return m_iNumDraws;  }
    size_t GetMaxTiles() const { return m_InstanceBuffer.GetReserved(); }

protected:
    ID3D11Device& m_Device;
    ID3D11DeviceContext& m_DeviceContext;

    ComPtr<ID3D11InputLayout> m_pInputLayout;
    ComPtr<ID3D11VertexShader> m_pVertexShader;
    ComPtr<ID3D11PixelShader> m_pPixelShader;

    DynamicGPUBuffer<Tile, D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER> m_InstanceBuffer;  //We only create a per-instance-data buffer, we don't use a vertex buffer as vertex positions are irrelevant

    size_t m_iNumDraws = 0; //Number of draw calls this frame, for stats
};