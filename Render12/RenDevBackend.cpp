#include "stdafx.h"
#include "RendevBackend.h"
#include "Helpers.h"

RenDevBackend::RenDevBackend()
{

}

RenDevBackend::~RenDevBackend()
{

}

bool RenDevBackend::Init(const HWND hWnd)
{
    ComPtr<IDXGIFactory4> pFactory;
    ThrowIfFail(CreateDXGIFactory1(__uuidof(IDXGIFactory4), &pFactory), L"Failed to create DXGI factory.");

    { //Scope for pointer
        ComPtr<ID3D12Debug> pDebugController;
        D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController));
        assert(pDebugController);
        pDebugController->EnableDebugLayer();
    }

    //Create device
    const D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_11_0;
    ComPtr<IDXGIAdapter1> pAdapter;
    for (UINT iAdapter = 0; SUCCEEDED(ThrowIfFail(pFactory->EnumAdapters1(iAdapter, &pAdapter), L"Failed to enumerate a suitable adapter.")); ++iAdapter)
    {
        DXGI_ADAPTER_DESC1 Desc;
        pAdapter->GetDesc1(&Desc);

        if (Desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }

        if (SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), FeatureLevel, __uuidof(ID3D12Device), nullptr)))
        {
            LOGMESSAGEF(L"Adapter: %s.", Desc.Description);
            break;
        }
    }

    ThrowIfFail(D3D12CreateDevice(pAdapter.Get(), FeatureLevel, __uuidof(m_pDevice12), &m_pDevice12), L"Failed to create device.");
    m_pDevice12->SetName(L"MainDevice");

    m_pAdapter = std::move(pAdapter);
    SetResourceName(m_pAdapter, "MainAdapter");

    //Create command queue
    D3D12_COMMAND_QUEUE_DESC QueueDesc;
    QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE;
    QueueDesc.Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;
    QueueDesc.NodeMask = 0;
    QueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

    ThrowIfFail(m_pDevice12->CreateCommandQueue(&QueueDesc, __uuidof(m_pCommandQueue), &m_pCommandQueue), L"Failed to create command queue.");
    m_pCommandQueue->SetName(L"MainCommandQueue");

    for (size_t i = 0; i < m_iNumFrames; i++)
    {
        m_pDevice12->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(m_pCommandAllocators[i]), &m_pCommandAllocators[i]);
        wchar_t buf[50];
        swprintf_s(buf, L"Command allocator for frame %Iu", i);
        m_pCommandAllocators[i]->SetName(buf);
    }

    ThrowIfFail(m_pDevice12->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, &GetCommandAllocator(), nullptr, __uuidof(m_pCommandList), &m_pCommandList), L"Failed to create command list.");
    m_pCommandList->SetName(L"The command list");
    m_pCommandList->Close(); //Close so we can also can reset at start of frame

    ThrowIfFail(m_pDevice12->CreateFence(0, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, __uuidof(m_pFence), &m_pFence), L"Failed to create fence.");
    m_pFence->SetName(L"Frame sync fence");

    //Create swap chain
    m_SwapChainDesc.BufferCount = m_iNumFrames;
    m_SwapChainDesc.Width = 0;
    m_SwapChainDesc.Height = 0;
    m_SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    m_SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    m_SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    m_SwapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> pSwapChain;
    ThrowIfFail(pFactory->CreateSwapChainForHwnd(m_pCommandQueue.Get(), hWnd, &m_SwapChainDesc, nullptr, nullptr, &pSwapChain), L"Failed to create swap chain.");
    SetResourceName(pSwapChain, "MainSwapChain");

    ThrowIfFail(pSwapChain.As(&m_pSwapChain), L"Failed to cast swap chain.");
    m_iCurrentFrame = m_pSwapChain->GetCurrentBackBufferIndex();

    return true;
}

void RenDevBackend::SetRes(const unsigned int iX, const unsigned int iY)
{
    assert(m_pSwapChain);

    m_pDepthStencil = nullptr;

    m_SwapChainDesc.Width = iX;
    m_SwapChainDesc.Height = iY;
    ThrowIfFail(m_pSwapChain->ResizeBuffers(m_SwapChainDesc.BufferCount, iX, iY, m_SwapChainDesc.Format, m_SwapChainDesc.Flags), L"Failed to resize swap chain (%u x %u).", iX, iY);

    CreateRenderTargetViews();
}

