#pragma once
#include "Shader.h"


namespace Render
{
	class VertexShader : public Shader
	{
	public:
		VertexShader() = default;
		~VertexShader() = default;

		bool LoadFromFile(const std::wstring& path, const std::string& entry = "main", const std::string& profile = "vs_5_0") override;
		bool CreateFromShaderBlob(ID3D11Device* pDevice) override;

		void Bind(ID3D11DeviceContext* ctx);

		// ShaderGroup이 InputLayout 생성 시 사용
		ID3DBlob*               GetBlob()       { return m_shaderBlob.Get(); }
		ID3D11ShaderReflection* GetReflection() { return m_reflection.Get(); }

	private:
		RefCom<ID3D11VertexShader> m_vertexShader;
		// InputLayout은 ShaderGroup으로 이관
	};

}


