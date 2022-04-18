#pragma once

class ShaderCompiler
{
public:
    explicit ShaderCompiler(ID3D11Device& Device, const wchar_t* const pszFileName);
    ShaderCompiler(const ShaderCompiler&) = delete;
    ShaderCompiler& operator=(const ShaderCompiler&) = delete;

    ComPtr<ID3D11VertexShader> CompileVertexShader();
    ComPtr<ID3D11GeometryShader> CompileGeometryShader();
    ComPtr<ID3D11PixelShader> CompilePixelShader();

    int GetResourceSlot(const char* const pszName);
    ComPtr<ID3D11InputLayout> CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* InputElementDescs, const UINT NumElements);
protected:
    //Prototype for the various ID3D11Device::Create[Vertex/Pixel/Etc.]Shader() functions.
    template<class T, class Func>
    ComPtr<T> CompileXShader(const char* const pszEntryPoint, const char* const pszTarget, const Func& CreationFunc);

    void ShaderCompiler::CompileShader(const char* const pszEntryPoint, const char* const pszTarget);

    ID3D11Device& m_Device;
    const wchar_t* const m_pszFileName;

    ComPtr<ID3DBlob> m_pBlob;
    ComPtr<ID3D11ShaderReflection> m_pReflection;
};

