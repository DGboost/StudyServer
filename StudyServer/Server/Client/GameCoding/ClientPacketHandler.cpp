#include "pch.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"
#include "DevScene.h"
#include "MyPlayer.h"
#include "SceneManager.h"

void ClientPacketHandler::HandlePacket(ServerSessionRef session, BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	switch (header.id)
	{
	case S_EnterGame:
		Handle_S_EnterGame(session, buffer, len);
		break;
	case S_MyPlayer:
		Handle_S_MyPlayer(session, buffer, len);
		break;
	case S_AddObject:
		Handle_S_AddObject(session, buffer, len);
		break;
	case S_RemoveObject:
		Handle_S_RemoveObject(session, buffer, len);
		break;
	case S_Move:
		Handle_S_Move(session, buffer, len);
		break;
	}
}

void ClientPacketHandler::Handle_S_TEST(ServerSessionRef session, BYTE* buffer, int32 len)
{
	PacketHeader* header = (PacketHeader*)buffer;
	//uint16 id = header->id;
	uint16 size = header->size;

	Protocol::S_TEST pkt;
	pkt.ParseFromArray(&header[1], size - sizeof(PacketHeader));

	uint64 id = pkt.id();
	uint32 hp = pkt.hp();
	uint16 attack = pkt.attack();

	//cout << "ID: " << id << " HP : " << hp << " ATT : " << attack << endl;

	for (int32 i = 0; i < pkt.buffs_size(); i++)
	{
		const Protocol::BuffData& data = pkt.buffs(i);
		//cout << "BuffInfo : " << data.buffid() << " " << data.remaintime() << endl;
	}
}

void ClientPacketHandler::Handle_S_EnterGame(ServerSessionRef session, BYTE* buffer, int32 len)
{
	PacketHeader* header = (PacketHeader*)buffer;
	//uint16 id = header->id;
	uint16 size = header->size;

	Protocol::S_EnterGame pkt;
	pkt.ParseFromArray(&header[1], size - sizeof(PacketHeader));

	bool success = pkt.success();
	uint64 accountId = pkt.accountid();

	// TODO
	
}

void ClientPacketHandler::Handle_S_MyPlayer(ServerSessionRef session, BYTE* buffer, int32 len)
{
	PacketHeader* header = (PacketHeader*)buffer;
	//uint16 id = header->id;
	uint16 size = header->size;

	Protocol::S_MyPlayer pkt;
	pkt.ParseFromArray(&header[1], size - sizeof(PacketHeader));

	//
	const Protocol::ObjectInfo& info = pkt.info();

	DevScene* scene = GET_SINGLE(SceneManager)->GetDevScene();
	if (scene)
	{
		MyPlayer* myPlayer = scene->SpawnObject<MyPlayer>(Vec2Int{info.posx(), info.posy()});
		myPlayer->info = info;
		GET_SINGLE(SceneManager)->SetMyPlayer(myPlayer);
	}
}

void ClientPacketHandler::Handle_S_AddObject(ServerSessionRef session, BYTE* buffer, int32 len)
{
	PacketHeader* header = (PacketHeader*)buffer;
	//uint16 id = header->id;
	uint16 size = header->size;

	Protocol::S_AddObject pkt;
	pkt.ParseFromArray(&header[1], size - sizeof(PacketHeader));

	DevScene* scene = GET_SINGLE(SceneManager)->GetDevScene();
	if (scene)
		scene->Handle_S_AddObject(pkt);
}

void ClientPacketHandler::Handle_S_RemoveObject(ServerSessionRef session, BYTE* buffer, int32 len)
{
	PacketHeader* header = (PacketHeader*)buffer;
	//uint16 id = header->id;
	uint16 size = header->size;

	Protocol::S_RemoveObject pkt;
	pkt.ParseFromArray(&header[1], size - sizeof(PacketHeader));

	DevScene* scene = GET_SINGLE(SceneManager)->GetDevScene();
	if (scene)
		scene->Handle_S_RemoveObject(pkt);
}

