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
    IDXGIAdapter1* const pSelectedAdapter = nullptr;
    const D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE;

    UINT iFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#ifdef _DEBUG
    iFlags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
#endif

    const D3D_FEATURE_LEVEL FeatureLevels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};
    D3D_FEATURE_LEVEL FeatureLevel;

    m_SwapChainDesc.BufferCount = 1;
    m_SwapChainDesc.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    m_SwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    m_SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    m_SwapChainDesc.BufferDesc.Height = 0;
    m_SwapChainDesc.BufferDesc.Width = 0;
    m_SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    m_SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;
    m_SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    m_SwapChainDesc.Flags = 0;
    m_SwapChainDesc.OutputWindow = hWnd;
    m_SwapChainDesc.Windowed = TRUE;
    m_SwapChainDesc.SampleDesc.Count = 1;
    m_SwapChainDesc.SampleDesc.Quality = 0;
    m_SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD; //Todo: Win8 DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
    //Todo: waitable swap chain IDXGISwapChain2::GetFrameLatencyWaitableObject

    ThrowIfFail(D3D11CreateDeviceAndSwapChain(pSelectedAdapter, DriverType, NULL, iFlags, FeatureLevels, _countof(FeatureLevels), D3D11_SDK_VERSION, &m_SwapChainDesc, &m_pSwapChain, &m_pDevice, &FeatureLevel, &m_pDeviceContext), L"Failed to create device and / or swap chain.");
    SetResourceName(m_pDeviceContext, "MainDeviceContext");

    LOGMESSAGEF(L"Device created with Feature Level %x.", FeatureLevel);

    ThrowIfFail(m_pDevice.As(&m_pDXGIDevice), L"Failed to get DXGI device.");

    ComPtr<IDXGIAdapter> pAdapter;
    ThrowIfFail(m_pDXGIDevice->GetAdapter(&pAdapter), L"Failed to get DXGI adapter.");
    ThrowIfFail(pAdapter.As(&m_pAdapter), L"Failed to cast DXGI adapter.");

    DXGI_ADAPTER_DESC1 AdapterDesc;
    ThrowIfFail(m_pAdapter->GetDesc1(&AdapterDesc), L"Failed to get adapter descriptor.");

    LOGMESSAGEF(L"Adapter: %s.", AdapterDesc.Description);

    return true;
}

void RenDevBackend::SetRes(const unsigned int iX, const unsigned int iY)
{
    assert(m_pSwapChain);

    ThrowIfFail(m_pSwapChain->ResizeBuffers(m_SwapChainDesc.BufferCount, iX, iY, m_SwapChainDesc.BufferDesc.Format, m_SwapChainDesc.Flags), L"Failed to resize swap chain (%u x %u)", iX, iY);

    CreateRenderTargetViews();
}

void RenDevBackend::CreateRenderTargetViews()
{
    assert(m_pSwapChain);
    assert(m_pDevice);

    ComPtr<ID3D11Texture2D> pBackBufferTex;
    ThrowIfFail(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(pBackBufferTex.GetAddressOf())), L"Failed to get back buffer texture.");
    SetResourceName(pBackBufferTex, "BackBuffer");

    ThrowIfFail(m_pDevice->CreateRenderTargetView(pBackBufferTex.Get(), nullptr, &m_pBackBufferRTV), L"Failed to create RTV for back buffer texture.");
    SetResourceName(m_pBackBufferRTV, "BackBufferRTV");

    m_pDeviceContext->OMSetRenderTargets(1, m_pBackBufferRTV.GetAddressOf(), nullptr);
}

void RenDevBackend::NewFrame()
{
    assert(m_pDeviceContext);
    assert(m_pBackBufferRTV);

    const float ClearColor[] = {0.0f, 0.0f, 0.0f, 0.0f};

    m_pDeviceContext->ClearRenderTargetView(m_pBackBufferRTV.Get(), ClearColor);
}

void RenDevBackend::Present()
{
    assert(m_pSwapChain);
    m_pSwapChain->Present(0, 0);
}

void RenDevBackend::SetViewport(const FSceneNode& SceneNode)
{
    if (m_Viewport.TopLeftX == static_cast<float>(SceneNode.XB) && m_Viewport.TopLeftY == static_cast<float>(SceneNode.YB) && m_Viewport.Width == SceneNode.FX && m_Viewport.Height == SceneNode.FY)
    {
        return;
    }

    m_Viewport.TopLeftX = static_cast<float>(SceneNode.XB);
    m_Viewport.TopLeftY = static_cast<float>(SceneNode.YB);
    m_Viewport.Width = SceneNode.FX;
    m_Viewport.Height = SceneNode.FY;
    m_Viewport.MinDepth = 0.0;
    m_Viewport.MaxDepth = 1.0;

    m_pDeviceContext->RSSetViewports(1, &m_Viewport);
}
