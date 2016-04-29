#pragma once

#include "DynamicBuffer12.h"
#include "Helpers.h"
#include "ShaderCompiler.h"

class TileRenderer : public XMMAligned
{
public:
    struct Tile
    {
        DirectX::XMFLOAT4 XYPos;
        DirectX::XMFLOAT4 TexCoord;
        DirectX::XMFLOAT3 Color;
        unsigned int PolyFlags;
    };

    explicit TileRenderer(ID3D12Device& Device, ID3D12RootSignature& RootSignature, ID3D12GraphicsCommandList& CommandList);
    TileRenderer(const TileRenderer&) = delete;
    TileRenderer& operator=(const TileRenderer&) = delete;

    void NewFrame(const size_t iFrameIndex);
    void StartBatch() { assert(!m_bInBatch);  m_InstanceBuffer.StartBatch(); m_bInBatch = true; }
    void StopBatch() { m_bInBatch = false; }
    bool InBatch() const { return m_bInBatch; }

    void Bind();
    void Draw();

    Tile& GetTile();

    //Diagnostics
    size_t GetNumTiles() const { return m_InstanceBuffer.GetSize(); }
    size_t GetNumDraws() const { return m_iNumDraws;  }
    size_t GetMaxTiles() const { return m_InstanceBuffer.GetReserved(); }

protected:
    ID3D12Device& m_Device;
    ID3D12RootSignature& m_RootSignature;
    ID3D12GraphicsCommandList& m_CommandList;

    ShaderCompiler::CompiledShader m_pVertexShader;
    ShaderCompiler::CompiledShader m_pPixelShader;

    ComPtr<ID3D12PipelineState> m_PipelineState;

    DynamicBuffer12<Tile> m_InstanceBuffer;  //We only create a per-instance-data buffer, we don't use a vertex buffer as vertex positions are irrelevant

    size_t m_iNumDraws = 0; //Number of draw calls this frame, for stats
    bool m_bInBatch = false;
};