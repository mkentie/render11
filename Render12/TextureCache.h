#pragma once
#include "TextureConverter.h"

class TextureCache
{
public:
    static const unsigned int sm_iMaxSlots = 2; //Maximum texture slot managed by the cache

    explicit TextureCache(ID3D12Device& Device, ID3D12GraphicsCommandList& CommandList);
    TextureCache(const TextureCache&) = delete;
    TextureCache& operator=(const TextureCache&) = delete;

    const TextureConverter::TextureData& FindOrInsert(FTextureInfo& Texture);
    const TextureConverter::TextureData& FindOrInsertAndPrepare(FTextureInfo& Texture, const unsigned int iSlot);

    /**
    Instead of checking what's actually bound, for our purposes it's enough to just check if someone else WANTED to bind something else.
    However this means that preparing a new texture and then not using it to render will result in a false positive for having to flush geometry.
    */
    bool IsPrepared(const FTextureInfo& Texture, const unsigned int iSlot) const { return m_iDirtyBeginSlot <= iSlot && m_iDirtyEndSlot >= iSlot && m_PreparedIds[iSlot] == Texture.CacheID; }

    void BindTextures();
    void Flush();

    size_t GetNumTextures() const { return m_Textures.size(); }

    void PrintSizeHistogram(UCanvas& c) const;

    static unsigned int GlobalNumTextures;

protected:
    void ResetDirtySlots();

    ID3D12Device& m_Device;
    ID3D12GraphicsCommandList& m_CommandList;

    ComPtr<ID3D12DescriptorHeap> m_pSRVDescriptorHeap;
    std::unique_ptr<TextureConverter> m_pTextureConverter; //pointer because it can only be created once srv descriptor heap exists -> TODO remove reliance on heap
    std::unordered_map<long long, TextureConverter::TextureData> m_Textures;

    

    std::array<decltype(FTextureInfo::CacheID), sm_iMaxSlots> m_PreparedIds;
    std::array<ID3D11ShaderResourceView*, sm_iMaxSlots> m_PreparedSRVs;

    //Tracking of which slots need to be bound on BindTextures()
    unsigned int m_iDirtyBeginSlot;
    unsigned int m_iDirtyEndSlot;
};

