#include "GameEnginePch.h"
#include "UIPanel.h"
#include "UIMaterial.h"
#include "GeometryManager.h"
#include "MaterialManager.h"
#include "TextureManager.h"
#include "RenderItem.h"
#include "RenderPipeline.h"
#include "Texture.h"
#include <Windows.h>

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
	UIPanel::~UIPanel() = default;

	void UIPanel::Initialize()
	{
		m_quad = GeometryManager::GetInstance()->Get("Quad");
		m_material = MaterialManager::GetInstance()->Get("UIMaterial");

		if (m_material)
		{
			auto* uiMat = static_cast<UIMaterial*>(m_material.get());
			uiMat->SetTintColor(m_tintColor);
			if (m_texture)
				uiMat->SetUITexture(m_texture);
		}
	}

	void UIPanel::SubmitUI(const float2& screenPos, const float2& size,
	                       float depth, const float4x4& orthoVP)
	{
		if (!m_quad || !m_material)
			return;

		using namespace DirectX;

		auto* uiMat = static_cast<UIMaterial*>(m_material.get());
		uiMat->SetTintColor(m_tintColor);

		float cx = screenPos.x + size.x * 0.5f;
		float cy = screenPos.y - size.y * 0.5f;

		XMMATRIX scale = XMMatrixScaling(size.x, size.y, 1.f);
		XMMATRIX trans = XMMatrixTranslation(cx, cy, depth);
		XMMATRIX world = scale * trans;

		Render::RenderCommand cmd;
		cmd.geometry = m_quad;
		cmd.material = m_material.get();
		XMStoreFloat4x4(&cmd.world, world);
		cmd.viewProj = orthoVP;

		Render::RenderPipeline::GetInstance().Submit(
			Render::RenderPassBase::Layer::UI, cmd);
	}

	void UIPanel::SetTexture(const std::string& key, const std::wstring& path)
	{
		m_textureKey  = key;
		m_texturePath = path;
		m_texture     = TextureManager::GetInstance()->LoadSync(key, path);

		if (m_material && m_texture)
		{
			auto* uiMat = static_cast<UIMaterial*>(m_material.get());
			uiMat->SetUITexture(m_texture);
		}
	}

	void UIPanel::SetColor(const float4& color)
	{
		m_tintColor = color;
	}

	float4 UIPanel::GetColor() const
	{
		return m_tintColor;
	}

	MyJson UIPanel::Serialize() const
	{
		MyJson j;
		if (!m_texturePath.empty())
			j["texture"] = WStringToUTF8(m_texturePath);
		j["tintR"] = m_tintColor.x;
		j["tintG"] = m_tintColor.y;
		j["tintB"] = m_tintColor.z;
		j["tintA"] = m_tintColor.w;
		return j;
	}

	void UIPanel::Deserialize(const MyJson& j)
	{
		if (j.contains("texture"))
		{
			std::string path = j["texture"].get<std::string>();
			std::wstring wpath = UTF8ToWString(path);
			SetTexture(path, wpath);
		}
		m_tintColor.x = j.value("tintR", 1.f);
		m_tintColor.y = j.value("tintG", 1.f);
		m_tintColor.z = j.value("tintB", 1.f);
		m_tintColor.w = j.value("tintA", 1.f);
	}
}
