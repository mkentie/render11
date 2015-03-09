#include "stdafx.h"
#include "TileRenderer.h"
#include "Helpers.h"
#include "ShaderCompiler.h"

TileRenderer::TileRenderer(ID3D11Device& Device, ID3D11DeviceContext& DeviceContext)
:m_Device(Device)
,m_DeviceContext(DeviceContext)
,m_InstanceBuffer(Device, DeviceContext, 4096)
{
    ShaderCompiler Compiler(m_Device, L"Render11\\Tile.hlsl");
    m_pVertexShader = Compiler.CompileVertexShader();

    D3D11_INPUT_ELEMENT_DESC InputElementDescs[] =
    {
        {"Position", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"TexCoord", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"TexCoord", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"BlendIndices", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1}
    };

    m_pInputLayout = Compiler.CreateInputLayout(InputElementDescs, _countof(InputElementDescs));

    m_pPixelShader = Compiler.CompilePixelShader();
}

void TileRenderer::NewFrame()
{
    m_InstanceBuffer.Clear();
    m_iNumDraws = 0;
}

void TileRenderer::Bind()
{
    assert(m_pInputLayout);
    assert(m_pVertexShader);
    assert(m_pPixelShader);

    m_DeviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    m_DeviceContext.IASetInputLayout(m_pInputLayout.Get());

    const UINT Strides[] = {sizeof(Tile)};
    const UINT Offsets[] = {0};

    m_DeviceContext.IASetVertexBuffers(0, 1, m_InstanceBuffer.GetAddressOf(), Strides, Offsets);
    m_DeviceContext.VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    m_DeviceContext.PSSetShader(m_pPixelShader.Get(), nullptr, 0);
}

void TileRenderer::Draw()
{
    assert(!IsMapped());
    m_DeviceContext.DrawInstanced(4, m_InstanceBuffer.GetNumNewElements(), 0, m_InstanceBuffer.GetFirstNewElementIndex()); //Just draw 4 non-existent vertices per quad, we're only interested in SV_VertexID.
    m_iNumDraws++;
}

TileRenderer::Tile& TileRenderer::GetTile()
{
    return m_InstanceBuffer.PushBack();
}
