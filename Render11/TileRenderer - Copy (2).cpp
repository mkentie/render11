#include "stdafx.h"
#include "TileRenderer.h"
#include "Helpers.h"
#include "ShaderCompiler.h"


TileRenderer::TileRenderer(ID3D11Device& Device, ID3D11DeviceContext& DeviceContext)
:m_Device(Device)
,m_DeviceContext(DeviceContext)
, m_Mapping()
{

}

bool TileRenderer::Init()
{
    HRESULT hResult;

    //We only create a per-instance-data buffer, we don't use a vertex buffer as vertex positions are irrelevant
    D3D11_BUFFER_DESC InstanceBufferDesc;
    InstanceBufferDesc.ByteWidth = sizeof(Tile) * sm_iBufferSize;
    InstanceBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
    InstanceBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
    InstanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
    InstanceBufferDesc.MiscFlags = 0;
    if (FAILED(hResult = m_Device.CreateBuffer(&InstanceBufferDesc, nullptr, &m_pInstanceBuffer)))
    {
        const _com_error ErrInfo(hResult);
        LOGWARNINGF(L"Failed to create buffer: %s", ErrInfo.ErrorMessage());
        return false;
    }
    SetResourceName(m_pInstanceBuffer, "TileInstanceBuffer");

    ShaderCompiler Compiler(m_Device);
    m_pVertexShader = Compiler.CompileVertexShader(L"Render11\\Tile.hlsl");
    if (!m_pVertexShader)
    {
        LOGWARNING(L"Failed to compile vertex shader.");
        return false;
    }

    D3D11_INPUT_ELEMENT_DESC InputElementDescs[] =
    {
        {"Position", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Tile, XYPos), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"Texcoord", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Tile, TexCoord), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"Texcoord", 1, DXGI_FORMAT_R32_FLOAT, 0, offsetof(Tile, Z), D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    m_pInputLayout = Compiler.CreateInputLayout(InputElementDescs, _countof(InputElementDescs));
    if (!m_pInputLayout)
    {
        LOGWARNING(L"Failed to create input layout.");
        return false;
    }
    SetResourceName(m_pInputLayout, "TileInputLayout");

    m_pGeometryShader = Compiler.CompileGeometryShader(L"Render11\\Tile.hlsl");

    m_pPixelShader = Compiler.CompilePixelShader(L"Render11\\Tile.hlsl");
    if (!m_pPixelShader)
    {
        LOGWARNING(L"Failed to compile vertex shader.");
        return false;
    }

    return true;
}

void TileRenderer::Map()
{
    assert(!IsMapped());
    m_DeviceContext.Map(m_pInstanceBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &m_Mapping);
    m_iIndex = 0;
}

void TileRenderer::Unmap()
{
    assert(IsMapped());
    m_DeviceContext.Unmap(m_pInstanceBuffer.Get(), 0);
    m_Mapping.pData = nullptr; //For IsMapped()
}

void TileRenderer::Draw()
{
    assert(!IsMapped());
    m_DeviceContext.IASetPrimitiveTopology(sm_PrimitiveTopology);
    m_DeviceContext.IASetInputLayout(m_pInputLayout.Get());

    const UINT Strides[] = {sizeof(Tile)};
    const UINT Offsets[] = {0};

    m_DeviceContext.IASetVertexBuffers(0, 1, m_pInstanceBuffer.GetAddressOf(), Strides, Offsets);
    m_DeviceContext.VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    m_DeviceContext.GSSetShader(m_pGeometryShader.Get(), nullptr, 0);
    m_DeviceContext.PSSetShader(m_pPixelShader.Get(), nullptr, 0);
    m_DeviceContext.Draw(m_iIndex,0);
}

TileRenderer::Tile& TileRenderer::GetTile()
{
    assert(m_Mapping.pData);
    return static_cast<Tile*>(m_Mapping.pData)[m_iIndex++];
}
