#pragma once
class BufferWriter
{
public:
	BufferWriter();
	BufferWriter(BYTE* pBuffer, uint32 size, uint32 pos = 0);
	~BufferWriter();

	BYTE* Buffer() { return m_pBuffer; }
	uint32 Size() { return m_size; }
	uint32 WriteSize() { return m_pos; }
	uint32 FreeSize() { return m_size - m_pos; }

	template<typename T>
	bool Write(T* pSrc) { return Write(pSrc, sizeof(T)); }
	bool Write(void* pSrc, uint32 len);

	template<typename T>
	T* Reserve(uint16 count = 1);

	template<typename T>
	BufferWriter& operator<<(const T& src);

	template<typename T>
	BufferWriter& operator<<(T&& src);

private:
	BYTE* m_pBuffer = nullptr;
	uint32		m_size = 0;
	uint32		m_pos = 0;
};

template<typename T>
inline T* BufferWriter::Reserve(uint16 count)
{
	if (FreeSize() < sizeof(T) * count)
		return nullptr;

	T* ret = reinterpret_cast<T*>(&m_pBuffer[m_pos]);
	m_pos += sizeof(T) * count;
	return ret;
}

template<typename T>
inline BufferWriter& BufferWriter::operator<<(const T& src)
{
	if (FreeSize() < sizeof(T))
		return *this;

	*reinterpret_cast<T*>(&m_pBuffer[m_pos]) = src;
	m_pos += sizeof(T);
	return *this;
}

template<typename T>
inline BufferWriter& BufferWriter::operator<<(T&& src)
{
	using DataType = std::remove_reference_t<T>;

	if (FreeSize() < sizeof(T))
		return *this;

	*reinterpret_cast<DataType*>(&m_pBuffer[m_pos]) = std::forward<DataType>(src);
	m_pos += sizeof(T);
	return *this;
}


