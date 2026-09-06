#include "GameEnginePch.h"
#include "UIImageComponent.h"
#include "UIMaterial.h"
#include "NineSliceGeometry.h"
#include "GeometryManager.h"
#include "MaterialManager.h"
#include "TextureManager.h"
#include "RenderDevice.h"
#include "Texture.h"
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
}

namespace GameEngine
{
	void UIImageComponent::Initialize()
	{
		UIComponent::Initialize();

		if (m_material && m_texture)
		{
			auto* uiMat = static_cast<UIMaterial*>(m_material.get());
			uiMat->SetUITexture(m_texture);
		}
	}

	void UIImageComponent::SubmitUI(float dt)
	{
		if (!m_material)
			return;

		float scale = m_animation.Update(dt);

		Render::Geometry* geo = m_quad;

		if (m_useNineSlice)
		{
			EnsureNineSlice();
			if (m_nineSlice)
			{
				if (m_dirty)
				{
					auto* ctx = Render::RenderDevice::GetInstance().GetContext().Get();
					float texW = m_texture ? static_cast<float>(m_texture->GetWidth())  : 0.f;
					float texH = m_texture ? static_cast<float>(m_texture->GetHeight()) : 0.f;
					m_nineSlice->UpdateVertices(ctx, m_size.x, m_size.y,
					                            m_borderL, m_borderT, m_borderR, m_borderB,
					                            texW, texH);
					m_dirty = false;
				}
				geo = m_nineSlice.get();
			}
		}
		else
		{
			m_dirty = false;
		}

		DoSubmitUI(geo, m_material.get(), scale);
	}

	void UIImageComponent::SetTexture(const std::string& key, const std::wstring& path)
	{
		m_textureKey  = key;
		m_texturePath = path;
		m_texture     = TextureManager::GetInstance()->LoadSync(key, path);

		if (m_texture && m_material)
		{
			auto* uiMat = static_cast<UIMaterial*>(m_material.get());
			uiMat->SetUITexture(m_texture);
		}
		m_dirty = true;
	}

	void UIImageComponent::SetColor(const float4& color)
	{
		if (m_material)
		{
			auto* uiMat = static_cast<UIMaterial*>(m_material.get());
			uiMat->SetTintColor(color);
		}
	}

	float4 UIImageComponent::GetColor() const
	{
		if (m_material)
		{
			auto* uiMat = static_cast<const UIMaterial*>(m_material.get());
			return uiMat->GetTintColor();
		}
		return { 1.f, 1.f, 1.f, 1.f };
	}

	void UIImageComponent::SetUVRect(const float4& rect)
	{
		if (m_material)
		{
			auto* uiMat = static_cast<UIMaterial*>(m_material.get());
			uiMat->SetUVRect(rect);
		}
	}

	void UIImageComponent::SetUVRectPixel(float x, float y, float w, float h, float texW, float texH)
	{
		if (texW > 0.f && texH > 0.f)
			SetUVRect({ x / texW, y / texH, w / texW, h / texH });
	}

	float4 UIImageComponent::GetUVRect() const
	{
		if (m_material)
		{
			auto* uiMat = static_cast<const UIMaterial*>(m_material.get());
			return uiMat->GetUVRect();
		}
		return { 0.f, 0.f, 1.f, 1.f };
	}

	void UIImageComponent::SetNineSlice(float borderL, float borderT, float borderR, float borderB)
	{
		m_useNineSlice = true;
		m_borderL = borderL;
		m_borderT = borderT;
		m_borderR = borderR;
		m_borderB = borderB;
		m_dirty   = true;
	}

	void UIImageComponent::DisableNineSlice()
	{
		m_useNineSlice = false;
		m_nineSlice.reset();
		m_dirty = true;
	}

	void UIImageComponent::EnsureNineSlice()
	{
		if (m_nineSlice)
			return;

		auto* device = Render::RenderDevice::GetInstance().GetDevice().Get();
		if (!device)
			return;

		float texW = m_texture ? static_cast<float>(m_texture->GetWidth())  : 0.f;
		float texH = m_texture ? static_cast<float>(m_texture->GetHeight()) : 0.f;

		m_nineSlice = std::make_unique<NineSliceGeometry>();
		m_nineSlice->Create(device, m_size.x, m_size.y,
		                    m_borderL, m_borderT, m_borderR, m_borderB,
		                    texW, texH);
		m_dirty = false;
	}

