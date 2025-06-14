#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"
#include <iostream>
using namespace std;


/*--------------
	IocpCore
---------------*/

IocpCore::IocpCore()
{
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	assert(_iocpHandle != INVALID_HANDLE_VALUE);
}

IocpCore::~IocpCore()
{
	StopWorkerThreads();
	::CloseHandle(_iocpHandle);
}

bool IocpCore::Register(IocpObjectRef iocpObject)
{
	return ::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, /*key*/0, 0);
}

bool IocpCore::Dispatch(uint32 timeoutMs)
{
	DWORD numOfBytes = 0;
	ULONG_PTR key = 0;
	IocpEvent* iocpEvent = nullptr;

	if (::GetQueuedCompletionStatus(_iocpHandle, OUT &numOfBytes, OUT reinterpret_cast<PULONG_PTR>(&key), OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs))
	{
		IocpObjectRef iocpObject = iocpEvent->owner;
		iocpObject->Dispatch(iocpEvent, numOfBytes);
	}
	else
	{
		int32 errCode = ::WSAGetLastError();
		switch (errCode)
		{
		case WAIT_TIMEOUT:
			return false;
		default:
			IocpObjectRef iocpObject = iocpEvent->owner;
			iocpObject->Dispatch(iocpEvent, numOfBytes);
			break;
		}
	}

	return true;
}

void IocpCore::StartWorkerThreads(int32 threadCount)
{
	cout << "StartWorkerThreads called with threadCount: " << threadCount << endl;
	
	// 이미 실행 중이면 중단
	if (_workerThreads.empty() == false)
	{
		cout << "Worker threads already running. Count: " << _workerThreads.size() << endl;
		return;
	}

	_stopFlag = false;
	cout << "Creating " << threadCount << " worker threads..." << endl;

	// 워커 스레드 생성
	for (int32 i = 0; i < threadCount; ++i)
	{
		cout << "Creating worker thread " << (i + 1) << "/" << threadCount << endl;
		_workerThreads.emplace_back([this, i]() { 
			cout << "Worker thread " << i << " started" << endl;
			WorkerThreadMain(); 
			cout << "Worker thread " << i << " ended" << endl;
		});
	}

	cout << "IOCP Worker Threads Started: " << threadCount << endl;
}

void IocpCore::StopWorkerThreads()
{
	_stopFlag = true;

	// 모든 워커 스레드에 종료 신호 전송
	for (size_t i = 0; i < _workerThreads.size(); ++i)
	{
		PostQueuedCompletionStatus(_iocpHandle, 0, 0, nullptr);
	}

	// 워커 스레드 종료 대기
	for (auto& thread : _workerThreads)
	{
		if (thread.joinable())
		{
			thread.join();
		}
	}

	_workerThreads.clear();
	cout << "IOCP Worker Threads Stopped" << endl;
}

void IocpCore::WorkerThreadMain()
{
	cout << "WorkerThreadMain started" << endl;
	
	while (_stopFlag == false)
	{
		DWORD numOfBytes = 0;
		ULONG_PTR key = 0;
		IocpEvent* iocpEvent = nullptr;

		if (::GetQueuedCompletionStatus(_iocpHandle, OUT &numOfBytes, OUT reinterpret_cast<PULONG_PTR>(&key), OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), INFINITE))
		{
			// 종료 신호 확인
			if (iocpEvent == nullptr && numOfBytes == 0 && key == 0)
			{
				cout << "Worker thread received shutdown signal" << endl;
				break;
			}

			if (iocpEvent != nullptr)
			{
				IocpObjectRef iocpObject = iocpEvent->owner;
				if (iocpObject != nullptr)
				{
					iocpObject->Dispatch(iocpEvent, numOfBytes);
				}
			}
		}
		else
		{
			int32 errCode = ::WSAGetLastError();
			switch (errCode)
			{
			case WAIT_TIMEOUT:
				continue;
			default:
				if (iocpEvent != nullptr)
				{
					IocpObjectRef iocpObject = iocpEvent->owner;
					if (iocpObject != nullptr)
					{
						iocpObject->Dispatch(iocpEvent, numOfBytes);
					}
				}
				break;
			}
		}
	}
	
	cout << "WorkerThreadMain ended" << endl;
}
