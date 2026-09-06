#pragma once
#include "UIComponent.h"
#include "NineSliceGeometry.h"

namespace Render { class Texture; }

namespace GameEngine
{
	class UIImageComponent : public UIComponent
	{
	public:
		void Initialize() override;
		void SubmitUI(float dt = 0.f) override;

		std::string GetTypeName() const override { return "UIImageComponent"; }
		MyJson      Serialize()   const override;
		void        Deserialize(const MyJson& j) override;

		void SetTexture(const std::string& key, const std::wstring& path);

		void SetColor(const float4& color);
		float4 GetColor() const;

		void SetUVRect(const float4& rect);
		void SetUVRectPixel(float x, float y, float w, float h, float texW, float texH);
		float4 GetUVRect() const;

		void SetNineSlice(float borderL, float borderT, float borderR, float borderB);
		void DisableNineSlice();

#ifdef _DEBUG
		void OnInspectorGUI() override;
#endif

	private:
		void EnsureNineSlice();

		bool m_useNineSlice = false;
		float m_borderL = 0.f;
		float m_borderT = 0.f;
		float m_borderR = 0.f;
		float m_borderB = 0.f;
		std::unique_ptr<NineSliceGeometry> m_nineSlice;

		Render::Texture* m_texture    = nullptr;
		std::string      m_textureKey;
		std::wstring     m_texturePath;
	};
}
