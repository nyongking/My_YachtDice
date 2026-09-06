#include "CorePch.h"
#include "BufferReader.h"

BufferReader::BufferReader()
{
}

BufferReader::BufferReader(BYTE* pBuffer, uint32 size, uint32 pos)
	: m_pBuffer(pBuffer)
	, m_size(size)
	, m_pos(pos)
{
}

BufferReader::~BufferReader()
{
}

bool BufferReader::Peek(void* pDest, uint32 len)
{
	if (FreeSize() < len)
		return false;

	memcpy(pDest, &m_pBuffer[m_pos], len);
	return true;
}

bool BufferReader::Read(void* pDest, uint32 len)
{
	if (false == Peek(pDest, len))
		return false;

	m_pos += len;
	return true;
}
