#include "CorePch.h"
#include "BufferWriter.h"

BufferWriter::BufferWriter()
{
}

BufferWriter::BufferWriter(BYTE* pBuffer, uint32 size, uint32 pos)
	: m_pBuffer(pBuffer)
	, m_size(size)
	, m_pos(pos)
{
}

BufferWriter::~BufferWriter()
{
}

bool BufferWriter::Write(void* pSrc, uint32 len)
{
	if (FreeSize() < len)
		return false;

	memcpy(&m_pBuffer[m_pos], pSrc, len);
	m_pos += len;
	return true;
}
