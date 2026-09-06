#pragma once

class RecvBuffer
{
public:
	RecvBuffer(int32 bufferSize);
	~RecvBuffer();

	void Clear();
	bool OnRead(int32 numofBytes);
	bool OnWrite(int32 numofBytes);

	BYTE* ReadPos() { return &m_buffer[m_readPos]; }
	BYTE* WritePos() { return &m_buffer[m_writePos]; }
	int32 DataSize() { return m_writePos - m_readPos; }
	int32 FreeSize() { return m_capacity - m_writePos; }

private:
	int32			m_capacity = 0;
	int32			m_bufferSize = 0;
	int32			m_readPos = 0;		// Recv 이후 데이터 처리를 시작할 위치
	int32			m_writePos = 0;		// 데이터를 밀어넣을 위치
	exvector<BYTE>	m_buffer;

};

