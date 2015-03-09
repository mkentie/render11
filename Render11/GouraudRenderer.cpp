#include "stdafx.h"
#include "GouraudRenderer.h"
#include "Helpers.h"
#include "ShaderCompiler.h"

//Todo constexpr
const size_t Fan2StripIndices(const size_t iSize) { return 2 * (iSize - 2) + 2; }
//const size_t Fan2StripIndices(const size_t iSize) { return (iSize - 2) * 3; }

GouraudRenderer::GouraudRenderer(ID3D11Device& Device, ID3D11DeviceContext& DeviceContext)
:m_Device(Device)
,m_DeviceContext(DeviceContext)
,m_VertexBuffer(Device, DeviceContext, 4096)
,m_IndexBuffer(Device, DeviceContext, Fan2StripIndices(m_VertexBuffer.GetReserved()))
{
    ShaderCompiler Compiler(m_Device, L"Render11\\Gouraud.hlsl");
    m_pVertexShader = Compiler.CompileVertexShader();

    D3D11_INPUT_ELEMENT_DESC InputElementDescs[] =
    {
        {"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"Color", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BlendIndices", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0} //Todo make 8 bits, if necessary at all -> can't, hlsl doesn't support 8 bit data type
    };

    m_pInputLayout = Compiler.CreateInputLayout(InputElementDescs, _countof(InputElementDescs));

    m_pPixelShader = Compiler.CompilePixelShader();
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
    //m_DeviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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
    //m_DeviceContext.Draw(m_VertexBuffer.GetNumNewElements(), m_VertexBuffer.GetFirstNewElementIndex());
    m_iNumDraws++;
}

GouraudRenderer::Vertex* GouraudRenderer::GetTriangleFan(const size_t iSize)
{
    //Generate indices
    assert(iSize >= 3);
    const size_t iNumIndices = Fan2StripIndices(iSize);
    unsigned short* const pIndices = m_IndexBuffer.PushBack(iNumIndices);

    const unsigned short iNumVerts = static_cast<unsigned short>(m_VertexBuffer.GetSize());
    pIndices[0] = iNumVerts + 1;
    for (unsigned short i = 1; i < iNumIndices - 1; i+=2)
    {
        pIndices[i] = iNumVerts + (i / 2) + 2;
        pIndices[i + 1] = iNumVerts; //Center point
    }
    pIndices[iNumIndices - 1] = std::numeric_limits<unsigned short>::max(); //Strip-cut index

//     for (unsigned short i = 0; i < iNumIndices; i+=3)
//     {
//         pIndices[i] = iNumVerts; //Center point
//         pIndices[i + 1] = iNumVerts + (i/3) + 1;
//         pIndices[i + 2] = iNumVerts + (i / 3) + 2;
//         
//     }

    return m_VertexBuffer.PushBack(iSize);
}


