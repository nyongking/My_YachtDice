#include "RenderPch.h"
#include "ShaderGroup.h"

#include "VertexShader.h"
#include "PixelShader.h"

namespace Render
{
	static DXGI_FORMAT DetermineFormat(D3D_REGISTER_COMPONENT_TYPE componentType, BYTE mask)
	{
		int componentCount = 0;
		if (mask & 1) ++componentCount;
		if (mask & 2) ++componentCount;
		if (mask & 4) ++componentCount;
		if (mask & 8) ++componentCount;

		if (componentType == D3D_REGISTER_COMPONENT_UINT32)
		{
			if (1 == componentCount) return DXGI_FORMAT_R32_UINT;
			if (2 == componentCount) return DXGI_FORMAT_R32G32_UINT;
			if (3 == componentCount) return DXGI_FORMAT_R32G32B32_UINT;
			if (4 == componentCount) return DXGI_FORMAT_R32G32B32A32_UINT;
		}
		else if (componentType == D3D_REGISTER_COMPONENT_SINT32)
		{
			if (1 == componentCount) return DXGI_FORMAT_R32_SINT;
			if (2 == componentCount) return DXGI_FORMAT_R32G32_SINT;
			if (3 == componentCount) return DXGI_FORMAT_R32G32B32_SINT;
			if (4 == componentCount) return DXGI_FORMAT_R32G32B32A32_SINT;
		}
		else if (componentType == D3D_REGISTER_COMPONENT_FLOAT32)
		{
			if (1 == componentCount) return DXGI_FORMAT_R32_FLOAT;
			if (2 == componentCount) return DXGI_FORMAT_R32G32_FLOAT;
			if (3 == componentCount) return DXGI_FORMAT_R32G32B32_FLOAT;
			if (4 == componentCount) return DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		return DXGI_FORMAT_UNKNOWN;
	}

	bool ShaderGroup::Initialize(VertexShader* vs,
		PixelShader* ps,
		ID3D11Device* device)
	{
		if (nullptr == vs || nullptr == ps || nullptr == device)
			return false;

		m_vertexShader = vs;
		m_pixelShader  = ps;

		// VS blob과 reflection으로 InputLayout 자동 생성 (Stage 2)
		ID3DBlob*               blob       = vs->GetBlob();
		ID3D11ShaderReflection* reflection = vs->GetReflection();

		if (nullptr == blob || nullptr == reflection)
		{
#ifdef _DEBUG
			OutputDebugStringA("ShaderGroup::Initialize - VS blob/reflection unavailable");
#endif
			return false;
		}

		D3D11_SHADER_DESC shaderDesc;
		reflection->GetDesc(&shaderDesc);

		std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDescs;
		inputElementDescs.reserve(shaderDesc.InputParameters);

		for (UINT i = 0; i < shaderDesc.InputParameters; ++i)
		{
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
			reflection->GetInputParameterDesc(i, &paramDesc);

			if (0 == _stricmp(paramDesc.SemanticName, "SV_InstanceID"))
				continue;

			D3D11_INPUT_ELEMENT_DESC elementDesc;
			elementDesc.SemanticName         = paramDesc.SemanticName;
			elementDesc.SemanticIndex        = paramDesc.SemanticIndex;
			elementDesc.AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
			elementDesc.Format               = DetermineFormat(paramDesc.ComponentType, paramDesc.Mask);

			if (0 == strncmp(paramDesc.SemanticName, "INST_", 5))
			{
				elementDesc.InputSlot             = 1;
				elementDesc.InputSlotClass        = D3D11_INPUT_PER_INSTANCE_DATA;
				elementDesc.InstanceDataStepRate  = 1;
			}
			else
			{
				elementDesc.InputSlot             = 0;
				elementDesc.InputSlotClass        = D3D11_INPUT_PER_VERTEX_DATA;
				elementDesc.InstanceDataStepRate  = 0;
			}

			inputElementDescs.push_back(elementDesc);
		}

		// 입력 파라미터가 없는 VS (SV_VertexID 전용, fullscreen 등)는 InputLayout 불필요
	if (inputElementDescs.empty())
		return true;

	if (FAILED(device->CreateInputLayout(
			inputElementDescs.data(),
			static_cast<UINT>(inputElementDescs.size()),
			blob->GetBufferPointer(),
			blob->GetBufferSize(),
			m_inputLayout.GetAddressOf())))
		{
#ifdef _DEBUG
			OutputDebugStringA("ShaderGroup::Initialize - CreateInputLayout Failed");
#endif
			return false;
		}

		return true;
	}

	void ShaderGroup::BindShaderAndLayout(ID3D11DeviceContext* ctx)
	{
		if (m_inputLayout)
			ctx->IASetInputLayout(m_inputLayout.Get());

		if (m_vertexShader)
			m_vertexShader->Bind(ctx);

		if (m_pixelShader)
			m_pixelShader->Bind(ctx);
	}

	int ShaderGroup::VSConstantBufferSlot(const std::string& name, uint32_t size) const
	{
		if (nullptr == m_vertexShader)
			return -1;

		return m_vertexShader->ConstantBufferSlot(name, size);
	}

	int ShaderGroup::PSConstantBufferSlot(const std::string& name, uint32_t size) const
	{
		if (nullptr == m_pixelShader)
			return -1;

		return m_pixelShader->ConstantBufferSlot(name, size);
	}

	int ShaderGroup::VSTextureSlot(const std::string& name) const
	{
		if (nullptr == m_vertexShader)
			return -1;

		return m_vertexShader->TextureSlot(name);
	}

	int ShaderGroup::PSTextureSlot(const std::string& name) const
	{
		if (nullptr == m_pixelShader)
			return -1;

		return m_pixelShader->TextureSlot(name);
	}
}
