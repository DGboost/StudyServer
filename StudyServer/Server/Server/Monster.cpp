#include "pch.h"
#include "Monster.h"
#include "GameRoom.h"
#include "Player.h"

Monster::Monster()
{
	info.set_name("MonsterName");
	info.set_hp(50);
	info.set_maxhp(50);
	info.set_attack(5);
	info.set_defence(0);
}

Monster::~Monster()
{

}

void Monster::Update()
{
	//Super::Update();

	switch (info.state())
	{
		case IDLE:
			UpdateIdle();
			break;
		case MOVE:
			UpdateMove();
			break;
		case SKILL:
			UpdateSkill();
			break;
	}
}

void Monster::UpdateIdle()
{
	if (room == nullptr)
		return;
	// Find Player - 주기적으로만 타겟 업데이트
	uint64 now = GetTickCount64();
	if (_target.lock() == nullptr || (now - _lastTargetUpdate >= TARGET_UPDATE_INTERVAL))
	{
		_target = room->FindClosestPlayer(GetCellPos());
		_lastTargetUpdate = now;
	}
	PlayerRef target = _target.lock();
	if (target)
	{
		Vec2Int dir = target->GetCellPos() - GetCellPos();
		int32 dist = abs(dir.x) + abs(dir.y);
		
		if (dist == 1)
		{
			// 인접한 플레이어 공격
			SetDir(GetLookAtDir(target->GetCellPos()), true);
			SetState(SKILL, true);
			_waitUntil = GetTickCount64() + 1000; // 1초 공격 쿨다운
			
			// 공격 실행 - 서버에서 직접 처리
			if (room)
			{
				Protocol::C_Attack attackPkt;
				Protocol::ObjectInfo* attackerInfo = attackPkt.mutable_attackerinfo();
				*attackerInfo = this->info;
				attackPkt.set_targetid(target->info.objectid());
				
				room->Handle_C_Attack(attackPkt);
			}
		}
		else
		{
			vector<Vec2Int> path;
			if (room->FindPath(GetCellPos(), target->GetCellPos(), OUT path))
			{				if (path.size() > 1)
				{
					Vec2Int nextPos = path[1];					if (room->CanGo(nextPos))
					{
						SetDir(GetLookAtDir(nextPos), false); // 브로드캐스트 비활성화
						SetCellPos(nextPos, true); // 위치 변경과 함께 즉시 브로드캐스트
						_waitUntil = GetTickCount64() + 500; // 500ms로 변경하여 클라이언트와 동기화
						SetState(MOVE, false); // 중복 브로드캐스트 방지
						
						// 디버그 로그 - 주기적으로만 출력
						static uint64 lastLogTime = 0;
						uint64 currentTime = GetTickCount64();
						if (currentTime - lastLogTime >= 1000) // 1초마다만 로그
						{
							cout << "Monster " << info.objectid() << " moving to (" << nextPos.x << ", " << nextPos.y << ")" << endl;
							lastLogTime = currentTime;
						}
					}
				}
				else
					SetCellPos(path[0], true);
			}
		}
	}
}

void Monster::UpdateMove()
{
	uint64 now = GetTickCount64();

	if (_waitUntil > now)
		return;

	SetState(IDLE);
}

void Monster::UpdateSkill()
{
	uint64 now = GetTickCount64();

	if (_waitUntil > now)
		return;

	SetState(IDLE);
}
