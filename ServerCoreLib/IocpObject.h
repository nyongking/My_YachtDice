#pragma once

class IocpObject : public std::enable_shared_from_this<IocpObject>
{
public:
	virtual ~IocpObject() = default;

public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class OverlappedEx* pOverlappedEx, int32 numofBytes = 0) abstract;
};

