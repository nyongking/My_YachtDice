#pragma once
#include "UIComponent.h"
#include <vector>

namespace Render { class Texture; }

namespace GameEngine
{
	struct UIPanelElement
	{
		float2 localOffset = {};
		float2 size        = { 51.f, 51.f };
		float  rotation    = 0.f;
		float4 uvRect      = { 0.f, 0.f, 1.f, 1.f };
		float4 tintColor   = { 1.f, 1.f, 1.f, 1.f };
	};

	class UIPanelComponent : public UIComponent
	{
	public:
		void Awake()              override;
		void LateUpdate(float dt) override;

		std::string GetTypeName() const override { return "UIPanelComponent"; }
		MyJson      Serialize()   const override;
		void        Deserialize(const MyJson& j) override;

		void SetTexture(const std::string& key, const std::wstring& path);

		void SetVisible(bool v) { m_visible = v; }
		bool IsVisible() const  { return m_visible; }

		void AddElement(const UIPanelElement& elem);
		void SetElement(int index, const UIPanelElement& elem);
		void ClearElements();
		UIPanelElement&       GetElement(int index);
		const UIPanelElement& GetElement(int index) const;
		int  GetElementCount() const { return static_cast<int>(m_elements.size()); }

#ifdef _DEBUG
		void OnInspectorGUI() override;
#endif

	private:
		void EnsureMaterials();

		bool m_visible = true;

		Render::Texture*  m_texture = nullptr;
		std::string       m_textureKey;
		std::wstring      m_texturePath;

		Render::Geometry* m_quad = nullptr;

		std::vector<UIPanelElement>                    m_elements;
		std::vector<std::unique_ptr<Render::Material>> m_materials;
	};
}
