#include "RenderPch.h"
#include "ConstantBuffer.h"

namespace Render
{


	bool ConstantBuffer::Create(ID3D11Device* device, uint32_t byteWidth, const void* pInitialData, D3D11_USAGE usage)
	{
		if (nullptr == device || byteWidth == 0)
			return false;

		uint32_t aligned = Align16(byteWidth);

		D3D11_BUFFER_DESC desc{};
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.ByteWidth = aligned;
		desc.StructureByteStride = 0;
		desc.MiscFlags = 0;
		desc.Usage = usage;
		desc.CPUAccessFlags = (usage == D3D11_USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;

		ID3D11Buffer* buf = nullptr;
		HRESULT hr;

		if (pInitialData)
		{
			D3D11_SUBRESOURCE_DATA dat;
			dat.pSysMem = pInitialData;

			hr = device->CreateBuffer(&desc, &dat, &buf);
		}
		else
		{
			hr = device->CreateBuffer(&desc, nullptr, &buf);
		}

		if (FAILED(hr) || nullptr == buf)
			return false;

		m_buffer.Attach(buf);

		m_size = aligned;
		m_usage = usage;

		return true;
	}

	bool ConstantBuffer::Update(ID3D11DeviceContext* context, const void* data, uint32_t byteWidth, uint32_t offset)
	{
		if (nullptr == context || nullptr == m_buffer.Get() || nullptr == data)
			return false;

		if (byteWidth == 0 || byteWidth > m_size)
			return false;

		// dynamic
		if (m_usage == D3D11_USAGE_DYNAMIC)
		{
			D3D11_MAPPED_SUBRESOURCE mapped{};
			HRESULT hr = context->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

			if (FAILED(hr))
				return false;

			BYTE* dest = static_cast<BYTE*>(mapped.pData) + offset;
			memcpy(dest, data, byteWidth);

			context->Unmap(m_buffer.Get(), 0);
		}
		// default
		else
		{
			context->UpdateSubresource(m_buffer.Get(), 0, nullptr, data, 0, 0);
		}

		return true;
	}

	void ConstantBuffer::BindVS(ID3D11DeviceContext* context, uint32_t slot) const
	{
		if (nullptr == context || nullptr == m_buffer.Get())
			return;

		context->VSSetConstantBuffers(slot, 1, m_buffer.GetAddressOf());
	}

	void ConstantBuffer::BindPS(ID3D11DeviceContext* context, uint32_t slot) const
	{
		if (nullptr == context || nullptr == m_buffer.Get())
			return;

		context->PSSetConstantBuffers(slot, 1, m_buffer.GetAddressOf());
	}

}
