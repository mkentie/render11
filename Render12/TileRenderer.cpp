#include "stdafx.h"
#include "TileRenderer.h"
#include "Helpers.h"
#include "ShaderCompiler.h"

//TODO: remove device reference
TileRenderer::TileRenderer(ID3D12Device& Device, ID3D12RootSignature& RootSignature, ID3D12GraphicsCommandList& CommandList)
:m_Device(Device)
,m_RootSignature(RootSignature)
,m_CommandList(CommandList)
,m_InstanceBuffer(Device, CommandList, 4096)
{
    ShaderCompiler Compiler(L"Render12\\Tile.hlsl");
    const auto VertexShader = Compiler.CompileVertexShader();
    const auto PixelShader = Compiler.CompilePixelShader();

    const D3D12_INPUT_ELEMENT_DESC InputElementDescs[] =
    {
        {"Position", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
        {"TexCoord", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
        {"TexCoord", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
        {"BlendIndices", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1}
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { InputElementDescs, _countof(InputElementDescs) };
    psoDesc.pRootSignature = &m_RootSignature;
    psoDesc.VS = VertexShader.GetByteCode();
    psoDesc.PS = PixelShader.GetByteCode();
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    ThrowIfFail(Device.CreateGraphicsPipelineState(&psoDesc, __uuidof(m_PipelineState), &m_PipelineState), L"Failed to create pipeline state object.");
}

void TileRenderer::NewFrame(const size_t iFrameIndex)
{
    m_iNumDraws = 0;
    m_InstanceBuffer.NewFrame(iFrameIndex);
}

void TileRenderer::Bind()
{
    m_CommandList.SetPipelineState(m_PipelineState.Get());
    m_CommandList.IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void TileRenderer::Draw()
{
    m_CommandList.IASetVertexBuffers(0, 1, &m_InstanceBuffer.GetView());
    m_CommandList.DrawInstanced(4, m_InstanceBuffer.GetNumNewElements(), 0, m_InstanceBuffer.GetFirstNewElementIndex()); //Just draw 4 non-existent vertices per quad, we're only interested in SV_VertexID.
    m_iNumDraws++;
}

TileRenderer::Tile& TileRenderer::GetTile()
{
    return m_InstanceBuffer.PushBack();
}
