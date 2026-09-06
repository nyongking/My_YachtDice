#include "ServerCorePch.h"
#include "SendBuffer.h"

SendBuffer::SendBuffer(RefSendBufferChunk refOwner, BYTE* pBuffer, uint32 allocSize)
	: m_refOwner(refOwner)
	, m_pBuffer(pBuffer)
	, m_allocSize(allocSize)
{
}

SendBuffer::~SendBuffer()
{
}

void SendBuffer::Close(uint32 writeSize)
{
	ASSERT_TRIG_CRASH(m_allocSize >= writeSize);
	m_writeSize = writeSize;
	m_refOwner->Close(writeSize);
}

RefSendBuffer SendBufferManager::Open(uint32 size)
{
	if (nullptr == LrefSendBufferChunk)
	{
		LrefSendBufferChunk = Pop();
		LrefSendBufferChunk->Reset();
	}

	ASSERT_TRIG_CRASH(false == LrefSendBufferChunk->IsOpen());

	if (LrefSendBufferChunk->FreeSize() < size)
	{
		LrefSendBufferChunk = Pop();
		LrefSendBufferChunk->Reset();
	}

	return LrefSendBufferChunk->Open(size);
}

RefSendBufferChunk SendBufferManager::Pop()
{
	{
		WRITE_LOCK;
		if (false == m_sendBufferChunks.empty())
		{
			RefSendBufferChunk refSendBufferChunk = m_sendBufferChunks.back();
			m_sendBufferChunks.pop_back();
			return refSendBufferChunk;
		}
	}

	return RefSendBufferChunk(exnew<SendBufferChunk>(), PushGlobal);
}

void SendBufferManager::Push(RefSendBufferChunk refBufferChunk)
{
	WRITE_LOCK;

	m_sendBufferChunks.push_back(refBufferChunk);
}

void SendBufferManager::PushGlobal(SendBufferChunk* pBufferChunk)
{
	GSendBufferManager->Push(RefSendBufferChunk(pBufferChunk, PushGlobal));
}

/*--------------- Chunk -----------------*/

SendBufferChunk::SendBufferChunk()
{
}

SendBufferChunk::~SendBufferChunk()
{
}

void SendBufferChunk::Reset()
{
	m_isOpen = false;
	m_usedSize = 0;
}

RefSendBuffer SendBufferChunk::Open(uint32 allocSize)
{
	ASSERT_TRIG_CRASH(allocSize <= SEND_BUFFER_CHUNK_SIZE);
	ASSERT_TRIG_CRASH(m_isOpen == false);

	if (allocSize > FreeSize())
		return nullptr;

	m_isOpen = true;
	return ObjectPool<SendBuffer>::MakeShared(shared_from_this(), Buffer(), allocSize);
}

void SendBufferChunk::Close(uint32 writeSize)
{
	ASSERT_TRIG_CRASH(m_isOpen == true);
	m_isOpen = false;

	m_usedSize += writeSize;
}
