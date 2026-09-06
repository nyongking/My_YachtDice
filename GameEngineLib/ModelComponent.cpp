#include "GameEnginePch.h"
#include "ModelComponent.h"

#ifdef _DEBUG
#include "Imgui/imgui.h"
#endif

#include "RenderItem.h"
#include "RenderPipeline.h"
#include "CameraManager.h"
#include "CameraComponent.h"
#include "GameObject.h"
#include "Transform.h"
#include "Material.h"
#include "ModelManager.h"
#include "EngineGlobal.h"
#include "GBufferMaterial.h"
#include "TextureManager.h"
#include "StringUtil.h"

namespace GameEngine
{
	static Render::RenderPassBase::Layer LayerFromString(const std::string& s)
	{
		if (s == "Transparent") return Render::RenderPassBase::Layer::Transparent;
		if (s == "UI")          return Render::RenderPassBase::Layer::UI;
		if (s == "Effect")      return Render::RenderPassBase::Layer::Effect;
		return Render::RenderPassBase::Layer::Opaque;
	}

	static std::string LayerToString(Render::RenderPassBase::Layer layer)
	{
		switch (layer)
		{
		case Render::RenderPassBase::Layer::Transparent: return "Transparent";
		case Render::RenderPassBase::Layer::UI:          return "UI";
		case Render::RenderPassBase::Layer::Effect:      return "Effect";
		default:                                         return "Opaque";
		}
	}

	void ModelComponent::SetModel(const std::string& key, const std::string& path)
	{
		m_modelKey  = key;
		m_modelPath = path;

		if (!GModelManager) return;

		// 캐시 히트면 바로 반환, 없으면 LoadSync로 로드
		m_model = GModelManager->Get(key);
		if (!m_model)
			m_model = GModelManager->LoadSync(key, path);

		// Model의 머티리얼을 Clone하여 per-instance 소유
		CloneMaterials();
	}

	void ModelComponent::CloneMaterials()
	{
		m_materials.clear();
		if (!m_model) return;

		m_materials.resize(m_model->GetSectionCount());
		for (size_t i = 0; i < m_model->GetSectionCount(); ++i)
		{
			auto* srcMat = m_model->GetSection(i).material;
			if (srcMat)
				m_materials[i] = srcMat->Clone();
		}
	}

	MyJson ModelComponent::Serialize() const
	{
		MyJson j;
		j["type"]      = GetTypeName();
		j["modelKey"]  = m_modelKey;
		j["modelPath"] = m_modelPath;
		j["layer"]     = LayerToString(m_layer);

		if (m_model)
		{
			MyJson sections = MyJson::array();
			for (size_t i = 0; i < m_model->GetSectionCount(); ++i)
			{
				MyJson sec;
				auto* mat = (i < m_materials.size() && m_materials[i])
					? dynamic_cast<Render::GBufferMaterial*>(m_materials[i].get())
					: dynamic_cast<Render::GBufferMaterial*>(m_model->GetSection(i).material);
				if (mat)
				{
					float4 col = mat->GetAlbedoColor();
					sec["albedoColor"] = { col.x, col.y, col.z, col.w };

					float2 tiling = mat->GetUVTiling();
					if (tiling.x != 1.f || tiling.y != 1.f)
						sec["uvTiling"] = { tiling.x, tiling.y };

					float2 offset = mat->GetUVOffset();
					if (offset.x != 0.f || offset.y != 0.f)
						sec["uvOffset"] = { offset.x, offset.y };

					float shin = mat->GetShininess();
					if (shin != 32.f)
						sec["shininess"] = shin;

					float specStr = mat->GetSpecularStrength();
					if (specStr != 0.5f)
						sec["specularStrength"] = specStr;
				}

				// 텍스처 경로 저장
				if (i < m_albedoTexturePaths.size() && !m_albedoTexturePaths[i].empty())
					sec["albedoTexture"] = m_albedoTexturePaths[i];
				if (i < m_normalTexturePaths.size() && !m_normalTexturePaths[i].empty())
					sec["normalTexture"] = m_normalTexturePaths[i];

				sections.push_back(sec);
			}
			j["sections"] = sections;
		}

		return j;
	}

