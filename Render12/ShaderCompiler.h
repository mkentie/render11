#pragma once
#include "Helpers.h"

class ShaderCompiler
{
public:
    class CompiledShader
    {
    public:
        explicit CompiledShader()
        {

        }

        explicit CompiledShader(ID3DBlob* const pBlob)
        :m_pBlob(pBlob)
        {

        }

        D3D12_SHADER_BYTECODE GetByteCode() const
        {
            return CD3DX12_SHADER_BYTECODE(m_pBlob.Get());
        }

        int GetResourceSlot(const char* const pszName)
        {
            assert(m_pBlob);

            if (!m_pReflection) //Create reflection on first use
            {
                ThrowIfFail(D3DReflect(m_pBlob->GetBufferPointer(), m_pBlob->GetBufferSize(), __uuidof(m_pReflection), &m_pReflection), L"Failed to create shader reflection instance.");
            }

            D3D12_SHADER_INPUT_BIND_DESC Desc;
            if (FAILED(m_pReflection->GetResourceBindingDescByName(pszName, &Desc)))
            {
                return -1;
            }

            return Desc.BindPoint;
        }

    private:
        ComPtr<ID3DBlob> m_pBlob;
        ComPtr<ID3D12ShaderReflection> m_pReflection;
    };

    explicit ShaderCompiler(const wchar_t* const pszFileName);
    ShaderCompiler(const ShaderCompiler&) = delete;
    ShaderCompiler& operator=(const ShaderCompiler&) = delete;

    CompiledShader CompileVertexShader();
    CompiledShader CompileGeometryShader();
    CompiledShader CompilePixelShader();

protected:
    CompiledShader CompileShader(const char* const pszEntryPoint, const char* const pszTarget);

    const wchar_t* const m_pszFileName;
};

