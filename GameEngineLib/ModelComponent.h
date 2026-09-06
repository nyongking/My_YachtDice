#pragma once
#include "Component.h"
#include "RenderPassBase.h"
#include "Model.h"
#include "GBufferMaterial.h"

namespace GameEngine
{
	class ModelComponent : public Component
	{
	public:
		void LateUpdate(float dt) override;

		void SetModel(Model* model)                            { m_model = model; }
		// 키 + 경로로 LoadSync 후 설정 (씬 로드 시 사용)
		void SetModel(const std::string& key, const std::string& path);
		void SetLayer(Render::RenderPassBase::Layer layer)     { m_layer = layer; }

		Model*             GetModel()     const { return m_model; }
		const std::string& GetModelKey()  const { return m_modelKey; }
		const std::string& GetModelPath() const { return m_modelPath; }

		// 직렬화
		std::string GetTypeName()            const override { return "ModelComponent"; }
		MyJson      Serialize()              const override;
		void        Deserialize(const MyJson& j)   override;

#ifdef _DEBUG
		void OnInspectorGUI() override;
#endif

	private:
		void CloneMaterials();

	private:
		Model*                        m_model     = nullptr;
		std::string                   m_modelKey;
		std::string                   m_modelPath;
		Render::RenderPassBase::Layer m_layer     = Render::RenderPassBase::Layer::Opaque;
		std::vector<std::string>      m_albedoTexturePaths;
		std::vector<std::string>      m_normalTexturePaths;

		// per-instance material (Model의 원본을 Clone하여 소유)
		std::vector<std::unique_ptr<Render::Material>> m_materials;
	};
}
