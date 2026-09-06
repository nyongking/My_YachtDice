#pragma once
#include "Material.h"

namespace Render
{
	// G-Buffer Pass용 Material
	// VTXPOSNORMTANUV 버텍스 + Albedo 텍스처 → RT0(Albedo), RT1(Normal), RT2(WorldPos)
	class GBufferMaterial : public Material
	{
	public:
		GBufferMaterial()  = default;
		~GBufferMaterial() = default;

		GBufferMaterial(const GBufferMaterial& rhs);

	public:
		bool Initialize(ShaderGroup* shaderGroup, ID3D11Device* device) override;
		std::unique_ptr<Material> Clone() const override;

		void   SetAlbedoColor(const float4& color)           { m_materialData.color = color; MarkParameterDirty(0); }
		float4 GetAlbedoColor() const                        { return m_materialData.color; }
		void   SetUVTiling(const float2& tiling)             { m_materialData.uvTiling = tiling; MarkParameterDirty(0); }
		float2 GetUVTiling() const                           { return m_materialData.uvTiling; }
		void   SetUVOffset(const float2& offset)             { m_materialData.uvOffset = offset; MarkParameterDirty(0); }
		float2 GetUVOffset() const                           { return m_materialData.uvOffset; }
		void   SetShininess(float v)                         { m_materialData.shininess = v; MarkParameterDirty(0); }
		float  GetShininess() const                          { return m_materialData.shininess; }
		void   SetSpecularStrength(float v)                  { m_materialData.specularStrength = v; MarkParameterDirty(0); }
		float  GetSpecularStrength() const                   { return m_materialData.specularStrength; }
		bool SetAlbedoTexture(Texture* tex)                  { return SetTexture("AlbedoMap", tex); }
		bool SetAlbedoTexture(ID3D11ShaderResourceView* srv) { return SetTexture("AlbedoMap", srv); }
		bool SetNormalTexture(Texture* tex)                  { return SetTexture("NormalMap", tex); }
		bool SetNormalTexture(ID3D11ShaderResourceView* srv) { return SetTexture("NormalMap", srv); }

	private:
		struct MaterialData
		{
			float4 color             = { 1.f, 1.f, 1.f, 1.f };
			float2 uvTiling          = { 1.f, 1.f };
			float2 uvOffset          = { 0.f, 0.f };
			float  shininess         = 32.f;
			float  specularStrength  = 0.5f;
			float2 _pad              = {};
		};

		MaterialData m_materialData = {};
	};
}
