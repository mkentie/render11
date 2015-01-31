#pragma once

class RenDevBackend
{
public:
    explicit RenDevBackend();
    RenDevBackend(const RenDevBackend&) = delete;
    RenDevBackend& operator=(const RenDevBackend&) = delete;
    ~RenDevBackend();

    bool Init(const HWND hWnd);
    void SetRes(const unsigned int iX, const unsigned int iY);

    void NewFrame();
    void Present();
    void SetViewport(const FSceneNode& SceneNode);

    ID3D11Device& GetDevice(){ return *m_pDevice.Get(); }
    ID3D11DeviceContext& GetDeviceContext() { return *m_pDeviceContext.Get(); }

protected:
    void CreateRenderTargetViews();

    D3D11_VIEWPORT m_Viewport;

    ComPtr<IDXGIDevice1> m_pDXGIDevice;
    ComPtr<IDXGIAdapter1> m_pAdapter;
    ComPtr<ID3D11Device> m_pDevice;
    ComPtr<ID3D11DeviceContext> m_pDeviceContext;
    ComPtr<IDXGISwapChain> m_pSwapChain;
    ComPtr<ID3D11RenderTargetView> m_pBackBufferRTV;

    DXGI_SWAP_CHAIN_DESC m_SwapChainDesc;
};
