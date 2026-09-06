#pragma once

class BufferReader
{
public:
	BufferReader();
	BufferReader(BYTE* pBuffer, uint32 size, uint32 pos = 0);
	~BufferReader();

	BYTE* Buffer() { return m_pBuffer; }
	uint32 Size() { return m_size; }
	uint32 ReadSize() { return m_pos; }
	uint32 FreeSize() { return m_size - m_pos; }

	template<typename T>
	bool Peek(T* pDest) { return Peek(pDest, sizeof(T)); }
	bool Peek(void* pDest, uint32 len);

	template<typename T>
	bool Read(T* pDest) { return Read(pDest, sizeof(T)); }
	bool Read(void* pDest, uint32 len);

	template<typename T>
	BufferReader& operator>>(OUT T& dest);


private:
	BYTE* m_pBuffer = nullptr;
	uint32		m_size = 0;
	uint32		m_pos = 0;
};

template<typename T>
inline BufferReader& BufferReader::operator>>(OUT T& dest)
{
	dest = *reinterpret_cast<T*>(&m_pBuffer[m_pos]);
	m_pos += sizeof(T);
	return *this;
}
