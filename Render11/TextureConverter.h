#pragma once

class TextureConverter
{
public:
    struct TextureData
    {
        TextureData(){}

        TextureData(const TextureData&) = default;
        TextureData(TextureData&& Other) //VS2015 doesn't support default move constructors
            :fMultU(Other.fMultU)
            ,fMultV(Other.fMultV)
            ,pTexture(std::move(Other.pTexture))
            ,pShaderResourceView(std::move(Other.pShaderResourceView))
        {

        }
        TextureData& operator=(const TextureData&) = delete;
        /*TextureData& operator=(TextureData&& Other)
        {
            fMultU = Other.fMultU;
            fMultV = Other.fMultV;
            pTexture = std::move(Other.pTexture);
            pShaderResourceView = std::move(Other.pShaderResourceView);

            return *this;
        }*/

        float fMultU;
        float fMultV;

        ComPtr<ID3D11Texture2D> pTexture;
        ComPtr<ID3D11ShaderResourceView> pShaderResourceView;
    };

    explicit TextureConverter(ID3D11Device& Device, ID3D11DeviceContext& DeviceContext);
    TextureConverter(const TextureConverter&) = delete;
    TextureConverter& operator=(const TextureConverter&) = delete;

    TextureData Convert(const FTextureInfo& Texture) const;
    void Update(const FTextureInfo& Source, TextureData& Dest) const;

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
        const D3D11_SUBRESOURCE_DATA* GetSubResourceDataArray() const { return m_SubResourceData.data(); }
        const void* GetSubResourceDataSysMem(const unsigned int iMipLevel) const { return m_SubResourceData[iMipLevel].pSysMem; }
		void SetSubResourceDataSysMem(const unsigned int iMipLevel, void* const p) { m_SubResourceData[iMipLevel].pSysMem = p; }

    private:
        std::vector<std::vector<PixelFormat>> m_Mips; //Scratch data, not required for all conversions
        std::vector<D3D11_SUBRESOURCE_DATA> m_SubResourceData; //References to Unreal data or converted scratch data
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
        virtual bool WantsBuffer() const = 0; //Whether converter requires scratch space for conversion result
    };

    class FormatConverterIdentity : public IFormatConverter
    {
    public:
        explicit FormatConverterIdentity(ConvertedTextureData& Buffer) : m_Buffer(Buffer) {};
        virtual void Convert(const FTextureInfo& Texture) override;
        virtual UINT GetStride(const FMipmapBase& Mip) const override { return Mip.USize*sizeof(ConvertedTextureData::PixelFormat); }
        virtual DXGI_FORMAT GetDXGIFormat() const override { return DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM; }
        virtual bool WantsBuffer() const override { return false; }
    private:
        ConvertedTextureData& m_Buffer;
    };

    class FormatConverterP8 : public IFormatConverter
    {
    public:
        explicit FormatConverterP8(ConvertedTextureData& Buffer) : m_Buffer(Buffer) {};
        virtual void Convert(const FTextureInfo& Texture) override;
        virtual UINT GetStride(const FMipmapBase& Mip) const override { return Mip.USize*sizeof(ConvertedTextureData::PixelFormat); }
        virtual DXGI_FORMAT GetDXGIFormat() const override { return DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM; }
        virtual bool WantsBuffer() const override { return true;  }
    private:
        ConvertedTextureData& m_Buffer;
    };

    class FormatConverterDXT : public FormatConverterIdentity
    {
    public:
        using FormatConverterIdentity::FormatConverterIdentity;
        virtual UINT GetStride(const FMipmapBase& Mip) const override { return (Mip.USize + sm_iBlockSizeInPixels - 1) / sm_iBlockSizeInPixels / sm_iBlockSizeInBytes; }
        virtual DXGI_FORMAT GetDXGIFormat() const override { return DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM; }
    private:
        static const size_t sm_iBlockSizeInPixels = 4;
        static const size_t sm_iBlockSizeInBytes = 8;
    };

    FormatConverterIdentity m_FormatConverterIdentity = FormatConverterIdentity(m_ConvertedTextureData);
    FormatConverterP8 m_FormatConverterP8 = FormatConverterP8(m_ConvertedTextureData);
    FormatConverterDXT m_FormatConverterDXT = FormatConverterDXT(m_ConvertedTextureData);
    std::array<IFormatConverter*, 6> const m_FormatConverters = {
        &m_FormatConverterP8, // TEXF_P8
        &m_FormatConverterIdentity, // TEXF_RGBA7
        nullptr, // TEXF_RGB16
        &m_FormatConverterDXT, // TEXF_DXT1
        nullptr, //TEXF_RGB8
        nullptr //TEXF_RGBA8
    };

    TextureData m_PlaceholderTexture; //Placeholder texture for when unable to convert
};