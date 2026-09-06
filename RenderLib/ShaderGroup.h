#pragma once
namespace Render
{
	class ShaderGroup
	{
	public:
		ShaderGroup() = default;
		~ShaderGroup() = default;

	public:
		// VS blob/reflection을 사용해 InputLayout 자동 생성
		bool Initialize(class VertexShader* vs,
			class PixelShader* ps,
			ID3D11Device* device);

		// Stage 2 (InputLayout) + Stage 4 (VS/PS) 한번에 바인드
		void BindShaderAndLayout(ID3D11DeviceContext* ctx);

		int VSConstantBufferSlot(const std::string& name, uint32_t size) const;
		int PSConstantBufferSlot(const std::string& name, uint32_t size) const;

		int VSTextureSlot(const std::string& name) const;
		int PSTextureSlot(const std::string& name) const;

	private:
		RefCom<ID3D11InputLayout> m_inputLayout;
		class VertexShader*       m_vertexShader = nullptr;
		class PixelShader*        m_pixelShader  = nullptr;
	};
}
