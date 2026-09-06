#pragma once

#include "d3dcompiler.h"
#pragma comment(lib, "d3dcompiler.lib")

namespace Render
{
	enum SHADER_INPUT_TYPE
	{
		CBUFFER,
		TEXTURE,
		SAMPLER,
		STRUCTURED_BUFFER,
		UAV_BUFFER,
		INPUT_TYPE_END
	};

	/*struct ResourceBindInfo
	{
		std::string name;
		D3D_SHADER_INPUT_TYPE type;
		uint32_t slot;
	};*/

	class Shader abstract
	{
		struct ResourceBindInfo
		{
			std::string name;
			uint32_t slot;
		};

		struct ConstBufferVariableInfo
		{
			std::string name;
			uint32_t startOffset;
			uint32_t size;
		};

		struct ConstantBufferInfo
		{
			std::string name;
			uint32_t slot;
			uint32_t size;
			std::vector<ConstBufferVariableInfo> variables;
		};

	public:
		Shader() = default;
		virtual ~Shader() = default;

		bool IsShaderCreated() { return m_isCreated; }
		bool ReflectShader();

		virtual bool LoadFromFile(const std::wstring& path, const std::string& entry = "main", const std::string& profile = "") abstract;
		virtual bool CreateFromShaderBlob(ID3D11Device* pDevice) abstract;

		int			 ConstantBufferSlot(const std::string& name, uint32_t size);
		int			 TextureSlot(const std::string& name) const;
		
	protected:
		bool			 m_isCreated = false;

		std::vector<ConstantBufferInfo> m_cbuffers;
		std::vector<ResourceBindInfo>	m_resources[INPUT_TYPE_END];


		RefCom<ID3DBlob> m_shaderBlob;
		RefCom<ID3DBlob> m_errorBlob;
		RefCom<ID3D11ShaderReflection> m_reflection;


	};

}

