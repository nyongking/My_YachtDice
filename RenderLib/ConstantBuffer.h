#pragma once

namespace Render
{
	class ConstantBuffer
	{
	public:
		ConstantBuffer() = default;
		~ConstantBuffer() = default;

		bool Create(ID3D11Device* device, uint32_t byteWidth, const void* pInitialData, D3D11_USAGE usage = D3D11_USAGE_DYNAMIC);

		bool Update(ID3D11DeviceContext* context, const void* data, uint32_t byteWidth, uint32_t offset = 0);

		void BindVS(ID3D11DeviceContext* context, uint32_t slot = 0) const;
		void BindPS(ID3D11DeviceContext* context, uint32_t slot = 0) const;

		ID3D11Buffer* Get() const { return m_buffer.Get(); }
		bool IsCreated() const { return m_buffer.Get() != nullptr; }
		uint32_t Size() const { return m_size; }

	public:
		bool WasLastUpdatedBy(const void* who) const { return m_lastUpdater == who; }
		void SetLastUpdater(const void* who) { m_lastUpdater = who; }

	private:
		static UINT Align16(UINT size) { return (size + 15u) & ~15u; }

	private:
		RefCom<ID3D11Buffer> m_buffer;

		uint32_t m_size = 0;
		D3D11_USAGE m_usage = D3D11_USAGE_DEFAULT;
		const void* m_lastUpdater = nullptr;
	};
}

