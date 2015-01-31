#pragma once
#include "TextureConverter.h"

class TextureCache
{
public:
    static const unsigned int sm_iMaxSlots = 1; //Maximum texture slot managed by the cache

    explicit TextureCache(ID3D11Device& Device, ID3D11DeviceContext& DeviceContext);
    TextureCache(const TextureCache&) = delete;
    TextureCache& operator=(const TextureCache&) = delete;

    const TextureConverter::TextureData* FindOrInsert(const FTextureInfo& Texture);
    const TextureConverter::TextureData* FindOrInsertAndPrepare(const FTextureInfo& Texture, const unsigned int iSlot);

    /**
    Instead of checking what's actually bound, for our purposes it's enough to just check if someone else WANTED to bind something else.
    However this means that preparing a new texture and then not using it to render will result in a false positive for having to flush geometry.
    */
    bool IsPrepared(const FTextureInfo& Texture, const unsigned int iSlot) const { return m_iDirtyBeginSlot <= iSlot && m_iDirtyEndSlot >= iSlot && m_PreparedIds[iSlot] == Texture.CacheID; }

    void BindTextures();
    void Flush();

    size_t GetNumTextures() const { return m_Textures.size(); }
    void PrintSizeHistogram() const;

protected:
    void ResetDirtySlots();

    ID3D11DeviceContext& m_DeviceContext;

    TextureConverter m_TextureConverter;
    std::unordered_map<long long, const TextureConverter::TextureData> m_Textures;

    std::array<decltype(FTextureInfo::CacheID), sm_iMaxSlots> m_PreparedIds;
    std::array<ID3D11ShaderResourceView*, sm_iMaxSlots> m_PreparedSRVs;

    //Tracking of which slots need to be bound on BindTextures()
    unsigned int m_iDirtyBeginSlot;
    unsigned int m_iDirtyEndSlot;
};