	MyJson UIImageComponent::Serialize() const
	{
		MyJson j = UIComponent::Serialize();
		j["type"] = GetTypeName();

		if (!m_texturePath.empty())
			j["texture"] = WStringToUTF8(m_texturePath);

		float4 uv = GetUVRect();
		if (uv.x != 0.f || uv.y != 0.f || uv.z != 1.f || uv.w != 1.f)
		{
			MyJson atlas;
			if (m_texture)
			{
				float texW = static_cast<float>(m_texture->GetWidth());
				float texH = static_cast<float>(m_texture->GetHeight());
				atlas["x"] = uv.x * texW;
				atlas["y"] = uv.y * texH;
				atlas["w"] = uv.z * texW;
				atlas["h"] = uv.w * texH;
			}
			else
			{
				atlas["x"] = uv.x;
				atlas["y"] = uv.y;
				atlas["w"] = uv.z;
				atlas["h"] = uv.w;
			}
			j["atlas"] = atlas;
		}

		if (m_material)
		{
			float4 c = GetColor();
			j["tintR"] = c.x;
			j["tintG"] = c.y;
			j["tintB"] = c.z;
			j["tintA"] = c.w;
		}

		j["useNineSlice"] = m_useNineSlice;
		if (m_useNineSlice)
		{
			j["borderL"] = m_borderL;
			j["borderT"] = m_borderT;
			j["borderR"] = m_borderR;
			j["borderB"] = m_borderB;
		}

		return j;
	}

	void UIImageComponent::Deserialize(const MyJson& j)
	{
		UIComponent::Deserialize(j);

		if (j.contains("texture"))
		{
			std::string path = j["texture"].get<std::string>();
			std::wstring wpath = UTF8ToWString(path);
			SetTexture(path, wpath);
		}

		if (j.contains("atlas"))
		{
			auto& a = j["atlas"];
			float x = a["x"].get<float>();
			float y = a["y"].get<float>();
			float w = a["w"].get<float>();
			float h = a["h"].get<float>();

			if (m_texture)
			{
				float texW = static_cast<float>(m_texture->GetWidth());
				float texH = static_cast<float>(m_texture->GetHeight());
				SetUVRectPixel(x, y, w, h, texW, texH);
			}
		}

		if (j.contains("tintR"))
		{
			float4 c;
			c.x = j["tintR"].get<float>();
			c.y = j["tintG"].get<float>();
			c.z = j["tintB"].get<float>();
			c.w = j["tintA"].get<float>();
			SetColor(c);
		}

		if (j.contains("useNineSlice") && j["useNineSlice"].get<bool>())
		{
			float bL = j.value("borderL", 0.f);
			float bT = j.value("borderT", 0.f);
			float bR = j.value("borderR", 0.f);
			float bB = j.value("borderB", 0.f);
			SetNineSlice(bL, bT, bR, bB);
		}
	}

#ifdef _DEBUG
	void UIImageComponent::OnInspectorGUI()
	{
		UIComponent::OnInspectorGUI();

		ImGui::Separator();

		float4 uv = GetUVRect();
		if (ImGui::DragFloat4("UV Rect", &uv.x, 0.01f, 0.f, 1.f))
			SetUVRect(uv);

		float4 tint = GetColor();
		if (ImGui::ColorEdit4("Tint", &tint.x))
			SetColor(tint);

		if (ImGui::Checkbox("9-Slice", &m_useNineSlice))
		{
			if (!m_useNineSlice)
				DisableNineSlice();
			else
				m_dirty = true;
		}

		if (m_useNineSlice)
		{
			bool changed = false;
			changed |= ImGui::DragFloat("Border L", &m_borderL, 1.f, 0.f, 512.f);
			changed |= ImGui::DragFloat("Border T", &m_borderT, 1.f, 0.f, 512.f);
			changed |= ImGui::DragFloat("Border R", &m_borderR, 1.f, 0.f, 512.f);
			changed |= ImGui::DragFloat("Border B", &m_borderB, 1.f, 0.f, 512.f);
			if (changed)
				m_dirty = true;
		}
	}
#endif
}
