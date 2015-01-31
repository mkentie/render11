#pragma once

class DeviceState
{
public:
    enum class RASTERIZER_STATE { DEFAULT, WIREFRAME, COUNT };
    enum class DEPTH_STENCIL_STATE { DEFAULT, NO_WRITE, COUNT };
    enum class BLEND_STATE { DEFAULT, MODULATE, TRANSLUCENT, TRANSLUCENT_FAKE_MULTIPASS, ALPHABLEND, INVIS, COUNT }; //TODO: for invis, just disable pixel shader
    enum class SAMPLER_STATE { LINEAR, POINT, COUNT };

    explicit DeviceState(ID3D11Device& Device, ID3D11DeviceContext& DeviceContext);
    DeviceState(const DeviceState&) = delete;
    DeviceState& operator= (const DeviceState&) = delete;

    bool Init();

    BLEND_STATE DeviceState::GetBlendStateForPolyFlags(const DWORD PolyFlags) const;
    bool IsBlendStatePrepared(const BLEND_STATE BlendState) const { return m_PreparedBlendState == BlendState; }
    void PrepareBlendState(const BLEND_STATE BlendState) { m_PreparedBlendState = BlendState; }
    /**
    Binds all sampler states to slots 0 - SAMPLER_STATE::COUNT -1; custom samplers should be bound to later slots.
    */
    void BindSamplerStates() const;

    void Bind() const;

protected:
    //Prototype for the various ID3D11Device::Create[Rasterizer/DepthStencil/Etc.]State() functions.
    template<class Desc, class StateType>
    using StateCreationFunc = HRESULT(__stdcall ID3D11Device::*)(const Desc*, StateType**);

    template<StateCreationFunc Func, class DescType, class StateType, const size_t Num>
    void CreateStates(const std::array<DescType, Num>& Descs, std::array<ComPtr<StateType>, Num>& States);

    void CreateRasterizerStates();
    void CreateDepthStencilStates();
    void CreateBlendStates();
    void CreateSamplerStates();

    ID3D11Device& m_Device;
    ID3D11DeviceContext& m_DeviceContext;

    std::array<ComPtr<ID3D11RasterizerState>, static_cast<size_t>(RASTERIZER_STATE::COUNT)> m_RasterizerStates;
    RASTERIZER_STATE m_PreparedRasterizerState;

    std::array<ComPtr<ID3D11DepthStencilState>, static_cast<size_t>(DEPTH_STENCIL_STATE::COUNT)> m_DepthStencilStates;
    DEPTH_STENCIL_STATE m_PreparedDepthStencilState;

    std::array<ComPtr<ID3D11BlendState>, static_cast<size_t>(BLEND_STATE::COUNT)> m_BlendStates;
    BLEND_STATE m_PreparedBlendState;

    std::array<ComPtr<ID3D11SamplerState>, static_cast<size_t>(SAMPLER_STATE::COUNT)> m_SamplerStates;

};
