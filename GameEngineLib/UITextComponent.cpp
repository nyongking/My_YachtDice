#include "GameEnginePch.h"
#include "UITextComponent.h"
#include "UICanvas.h"
#include "FontManager.h"
#include "RenderPipeline.h"
#include "Renderer.h"
#include <Windows.h>

#ifdef _DEBUG
#include "Imgui/imgui.h"
#endif

namespace
{
	std::string WStringToUTF8(const std::wstring& wstr)
	{
		if (wstr.empty()) return {};
		int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
		if (len <= 0) return {};
		std::string str(len, '\0');
		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], len, nullptr, nullptr);
		str.resize(len - 1);
		return str;
	}

	std::wstring UTF8ToWString(const std::string& str)
	{
		if (str.empty()) return {};
		int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
		if (len <= 0) return {};
		std::wstring wstr(len, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], len);
		wstr.resize(len - 1);
		return wstr;
	}

	std::string AlignmentToString(Render::TextAlignment a)
	{
		switch (a)
		{
		case Render::TextAlignment::Center: return "Center";
		case Render::TextAlignment::Right:  return "Right";
		default:                            return "Left";
		}
	}

	Render::TextAlignment AlignmentFromString(const std::string& s)
	{
		if (s == "Center") return Render::TextAlignment::Center;
		if (s == "Right")  return Render::TextAlignment::Right;
		return Render::TextAlignment::Left;
	}
}

namespace GameEngine
{
	void UITextComponent::Initialize()
	{
		// Text component does not need quad/material from UIComponent
		// It uses SpriteBatch/SpriteFont directly

		if (!m_fontKey.empty())
			m_font = FontManager::GetInstance().GetFont(m_fontKey);

		if (!m_font)
			m_font = FontManager::GetInstance().GetDefaultFont();
	}

	void UITextComponent::SetNumber(float value, int decimals)
	{
		wchar_t buf[64];
		swprintf_s(buf, L"%.*f", decimals, value);
		m_text = buf;
	}

	void UITextComponent::SetFontKey(const std::string& key)
	{
		m_fontKey = key;
		m_font = FontManager::GetInstance().GetFont(key);
	}

	void UITextComponent::SubmitUI(float dt)
	{
		if (!m_font || m_text.empty())
			return;

		// Compute screen position from anchor + offset (top-left origin for SpriteBatch)
		float resolutionScale = GetResolutionScale();
		float2 anchorPt = GetAnchorPoint();
		float posX = anchorPt.x + m_position.x * resolutionScale;
		float posY = anchorPt.y + m_position.y * resolutionScale;

		Render::TextCommand cmd;
		cmd.text      = m_text;
		cmd.position  = { posX, posY };
		cmd.color     = m_color;
		cmd.scale     = m_fontScale * resolutionScale;
		cmd.rotation  = m_rotation;
		cmd.depth     = m_depth;
		cmd.alignment = m_alignment;
		cmd.font      = m_font;

		Render::RenderPipeline::GetInstance().SubmitText(cmd);
	}

	MyJson UITextComponent::Serialize() const
	{
		MyJson j = UIComponent::Serialize();
		j["type"] = GetTypeName();

		if (!m_text.empty())
			j["text"] = WStringToUTF8(m_text);

		if (!m_fontKey.empty())
			j["fontKey"] = m_fontKey;

		j["colorR"] = m_color.x;
		j["colorG"] = m_color.y;
		j["colorB"] = m_color.z;
		j["colorA"] = m_color.w;

		if (m_fontScale != 1.f)
			j["fontScale"] = m_fontScale;

		if (m_alignment != Render::TextAlignment::Left)
			j["alignment"] = AlignmentToString(m_alignment);

		return j;
	}

	void UITextComponent::Deserialize(const MyJson& j)
	{
		UIComponent::Deserialize(j);

		if (j.contains("text"))
			m_text = UTF8ToWString(j["text"].get<std::string>());

		if (j.contains("fontKey"))
		{
			m_fontKey = j["fontKey"].get<std::string>();
			m_font = FontManager::GetInstance().GetFont(m_fontKey);
		}

		if (j.contains("colorR"))
		{
			m_color.x = j["colorR"].get<float>();
			m_color.y = j["colorG"].get<float>();
			m_color.z = j["colorB"].get<float>();
			m_color.w = j["colorA"].get<float>();
		}

		if (j.contains("fontScale"))
			m_fontScale = j["fontScale"].get<float>();

		if (j.contains("alignment"))
			m_alignment = AlignmentFromString(j["alignment"].get<std::string>());

		if (!m_font)
			m_font = FontManager::GetInstance().GetDefaultFont();
	}

#ifdef _DEBUG
	void UITextComponent::OnInspectorGUI()
	{
		UIComponent::OnInspectorGUI();

		ImGui::Separator();
		ImGui::Text("Text Component");

		// Text input (UTF-8 conversion for ImGui)
		std::string utf8 = WStringToUTF8(m_text);
		char buf[512] = {};
		strncpy_s(buf, utf8.c_str(), sizeof(buf) - 1);
		if (ImGui::InputText("Text", buf, sizeof(buf)))
			m_text = UTF8ToWString(std::string(buf));

		if (ImGui::ColorEdit4("Color", &m_color.x))
		{
		}

		ImGui::DragFloat("Font Scale", &m_fontScale, 0.01f, 0.1f, 10.f);

		const char* alignNames[] = { "Left", "Center", "Right" };
		int alignInt = static_cast<int>(m_alignment);
		if (ImGui::Combo("Alignment", &alignInt, alignNames, 3))
			m_alignment = static_cast<Render::TextAlignment>(alignInt);
	}
#endif
}
