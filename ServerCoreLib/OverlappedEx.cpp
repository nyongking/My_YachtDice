#include "ServerCorePch.h"
#include "OverlappedEx.h"

OverlappedEx::OverlappedEx(NetworkEventType type)
	: m_EventType(type)
{
	Init();
}

void OverlappedEx::Init()
{
	OVERLAPPED::Internal = 0;
	OVERLAPPED::InternalHigh = 0;
	OVERLAPPED::Offset = 0;
	OVERLAPPED::OffsetHigh = 0;
	OVERLAPPED::hEvent = 0;
}
