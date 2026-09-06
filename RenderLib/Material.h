#pragma once
#include "ShaderGroup.h"
#include "Materialparameter.h"
#include "MaterialSlot.h"
#include "ConstantBuffer.h"
#include "TextureSlot.h"

namespace Render
{
	class Texture;

	class Material
	{
	public:
		Material() = default;
		Material(const Material& rhs);
		virtual ~Material();

	public:
		virtual bool Initialize(ShaderGroup* shaderGroup, ID3D11Device* device) abstract;
		virtual std::unique_ptr<Material> Clone() const = 0;

		bool		 BindMaterial(ID3D11DeviceContext* pContext);

		ShaderGroup* GetShaderGroup() const { return m_shaderGroup; }

		virtual void SetLights(const struct LightData* lights, int count) {}

		template<typename T>
		bool SetParam(const std::string& name, const T& value)
		{
			for (auto& param : m_constantBufferParameters)
			{
				if (param.Name() == name)
					return param.WriteValue(&value, sizeof(T));
			}
			return false;
		}

		bool SetTexture(const std::string& name, Texture* tex);
		bool SetTexture(const std::string& name, ID3D11ShaderResourceView* srv);

	protected:
		template<typename T>
		int			RegisterConstantBuffer(const std::string& name, T* const pData, bool isDynamic = true);
		int			RegisterConstantBufferSlot(class ConstantBuffer* pBuffer);
		bool		ConnectConstantBufferToVSSlot(const std::string& name, int index, class ShaderGroup* pShaderGroup);
		bool		ConnectConstantBufferToPSSlot(const std::string& name, int index, class ShaderGroup* pShaderGroup);

		template<typename T>
		bool		AutomaticRegisterVS(const std::string& name, const std::string& slotname, T* const pData, class ShaderGroup* pShaderGroup, bool isDynamic = true);

		template<typename T>
		bool		AutomaticRegisterPS(const std::string& name, const std::string& slotname, T* const pData, class ShaderGroup* pShaderGroup, bool isDynamic = true);

		template<typename T>
		bool		ChangeParameterAddress(uint32_t index, T* pData);

		bool		AutomaticRegisterVSTexture(const std::string& name, const std::string& slotname, ShaderGroup* sg);
		bool		AutomaticRegisterPSTexture(const std::string& name, const std::string& slotname, ShaderGroup* sg);

		void		MarkParameterDirty(uint32_t index);

	protected:
		ShaderGroup*  m_shaderGroup = nullptr;
		ID3D11Device* m_device      = nullptr;

	private:
		std::vector<ConstantBufferParameter> m_constantBufferParameters;
		std::vector<ConstantBufferSlot>		 m_constantBufferSlots;
		std::vector<TextureSlot>			 m_textureSlots;
	};

	template<typename T>
	inline int Material::RegisterConstantBuffer(const std::string& name, T* const pData, bool isDynamic)
	{
		if (nullptr == pData || nullptr == m_device)
			return -1;

		auto buffer = std::make_shared<class ConstantBuffer>();
		D3D11_USAGE usage = isDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		if (!buffer->Create(m_device, sizeof(T), nullptr, usage))
			return -1;

		m_constantBufferParameters.push_back(ConstantBufferParameter{name, sizeof(T), pData, std::move(buffer)});

		return static_cast<int>(m_constantBufferParameters.size() - 1);
	}

	template<typename T>
	inline bool Material::AutomaticRegisterVS(const std::string& name, const std::string& slotname, T* const pData, ShaderGroup* pShaderGroup, bool isDynamic)
	{
		int index = RegisterConstantBuffer(name, pData, isDynamic);

		if (-1 == index)
			return false;

		int slotIndex = RegisterConstantBufferSlot(m_constantBufferParameters[index].Buffer());

		ConnectConstantBufferToVSSlot(slotname, slotIndex, pShaderGroup);

		return true;
	}

	template<typename T>
	inline bool Material::AutomaticRegisterPS(const std::string& name, const std::string& slotname, T* const pData, ShaderGroup* pShaderGroup, bool isDynamic)
	{
		int index = RegisterConstantBuffer(name, pData, isDynamic);

		if (-1 == index)
			return false;

		int slotIndex = RegisterConstantBufferSlot(m_constantBufferParameters[index].Buffer());

		ConnectConstantBufferToPSSlot(slotname, slotIndex, pShaderGroup);

		return true;
	}

	template<typename T>
	inline bool Material::ChangeParameterAddress(uint32_t index, T* pData)
	{
		if (index >= m_constantBufferParameters.size())
			return false;

		auto& param = m_constantBufferParameters[index];

		if (!param.ChangeAddress(pData, sizeof(T)))
			return false;

		return true;
	}
}
