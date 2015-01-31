#include "stdafx.h"
#include "TextureConverter.h"
#include "Helpers.h"

void TextureConverter::ConvertedTextureData::Resize(const FTextureInfo& Texture, const IFormatConverter& FormatConverter)
{
    m_SubResourceData.resize(Texture.NumMips);

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
            m_SubResourceData[i].pSysMem = m_Mips[i].data();
        }
        m_SubResourceData[i].SysMemPitch = FormatConverter.GetStride(UnrealMip);
    }
}

void TextureConverter::FormatConverterP8::Convert(const FTextureInfo& Texture)
{
    assert(Texture.Format == ETextureFormat::TEXF_P8);
    assert(Texture.Palette);

    FColor* const Palette = Texture.Palette;

    //Palette color 0 is the alpha mask color, we always give it alpha 0
    Palette[0].A = 0;

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

void TextureConverter::FormatConverterDXT::Convert(const FTextureInfo& Texture)
{
    assert(Texture.Format == ETextureFormat::TEXF_DXT1);

    m_Buffer.Resize(Texture, *this);
    for (INT i = 0; i < Texture.NumMips; i++)
    {
        assert(Texture.Mips[i]);
        const FMipmapBase& UnrealMip = *Texture.Mips[i];
        m_Buffer.GetSubResourceDataMemPtr(i) = UnrealMip.DataPtr;
    }
}


TextureConverter::TextureConverter(ID3D11Device& Device, ID3D11DeviceContext& DeviceContext)
:m_Device(Device)
,m_DeviceContext(DeviceContext)
,m_FormatConverterP8(m_ConvertedTextureData)
,m_FormatConverterDXT(m_ConvertedTextureData)
,m_FormatConverters({
    &m_FormatConverterP8, // TEXF_P8
    nullptr, // TEXF_RGBA7
    nullptr, // TEXF_RGB16
    &m_FormatConverterDXT, // TEXF_DXT1
    nullptr, //TEXF_RGB8
    nullptr //TEXF_RGBA8
        })
{

    //Create placeholder texture
    m_PlaceholderTexture.fMultU = 1.0f;
    m_PlaceholderTexture.fMultV = 1.0f;

    D3D11_TEXTURE2D_DESC TextureDesc;
    TextureDesc.Width = 1;
    TextureDesc.Height = 1;
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    TextureDesc.SampleDesc.Count = 1;
    TextureDesc.SampleDesc.Quality = 0;
    TextureDesc.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
    TextureDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
    TextureDesc.CPUAccessFlags = 0;
    TextureDesc.MiscFlags = 0;

    const uint32_t PlaceholderPixel = 0xff00ffff;
    D3D11_SUBRESOURCE_DATA PlaceHolderData;
    PlaceHolderData.pSysMem = &PlaceholderPixel;
    PlaceHolderData.SysMemPitch = sizeof(uint32_t);

    ThrowIfFail(m_Device.CreateTexture2D(&TextureDesc, &PlaceHolderData, &m_PlaceholderTexture.pTexture), L"Failed to create placeholder texture.");
    SetResourceName(m_PlaceholderTexture.pTexture, "Placeholder texture");

    D3D11_SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDesc;
    ShaderResourceViewDesc.Format = TextureDesc.Format;
    ShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D10_1_SRV_DIMENSION_TEXTURE2D;
    ShaderResourceViewDesc.Texture2D.MipLevels = 1;
    ShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;

    ThrowIfFail(m_Device.CreateShaderResourceView(m_PlaceholderTexture.pTexture.Get(), &ShaderResourceViewDesc, &m_PlaceholderTexture.pShaderResourceView), L"Failed to create placeholder texture SRV.");
    SetResourceName(m_PlaceholderTexture.pShaderResourceView, "Placeholder texture");
}

TextureConverter::TextureData TextureConverter::Convert(const FTextureInfo& Texture)
{
    IFormatConverter* const pConverter = m_FormatConverters[Texture.Format];
    if (pConverter == nullptr)
    {
        return m_PlaceholderTexture;
    }

    pConverter->Convert(Texture);

    TextureData OutputTexture;

    D3D11_TEXTURE2D_DESC TextureDesc;
    TextureDesc.Width = Texture.USize;
    TextureDesc.Height = Texture.VSize;
    TextureDesc.MipLevels = Texture.NumMips;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = pConverter->GetDXGIFormat();
    TextureDesc.SampleDesc.Count = 1;
    TextureDesc.SampleDesc.Quality = 0;
    TextureDesc.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
    TextureDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
    TextureDesc.CPUAccessFlags = 0;
    TextureDesc.MiscFlags = 0;

    ThrowIfFail(m_Device.CreateTexture2D(&TextureDesc, m_ConvertedTextureData.GetSubResourceData(), &OutputTexture.pTexture), L"Failed to create texture '%s'.", Texture.Texture->GetName());
    SetResourceNameW(OutputTexture.pTexture, Texture.Texture->GetName());

    D3D11_SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDesc;
    ShaderResourceViewDesc.Format = TextureDesc.Format;
    ShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D10_1_SRV_DIMENSION_TEXTURE2D;
    ShaderResourceViewDesc.Texture2D.MipLevels = Texture.NumMips;
    ShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;

    ThrowIfFail(m_Device.CreateShaderResourceView(OutputTexture.pTexture.Get(), &ShaderResourceViewDesc, &OutputTexture.pShaderResourceView), L"Failed to create SRV for '%s'.", Texture.Texture->GetName());
    SetResourceNameW(OutputTexture.pShaderResourceView, Texture.Texture->GetName());

    return OutputTexture;
}
