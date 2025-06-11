#include "pch.h"
#include "GameObject.h"
#include "Creature.h"
#include "InputManager.h"
#include "TimeManager.h"
#include "ResourceManager.h"
#include "Flipbook.h"
#include "CameraComponent.h"
#include "SceneManager.h"
#include "DevScene.h"

GameObject::GameObject()
{

}

GameObject::~GameObject()
{

}

void GameObject::BeginPlay()
{
	Super::BeginPlay();

	SetState(MOVE);
	SetState(IDLE);
}

void GameObject::Tick()
{
	_dirtyFlag = false;

	Super::Tick();

	switch (info.state())
	{
	case IDLE:
		TickIdle();
		break;
	case MOVE:
		TickMove();
		break;
	case SKILL:
		TickSkill();
		break;
	}
}

void GameObject::Render(HDC hdc)
{
	Super::Render(hdc);
}

void GameObject::SetState(ObjectState state)
{
	if (info.state() == state)
		return;

	info.set_state(state);
	UpdateAnimation();
	_dirtyFlag = true;
}

void GameObject::SetDir(Dir dir)
{
	info.set_dir(dir);
	UpdateAnimation();
	_dirtyFlag = true;
}

bool GameObject::HasReachedDest()
{
	Vec2 dir = (_destPos - _pos);
	return (dir.Length() < 5.f);
}

bool GameObject::CanGo(Vec2Int cellPos)
{
	DevScene* scene = dynamic_cast<DevScene*>(GET_SINGLE(SceneManager)->GetCurrentScene());
	if (scene == nullptr)
		return false;

	return scene->CanGo(cellPos);
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

void GameObject::SetCellPos(Vec2Int cellPos, bool teleport /*= false*/)
{
	info.set_posx(cellPos.x);
	info.set_posy(cellPos.y);

	DevScene* scene = dynamic_cast<DevScene*>(GET_SINGLE(SceneManager)->GetCurrentScene());
	if (scene == nullptr)
		return;

	_destPos = scene->ConvertPos(cellPos);

	if (teleport)
	{
		_pos = _destPos;
		
		// 몬스터의 경우 위치 동기화 디버그 로그
		if (info.objecttype() == Protocol::OBJECT_TYPE_MONSTER)
		{
			static uint64 lastLogTime = 0;
			uint64 currentTime = GetTickCount64();
			if (currentTime - lastLogTime >= 1000) // 1초마다만 로그
			{
				cout << "Monster " << info.objectid() << " teleported to cell(" << cellPos.x << ", " << cellPos.y << ") pixel(" << _pos.x << ", " << _pos.y << ")" << endl;
				lastLogTime = currentTime;
			}
		}
	}
	else
	{
		// teleport=false여도 몬스터가 IDLE 상태이면 즉시 위치 업데이트
		if (info.objecttype() == Protocol::OBJECT_TYPE_MONSTER && info.state() == IDLE)
		{
			_pos = _destPos;
			cout << "Monster " << info.objectid() << " IDLE position updated to cell(" << cellPos.x << ", " << cellPos.y << ") pixel(" << _pos.x << ", " << _pos.y << ")" << endl;
		}
	}

	_dirtyFlag = true;
}

Vec2Int GameObject::GetCellPos()
{
	return Vec2Int{info.posx(), info.posy()};
}

Vec2Int GameObject::GetFrontCellPos()
{
	switch (info.dir())
	{
		case DIR_DOWN:
			return GetCellPos() + Vec2Int{ 0, 1 };
		case DIR_LEFT:
			return GetCellPos() + Vec2Int{ -1, 0 };
		case DIR_RIGHT:
			return GetCellPos() + Vec2Int{ 1, 0 };
		case DIR_UP:
			return GetCellPos() + Vec2Int{ 0, -1 };
	}

	return GetCellPos();
}