void RenDevBackend::CreateRenderTargetViews()
{
    assert(m_pSwapChain);
    assert(m_pDevice12);

    //Backbuffers
    D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc = {};
    RTVHeapDesc.NumDescriptors = m_iNumFrames;
    RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFail(m_pDevice12->CreateDescriptorHeap(&RTVHeapDesc, __uuidof(m_pRTVHeap), &m_pRTVHeap), L"Failed to create RTV descriptor heap.");
    m_pRTVHeap->SetName(L"Render target view heap");

    CD3DX12_CPU_DESCRIPTOR_HANDLE RTVDescriptor(m_pRTVHeap->GetCPUDescriptorHandleForHeapStart());
    const auto RTVDescriptorSize = m_pDevice12->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    for (size_t i = 0; i < m_iNumFrames; i++)
    {
        ThrowIfFail(m_pSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), &m_pRenderTargets[i]), L"Failed to get swap chain buffer %Iu.", i);
        m_pDevice12->CreateRenderTargetView(m_pRenderTargets[i].Get(), nullptr, RTVDescriptor);
        m_RenderTargetViews[i] = RTVDescriptor;
        RTVDescriptor.Offset(RTVDescriptorSize);

        wchar_t buf[50];
        swprintf_s(buf, L"Main RT resource %Iu", i);
        m_pRenderTargets[i]->SetName(buf);
    }

    //Depth stencil
    const D3D12_HEAP_PROPERTIES DSHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT);

    const D3D12_RESOURCE_DESC DSResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT, m_SwapChainDesc.Width, m_SwapChainDesc.Height, 1, 0, 1, 0, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

    D3D12_CLEAR_VALUE DepthOptimizedClearValue = {};
    DepthOptimizedClearValue.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
    DepthOptimizedClearValue.DepthStencil.Depth = m_fDepthClearValue;
    DepthOptimizedClearValue.DepthStencil.Stencil = 0;

    ThrowIfFail(m_pDevice12->CreateCommittedResource(&DSHeapProperties, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, &DSResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE, &DepthOptimizedClearValue, __uuidof(m_pDepthStencil), &m_pDepthStencil), L"Failed to create depth-stencil resource.");
    m_pDepthStencil->SetName(L"Main depth-stencil resource");

    D3D12_DESCRIPTOR_HEAP_DESC DSVHeapDesc = {};
    DSVHeapDesc.NumDescriptors = 1;
    DSVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    DSVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFail(m_pDevice12->CreateDescriptorHeap(&DSVHeapDesc, __uuidof(m_pDSVHeap), &m_pDSVHeap), L"Failed to create DSV descriptor heap.");
    m_pDSVHeap->SetName(L"Main depth-stencil view heap");

    m_DepthStencilView = m_pDSVHeap->GetCPUDescriptorHandleForHeapStart();
    m_pDevice12->CreateDepthStencilView(m_pDepthStencil.Get(), nullptr, m_DepthStencilView);
}

size_t RenDevBackend::NewFrame()
{
    assert(m_pSwapChain);
    assert(m_pCommandList);
    assert(m_pRTVHeap);
    assert(m_pDSVHeap);

    //If we're in frame 0, wait for the previous frame 0 to complete, etc.
    m_iCurrentFrame = m_pSwapChain->GetCurrentBackBufferIndex();

    if (m_pFence->GetCompletedValue() < m_iFrameFenceValues[m_iCurrentFrame]) //From MS example -> perf optimization?
    {
        ThrowIfFail(m_pFence->SetEventOnCompletion(m_iFrameFenceValues[m_iCurrentFrame], m_FenceEvent.get()), L"Fence SetEventOnCompletion() failed.");
        WaitForSingleObject(m_FenceEvent.get(), INFINITE);
    }

    ThrowIfFail(GetCommandAllocator().Reset(), L"Failed to reset command allocator %Iu.", m_iCurrentFrame);
    ThrowIfFail(m_pCommandList->Reset(&GetCommandAllocator(), nullptr), L"Failed to reset command list");

    CD3DX12_RESOURCE_BARRIER RenderTargetResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(&GetRenderTargetView(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_pCommandList->ResourceBarrier(1, &RenderTargetResourceBarrier);

    const float ClearColor[] = { 1.0f, 0.0f, 0.0f, 0.0f };
    m_pCommandList->ClearRenderTargetView(m_RenderTargetViews[m_iCurrentFrame], ClearColor, 0, nullptr);
    m_pCommandList->ClearDepthStencilView(m_DepthStencilView, D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH, m_fDepthClearValue, 0, 0, nullptr);
    m_pCommandList->OMSetRenderTargets(1, &m_RenderTargetViews[m_iCurrentFrame], TRUE, &m_DepthStencilView);

    return m_iCurrentFrame; //Other parts of the application are now allowed to destroy stuff from the previous iteration of this frame
}


void RenDevBackend::Present()
{
    assert(m_pSwapChain);
    assert(m_pCommandList);
    assert(m_pCommandQueue);

    CD3DX12_RESOURCE_BARRIER PresentResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(&GetRenderTargetView(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_pCommandList->ResourceBarrier(1, &PresentResourceBarrier);

    ThrowIfFail(m_pCommandList->Close(), L"Failed to close command list.");

    const std::array<ID3D12CommandList*, 1> Lists = { m_pCommandList.Get() };
    m_pCommandQueue->ExecuteCommandLists(Lists.size(), Lists.data());
    m_pSwapChain->Present(0, 0);

    // Signal fence
    m_pCommandQueue->Signal(m_pFence.Get(), m_iFrameFenceValue++);
    m_iFrameFenceValues[m_iCurrentFrame] = m_iFrameFenceValue;
}
