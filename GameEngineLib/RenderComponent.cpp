#include "GameEnginePch.h"
#include "RenderComponent.h"

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

#ifdef _DEBUG
	void RenderComponent::OnInspectorGUI()
	{
		const char* layers[] = { "Opaque", "Transparent", "UI", "Effect" };
		int layerInt = static_cast<int>(m_layer);
		if (ImGui::Combo("Layer", &layerInt, layers, 4))
			m_layer = static_cast<Render::RenderPassBase::Layer>(layerInt);
	}
#endif

	MyJson RenderComponent::Serialize() const
	{
		MyJson j;
		j["type"]  = GetTypeName();
		j["layer"] = LayerToString(m_layer);
		return j;
	}

	void RenderComponent::Deserialize(const MyJson& j)
	{
		if (j.contains("layer"))
			m_layer = LayerFromString(j["layer"].get<std::string>());
	}

	void RenderComponent::LateUpdate(float dt)
	{
		if (!m_material || !m_geometry)
			return;

		Render::RenderCommand cmd;
		cmd.geometry = m_geometry;
		cmd.material = m_material.get();
		cmd.world    = GetOwner()->GetTransform()->GetWorldMatrix();

		auto* cam = CameraManager::GetInstance().GetMainCamera();
		if (cam)
		{
			const float4x4* pVP = cam->GetViewProj();
			if (pVP)
				cmd.viewProj = *pVP;
		}

		Render::RenderPipeline::GetInstance().Submit(m_layer, cmd);
	}
}
