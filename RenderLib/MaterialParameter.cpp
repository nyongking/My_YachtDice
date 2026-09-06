#include "RenderPch.h"
#include "MaterialParameter.h"
#include "ConstantBuffer.h"

namespace Render
{
	bool ConstantBufferParameter::ChangeAddress(void* pData, uint32_t size)
	{
		if (nullptr == pData || !m_buffer || size != m_buffer->Size())
			return false;

		m_pData = pData;
		m_dirty = true;

		return true;
	}
	bool ConstantBufferParameter::WriteValue(const void* src, uint32_t size)
	{
		if (!src || !m_pData || size != m_dataSize)
			return false;

		memcpy(m_pData, src, size);
		m_dirty = true;
		return true;
	}

	void ConstantBufferParameter::Update(ID3D11DeviceContext* ctx)
	{
		if (nullptr == ctx || !m_buffer || nullptr == m_pData)
			return;

		if (!m_dirty && m_buffer->WasLastUpdatedBy(this))
			return;

		m_buffer->Update(ctx, m_pData, m_buffer->Size());
		m_buffer->SetLastUpdater(this);
		m_dirty = false;
	}
}
