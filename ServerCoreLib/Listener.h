#pragma once
#include "IocpObject.h"
#include "NetAddress.h"

class OverlappedAccept;
class OverlappedEx;

class Listener : public IocpObject
{
public:
	Listener() = default;
	virtual ~Listener();

public:
	bool StartAccept(RefServerService refServerService);
	void CloseSocket();

public:
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(OverlappedEx* pOverlappedEx, int32 numofBytes) override;

private:
	void	RegisterAccept(OverlappedAccept* pAccept);
	void	ProcessAccept(OverlappedAccept* pAccept);

private:
	SOCKET								m_socket = INVALID_SOCKET;
	exvector<class OverlappedAccept*>	m_OverlappedAccepts;
	RefServerService					m_ServerService;
};

