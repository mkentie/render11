#include "stdafx.h"
#include "TextureCache.h"
#include "Helpers.h"


TextureCache::TextureCache(ID3D11Device& Device, ID3D11DeviceContext& DeviceContext)
:m_DeviceContext(DeviceContext)
,m_TextureConverter(Device, DeviceContext)
{
    ResetDirtySlots();
}

const TextureConverter::TextureData* TextureCache::FindOrInsert(const FTextureInfo& Texture)
{
    const auto it = m_Textures.find(Texture.CacheID);
    if (it != m_Textures.cend())
    {
        return &it->second;
    }

    TextureConverter::TextureData NewData = m_TextureConverter.Convert(Texture);
    const TextureConverter::TextureData* const p = &m_Textures.insert(std::make_pair(Texture.CacheID, std::move(NewData))).first->second;
    assert(p);

    return p;
}


const TextureConverter::TextureData* TextureCache::FindOrInsertAndPrepare(const FTextureInfo& Texture, const size_t iSlot)
{
    const TextureConverter::TextureData* const pData = FindOrInsert(Texture);
    if (!pData)
    {
        return nullptr;
    }

    m_iDirtyBeginSlot = std::min(m_iDirtyBeginSlot, iSlot);
    m_iDirtyEndSlot = std::max(m_iDirtyEndSlot, iSlot);
    m_PreparedSRVs[iSlot] = pData->pShaderResourceView.Get();
    m_PreparedIds[iSlot] = Texture.CacheID;

    return pData;
}

void TextureCache::BindTextures()
{
    if (m_iDirtyBeginSlot > m_iDirtyEndSlot) //Anything prepared?
    {
        return;
    }
    
    m_DeviceContext.PSSetShaderResources(m_iDirtyBeginSlot, m_iDirtyEndSlot - m_iDirtyBeginSlot + 1, &m_PreparedSRVs[m_iDirtyBeginSlot]);

    ResetDirtySlots();
}

void TextureCache::Flush()
{
    m_DeviceContext.PSSetShaderResources(0, sm_iMaxSlots, nullptr); //To be able to release textures
    m_Textures.clear();

    ResetDirtySlots();
}

void TextureCache::ResetDirtySlots()
{
    m_iDirtyBeginSlot = m_PreparedIds.size() - 1;
    m_iDirtyEndSlot = 0;
}

void TextureCache::PrintSizeHistogram() const
{
    for (const auto& t : m_Textures)
    {
        D3D11_TEXTURE2D_DESC Desc;
        t.second.pTexture->GetDesc(&Desc);
        LOGMESSAGEF(L"%u x %u", Desc.Width, Desc.Height);
    }
}

