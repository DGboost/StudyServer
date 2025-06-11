#include "pch.h"
#include <iostream>
#include <thread>
#include <vector>
using namespace std;
#include <atomic>
#include <mutex>
#include "ThreadManager.h"
#include "SocketUtils.h"
#include "Listener.h"
#include "Service.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ServerPacketHandler.h"
#include "GameRoom.h"

int main()
{
	SocketUtils::Init();
	GRoom->Init();

	ServerServiceRef service = make_shared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		make_shared<IocpCore>(),
		[](){ return make_shared<GameSession>(); }, // TODO : SessionManager 등
		100);
	assert(service->Start());
	uint64 lastUpdate = GetTickCount64();
	const uint64 UPDATE_INTERVAL = 16; // 16ms마다 업데이트 (약 60fps)

	while (true)
	{
		service->GetIocpCore()->Dispatch(0);
		
		uint64 now = GetTickCount64();
		if (now - lastUpdate >= UPDATE_INTERVAL)
		{
			GRoom->Update();
			lastUpdate = now;
		}
		
		// CPU 사용률 줄이기 위한 짧은 sleep
		Sleep(1);
	}

	GThreadManager->Join();
	
	// 윈속 종료
	SocketUtils::Clear();	
}