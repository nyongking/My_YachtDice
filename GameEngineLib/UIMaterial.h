#pragma once
#include "Material.h"

namespace GameEngine
{
	class UIMaterial : public Render::Material
	{
	public:
		UIMaterial()  = default;
		~UIMaterial() = default;

		UIMaterial(const UIMaterial& rhs);

	public:
		bool Initialize(Render::ShaderGroup* shaderGroup, ID3D11Device* device) override;
		std::unique_ptr<Render::Material> Clone() const override;

		void SetTintColor(const float4& color) { m_uiData.tintColor = color; MarkParameterDirty(0); }
		float4 GetTintColor() const            { return m_uiData.tintColor; }

		void SetUVRect(const float4& rect) { m_uvRectData.uvRect = rect; MarkParameterDirty(1); }
		float4 GetUVRect() const           { return m_uvRectData.uvRect; }

		bool SetUITexture(Render::Texture* tex)                  { return SetTexture("UITexture", tex); }
		bool SetUITexture(ID3D11ShaderResourceView* srv)         { return SetTexture("UITexture", srv); }

	private:
		struct UIData
		{
			float4 tintColor = { 1.f, 1.f, 1.f, 1.f };
		};

		struct UVRectData
		{
			float4 uvRect = { 0.f, 0.f, 1.f, 1.f };
		};

		UIData     m_uiData     = {};
		UVRectData m_uvRectData = {};
	};
}