	void ModelComponent::Deserialize(const MyJson& j)
	{
		if (j.contains("modelKey") && j.contains("modelPath"))
			SetModel(j["modelKey"].get<std::string>(), j["modelPath"].get<std::string>());

		if (j.contains("layer"))
			m_layer = LayerFromString(j["layer"].get<std::string>());

		// 모델 로드 후 section별 색상·텍스처 복원
		if (m_model && j.contains("sections"))
		{
			const auto& sections = j["sections"];
			m_albedoTexturePaths.resize(m_model->GetSectionCount());
			m_normalTexturePaths.resize(m_model->GetSectionCount());

			for (size_t i = 0; i < m_model->GetSectionCount() && i < sections.size(); ++i)
			{
				const auto& sec = sections[i];
				auto* mat = (i < m_materials.size() && m_materials[i])
					? dynamic_cast<Render::GBufferMaterial*>(m_materials[i].get())
					: dynamic_cast<Render::GBufferMaterial*>(m_model->GetSection(i).material);
				if (!mat) continue;

				if (sec.contains("albedoColor"))
				{
					const auto& c = sec["albedoColor"];
					mat->SetAlbedoColor({ c[0].get<float>(), c[1].get<float>(), c[2].get<float>(), c[3].get<float>() });
				}

				if (sec.contains("uvTiling"))
				{
					const auto& t = sec["uvTiling"];
					mat->SetUVTiling({ t[0].get<float>(), t[1].get<float>() });
				}

				if (sec.contains("uvOffset"))
				{
					const auto& o = sec["uvOffset"];
					mat->SetUVOffset({ o[0].get<float>(), o[1].get<float>() });
				}

				if (sec.contains("shininess"))
					mat->SetShininess(sec["shininess"].get<float>());

				if (sec.contains("specularStrength"))
					mat->SetSpecularStrength(sec["specularStrength"].get<float>());

				if (GTextureManager && sec.contains("albedoTexture"))
				{
					std::string path = sec["albedoTexture"].get<std::string>();
					m_albedoTexturePaths[i] = path;
					std::string key = m_modelKey + "_albedo_" + std::to_string(i);
					auto* tex = GTextureManager->LoadSync(key, StringToWString(path));
					if (tex) mat->SetAlbedoTexture(tex);
				}

				if (GTextureManager && sec.contains("normalTexture"))
				{
					std::string path = sec["normalTexture"].get<std::string>();
					m_normalTexturePaths[i] = path;
					std::string key = m_modelKey + "_normal_" + std::to_string(i);
					auto* tex = GTextureManager->LoadSync(key, StringToWString(path));
					if (tex) mat->SetNormalTexture(tex);
				}
			}
		}
	}

#ifdef _DEBUG
	void ModelComponent::OnInspectorGUI()
	{
		ImGui::Text("Key:  %s", m_modelKey.empty()  ? "(none)" : m_modelKey.c_str());
		ImGui::Text("Path: %s", m_modelPath.empty() ? "(none)" : m_modelPath.c_str());

		const char* layers[] = { "Opaque", "Transparent", "UI", "Effect" };
		int layerInt = static_cast<int>(m_layer);
		if (ImGui::Combo("Layer", &layerInt, layers, 4))
			m_layer = static_cast<Render::RenderPassBase::Layer>(layerInt);

		if (m_model && ImGui::CollapsingHeader("Materials"))
		{
			for (size_t i = 0; i < m_model->GetSectionCount(); ++i)
			{
				auto* mat = (i < m_materials.size() && m_materials[i])
					? dynamic_cast<Render::GBufferMaterial*>(m_materials[i].get())
					: dynamic_cast<Render::GBufferMaterial*>(m_model->GetSection(i).material);
				if (!mat) continue;

				ImGui::PushID(static_cast<int>(i));
				ImGui::Text("Section %zu", i);
				float4 col = mat->GetAlbedoColor();
				float color[4] = { col.x, col.y, col.z, col.w };
				if (ImGui::ColorEdit4("Albedo", color))
					mat->SetAlbedoColor({ color[0], color[1], color[2], color[3] });

				float2 tiling = mat->GetUVTiling();
				float uvTiling[2] = { tiling.x, tiling.y };
				if (ImGui::DragFloat2("UV Tiling", uvTiling, 0.01f))
					mat->SetUVTiling({ uvTiling[0], uvTiling[1] });

				float2 offset = mat->GetUVOffset();
				float uvOffset[2] = { offset.x, offset.y };
				if (ImGui::DragFloat2("UV Offset", uvOffset, 0.01f))
					mat->SetUVOffset({ uvOffset[0], uvOffset[1] });

				float shininess = mat->GetShininess();
				if (ImGui::DragFloat("Shininess", &shininess, 1.f, 1.f, 256.f))
					mat->SetShininess(shininess);

				float specStr = mat->GetSpecularStrength();
				if (ImGui::DragFloat("Specular", &specStr, 0.01f, 0.f, 1.f))
					mat->SetSpecularStrength(specStr);

				ImGui::PopID();
				ImGui::Spacing();
			}
		}
	}
#endif

	void ModelComponent::LateUpdate(float dt)
	{
		if (!m_model || m_model->GetSectionCount() == 0)
			return;

		float4x4 world = GetOwner()->GetTransform()->GetWorldMatrix();

		float4x4 viewProj = {};
		auto* cam = CameraManager::GetInstance().GetMainCamera();
		if (cam)
		{
			const float4x4* pVP = cam->GetViewProj();
			if (pVP)
				viewProj = *pVP;
		}

		for (size_t i = 0; i < m_model->GetSectionCount(); ++i)
		{
			auto& section = m_model->GetSection(i);

			// per-instance clone이 있으면 사용, 없으면 Model 원본 사용
			Render::Material* mat = (i < m_materials.size() && m_materials[i])
				? m_materials[i].get()
				: section.material;
			if (!mat)
				continue;

			Render::RenderCommand cmd;
			cmd.geometry = &section.geometry;
			cmd.material = mat;
			cmd.world    = world;
			cmd.viewProj = viewProj;
			Render::RenderPipeline::GetInstance().Submit(m_layer, cmd);
		}
	}
}
