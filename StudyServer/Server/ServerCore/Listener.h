#pragma once
#include "IocpCore.h"
#include "NetAddress.h"

class AcceptEvent;

/*--------------
	Listener
---------------*/

class Listener : public IocpObject
{
public:
	Listener() = default;
	~Listener();

public:
	/* 세션 시작 */
	bool StartAccept(ServerServiceRef service);
	void CloseSocket();

public:
	/* IocpObject 관련 */
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(struct IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

private:
	/* 내부 함수 */
	void RegisterAccept(IocpEvent* acceptEvent);
	void ProcessAccept(IocpEvent* acceptEvent);

protected:
	SOCKET _socket = INVALID_SOCKET;
	vector<IocpEvent*> _acceptEvents;
	ServerServiceRef _service;
};

