#pragma once
#include "RenderComponent.h"

namespace Render { class Texture; }

namespace GameEngine
{
	class BillboardComponent : public RenderComponent
	{
	public:
		void Awake() override;
		void LateUpdate(float dt) override;

		void SetOffset(const float3& offset)    { m_offset = offset; }
		const float3& GetOffset() const         { return m_offset; }

		void  SetRotation(float degrees)        { m_rotationDeg = degrees; }
		float GetRotation() const               { return m_rotationDeg; }

		// UIMaterial features
		void SetTexture(const std::string& key, const std::wstring& path);
		void SetTintColor(const float4& color);
		float4 GetTintColor() const;
		void SetUVRect(const float4& rect);
		void SetUVRectPixel(float x, float y, float w, float h, float texW, float texH);
		float4 GetUVRect() const;

		std::string GetTypeName() const override { return "BillboardComponent"; }
		MyJson      Serialize()              const override;
		void        Deserialize(const MyJson& j)   override;

#ifdef _DEBUG
		void OnInspectorGUI() override;
#endif

	private:
		float3 m_offset      = { 0.f, 0.f, 0.f };
		float  m_rotationDeg = 0.f;

		Render::Texture* m_texture    = nullptr;
		std::string      m_textureKey;
		std::wstring     m_texturePath;
	};
}
