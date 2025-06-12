#include "pch.h"
#include "GameObject.h"
#include "Player.h"
#include "Monster.h"
#include "GameRoom.h"

atomic<uint64> GameObject::s_idGenerator = 1;

void GameObject::Update()
{

}

PlayerRef GameObject::CreatePlayer()
{
	PlayerRef player = make_shared<Player>();
	player->info.set_objectid(s_idGenerator++);
	player->info.set_objecttype(Protocol::OBJECT_TYPE_PLAYER);
	
	// 플레이어 기본 스탯 설정
	player->info.set_hp(100);
	player->info.set_maxhp(100);
	player->info.set_attack(15);
	player->info.set_defence(3);

	return player;
}

MonsterRef GameObject::CreateMonster()
{
	MonsterRef monster = make_shared<Monster>();
	monster->info.set_objectid(s_idGenerator++);
	monster->info.set_objecttype(Protocol::OBJECT_TYPE_MONSTER);
	
	// 몬스터 기본 스탯 설정
	monster->info.set_hp(100);
	monster->info.set_maxhp(100);
	monster->info.set_attack(10);
	monster->info.set_defence(2);

	return monster;
}

void GameObject::SetState(ObjectState state, bool broadcast)
{
	if (info.state() == state)
		return;

	info.set_state(state);
	_dirtyFlag = true; // 상태 변경 플래그 설정

	if (broadcast)
	{
		// 몬스터 상태 변경 로그 추가
		if (info.objecttype() == Protocol::OBJECT_TYPE_MONSTER)
		{
			static uint64 lastLogTime = 0;
			uint64 currentTime = GetTickCount64();
			if (currentTime - lastLogTime >= 1000) // 1초마다만 로그
			{
				cout << "Monster " << info.objectid() << " state changed to " << state << " at (" << info.posx() << ", " << info.posy() << ")" << endl;
				lastLogTime = currentTime;
			}
		}
		BroadcastMove();
	}
}

void GameObject::SetDir(Dir dir, bool broadcast)
{
	if (info.dir() == dir)
		return;
		
	info.set_dir(dir);
	_dirtyFlag = true; // 상태 변경 플래그 설정

	if (broadcast)
		BroadcastMove();
}

bool GameObject::CanGo(Vec2Int cellPos)
{
	if (room == nullptr)
		return false;

	return room->CanGo(cellPos);
}

Dir GameObject::GetLookAtDir(Vec2Int cellPos)
{
	Vec2Int dir = cellPos - GetCellPos();
	if (dir.x > 0)
		return DIR_RIGHT;
	else if (dir.x < 0)
		return DIR_LEFT;
	else if (dir.y > 0)
		return DIR_DOWN;
	else
		return DIR_UP;
}

void GameObject::SetCellPos(Vec2Int cellPos, bool broadcast)
{
	// 위치 변경 확인
	if (info.posx() != cellPos.x || info.posy() != cellPos.y)
	{
		info.set_posx(cellPos.x);
		info.set_posy(cellPos.y);
		
		// 픽셀 단위 위치도 업데이트 (타일 중앙으로)
		info.set_worldx(cellPos.x * 48.0f + 24.0f);
		info.set_worldy(cellPos.y * 48.0f + 24.0f);
		
		_dirtyFlag = true; // 위치 변경 플래그 설정
	}

	if (broadcast)
		BroadcastMove();
}


Vec2Int GameObject::GetFrontCellPos()
{
	Vec2Int pos = GetCellPos();

	switch (info.dir())
	{
	case DIR_DOWN:
		return pos + Vec2Int{ 0, 1 };
	case DIR_LEFT:
		return pos + Vec2Int{ -1, 0 };
	case DIR_RIGHT:
		return pos + Vec2Int{ 1, 0 };
	case DIR_UP:
		return pos + Vec2Int{ 0, -1 };
	}

	return pos;
}


void GameObject::BroadcastMove()
{
	if (room)
	{
		SendBufferRef sendBuffer = ServerPacketHandler::Make_S_Move(info);
		room->Broadcast(sendBuffer);
	}
}