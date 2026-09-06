#pragma once
#include "UIComponent.h"
#include "TextItem.h"
#include <string>

namespace DirectX { inline namespace DX11 { class SpriteFont; } }

namespace GameEngine
{
	class UITextComponent : public UIComponent
	{
	public:
		void Initialize() override;
		void SubmitUI(float dt = 0.f) override;

		std::string GetTypeName() const override { return "UITextComponent"; }
		MyJson      Serialize()   const override;
		void        Deserialize(const MyJson& j) override;

		void SetText(const std::wstring& text)   { m_text = text; }
		void SetNumber(int value)                { m_text = std::to_wstring(value); }
		void SetNumber(float value, int decimals = 1);
		const std::wstring& GetText() const      { return m_text; }

		void SetFontKey(const std::string& key);
		const std::string& GetFontKey() const    { return m_fontKey; }

		void SetColor(const float4& color)       { m_color = color; }
		float4 GetColor() const                  { return m_color; }

		void SetFontScale(float scale)           { m_fontScale = scale; }
		float GetFontScale() const               { return m_fontScale; }

		void SetAlignment(Render::TextAlignment align) { m_alignment = align; }
		Render::TextAlignment GetAlignment() const     { return m_alignment; }

#ifdef _DEBUG
		void OnInspectorGUI() override;
#endif

	private:
		std::wstring m_text;
		std::string  m_fontKey;
		float4       m_color     = { 1.f, 1.f, 1.f, 1.f };
		float        m_fontScale = 1.f;
		Render::TextAlignment m_alignment = Render::TextAlignment::Left;

		DirectX::SpriteFont* m_font = nullptr;
	};
}
