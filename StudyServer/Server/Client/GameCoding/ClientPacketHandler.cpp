#include "pch.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"
#include "DevScene.h"
#include "MyPlayer.h"
#include "SceneManager.h"
#include "HitEffect.h"

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
		break;	case S_Move:
		Handle_S_Move(session, buffer, len);
		break;
	case S_Attack:
		Handle_S_Attack(session, buffer, len);
		break;
	case S_Die:
		Handle_S_Die(session, buffer, len);
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
	uint16 size = header->size;

	Protocol::S_Move pkt;
	pkt.ParseFromArray(&header[1], size - sizeof(PacketHeader));
	
	const Protocol::ObjectInfo& info = pkt.info();
	DevScene* scene = GET_SINGLE(SceneManager)->GetDevScene();
	if (scene)
	{
		GameObject* gameObject = scene->GetObject(info.objectid());
		if (gameObject)
		{
			// 서버 권위 구조: 모든 오브젝트는 서버에서 받은 상태를 그대로 적용
			Vec2Int serverPos = Vec2Int{info.posx(), info.posy()};
			
			// 서버에서 받은 위치, 상태, 방향을 즉시 적용
			gameObject->SetCellPos(serverPos, true); // 항상 즉시 적용
			gameObject->SetDir(info.dir());
			gameObject->SetState(info.state());
			
			MyPlayer* myPlayer = GET_SINGLE(SceneManager)->GetMyPlayer();
			if (myPlayer && gameObject == myPlayer)
			{
				cout << "MyPlayer position updated by server: (" << serverPos.x << ", " << serverPos.y << "), state: " << info.state() << ", dir: " << info.dir() << endl;
			}
			else
			{
				cout << "Object " << info.objectid() << " updated by server: (" << serverPos.x << ", " << serverPos.y << "), state: " << info.state() << ", dir: " << info.dir() << endl;
			}
		}
		else
		{
			cout << "GameObject not found for S_Move: " << info.objectid() << endl;
		}
	}
}

void ClientPacketHandler::Handle_S_Attack(ServerSessionRef session, BYTE* buffer, int32 len)
{
	PacketHeader* header = (PacketHeader*)buffer;
	uint16 size = header->size;

	Protocol::S_Attack pkt;
	pkt.ParseFromArray(&header[1], size - sizeof(PacketHeader));

	const Protocol::ObjectInfo& attackerInfo = pkt.attackerinfo();
	uint64 targetId = pkt.targetid();
	uint32 damage = pkt.damage();

	DevScene* scene = GET_SINGLE(SceneManager)->GetDevScene();
	if (scene)
	{
		GameObject* attacker = scene->GetObject(attackerInfo.objectid());
		
		if (attacker)
		{
			// 항상 공격 애니메이션 시작 (타겟이 있든 없든)
			attacker->SetState(SKILL);
			
			// 타겟이 있는 경우에만 히트 이펙트와 데미지 처리
			if (targetId != 0)
			{
				GameObject* target = scene->GetObject(targetId);
				if (target)
				{
					// 타겟의 HP 업데이트
					target->info.set_hp(target->info.hp() - damage);
					
					// 히트 이펙트 생성
					Vec2 targetWorldPos = target->GetPos();
					HitEffect* hitEffect = new HitEffect();
					hitEffect->SetPos(targetWorldPos);
					scene->AddActor(hitEffect);
					
					cout << "Attack with target: " << attackerInfo.objectid() << " -> " << targetId 
						 << " (Damage: " << damage << ")" << endl;
				}
			}
			else
			{
				cout << "Air attack by: " << attackerInfo.objectid() << " (no target)" << endl;
			}
		}
	}
}

void ClientPacketHandler::Handle_S_Die(ServerSessionRef session, BYTE* buffer, int32 len)
{
	PacketHeader* header = (PacketHeader*)buffer;
	uint16 size = header->size;

	Protocol::S_Die pkt;
	pkt.ParseFromArray(&header[1], size - sizeof(PacketHeader));

	uint64 objectId = pkt.objectid();
	uint64 killerId = pkt.killerid();

	DevScene* scene = GET_SINGLE(SceneManager)->GetDevScene();
	if (scene)
	{
		GameObject* deadObject = scene->GetObject(objectId);
		if (deadObject)
		{
			// 사망 이펙트 등 시각적 처리
			cout << "Object " << objectId << " died, killed by " << killerId << endl;
			
			// 플레이어가 죽은 경우가 아니라면 오브젝트는 서버에서 제거 처리됨
		}
	}
}

SendBufferRef ClientPacketHandler::Make_C_Move(Dir dir, bool isStop)
{
	Protocol::C_Move pkt;

	MyPlayer* myPlayer = GET_SINGLE(SceneManager)->GetMyPlayer();
	
	// 서버 권위 구조: 최소한의 정보만 전송 (플레이어 ID, 입력 방향)
	pkt.mutable_info()->set_objectid(myPlayer->info.objectid());
	pkt.mutable_info()->set_dir(dir);
	
	// 정지 요청인지 이동 요청인지 구분
	if (isStop)
	{
		pkt.mutable_info()->set_state(IDLE);
	}
	else
	{
		pkt.mutable_info()->set_state(MOVE);
	}
	
	// 예측 정보는 제거 (서버에서만 위치 계산)

	return MakeSendBuffer(pkt, C_Move);
}

SendBufferRef ClientPacketHandler::Make_C_Attack(uint64 targetId)
{
	Protocol::C_Attack pkt;

	MyPlayer* myPlayer = GET_SINGLE(SceneManager)->GetMyPlayer();
	if (myPlayer)
	{
		// 공격자 정보 설정
		Protocol::ObjectInfo* attackerInfo = pkt.mutable_attackerinfo();
		*attackerInfo = myPlayer->info;
		
		// 타겟 ID 설정
		pkt.set_targetid(targetId);
	}

	return MakeSendBuffer(pkt, C_Attack);
}
