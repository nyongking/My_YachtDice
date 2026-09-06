#include "GameEnginePch.h"
#include "BillboardComponent.h"
#include "UIMaterial.h"
#include "GeometryManager.h"
#include "MaterialManager.h"
#include "TextureManager.h"
#include "CameraManager.h"
#include "CameraComponent.h"
#include "GameObject.h"
#include "Transform.h"
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
	void BillboardComponent::Awake()
	{
		Render::Geometry* quad = GeometryManager::GetInstance()->Get("Quad");
		assert(quad && "Quad geometry not loaded");
		SetGeometry(quad);

		m_layer = Render::RenderPassBase::Layer::UI;

		// Clone UIMaterial (supports tint + atlas UV + texture)
		auto mat = MaterialManager::GetInstance()->Get("UIMaterial");
		if (mat)
			SetMaterial(std::move(mat));
	}

	void BillboardComponent::LateUpdate(float dt)
	{
		if (!m_material || !m_geometry)
			return;

		auto* cam = CameraManager::GetInstance().GetMainCamera();
		if (!cam)
			return;

		const float4x4* pInvView = cam->GetInverseView();
		const float4x4* pVP      = cam->GetViewProj();
		if (!pInvView || !pVP)
			return;

		// Extract camera right/up from InverseView matrix
		float3 camRight = { pInvView->_11, pInvView->_12, pInvView->_13 };
		float3 camUp    = { pInvView->_21, pInvView->_22, pInvView->_23 };

		// Apply 2D rotation around the billboard normal (camera forward axis)
		if (m_rotationDeg != 0.f)
		{
			float rad = DirectX::XMConvertToRadians(m_rotationDeg);
			float c = cosf(rad);
			float s = sinf(rad);

			float3 r = camRight;
			float3 u = camUp;
			// rotated_right = cos*right + sin*up
			camRight = { c * r.x + s * u.x, c * r.y + s * u.y, c * r.z + s * u.z };
			// rotated_up   = -sin*right + cos*up
			camUp    = { -s * r.x + c * u.x, -s * r.y + c * u.y, -s * r.z + c * u.z };
		}

		const float3& pos   = GetOwner()->GetTransform()->GetPosition();
		const float3& scale = GetOwner()->GetTransform()->GetScale();

		float px = pos.x + m_offset.x;
		float py = pos.y + m_offset.y;
		float pz = pos.z + m_offset.z;

		// Forward faces TOWARD camera = -cross(right, up)
		float fx = -(camRight.y * camUp.z - camRight.z * camUp.y);
		float fy = -(camRight.z * camUp.x - camRight.x * camUp.z);
		float fz = -(camRight.x * camUp.y - camRight.y * camUp.x);

		float4x4 world;
		world._11 = camRight.x * scale.x;  world._12 = camRight.y * scale.x;  world._13 = camRight.z * scale.x;  world._14 = 0.f;
		world._21 = camUp.x    * scale.y;  world._22 = camUp.y    * scale.y;  world._23 = camUp.z    * scale.y;  world._24 = 0.f;
		world._31 = fx;                     world._32 = fy;                     world._33 = fz;                     world._34 = 0.f;
		world._41 = px;                     world._42 = py;                     world._43 = pz;                     world._44 = 1.f;

		Render::RenderCommand cmd;
		cmd.geometry = m_geometry;
		cmd.material = m_material.get();
		cmd.world    = world;
		cmd.viewProj = *pVP;

		Render::RenderPipeline::GetInstance().Submit(m_layer, cmd);
	}

	// ── UIMaterial feature wrappers ──

	void BillboardComponent::SetTexture(const std::string& key, const std::wstring& path)
	{
		m_textureKey  = key;
		m_texturePath = path;
		m_texture     = TextureManager::GetInstance()->LoadSync(key, path);

		if (m_texture && m_material)
		{
			auto* uiMat = static_cast<UIMaterial*>(m_material.get());
			uiMat->SetUITexture(m_texture);
		}
	}

	void BillboardComponent::SetTintColor(const float4& color)
	{
		if (m_material)
			static_cast<UIMaterial*>(m_material.get())->SetTintColor(color);
	}

	float4 BillboardComponent::GetTintColor() const
	{
		if (m_material)
			return static_cast<const UIMaterial*>(m_material.get())->GetTintColor();
		return { 1.f, 1.f, 1.f, 1.f };
	}

	void BillboardComponent::SetUVRect(const float4& rect)
	{
		if (m_material)
			static_cast<UIMaterial*>(m_material.get())->SetUVRect(rect);
	}

	void BillboardComponent::SetUVRectPixel(float x, float y, float w, float h, float texW, float texH)
	{
		if (texW > 0.f && texH > 0.f)
			SetUVRect({ x / texW, y / texH, w / texW, h / texH });
	}

	float4 BillboardComponent::GetUVRect() const
	{
		if (m_material)
			return static_cast<const UIMaterial*>(m_material.get())->GetUVRect();
		return { 0.f, 0.f, 1.f, 1.f };
	}

	// ── Serialization ──

	MyJson BillboardComponent::Serialize() const
	{
		MyJson j = RenderComponent::Serialize();
		j["type"] = GetTypeName();
		j["offset"] = { m_offset.x, m_offset.y, m_offset.z };
		if (m_rotationDeg != 0.f)
			j["rotation"] = m_rotationDeg;

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
			float4 c = GetTintColor();
			j["tintR"] = c.x;
			j["tintG"] = c.y;
			j["tintB"] = c.z;
			j["tintA"] = c.w;
		}

		return j;
	}

	void BillboardComponent::Deserialize(const MyJson& j)
	{
		RenderComponent::Deserialize(j);

		if (j.contains("offset"))
		{
			auto& arr  = j["offset"];
			m_offset.x = arr[0].get<float>();
			m_offset.y = arr[1].get<float>();
			m_offset.z = arr[2].get<float>();
		}

		if (j.contains("rotation"))
			m_rotationDeg = j["rotation"].get<float>();

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
			SetTintColor(c);
		}
	}

#ifdef _DEBUG
	void BillboardComponent::OnInspectorGUI()
	{
		RenderComponent::OnInspectorGUI();
		ImGui::DragFloat3("Offset", &m_offset.x, 0.1f);
		ImGui::DragFloat("Rotation", &m_rotationDeg, 1.f, -360.f, 360.f, "%.1f deg");

		float4 uv = GetUVRect();
		if (ImGui::DragFloat4("UV Rect", &uv.x, 0.01f, 0.f, 1.f))
			SetUVRect(uv);

		float4 tint = GetTintColor();
		if (ImGui::ColorEdit4("Tint", &tint.x))
			SetTintColor(tint);
	}
#endif
}
