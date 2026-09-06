#include "RenderPch.h"
#include "VertexShader.h"

namespace Render
{
    bool VertexShader::LoadFromFile(const std::wstring& path, const std::string& entry, const std::string& profile)
    {
        UINT Flag = 0;

#ifdef _DEBUG
        Flag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        Flag = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

        if (FAILED(D3DCompileFromFile(
            path.c_str(),
            nullptr,
            nullptr,
            entry.c_str(),
            profile.c_str(),
            Flag,
            0,
            m_shaderBlob.GetAddressOf(),
            m_errorBlob.GetAddressOf()
        )))
        {
            return false;
        }

        if (FAILED(D3DReflect(
            m_shaderBlob->GetBufferPointer(),
            m_shaderBlob->GetBufferSize(),
            IID_ID3D11ShaderReflection, (void**)m_reflection.GetAddressOf())))
        {
#ifdef _DEBUG
            OutputDebugStringA("VertexShader Reflect Failed");
#endif
            return false;
        }

        if (!ReflectShader())
            return false;

        return true;
    }

    bool VertexShader::CreateFromShaderBlob(ID3D11Device* pDevice)
    {
        if (nullptr == m_shaderBlob)
        {
#ifdef _DEBUG
            OutputDebugStringA("Invalid ShaderBlob");
#endif
            return false;
        }

        if (nullptr == m_reflection)
        {
#ifdef _DEBUG
            OutputDebugStringA("Invalid Reflection");
#endif
            return false;
        }

        if (FAILED(pDevice->CreateVertexShader(
            m_shaderBlob->GetBufferPointer(),
            m_shaderBlob->GetBufferSize(),
            nullptr,
            m_vertexShader.GetAddressOf()
        )))
        {
#ifdef _DEBUG
            OutputDebugStringA("CreateVertexShader Failed");
#endif
            return false;
        }

        // blob/reflection은 리셋하지 않음 - ShaderGroup이 InputLayout 생성 시 필요
        m_errorBlob.Reset();

        m_isCreated = true;

        return true;
    }

    void VertexShader::Bind(ID3D11DeviceContext* ctx)
    {
        ctx->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    }

}
