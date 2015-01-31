class TileRenderer
{
public:
    struct Tile
    {
        DirectX::XMFLOAT4 XYPos;
        DirectX::XMFLOAT4 TexCoord;
        float Z;
    };

    explicit TileRenderer(ID3D11Device& Device, ID3D11DeviceContext& DeviceContext);
    TileRenderer(const TileRenderer&) = delete;
    TileRenderer& operator=(const TileRenderer&) = delete;

    bool Init();
    
    void Reset(){ m_iIndex = 0; }
    void Map();
    void Unmap();
    bool IsMapped() const { return m_Mapping.pData != nullptr; }

    void Draw();

    Tile& GetTile();

protected:
    static const size_t sm_iBufferSize = 1000;
    static const D3D11_PRIMITIVE_TOPOLOGY sm_PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_POINTLIST;


    ID3D11Device& m_Device;
    ID3D11DeviceContext& m_DeviceContext;

    ComPtr<ID3D11InputLayout> m_pInputLayout;
    ComPtr<ID3D11VertexShader> m_pVertexShader;
    ComPtr<ID3D11GeometryShader> m_pGeometryShader;
    ComPtr<ID3D11PixelShader> m_pPixelShader;

    ComPtr<ID3D11Buffer> m_pInstanceBuffer;

    D3D11_MAPPED_SUBRESOURCE m_Mapping;
    size_t m_iIndex;

};