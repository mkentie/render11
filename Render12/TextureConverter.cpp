#include "stdafx.h"
#include "TextureConverter.h"
#include "Helpers.h"
#include "TextureCache.h"

void TextureConverter::ConvertedTextureData::Resize(const FTextureInfo& Texture, const IFormatConverter& FormatConverter)
{
    m_Mips.clear();

    m_SubResourceData.resize(Texture.NumMips);
    m_Footprints.resize(Texture.NumMips);

    const bool bWantsBuffer = FormatConverter.WantsBuffer();
    if (bWantsBuffer)
    {
        m_Mips.resize(Texture.NumMips);
    }

    for (size_t i = 0; i < m_SubResourceData.size(); i++)
    {
        assert(Texture.Mips[i]);
        const FMipmapBase& UnrealMip = *Texture.Mips[i];
        if (bWantsBuffer)
        {
            m_Mips[i].resize(UnrealMip.USize * UnrealMip.VSize);
            m_SubResourceData[i].pData = m_Mips[i].data();
        }
        m_SubResourceData[i].RowPitch = FormatConverter.GetStride(UnrealMip);
        m_SubResourceData[i].SlicePitch = 0;
    }
}

void TextureConverter::FormatConverterIdentity::Convert(const FTextureInfo& Texture)
{
    m_Buffer.Resize(Texture, *this);
    for (INT i = 0; i < Texture.NumMips; i++)
    {
        assert(Texture.Mips[i]);
        const FMipmapBase& UnrealMip = *Texture.Mips[i];
        m_Buffer.SetSubResourceDataSysMem(i, UnrealMip.DataPtr);
    }
}

void TextureConverter::FormatConverterP8::Convert(const FTextureInfo& Texture)
{
    assert(Texture.Format == ETextureFormat::TEXF_P8);
    assert(Texture.Palette);

    FColor* const Palette = Texture.Palette;

    //Palette color 0 is the alpha mask color, we always give it alpha 0 and make it look black
    Palette[0].R = Palette[0].G = Palette[0].B = Palette[0].A = 0;

    m_Buffer.Resize(Texture, *this);
    for (INT i = 0; i < Texture.NumMips; i++)
    {
        assert(Texture.Mips[i]);
        const FMipmapBase& UnrealMip = *Texture.Mips[i];

        const auto* const pSourceBegin = UnrealMip.DataPtr;
        const auto* const pSourceEnd = UnrealMip.DataPtr + UnrealMip.USize*UnrealMip.VSize;
        auto pDest = m_Buffer.GetMipBuffer(i);
        
        for (const auto* pSource = pSourceBegin; pSource != pSourceEnd; pSource++, pDest++)
        {
            *pDest = reinterpret_cast<uint32_t&>(Palette[*pSource]);
        }
    }
}

TextureConverter::TextureConverter(ID3D12Device& Device, ID3D12GraphicsCommandList& CommandList, ID3D12DescriptorHeap& SRVDescriptorHeap)
:m_Device(Device)
,m_CommandList(CommandList)
,m_SRVDescriptorHeap(SRVDescriptorHeap)
{
    //TODO: Create placeholder texture
}

