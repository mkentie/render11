#include "stdafx.h"
#include "ShaderCompiler.h"
#include "Helpers.h"

ShaderCompiler::ShaderCompiler(ID3D11Device& Device, const wchar_t* const pszFileName)
:m_Device(Device)
,m_pszFileName(pszFileName)
{

}

void ShaderCompiler::CompileShader(const char* const pszEntryPoint, const char* const pszTarget)
{
    LOGMESSAGEF(L"Compiling \"%s\" for target %S.", m_pszFileName, pszTarget);

    UINT iFlags = 0;
#ifdef _DEBUG
    iFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> pMessages;

    const HRESULT hResult = D3DCompileFromFile(m_pszFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, pszEntryPoint, pszTarget, iFlags, 0, &m_pBlob, &pMessages);

    if (pMessages)
    {
        LOGWARNINGF(L"Shader compilation:\r\n%S", static_cast<char*>(pMessages->GetBufferPointer()));
        OutputDebugStringA(static_cast<char*>(pMessages->GetBufferPointer()));
        DebugBreak();
    }

    ThrowIfFail(hResult, L"Failed to compile shader '%s'.", m_pszFileName);
}

template<class T, ShaderCompiler::ShaderCreationFunc Func>
ComPtr<T> ShaderCompiler::CompileXShader(const char* const pszEntryPoint, const char* const pszTarget)
{
    CompileShader(pszEntryPoint, pszTarget);

    ComPtr<T> pShader;
    ThrowIfFail((m_Device.*Func)(m_pBlob->GetBufferPointer(), m_pBlob->GetBufferSize(), nullptr, &pShader), L"Failed to create shader '%s'.", m_pszFileName);
    SetResourceNameW(pShader, m_pszFileName);

    return pShader;
}

ComPtr<ID3D11VertexShader> ShaderCompiler::CompileVertexShader()
{
    return CompileXShader<ID3D11VertexShader, &ID3D11Device::CreateVertexShader>("VSMain", "vs_4_0");
}

ComPtr<ID3D11GeometryShader> ShaderCompiler::CompileGeometryShader()
{
    return CompileXShader<ID3D11GeometryShader, &ID3D11Device::CreateGeometryShader>("GSMain", "gs_4_0");
}

ComPtr<ID3D11PixelShader> ShaderCompiler::CompilePixelShader()
{
    return CompileXShader<ID3D11PixelShader, &ID3D11Device::CreatePixelShader>("PSMain", "ps_4_0");
}

int ShaderCompiler::GetResourceSlot(const char* const pszName)
{
    assert(m_pBlob);

    if (!m_pReflection) //Create reflection on first use
    {
        ThrowIfFail(D3D11Reflect(m_pBlob->GetBufferPointer(), m_pBlob->GetBufferSize(), &m_pReflection), L"Failed to create shader reflection instance.");
    }

    D3D11_SHADER_INPUT_BIND_DESC Desc;
    if (FAILED(m_pReflection->GetResourceBindingDescByName(pszName, &Desc)))
    {
        return -1;
    }

    return Desc.BindPoint;
}

ComPtr<ID3D11InputLayout> ShaderCompiler::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* InputElementDescs, const UINT NumElements)
{
    assert(m_pBlob);

    ComPtr<ID3D11InputLayout> pInputLayout;

    ThrowIfFail(m_Device.CreateInputLayout(InputElementDescs, NumElements, m_pBlob->GetBufferPointer(), m_pBlob->GetBufferSize(), &pInputLayout), L"Failed to create input layout from '%s'.", m_pszFileName);
    SetResourceNameW(pInputLayout, m_pszFileName);

    return pInputLayout;
}
