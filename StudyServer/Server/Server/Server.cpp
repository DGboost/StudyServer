#include "pch.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
using namespace std;
#include <atomic>
#include <mutex>
#include "ThreadManager.h"
#include "SocketUtils.h"
#include "Listener.h"

// 멀티 스레드 설정
const int32 WORKER_THREAD_COUNT = 4; // 6코어 시스템에서 4개 워커 스레드 사용

#include "Service.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ServerPacketHandler.h"
#include "GameRoom.h"

int main()
{	cout << "=== Server Initialization ===" << endl;
	
	SocketUtils::Init();
	cout << "SocketUtils initialized" << endl;
	
	GRoom->Init();
	cout << "GameRoom initialized" << endl;	ServerServiceRef service = make_shared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		make_shared<IocpCore>(),
		[](){ return make_shared<GameSession>(); }, // TODO : SessionManager 등
		100);
	
	cout << "ServerService created" << endl;
	assert(service->Start());
	cout << "ServerService started" << endl;
	// Phase 1 테스트: 워커 스레드 시작
	cout << "=== IOCP Worker Thread Test Start ===" << endl;
	int32 workerThreadCount = 4; // 6코어 중 4개 사용
	service->GetIocpCore()->StartWorkerThreads(workerThreadCount);
	cout << "Worker threads started successfully!" << endl;

	uint64 lastUpdate = GetTickCount64();	const uint64 UPDATE_INTERVAL = 33; // 30 FPS로 게임 로직 업데이트
	
	cout << "=== Multi-threaded Server Started ===" << endl;
	cout << "[MAIN] Game logic thread: 30 FPS update loop" << endl;
	cout << "[IOCP] Worker threads: " << WORKER_THREAD_COUNT << " threads handling I/O" << endl;
	
	uint64 frameCount = 0;
	uint64 lastFpsReport = lastUpdate;

	while (true)
	{
		uint64 now = GetTickCount64();
		
		// 게임 로직 업데이트 (30 FPS)
		if (now - lastUpdate >= UPDATE_INTERVAL)
		{
			GRoom->Update();
			lastUpdate = now;
			frameCount++;
			
			// 성능 모니터링 (5초마다 FPS 출력)
			if (now - lastFpsReport >= 5000)
			{
				double fps = frameCount * 1000.0 / (now - lastFpsReport);
				cout << "[MAIN] Game logic FPS: " << fixed << setprecision(1) << fps << endl;
				frameCount = 0;
				lastFpsReport = now;
			}
		}

		// 메인 스레드 CPU 사용률 최적화
		Sleep(1);
	}

	cout << "=== Server Shutdown ===" << endl;
	cout << "Worker threads will be stopped automatically in IocpCore destructor" << endl;

	GThreadManager->Join();
	
	// 윈속 종료
	SocketUtils::Clear();	
}