TextureConverter::TextureData TextureConverter::Convert(const FTextureInfo& Texture)
{
    IFormatConverter* const pConverter = m_FormatConverters[Texture.Format];
    if (pConverter == nullptr)
    {
         return m_PlaceholderTexture;
    }

    const bool bDynamic = Texture.bRealtimeChanged; //bRealtime isn't always set

    pConverter->Convert(Texture);

    TextureData OutputTexture;

    D3D12_RESOURCE_DESC TextureDesc;
    TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    TextureDesc.Alignment = 0;
    TextureDesc.Width = Texture.UClamp;
    TextureDesc.Height = Texture.VClamp;
    TextureDesc.DepthOrArraySize = 1;
    TextureDesc.MipLevels = static_cast<UINT16>(Texture.NumMips);
    TextureDesc.Format = pConverter->GetDXGIFormat();
    TextureDesc.SampleDesc.Count = 1;
    TextureDesc.SampleDesc.Quality = 0;
    TextureDesc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_UNKNOWN;
    TextureDesc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;

    const wchar_t* const pszTexName = Texture.Texture ? Texture.Texture->GetName() : nullptr;

    const D3D12_HEAP_PROPERTIES HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT);

    ThrowIfFail(m_Device.CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &TextureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, __uuidof(OutputTexture.pTexture), &OutputTexture.pTexture), L"Failed to create texture '%s'.", pszTexName);
    SetResourceNameW(OutputTexture.pTexture, pszTexName);

    UINT64 iUploadSize;
    m_Device.GetCopyableFootprints(&TextureDesc, 0, TextureDesc.MipLevels, 0, m_ConvertedTextureData.GetFootprintArray(), nullptr, nullptr, &iUploadSize);

    D3D12_RESOURCE_DESC BufferDesc;
    BufferDesc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
    BufferDesc.Alignment = 0;
    BufferDesc.Width = iUploadSize;
    BufferDesc.Height = 1;
    BufferDesc.DepthOrArraySize = 1;
    BufferDesc.MipLevels = 1;
    BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
    BufferDesc.SampleDesc.Count = 1;
    BufferDesc.SampleDesc.Quality = 0;
    BufferDesc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    BufferDesc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;

    ComPtr<ID3D12Resource> pUploadHeap;
    const D3D12_HEAP_PROPERTIES UploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD);
    ThrowIfFail(m_Device.CreateCommittedResource(&UploadHeapProperties, D3D12_HEAP_FLAG_NONE, &BufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(pUploadHeap), &pUploadHeap), L"Failed to upload buffer '%s'.", pszTexName);
    SetResourceNameW(pUploadHeap, pszTexName);

    const D3D12_RANGE ReadRange = {};
    void* pMappedData;
    ThrowIfFail(pUploadHeap->Map(0, &ReadRange, &pMappedData), L"Failed to map upload heap.");

    //Todo: optimize out for converted textures
    for (size_t iMip = 0; iMip < Texture.NumMips; iMip++)
    {
        BYTE* pRowDst = static_cast<BYTE*>(pMappedData) + m_ConvertedTextureData.GetFootprintArray()[iMip].Offset;
        const BYTE* pRowSrc = static_cast<const BYTE*>(m_ConvertedTextureData.GetSubResourceDataArray()[iMip].pData);
        
        for (size_t iRow = 0; iRow < m_ConvertedTextureData.GetFootprintArray()[iMip].Footprint.Height; iRow++)
        {
            memcpy(pRowDst, pRowSrc, m_ConvertedTextureData.GetSubResourceDataArray()[iMip].RowPitch);
            pRowDst += m_ConvertedTextureData.GetFootprintArray()[iMip].Footprint.RowPitch;
            pRowSrc += m_ConvertedTextureData.GetSubResourceDataArray()[iMip].RowPitch;
        }
    }

    pUploadHeap->Unmap(0, nullptr);

    for (size_t iMip = 0; iMip < Texture.NumMips; iMip++)
    {
        D3D12_TEXTURE_COPY_LOCATION LocationSource;
        LocationSource.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        LocationSource.PlacedFootprint = m_ConvertedTextureData.GetFootprintArray()[iMip];
        LocationSource.pResource = pUploadHeap.Get();

        D3D12_TEXTURE_COPY_LOCATION LocationDest;
        LocationDest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        LocationDest.SubresourceIndex = iMip;
        LocationDest.pResource = OutputTexture.pTexture.Get();

        m_CommandList.CopyTextureRegion(&LocationDest, 0, 0, 0, &LocationSource, nullptr);
    }

    const auto SRVBarrier = CD3DX12_RESOURCE_BARRIER::Transition(OutputTexture.pTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    m_CommandList.ResourceBarrier(1, &SRVBarrier);
    pUploadHeap.Detach();


    

    D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc;
    SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    SRVDesc.Format = TextureDesc.Format;
    SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    SRVDesc.Texture2D.MipLevels = Texture.NumMips;
    SRVDesc.Texture2D.MostDetailedMip = 0;
    SRVDesc.Texture2D.PlaneSlice = 0;
    SRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;

    auto Handle = m_SRVDescriptorHeap.GetCPUDescriptorHandleForHeapStart();
    Handle.ptr += m_Device.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * TextureCache::GlobalNumTextures;
    m_Device.CreateShaderResourceView(OutputTexture.pTexture.Get(), &SRVDesc, Handle);


    OutputTexture.fMultU = 1.0f / (Texture.UClamp * Texture.UScale);
    OutputTexture.fMultV = 1.0f / (Texture.VClamp * Texture.VScale);
    
    return OutputTexture;
}

void TextureConverter::Update(const FTextureInfo& Source, TextureData& Dest) const
{
    assert(Source.bRealtimeChanged);

    IFormatConverter* const pConverter = m_FormatConverters[Source.Format];
    assert(pConverter); //Should have a converter as it was converted succesfully before

    pConverter->Convert(Source);

    for (int i = 0; i < Source.NumMips; i++)
    {
//        m_DeviceContext.UpdateSubresource(Dest.pTexture.Get(), i, nullptr, m_ConvertedTextureData.GetSubResourceDataSysMem(i), sizeof(DWORD)*Source.Mips[i]->USize, 0);

        //D3D11_MAPPED_SUBRESOURCE Mapping;
        //m_DeviceContext.Map(Dest.pTexture.Get(), i, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &Mapping);

        //char* pDst = static_cast<char*>(Mapping.pData);
        //for (int y = 0; y < Source.VClamp; y++)
        //{
        //    memcpy(pDst, &Source.Mips[i]->DataPtr[y*sizeof(DWORD)*Source.Mips[i]->USize], sizeof(DWORD)*Source.UClamp);
        //    pDst += Mapping.RowPitch;
        //}

        //m_DeviceContext.Unmap(Dest.pTexture.Get(), i);
    }
}
