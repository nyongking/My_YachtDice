#pragma once

namespace Render
{
	class ConstantBufferParameter
	{
	public:
		ConstantBufferParameter(std::string name, uint32_t dataSize, void* pData, std::shared_ptr<class ConstantBuffer> buffer)
			: m_name(std::move(name))
			, m_dataSize(dataSize)
			, m_pData(pData)
			, m_buffer(std::move(buffer))
		{
		}

		ConstantBufferParameter(const ConstantBufferParameter& rhs)
			: m_name(rhs.m_name)
			, m_dataSize(rhs.m_dataSize)
			, m_buffer(rhs.m_buffer)
			, m_pData(nullptr)
		{
		}

		ConstantBufferParameter(ConstantBufferParameter&& rhs) noexcept
			: m_name(std::move(rhs.m_name))
			, m_dataSize(rhs.m_dataSize)
			, m_pData(rhs.m_pData)
			, m_buffer(std::move(rhs.m_buffer))
			, m_dirty(rhs.m_dirty)
		{
			rhs.m_pData = nullptr;
		}

	public:
		class ConstantBuffer*	Buffer()       { return m_buffer.get(); }
		const std::string&		Name()   const { return m_name; }
		uint32_t				DataSize() const { return m_dataSize; }
		void					MarkDirty()    { m_dirty = true; }

		bool ChangeAddress(void* pData, uint32_t size);
		bool WriteValue(const void* src, uint32_t size);

		void Update(ID3D11DeviceContext* ctx);

	private:
		std::string						m_name;
		uint32_t						m_dataSize = 0;
		void*							m_pData    = nullptr;
		std::shared_ptr<class ConstantBuffer>	m_buffer;
		bool							m_dirty    = true;
	};
}
