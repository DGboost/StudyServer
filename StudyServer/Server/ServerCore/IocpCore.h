#pragma once
#include <thread>
#include <vector>
#include <atomic>

/*----------------
	IocpObject
-----------------*/

class IocpObject : public enable_shared_from_this<IocpObject>
{
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(struct IocpEvent* iocpEvent, int32 numOfBytes = 0) abstract;
};

/*--------------
	IocpCore
---------------*/

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	HANDLE		GetHandle() { return _iocpHandle; }

	bool		Register(IocpObjectRef iocpObject);
	bool		Dispatch(uint32 timeoutMs = INFINITE);

	// 워커 스레드 관리
	void		StartWorkerThreads(int32 threadCount);
	void		StopWorkerThreads();

private:
	void		WorkerThreadMain();

private:
	HANDLE		_iocpHandle;
	
	// 워커 스레드 관련
	vector<thread>	_workerThreads;
	atomic<bool>	_stopFlag = false;
};