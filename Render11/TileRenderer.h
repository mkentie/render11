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
    unsigned int GetNumTiles() const { return m_InstanceBuffer.GetSize(); }
    unsigned int GetNumDraws() const { return m_iNumDraws;  }
    unsigned int GetMaxTiles() const { return m_InstanceBuffer.GetMaxSize();  }

protected:
    static const D3D11_PRIMITIVE_TOPOLOGY sm_PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

    ID3D11Device& m_Device;
    ID3D11DeviceContext& m_DeviceContext;

    ComPtr<ID3D11InputLayout> m_pInputLayout;
    ComPtr<ID3D11VertexShader> m_pVertexShader;
    ComPtr<ID3D11PixelShader> m_pPixelShader;

    DynamicGPUBuffer<Tile> m_InstanceBuffer;  //We only create a per-instance-data buffer, we don't use a vertex buffer as vertex positions are irrelevant

    unsigned int m_iNumDraws; //Number of draw calls this frame, for stats

};