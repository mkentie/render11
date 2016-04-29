#pragma once

#include "Helpers.h"

template<class T, D3D11_BIND_FLAG BindFlag>
class DynamicGPUBuffer
{
public:
    explicit DynamicGPUBuffer(ID3D11Device& Device, ID3D11DeviceContext& DeviceContext, const size_t iReserve)
    :m_Device(Device)
    ,m_DeviceContext(DeviceContext)
    {
        Alloc(iReserve);
    }

    DynamicGPUBuffer(const DynamicGPUBuffer&) = delete;
    DynamicGPUBuffer& operator=(const DynamicGPUBuffer&) = delete;

    /**
    It's best to call this at the start of each frame
    */
    void Clear()
    {
        m_iSize = 0;
    }

    size_t GetSize() const
    {
        return m_iSize;
    }

    size_t GetReserved() const
    {
        return m_iReserved;
    }

    size_t GetNumNewElements() const
    {
        return m_iSize - m_iMapStart;
    }

    size_t GetFirstNewElementIndex() const
    {
        return m_iMapStart;
    }

    ID3D11Buffer* Get() const
    {
        assert(m_pBuffer);
        return m_pBuffer.Get();
    }

    ID3D11Buffer* const * GetAddressOf() const
    {
        assert(m_pBuffer);
        return m_pBuffer.GetAddressOf();
    }

    bool IsMapped() const
    {
        return m_Mapping.pData != nullptr;
    }

    void Map()
    {
        MapInternal();
        m_iMapStart = m_iSize; //Track where fresh data begins so users can draw the new data
    }

    void Unmap()
    {
        assert(IsMapped());
        m_DeviceContext.Unmap(m_pBuffer.Get(), 0);
        m_Mapping.pData = nullptr; //For IsMapped()
    }

    void Reserve(const size_t iReserve)
    {
        assert(IsMapped());
        if (m_iReserved < iReserve)
        {
            Unmap();
            Grow(iReserve * 2);
            MapInternal();
        }
    }

    T* PushBack(const size_t iSize)
    {
        assert(IsMapped());
        const size_t iNewSize = m_iSize + iSize;
        Reserve(iNewSize);
        auto* const p = &static_cast<T*>(m_Mapping.pData)[m_iSize];
        m_iSize = iNewSize;
        return p;
    }

    T& PushBack()
    {
        return *PushBack(1);
    }


protected:
    void MapInternal()
    {
        assert(!IsMapped());

        //MS recommends reusing a buffer with NO_OVERWRITE during a frame, and using DISCARD at the start of a frame.
        //Makes sense, because using only DISCARD would result in the driver creating a ton of buffers.
        m_DeviceContext.Map(m_pBuffer.Get(), 0, m_iSize == 0 ? D3D11_MAP::D3D11_MAP_WRITE_DISCARD : D3D11_MAP::D3D11_MAP_WRITE_NO_OVERWRITE, 0, &m_Mapping);
    }


    void Grow(const size_t iAmount)
    {
        assert(m_pBuffer);
        assert(m_iSize + iAmount > m_iReserved);
        ComPtr<ID3D11Buffer> pOldBuffer(std::move(m_pBuffer));

        Alloc(iAmount);

        D3D11_BOX Box;
        Box.left = 0;
        Box.right = m_iSize * sizeof(T);
        Box.top = 0;
        Box.bottom = 1;
        Box.front = 0;
        Box.back = 1;
        m_DeviceContext.CopySubresourceRegion(m_pBuffer.Get(), 0, 0, 0, 0, pOldBuffer.Get(), 0, &Box); //Todo: 11.2 D3D11_COPY_DISCARD
    }

    void Alloc(const size_t iSize)
    {
        D3D11_BUFFER_DESC Desc;
        Desc.ByteWidth = sizeof(T) * iSize;
        Desc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
        Desc.BindFlags = BindFlag;
        Desc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
        Desc.MiscFlags = 0;

        ThrowIfFail(m_Device.CreateBuffer(&Desc, nullptr, &m_pBuffer), L"Failed to create buffer %s.", typeid(T).name());
        SetResourceName(m_pBuffer, typeid(T).name());

        m_iReserved = iSize;
    }

    ID3D11Device& m_Device;
    ID3D11DeviceContext& m_DeviceContext;

    ComPtr<ID3D11Buffer> m_pBuffer;

    D3D11_MAPPED_SUBRESOURCE m_Mapping = {};

    size_t m_iReserved = 0;
    size_t m_iSize = 0;
    size_t m_iMapStart = 0; //!< Start index of current Map() call, so users know which data to draw
};

namespace DynamicGPUBufferHelpers
{
    constexpr size_t Fan2StripIndices(const size_t iSize)
    {
        return 2 * (iSize - 2) + 2;
    }

    template<class VertType, class IndexType>
    VertType* GetTriangleFan(DynamicGPUBuffer<VertType, D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER>& VertexBuffer, DynamicGPUBuffer<IndexType, D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER>& IndexBuffer, const size_t iSize)
    {
        //Generate indices
        assert(iSize >= 3);
        const size_t iNumIndices = Fan2StripIndices(iSize);
        IndexType* const pIndices = IndexBuffer.PushBack(iNumIndices);

        assert(VertexBuffer.GetSize() < std::numeric_limits<IndexType>::max());
        const IndexType iNumVerts = static_cast<IndexType>(VertexBuffer.GetSize());
        pIndices[0] = iNumVerts + 1;
        for (IndexType i = 1; i < iNumIndices - 1; i += 2)
        {
            pIndices[i] = iNumVerts + (i / 2) + 2;
            pIndices[i + 1] = iNumVerts; //Center point
        }
        pIndices[iNumIndices - 1] = std::numeric_limits<IndexType>::max(); //Strip-cut index

        return VertexBuffer.PushBack(iSize);
    }
}
