#pragma once

#include "Helpers.h"

template <class T>
class ConstantBuffer
{
    static_assert(sizeof(T) % 16 == 0, "Constant buffer size must be multiple of 16");

public:
    explicit ConstantBuffer(ID3D11Device& Device, ID3D11DeviceContext& DeviceContext)
    :m_DeviceContext(DeviceContext)
    {
        D3D11_BUFFER_DESC BufferDesc;
        BufferDesc.ByteWidth = sizeof(T);
        BufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
        BufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
        BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
        BufferDesc.MiscFlags = 0;

        ThrowIfFail(Device.CreateBuffer(&BufferDesc, nullptr, &m_pBuffer), L"Failed to create constant buffer %s.", typeid(T).name());
        SetResourceName(m_pBuffer, typeid(T).name());
    }

    ConstantBuffer(const ConstantBuffer&) = delete;
    ConstantBuffer& operator=(const ConstantBuffer&) = delete;

    void MarkAsDirty() { m_bDirty = true; }

    bool IsDirty() const { return m_bDirty; }

    void Update()
    {
        assert(m_bDirty);
        D3D11_MAPPED_SUBRESOURCE Mapping;
        m_DeviceContext.Map(m_pBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &Mapping);
        assert(Mapping.pData);
        *static_cast<T*>(Mapping.pData) = m_Data;
        m_DeviceContext.Unmap(m_pBuffer.Get(), 0);
    }

    void Bind(const unsigned int iSlot)
    {
        //TODO: constant buffer registration pool
        m_DeviceContext.VSSetConstantBuffers(iSlot, 1, m_pBuffer.GetAddressOf());
        m_DeviceContext.PSSetConstantBuffers(iSlot, 1, m_pBuffer.GetAddressOf());
    }

#pragma warning(suppress: 4324) //structure was padded due to __declspec(align())
    T m_Data;

private:
    ID3D11DeviceContext& m_DeviceContext;
    ComPtr<ID3D11Buffer> m_pBuffer;

    bool m_bDirty;
};
