#pragma once

class SendBufferChunk;
class SendBuffer/* : public enable_shared_from_this<SendBuffer>*/
{
public:
	SendBuffer(RefSendBufferChunk refOwner, BYTE* pBuffer, uint32 allocSize);
	~SendBuffer();

	BYTE* Buffer() { return m_pBuffer; }
	uint32 AllocSize() { return m_allocSize; }
	uint32 WriteSize() { return m_writeSize; }
	void  Close(uint32 writeSize);

private:
	BYTE* m_pBuffer;
	uint32			m_allocSize = 0;
	uint32			m_writeSize = 0;
	RefSendBufferChunk m_refOwner;
};

class SendBufferChunk : public std::enable_shared_from_this<SendBufferChunk>
{
	enum
	{
		SEND_BUFFER_CHUNK_SIZE = 0x10000,
	};

public:
	SendBufferChunk();
	~SendBufferChunk();

public:
	void Reset();
	RefSendBuffer Open(uint32 allocSize); // 얼마만큼 할당이 필요한가
	void Close(uint32 writeSize);	// 실질적으로 사용할 공간

	bool	IsOpen() { return m_isOpen; }
	BYTE* Buffer() { return &m_buffer[m_usedSize]; }
	uint32	FreeSize() { return static_cast<uint32>(m_buffer.size()) - m_usedSize; }

private:
	exarray<BYTE, SEND_BUFFER_CHUNK_SIZE> m_buffer = {};
	bool		m_isOpen = false;
	uint32		m_usedSize = 0;


};

class SendBufferManager
{
public:
	RefSendBuffer		Open(uint32 size);

private:
	RefSendBufferChunk	Pop();
	void				Push(RefSendBufferChunk refBufferChunk);

	static void			PushGlobal(SendBufferChunk* pBufferChunk);

private:
	USE_LOCK;

	exvector<RefSendBufferChunk> m_sendBufferChunks;
};