#include "stdafx.h"
#include "TextureCache.h"
#include "Helpers.h"

unsigned int TextureCache::GlobalNumTextures;


TextureCache::TextureCache(ID3D12Device& Device, ID3D12GraphicsCommandList& CommandList)
:m_Device(Device)
,m_CommandList(CommandList)
{
    ResetDirtySlots();

    D3D12_DESCRIPTOR_HEAP_DESC SRVHeapDesc = {};
    SRVHeapDesc.NumDescriptors = 8; //TODO how many?
    SRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    SRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFail(m_Device.CreateDescriptorHeap(&SRVHeapDesc, __uuidof(m_pSRVDescriptorHeap), &m_pSRVDescriptorHeap), L"Failed to create SRV descriptor heap.");


    m_pTextureConverter = std::make_unique<TextureConverter>(Device, CommandList, *m_pSRVDescriptorHeap.Get());
}

const TextureConverter::TextureData& TextureCache::FindOrInsert(FTextureInfo& Texture)
{
    auto it = m_Textures.find(Texture.CacheID);
    if (it != m_Textures.end())
    {
        if (Texture.bRealtimeChanged)
        {
            m_pTextureConverter->Update(Texture, it->second);
            Texture.bRealtimeChanged = 0; //Clear this flag (from other renderers)
        }

        return it->second;
    }

    TextureConverter::TextureData NewData = m_pTextureConverter->Convert(Texture);
    const TextureConverter::TextureData& Data = m_Textures.emplace(Texture.CacheID, std::move(NewData)).first->second;

    return Data;
}


const TextureConverter::TextureData& TextureCache::FindOrInsertAndPrepare(FTextureInfo& Texture, const size_t iSlot)
{
    const TextureConverter::TextureData& Data = FindOrInsert(Texture);

    m_iDirtyBeginSlot = std::min(m_iDirtyBeginSlot, iSlot);
    m_iDirtyEndSlot = std::max(m_iDirtyEndSlot, iSlot);
    m_PreparedSRVs[iSlot] = Data.pShaderResourceView.Get();
    m_PreparedIds[iSlot] = Texture.CacheID;

    return Data;
}

void TextureCache::BindTextures()
{
    if (m_iDirtyBeginSlot > m_iDirtyEndSlot) //Anything prepared?
    {
        return;
    }
    
    //m_DeviceContext.PSSetShaderResources(m_iDirtyBeginSlot, m_iDirtyEndSlot - m_iDirtyBeginSlot + 1, &m_PreparedSRVs[m_iDirtyBeginSlot]);


    ID3D12DescriptorHeap* ppHeaps[] = { m_pSRVDescriptorHeap.Get() };
    m_CommandList.SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);



    m_CommandList.SetGraphicsRootDescriptorTable(1, m_pSRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

    ResetDirtySlots();
}

void TextureCache::Flush()
{
	ID3D11ShaderResourceView* pEmpty[sm_iMaxSlots] = {};
//    m_DeviceContext.PSSetShaderResources(0, sm_iMaxSlots, pEmpty); //To be able to release textures
    m_Textures.clear();

    ResetDirtySlots();
}

void TextureCache::ResetDirtySlots()
{
    m_iDirtyBeginSlot = m_PreparedIds.size() - 1;
    m_iDirtyEndSlot = 0;
}

void TextureCache::PrintSizeHistogram(UCanvas& c) const
{
   /* typedef decltype(D3D11_TEXTURE2D_DESC::Width) st;
    std::map<st, std::map<st, size_t>> Histogram;
    for (const auto& t : m_Textures)
    {
        D3D11_TEXTURE2D_DESC Desc;
        t.second.pTexture->GetDesc(&Desc);

        auto it = Histogram.emplace(Desc.Width, std::map<st, size_t>()).first;
        auto it2 = it->second.emplace(Desc.Height, 0).first;
        it2->second++;
    }
    for (const auto& Width : Histogram)
    {
        for (const auto& Height : Width.second)
        {
            c.WrappedPrintf(c.SmallFont, 0, L"%u x %u : %Iu", Width.first, Height.first, Height.second);
        }
    }*/
}
