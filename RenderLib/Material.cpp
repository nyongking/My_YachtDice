#include "RenderPch.h"
#include "Material.h"
#include "Texture.h"
#include "ConstantBuffer.h"

namespace Render
{
	Material::Material(const Material& rhs)
		: m_shaderGroup(rhs.m_shaderGroup)
		, m_constantBufferParameters(rhs.m_constantBufferParameters)
		, m_constantBufferSlots(rhs.m_constantBufferSlots)
		, m_textureSlots(rhs.m_textureSlots)
	{
	}

	Material::~Material()
	{
		m_textureSlots.clear();
		m_constantBufferSlots.clear();
		m_constantBufferParameters.clear();
	}

	bool Material::BindMaterial(ID3D11DeviceContext* pContext)
	{
		if (nullptr == pContext)
			return false;

		for (auto& param : m_constantBufferParameters)
			param.Update(pContext);

		for (auto& slot : m_constantBufferSlots)
			slot.BindBufferToSlot(pContext);

		for (auto& slot : m_textureSlots)
			slot.Bind(pContext);

		return true;
	}

	int Material::RegisterConstantBufferSlot(class ConstantBuffer* pBuffer)
	{
		if (nullptr == pBuffer)
			return -1;

		m_constantBufferSlots.push_back(ConstantBufferSlot(pBuffer));

		return static_cast<int>(m_constantBufferSlots.size() - 1);
	}

	bool Material::ConnectConstantBufferToVSSlot(const std::string& name, int index, ShaderGroup* pShaderGroup)
	{
		if (index < 0 || index >= static_cast<int>(m_constantBufferSlots.size()))
			return false;

		ConstantBufferSlot& dat = m_constantBufferSlots[index];

		return dat.ConnectVSSlot(name, pShaderGroup);
	}

	bool Material::ConnectConstantBufferToPSSlot(const std::string& name, int index, ShaderGroup* pShaderGroup)
	{
		if (index < 0 || index >= static_cast<int>(m_constantBufferSlots.size()))
			return false;

		ConstantBufferSlot& dat = m_constantBufferSlots[index];

		return dat.ConnectPSSlot(name, pShaderGroup);
	}

	bool Material::AutomaticRegisterVSTexture(const std::string& name, const std::string& slotname, ShaderGroup* sg)
	{
		if (!sg)
			return false;

		int slotVS = sg->VSTextureSlot(slotname);
		m_textureSlots.push_back(TextureSlot(name, slotVS, -1));

		return slotVS >= 0;
	}

	bool Material::AutomaticRegisterPSTexture(const std::string& name, const std::string& slotname, ShaderGroup* sg)
	{
		if (!sg)
			return false;

		int slotPS = sg->PSTextureSlot(slotname);
		m_textureSlots.push_back(TextureSlot(name, -1, slotPS));

		return slotPS >= 0;
	}

	void Material::MarkParameterDirty(uint32_t index)
	{
		if (index < m_constantBufferParameters.size())
			m_constantBufferParameters[index].MarkDirty();
	}

	bool Material::SetTexture(const std::string& name, Texture* tex)
	{
		return SetTexture(name, tex ? tex->SRV() : nullptr);
	}

	bool Material::SetTexture(const std::string& name, ID3D11ShaderResourceView* srv)
	{
		for (auto& slot : m_textureSlots)
		{
			if (slot.Name() == name)
			{
				slot.SetSRV(srv);
				return true;
			}
		}
		return false;
	}
}

