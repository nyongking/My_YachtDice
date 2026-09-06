#pragma once

namespace Render
{	
	class ConstantBufferSlot
	{
	public:
		ConstantBufferSlot(class ConstantBuffer* pBuffer)
			: m_pBuffer(pBuffer)
		{

		}

		ConstantBufferSlot(const ConstantBufferSlot& rhs)
			: m_slotVS(rhs.m_slotVS)
			, m_slotPS(rhs.m_slotPS)
			, m_pBuffer(rhs.m_pBuffer)
		{

		}

	public:
		bool			ConnectVSSlot(const std::string& name, const class ShaderGroup* pShaderGroup);
		bool			ConnectPSSlot(const std::string& name, const class ShaderGroup* pShaderGroup);

		bool			BindBufferToSlot(ID3D11DeviceContext* pContext);

	private:
		int				m_slotVS = -1;
		int				m_slotPS = -1;
		class ConstantBuffer* const m_pBuffer = nullptr;
	};

}

