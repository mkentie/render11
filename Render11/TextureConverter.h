#pragma once

class TextureConverter
{
public:
    struct TextureData
    {
        TextureData(){}
        TextureData(TextureData&& Other) //VS2013 doesn't support default move constructors
            :fMultU(Other.fMultU)
            ,fMultV(Other.fMultV)
            ,pTexture(std::move(Other.pTexture))
            ,pShaderResourceView(std::move(Other.pShaderResourceView))
        {

        }
        float fMultU;
        float fMultV;
        ComPtr<ID3D11Texture2D> pTexture;
        ComPtr<ID3D11ShaderResourceView> pShaderResourceView;
    };

    explicit TextureConverter(ID3D11Device& Device, ID3D11DeviceContext& DeviceContext);
    TextureConverter(const TextureConverter&) = delete;
    TextureConverter& operator=(const TextureConverter&) = delete;

    TextureData Convert(const FTextureInfo& Texture);

protected:
    ID3D11Device& m_Device;
    ID3D11DeviceContext& m_DeviceContext;

    class IFormatConverter;
    /**
    Scratch data buffer, used to initialize D3D textures.
    */
    class ConvertedTextureData
    {
    public:
        typedef uint32_t PixelFormat;
        void Resize(const FTextureInfo& Texture, const IFormatConverter& FormatConverter);

        PixelFormat* GetMipBuffer(const unsigned int iMipLevel) { return m_Mips[iMipLevel].data(); }
        const D3D11_SUBRESOURCE_DATA* GetSubResourceData() const { return m_SubResourceData.data(); }
        const void*& GetSubResourceDataMemPtr(const unsigned int iMipLevel) { return m_SubResourceData[iMipLevel].pSysMem; }

    private:
        std::vector<std::vector<PixelFormat>> m_Mips;
        std::vector<D3D11_SUBRESOURCE_DATA> m_SubResourceData;
    };
    ConvertedTextureData m_ConvertedTextureData;

    /**
    Interface for format converters
    */
    class IFormatConverter
    {
    public:
        IFormatConverter& operator=(const IFormatConverter&) = delete;
        virtual ~IFormatConverter() {};
        virtual void Convert(const FTextureInfo& Texture) = 0;
        virtual UINT GetStride(const FMipmapBase& Mip) const = 0;
        virtual DXGI_FORMAT GetDXGIFormat() const = 0;
        virtual bool WantsBuffer() const = 0;
    };

    class FormatConverterP8 : public IFormatConverter
    {
    public:
        FormatConverterP8(ConvertedTextureData& Buffer) : m_Buffer(Buffer) {};
        virtual void Convert(const FTextureInfo& Texture) override;
        virtual UINT GetStride(const FMipmapBase& Mip) const override { return Mip.USize*sizeof(ConvertedTextureData::PixelFormat); }
        virtual DXGI_FORMAT GetDXGIFormat() const override { return DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM; }
        virtual bool WantsBuffer() const override { return true;  }
    private:
        ConvertedTextureData& m_Buffer;
    };

    class FormatConverterDXT : public IFormatConverter
    {
    public:
        FormatConverterDXT(ConvertedTextureData& Buffer) : m_Buffer(Buffer) {};
        virtual void Convert(const FTextureInfo& Texture) override;
        virtual UINT GetStride(const FMipmapBase& Mip) const override { return (Mip.USize + sm_iBlockSizeInPixels - 1) / sm_iBlockSizeInPixels / sm_iBlockSizeInBytes; }
        virtual DXGI_FORMAT GetDXGIFormat() const override { return DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM; }
        virtual bool WantsBuffer() const override { return false; }
    private:
        ConvertedTextureData& m_Buffer;
        static const size_t sm_iBlockSizeInPixels = 4;
        static const size_t sm_iBlockSizeInBytes = 8;
    };

    FormatConverterP8 m_FormatConverterP8;
    FormatConverterDXT m_FormatConverterDXT;
    std::array<IFormatConverter*, 6> const m_FormatConverters;

    TextureData m_PlaceholderTexture; //Placeholder texture for when unable to convert
};