#include "ServerCorePch.h"
#include "RecvBuffer.h"

const int BUFFER_MULTIPLIED = 10;

RecvBuffer::RecvBuffer(int32 bufferSize)
	: m_bufferSize(bufferSize)
	, m_capacity(bufferSize* BUFFER_MULTIPLIED)
	, m_buffer(bufferSize* BUFFER_MULTIPLIED)
{
}

RecvBuffer::~RecvBuffer()
{
}

void RecvBuffer::Clear()
{
	int32 dataSize = DataSize();
	if (0 == dataSize)
	{
		m_readPos = m_writePos = 0;
	}
	else if (FreeSize() < m_bufferSize)
	{
		memcpy(&m_buffer[0], &m_buffer[m_readPos], dataSize);
		m_readPos = 0;
		m_writePos = dataSize;

	}
}

bool RecvBuffer::OnRead(int32 numofBytes)
{
	// 유효한 데이터보다 더 크게 읽는 경우
	if (DataSize() < numofBytes)
		return false;

	m_readPos += numofBytes;


	return true;
}

bool RecvBuffer::OnWrite(int32 numofBytes)
{
	// 쓰려는 데이터보다 남는 공간이 더 작은 경우
	if (FreeSize() < numofBytes)
		return false;

	m_writePos += numofBytes;


	return true;
}