void ClientPacketHandler::Handle_S_Move(ServerSessionRef session, BYTE* buffer, int32 len)
{
	PacketHeader* header = (PacketHeader*)buffer;
	//uint16 id = header->id;
	uint16 size = header->size;

	Protocol::S_Move pkt;
	pkt.ParseFromArray(&header[1], size - sizeof(PacketHeader));
	//
	const Protocol::ObjectInfo& info = pkt.info();
	DevScene* scene = GET_SINGLE(SceneManager)->GetDevScene();
	if (scene)
	{
		GameObject* gameObject = scene->GetObject(info.objectid());
		if (gameObject)
		{
			MyPlayer* myPlayer = GET_SINGLE(SceneManager)->GetMyPlayer();
					// 내 플레이어인 경우 서버 권위적 보정 (더 관대하게)
			if (myPlayer && gameObject == myPlayer)
			{
				Vec2Int serverPos = Vec2Int{info.posx(), info.posy()};
				Vec2Int clientPos = myPlayer->GetCellPos();

				// 위치 차이가 1칸 이상일 때만 보정
				int diffX = abs(serverPos.x - clientPos.x);
				int diffY = abs(serverPos.y - clientPos.y);
				
				if (diffX > 1 || diffY > 1)
				{
					cout << "Major position correction by server: (" << clientPos.x << ", " << clientPos.y << ") -> (" << serverPos.x << ", " << serverPos.y << ")" << endl;
					myPlayer->SetCellPos(serverPos, true); // teleport = true로 즉시 보정
				}
				else if (diffX > 0 || diffY > 0)
				{
					cout << "Minor position correction by server: (" << clientPos.x << ", " << clientPos.y << ") -> (" << serverPos.x << ", " << serverPos.y << ")" << endl;
					myPlayer->SetCellPos(serverPos, false); // 부드럽게 보정
				}
				
				// 상태와 방향 동기화
				myPlayer->SetDir(info.dir());
				myPlayer->SetState(info.state());
			}else
			{
				// 다른 플레이어/몬스터는 부드럽게 이동
				gameObject->SetDir(info.dir());
				gameObject->SetState(info.state());
				
				// 서버에서 받은 정확한 위치로 설정
				Vec2Int serverPos = Vec2Int{info.posx(), info.posy()};				// 몬스터의 경우 상태에 따라 다르게 처리
				if (info.objecttype() == Protocol::OBJECT_TYPE_MONSTER)
				{
					// 위치 차이 계산
					Vec2Int currentPos = gameObject->GetCellPos();
					int32 distance = abs(serverPos.x - currentPos.x) + abs(serverPos.y - currentPos.y);
					
					// 몬스터는 항상 서버 위치를 즉시 반영 (렌더링 문제 해결)
					gameObject->SetCellPos(serverPos, true); // 즉시 이동으로 변경
					
					// 몬스터 위치 동기화 로그
					static uint64 lastLogTime = 0;
					uint64 currentTime = GetTickCount64();
					if (currentTime - lastLogTime >= 2000) // 2초마다만 로그
					{
						cout << "Monster " << info.objectid() << " synced to (" << info.posx() << ", " << info.posy() << "), state: " << info.state() << ", dir: " << info.dir() << endl;
						lastLogTime = currentTime;
					}
				}
				else
				{
					gameObject->SetCellPos(serverPos, false); // 부드러운 이동
					cout << "Other object " << info.objectid() << " moved to (" << info.posx() << ", " << info.posy() << ")" << endl;
				}
			}
		}
		else
		{
			cout << "GameObject not found for S_Move: " << info.objectid() << endl;
		}
	}
}

SendBufferRef ClientPacketHandler::Make_C_Move()
{
	Protocol::C_Move pkt;

	MyPlayer* myPlayer = GET_SINGLE(SceneManager)->GetMyPlayer();

	*pkt.mutable_info() = myPlayer->info;
	
	// 클라이언트 예측 위치 설정 (현재 위치와 동일)
	Vec2Int currentPos = myPlayer->GetCellPos();
	pkt.set_predictedx(currentPos.x * 48.0f + 24.0f); // 타일 중앙 픽셀 위치
	pkt.set_predictedy(currentPos.y * 48.0f + 24.0f);

	return MakeSendBuffer(pkt, C_Move);
}
