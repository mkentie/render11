#pragma once
#include "Helpers.h"
#include <d3d12.h>
#include "d3dx12.h"

//TODO: have multiple types of data share a buffer
template<class T>
class DynamicBuffer12
{
public:
    explicit DynamicBuffer12(ID3D12Device& Device, ID3D12GraphicsCommandList& CommandList, const size_t iReserve)
        :m_Device(Device)
        ,m_CommandList(CommandList)
    {
        Alloc(iReserve);
    }

    DynamicBuffer12(const DynamicBuffer12&) = delete;
    DynamicBuffer12& operator=(const DynamicBuffer12&) = delete;

    D3D12_VERTEX_BUFFER_VIEW& GetView()
    {
        return m_View;
    }

    T* PushBack(const size_t iSize)
    {
        assert(m_pData);
        const size_t iNewSize = m_iSize + iSize;
        if (iNewSize > m_iReserved)
        {
            Grow(iNewSize * 2);
        }
        T* const p = &m_pData[GetFirstElementForFrame() + m_iSize];
        m_iSize = iNewSize;
        return p;
    }

    T& PushBack()
    {
        return *PushBack(1);
    }


    size_t GetSize() const { return m_iSize; }
    size_t GetReserved() const { return m_iReserved; }

    void NewFrame(const size_t iFrameIndex)
    {
        m_iCurrFrameIndex = iFrameIndex;
        m_OldBuffers[iFrameIndex].clear(); //If we've reached frame index 'x' again, all of the previous frame 'x''s resources can be destroyed
        m_iSize = 0;
        m_iBatchStart = 0;
    }

    void StartBatch()
    {
        m_iBatchStart = m_iSize;
    }

    size_t GetNumNewElements() const
    {
        return m_iSize - m_iBatchStart;
    }

    size_t GetFirstNewElementIndex() const
    {
        return GetFirstElementForFrame() + m_iBatchStart; //Return index starting from buffer for this frame
    }

    size_t GetFirstElementForFrame() const
    {
        return m_iCurrFrameIndex * m_iReserved;
    }

public:

    void Grow(const size_t iNewSize)
    {
        assert(m_pBuffer);
        assert(iNewSize > m_iReserved);

        Unmap();
        ComPtr<ID3D12Resource> pOldBuffer(std::move(m_pBuffer));
        const size_t iOldReserved = m_iReserved;

        Alloc(iNewSize);
        assert(m_pBuffer);
        //assert(m_iReserved > iOldReserved);

        //Copy only the items that haven't been drawn yet
        //m_CommandList.CopyBufferRegion(m_pBuffer.Get(), GetFirstElementForFrame()*sizeof(T), pOldBuffer.Get(), m_iCurrFrameIndex*iOldReserved*sizeof(T), m_iSize*sizeof(T));
        //m_iSize = GetNumNewElements();
        //m_iBatchStart = 0;
        
        //m_CommandList.CopyBufferRegion(m_pBuffer.Get(), 0, pOldBuffer.Get(), 0, iOldReserved*sizeof(T));
        //m_CommandList.CopyBufferRegion(m_pBuffer.Get(), m_iReserved*sizeof(T), pOldBuffer.Get(), iOldReserved*sizeof(T), iOldReserved*sizeof(T));
        
        //m_CommandList.CopyBufferRegion(m_pBuffer.Get(), GetFirstElementForFrame()*sizeof(T), pOldBuffer.Get(), m_iCurrFrameIndex*iOldReserved*sizeof(T), m_iSize*sizeof(T));
      
        m_OldBuffers[m_iCurrFrameIndex].emplace_back(std::move(pOldBuffer));

    }

    void Alloc(const size_t iReserve)
    {
        const D3D12_HEAP_PROPERTIES HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD);
        const D3D12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sm_iNumFrames * iReserve * sizeof(T)); //Allocate buffer for all frames (so we don't have to (re)allocate n buffers)

        ThrowIfFail(m_Device.CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(m_pBuffer), &m_pBuffer), L"Failed to create dynamic vertex buffer.");
        wchar_t buf[50];
        swprintf_s(buf, L"Dynamic buffer of %S (%Iu)", typeid(T).name(), m_iAllocCount);
        m_pBuffer->SetName(buf);

        m_View.BufferLocation = m_pBuffer->GetGPUVirtualAddress();
        m_View.SizeInBytes = sm_iNumFrames*iReserve*sizeof(T);
        m_View.StrideInBytes = sizeof(T);

        m_iReserved = iReserve;
        m_iAllocCount++;
        Map();
    }

    void Map()
    {
        //const D3D12_RANGE ReadRange = {};
        m_pBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_pData));
    }

    void Unmap()
    {
        const D3D12_RANGE WriteRange = {};
        //TODO
        m_pBuffer->Unmap(0, nullptr);
    }

    ID3D12Device& m_Device;
    ID3D12GraphicsCommandList& m_CommandList;

    static constexpr size_t sm_iNumFrames = 2;
    size_t m_iSize = 0;
    size_t m_iReserved = 0;
    size_t m_iBatchStart = 0; //Keep track of the first undrawn element, so users know where to start drawing
    size_t m_iAllocCount = 0;
    size_t m_iCurrFrameIndex = 0; //Index of frame, i.e. 0/1/0/1 with double buffering

    std::array<std::vector<ComPtr<ID3D12Resource>>, sm_iNumFrames> m_OldBuffers; //Buffers that have been superseded by resize operations, but must be kept alive, for each frame index
    ComPtr<ID3D12Resource> m_pBuffer;

    T* m_pData = nullptr; //Start of buffer
    D3D12_VERTEX_BUFFER_VIEW m_View;
};
