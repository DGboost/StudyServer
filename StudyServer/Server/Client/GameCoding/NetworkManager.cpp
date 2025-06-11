#include "pch.h"
#include "NetworkManager.h"
#include "Service.h"
#include "ThreadManager.h"
#include "ServerSession.h"

void NetworkManager::Init()
{
	SocketUtils::Init();

	_service = make_shared<ClientService>(
		NetAddress(L"127.0.0.1", 7777),
		make_shared<IocpCore>(),
		[=]() { return CreateSession(); }, // TODO : SessionManager ��
		1);

	assert(_service->Start());

	/*for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([=]()
			{
				while (true)
				{
					service->GetIocpCore()->Dispatch();
				}
			});
	}*/
}

void NetworkManager::Update()
{
	// 더 많은 패킷을 한 번에 처리하여 응답성 향상
	for (int i = 0; i < 10; ++i)
	{
		if (!_service->GetIocpCore()->Dispatch(0))
			break;
	}
}

ServerSessionRef NetworkManager::CreateSession()
{
	return _session = make_shared<ServerSession>();
}

void NetworkManager::SendPacket(SendBufferRef sendBuffer)
{
	if (_session)
		_session->Send(sendBuffer);
}
