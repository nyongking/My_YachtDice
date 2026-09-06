#include "RenderPch.h"
#include "PixelShader.h"

namespace Render
{
    bool PixelShader::LoadFromFile(const std::wstring& path, const std::string& entry, const std::string& profile)
    {
        UINT Flag = 0;

#ifdef _DEBUG
        Flag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        Flag = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
        // compile
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

        // reflection
        if (FAILED(D3DReflect(
            m_shaderBlob->GetBufferPointer(),
            m_shaderBlob->GetBufferSize(),
            IID_ID3D11ShaderReflection, (void**)m_reflection.GetAddressOf())))
        {
#ifdef _DEBUG
            OutputDebugStringA("PixelShader Reflect Failed");
#endif
            return false;
        }

        // super::
        if (!ReflectShader())
        {
            return false;
        }

        return true;
    }

    bool PixelShader::CreateFromShaderBlob(ID3D11Device* pDevice)
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

        // pixel shader
        if (FAILED(pDevice->CreatePixelShader(
            m_shaderBlob->GetBufferPointer(),
            m_shaderBlob->GetBufferSize(),
            nullptr,
            m_pixelShader.GetAddressOf()
        )))
        {
#ifdef _DEBUG
            OutputDebugStringA("CreatePixelShader Failed");
#endif
            return false;
        }

        m_shaderBlob.Reset();
        m_errorBlob.Reset();
        m_reflection.Reset();

        m_isCreated = true;

        return true;
    }

    void PixelShader::Bind(ID3D11DeviceContext* ctx)
    {
        ctx->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    }

}


