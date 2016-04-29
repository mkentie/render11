#include "stdafx.h"
#include "GouraudRenderer.h"
#include "Helpers.h"
#include "ShaderCompiler.h"

GouraudRenderer::GouraudRenderer(ID3D11Device& Device, ID3D11DeviceContext& DeviceContext)
:m_Device(Device)
,m_DeviceContext(DeviceContext)
,m_VertexBuffer(Device, DeviceContext, 4096)
,m_IndexBuffer(Device, DeviceContext, DynamicGPUBufferHelpers::Fan2StripIndices(m_VertexBuffer.GetReserved()))
{
    //ShaderCompiler Compiler(m_Device, L"Render12\\Gouraud.hlsl");
    //m_pVertexShader = Compiler.CompileVertexShader();

    //const D3D11_INPUT_ELEMENT_DESC InputElementDescs[] =
    //{
    //    {"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"Color", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"BlendIndices", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0} //Todo make 8 bits, if necessary at all -> can't, hlsl doesn't support 8 bit data type
    //};

    //m_pInputLayout = Compiler.CreateInputLayout(InputElementDescs, _countof(InputElementDescs));

    //m_pPixelShader = Compiler.CompilePixelShader();
}


void GouraudRenderer::NewFrame()
{
    m_VertexBuffer.Clear();
    m_IndexBuffer.Clear();
    m_iNumDraws = 0;
}

void GouraudRenderer::Bind()
{
    assert(m_pInputLayout);
    assert(m_pVertexShader);
    assert(m_pPixelShader);

    m_DeviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    m_DeviceContext.IASetInputLayout(m_pInputLayout.Get());

    const UINT Strides[] = {sizeof(Vertex)};
    const UINT Offsets[] = {0};

    m_DeviceContext.IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), Strides, Offsets);
    m_DeviceContext.IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    m_DeviceContext.VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    m_DeviceContext.PSSetShader(m_pPixelShader.Get(), nullptr, 0);
}

void GouraudRenderer::Draw()
{
    assert(!IsMapped());
    m_DeviceContext.DrawIndexed(m_IndexBuffer.GetNumNewElements(), m_IndexBuffer.GetFirstNewElementIndex(), 0);
    m_iNumDraws++;
}

