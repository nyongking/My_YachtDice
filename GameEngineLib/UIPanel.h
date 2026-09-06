#pragma once
#include "RenderTypes.h"
#include <memory>
#include <string>

using MyJson = nlohmann::json;

namespace Render { class Geometry; class Material; class Texture; }

namespace GameEngine
{
	class UIPanel
	{
	public:
		~UIPanel();
		void Initialize();

		void SubmitUI(const float2& screenPos, const float2& size,
		              float depth, const float4x4& orthoVP);

		void SetTexture(const std::string& key, const std::wstring& path);
		void SetColor(const float4& color);
		float4 GetColor() const;

		MyJson Serialize() const;
		void   Deserialize(const MyJson& j);

	private:
		Render::Geometry*                 m_quad = nullptr;
		std::unique_ptr<Render::Material> m_material;
		Render::Texture*                  m_texture = nullptr;
		std::string                       m_textureKey;
		std::wstring                      m_texturePath;
		float4                            m_tintColor = { 1.f, 1.f, 1.f, 1.f };
	};
}
