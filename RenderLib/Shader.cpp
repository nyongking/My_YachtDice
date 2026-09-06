#include "RenderPch.h"
#include "Shader.h"

namespace Render
{
	bool Shader::ReflectShader()
	{
		if (nullptr == m_reflection)
			return false;

		D3D11_SHADER_DESC shaderDesc;
		m_reflection->GetDesc(&shaderDesc);

        // cbuffers,
        for (UINT i = 0; i < shaderDesc.ConstantBuffers; ++i) 
        {
            ID3D11ShaderReflectionConstantBuffer* pCB = m_reflection->GetConstantBufferByIndex(i);
            D3D11_SHADER_BUFFER_DESC cbDesc;
            pCB->GetDesc(&cbDesc);

            ConstantBufferInfo info;
            info.name = cbDesc.Name;
            info.size = cbDesc.Size;

            for (UINT j = 0; j < cbDesc.Variables; ++j) 
            {
                ID3D11ShaderReflectionVariable* pVar = pCB->GetVariableByIndex(j);
                D3D11_SHADER_VARIABLE_DESC varDesc;
                pVar->GetDesc(&varDesc);

                info.variables.push_back({ varDesc.Name, varDesc.StartOffset, varDesc.Size });
            }

            m_cbuffers.push_back(info);
        }

        // resources (all type)
        for (UINT i = 0; i < shaderDesc.BoundResources; ++i) 
        {
            D3D11_SHADER_INPUT_BIND_DESC resDesc;
            m_reflection->GetResourceBindingDesc(i, &resDesc);

            ResourceBindInfo resInfo;
            resInfo.name = resDesc.Name;
            resInfo.slot = resDesc.BindPoint; // ��: t0���� 0
            
            int index = -1;

            switch (resDesc.Type)
            {
            case D3D_SIT_CBUFFER:
                index = SHADER_INPUT_TYPE::CBUFFER;
                break;
            case D3D_SIT_TEXTURE:
                index = SHADER_INPUT_TYPE::TEXTURE;
                break;
            case D3D_SIT_SAMPLER:
                index = SHADER_INPUT_TYPE::SAMPLER;
                break;
            case D3D_SIT_STRUCTURED:
                index = SHADER_INPUT_TYPE::STRUCTURED_BUFFER;
                break;
            case D3D_SIT_UAV_RWSTRUCTURED:
                index = SHADER_INPUT_TYPE::UAV_BUFFER;
                break;
            }

            if (0 <= index)
                m_resources[index].push_back(resInfo);
        }

		return true;
	}

    int Shader::ConstantBufferSlot(const std::string& name, uint32_t size)
    {
        // m_cbuffers has size info, m_resources[CBUFFER] has slot info
        // cross-reference by name
        for (auto& cbInfo : m_cbuffers)
        {
            if (cbInfo.size != size || 0 != strcmp(cbInfo.name.c_str(), name.c_str()))
                continue;

            for (auto& res : m_resources[CBUFFER])
            {
                if (0 == strcmp(res.name.c_str(), name.c_str()))
                    return static_cast<int>(res.slot);
            }
        }

        return -1;
    }

    int Shader::TextureSlot(const std::string& name) const
    {
        for (auto& res : m_resources[TEXTURE])
        {
            if (res.name == name)
                return static_cast<int>(res.slot);
        }
        return -1;
    }
}