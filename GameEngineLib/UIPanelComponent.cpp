#include "GameEnginePch.h"
#include "UIPanelComponent.h"
#include "UIMaterial.h"
#include "GeometryManager.h"
#include "MaterialManager.h"
#include "TextureManager.h"
#include "RenderItem.h"
#include "RenderPipeline.h"
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
	void UIPanelComponent::Awake()
	{
		m_quad = GeometryManager::GetInstance()->Get("Quad");
		assert(m_quad && "Quad geometry not loaded");
	}

	void UIPanelComponent::EnsureMaterials()
	{
		while (m_materials.size() < m_elements.size())
		{
			auto mat = MaterialManager::GetInstance()->Get("UIMaterial");
			if (mat && m_texture)
			{
				auto* uiMat = static_cast<UIMaterial*>(mat.get());
				uiMat->SetUITexture(m_texture);
			}
			m_materials.push_back(std::move(mat));
		}

		if (m_materials.size() > m_elements.size())
			m_materials.resize(m_elements.size());
	}

	void UIPanelComponent::LateUpdate(float dt)
	{
		using namespace DirectX;

		if (!m_visible || m_elements.empty() || !m_quad)
			return;

		EnsureMaterials();

		float2 anchor    = GetAnchorBasePosition();
		float4x4 orthoVP = GetOrthoViewProj();

		for (int i = 0; i < static_cast<int>(m_elements.size()); ++i)
		{
			auto& elem = m_elements[i];
			auto& mat  = m_materials[i];
			if (!mat) continue;

			auto* uiMat = static_cast<UIMaterial*>(mat.get());
			uiMat->SetUVRect(elem.uvRect);
			uiMat->SetTintColor(elem.tintColor);

			float cx = anchor.x + m_position.x + elem.localOffset.x + elem.size.x * 0.5f;
			float cy = anchor.y - m_position.y - elem.localOffset.y - elem.size.y * 0.5f;

			XMMATRIX scale = XMMatrixScaling(elem.size.x, elem.size.y, 1.f);
			XMMATRIX rot   = XMMatrixRotationZ(elem.rotation);
			XMMATRIX trans = XMMatrixTranslation(cx, cy, m_depth);
			XMMATRIX world = scale * rot * trans;

			Render::RenderCommand cmd;
			cmd.geometry = m_quad;
			cmd.material = mat.get();
			XMStoreFloat4x4(&cmd.world, world);
			cmd.viewProj = orthoVP;

			Render::RenderPipeline::GetInstance().Submit(
				Render::RenderPassBase::Layer::UI, cmd);
		}
	}

	void UIPanelComponent::SetTexture(const std::string& key, const std::wstring& path)
	{
		m_textureKey  = key;
		m_texturePath = path;
		m_texture     = TextureManager::GetInstance()->LoadSync(key, path);

		for (auto& mat : m_materials)
		{
			if (mat && m_texture)
			{
				auto* uiMat = static_cast<UIMaterial*>(mat.get());
				uiMat->SetUITexture(m_texture);
			}
		}
	}

	void UIPanelComponent::AddElement(const UIPanelElement& elem)
	{
		m_elements.push_back(elem);
	}

	void UIPanelComponent::SetElement(int index, const UIPanelElement& elem)
	{
		if (index >= 0 && index < static_cast<int>(m_elements.size()))
			m_elements[index] = elem;
	}

	void UIPanelComponent::ClearElements()
	{
		m_elements.clear();
		m_materials.clear();
	}

	UIPanelElement& UIPanelComponent::GetElement(int index)
	{
		return m_elements[index];
	}

	const UIPanelElement& UIPanelComponent::GetElement(int index) const
	{
		return m_elements[index];
	}

	MyJson UIPanelComponent::Serialize() const
	{
		MyJson j = UIComponent::Serialize();
		j["type"] = GetTypeName();

		if (!m_texturePath.empty())
			j["texture"] = WStringToUTF8(m_texturePath);

		MyJson elems = MyJson::array();
		for (auto& elem : m_elements)
		{
			MyJson e;
			e["offsetX"]  = elem.localOffset.x;
			e["offsetY"]  = elem.localOffset.y;
			e["w"]        = elem.size.x;
			e["h"]        = elem.size.y;
			e["rotation"] = elem.rotation;

			if (m_texture)
			{
				float texW = static_cast<float>(m_texture->GetWidth());
				float texH = static_cast<float>(m_texture->GetHeight());
				MyJson atlas;
				atlas["x"] = elem.uvRect.x * texW;
				atlas["y"] = elem.uvRect.y * texH;
				atlas["w"] = elem.uvRect.z * texW;
				atlas["h"] = elem.uvRect.w * texH;
				e["atlas"] = atlas;
			}

			e["tintR"] = elem.tintColor.x;
			e["tintG"] = elem.tintColor.y;
			e["tintB"] = elem.tintColor.z;
			e["tintA"] = elem.tintColor.w;

			elems.push_back(e);
		}
		j["elements"] = elems;

		return j;
	}

	void UIPanelComponent::Deserialize(const MyJson& j)
	{
		UIComponent::Deserialize(j);

		if (j.contains("texture"))
		{
			std::string path = j["texture"].get<std::string>();
			std::wstring wpath = UTF8ToWString(path);
			SetTexture(path, wpath);
		}

		if (j.contains("elements"))
		{
			float texW = m_texture ? static_cast<float>(m_texture->GetWidth())  : 0.f;
			float texH = m_texture ? static_cast<float>(m_texture->GetHeight()) : 0.f;

			for (auto& e : j["elements"])
			{
				UIPanelElement elem;
				elem.localOffset.x = e.value("offsetX", 0.f);
				elem.localOffset.y = e.value("offsetY", 0.f);
				elem.size.x        = e.value("w", 51.f);
				elem.size.y        = e.value("h", 51.f);
				elem.rotation      = e.value("rotation", 0.f);

				if (e.contains("atlas") && texW > 0.f && texH > 0.f)
				{
					auto& a = e["atlas"];
					float ax = a["x"].get<float>();
					float ay = a["y"].get<float>();
					float aw = a["w"].get<float>();
					float ah = a["h"].get<float>();
					elem.uvRect = { ax / texW, ay / texH, aw / texW, ah / texH };
				}

				elem.tintColor.x = e.value("tintR", 1.f);
				elem.tintColor.y = e.value("tintG", 1.f);
				elem.tintColor.z = e.value("tintB", 1.f);
				elem.tintColor.w = e.value("tintA", 1.f);

				AddElement(elem);
			}
		}
	}

#ifdef _DEBUG
	void UIPanelComponent::OnInspectorGUI()
	{
		ImGui::Checkbox("Visible", &m_visible);
		ImGui::Text("Elements: %d", GetElementCount());

		for (int i = 0; i < GetElementCount(); ++i)
		{
			ImGui::PushID(i);
			if (ImGui::TreeNode("Element", "Element %d", i))
			{
				auto& elem = m_elements[i];
				ImGui::DragFloat2("Offset", &elem.localOffset.x, 1.f);
				ImGui::DragFloat2("Size",   &elem.size.x, 1.f, 1.f, 4096.f);

				float rotDeg = DirectX::XMConvertToDegrees(elem.rotation);
				if (ImGui::DragFloat("Rotation", &rotDeg, 1.f, -360.f, 360.f))
					elem.rotation = DirectX::XMConvertToRadians(rotDeg);

				ImGui::ColorEdit4("Tint", &elem.tintColor.x);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}
#endif
}
