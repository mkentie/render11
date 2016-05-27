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

    size_t NewFrame();

    void Present();

    ID3D12Device& GetDevice() const { return *m_pDevice12.Get(); }
    ID3D12CommandAllocator& GetCommandAllocator() const { assert(m_pCommandAllocators[m_iCurrentFrame]); return *m_pCommandAllocators[m_iCurrentFrame].Get(); }
    ID3D12GraphicsCommandList& GetCommandList() const { assert(m_pCommandList); return *m_pCommandList.Get(); }
    ID3D12Resource& GetRenderTargetView() const { assert(m_pRenderTargets[m_iCurrentFrame]); return *m_pRenderTargets[m_iCurrentFrame].Get(); }

protected:
    void CreateRenderTargetViews();

    static constexpr size_t m_iNumFrames = 2; //Number of frames to render
    static constexpr float m_fDepthClearValue = 0.0f;
    size_t m_iCurrentFrame = 0;

    D3D11_VIEWPORT m_Viewport;

    ComPtr<IDXGIAdapter1> m_pAdapter;
    ComPtr<ID3D12Device> m_pDevice12;
    DXGI_SWAP_CHAIN_DESC1 m_SwapChainDesc;
    ComPtr<IDXGISwapChain3> m_pSwapChain;

    ComPtr<ID3D12CommandQueue> m_pCommandQueue;
    std::array<ComPtr<ID3D12CommandAllocator>, m_iNumFrames> m_pCommandAllocators;
    ComPtr<ID3D12GraphicsCommandList> m_pCommandList;

    ComPtr<ID3D12Fence> m_pFence;
    UINT64 m_iFrameFenceValue = 0;
    std::array<UINT64, m_iNumFrames> m_iFrameFenceValues = {};
    std::unique_ptr<std::remove_pointer<HANDLE>::type, decltype(&::CloseHandle)> m_FenceEvent = decltype(m_FenceEvent)(CreateEvent(nullptr, FALSE, FALSE, L"FenceEvent"), &CloseHandle);



    ComPtr<ID3D12DescriptorHeap> m_pRTVHeap;
    std::array<ComPtr<ID3D12Resource>, m_iNumFrames> m_pRenderTargets;
    std::array<D3D12_CPU_DESCRIPTOR_HANDLE, m_iNumFrames> m_RenderTargetViews;


    ComPtr<ID3D12DescriptorHeap> m_pDSVHeap;
    ComPtr<ID3D12Resource> m_pDepthStencil;
    D3D12_CPU_DESCRIPTOR_HANDLE m_DepthStencilView;
};
