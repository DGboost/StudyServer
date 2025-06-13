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

#define DEBUG_INTERPOLATION 0  // Set to 1 to enable interpolation debugging

const float GameObject::INTERPOLATION_DURATION = 0.15f;

GameObject::GameObject()
{
	_visualPosition = {0, 0};
	_targetCellPos = {0, 0};
	_interpolationStartPos = {0, 0};
	_interpolationElapsed = 0.0f;
}

GameObject::~GameObject()
{

}

void GameObject::BeginPlay()
{
	Super::BeginPlay();

	SetState(MOVE);
	SetState(IDLE);
	
	// 보간 변수 초기화
	_visualPosition = _pos;
	_targetCellPos = GetCellPos();
	_interpolationStartPos = _pos;
	_interpolationElapsed = INTERPOLATION_DURATION;
}

void GameObject::Tick()
{
	_dirtyFlag = false;

	Super::Tick();
	
	// 보간 업데이트
	float deltaTime = GET_SINGLE(TimeManager)->GetDeltaTime();
	UpdateInterpolation(deltaTime);

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
	// 보간된 위치(_visualPosition)를 사용하여 렌더링
	Vec2 originalPos = _pos;
	_pos = _visualPosition;
	
	Super::Render(hdc);
	
	_pos = originalPos;
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
	// 보간이 완료되었는지 확인
	return _interpolationElapsed >= INTERPOLATION_DURATION;
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

	Vec2 targetPixelPos = scene->ConvertPos(cellPos);
	
	// 스마트 텔레포트 판단: 거리가 너무 멀면 자동으로 텔레포트
	Vec2Int currentCellPos = Vec2Int{(int)(_visualPosition.x / 48), (int)(_visualPosition.y / 48)};
	int32 distance = abs(cellPos.x - currentCellPos.x) + abs(cellPos.y - currentCellPos.y);
	
	// 2타일 이상 떨어져 있으면 텔레포트, 처음 스폰이면 텔레포트
	bool shouldTeleport = teleport || distance > 2 || (_interpolationElapsed >= INTERPOLATION_DURATION && _targetCellPos.x == 0 && _targetCellPos.y == 0);

	if (shouldTeleport)
	{
		// 즉시 이동 (텔레포트)
		_pos = targetPixelPos;
		_visualPosition = targetPixelPos;
		_targetCellPos = cellPos;
		_interpolationStartPos = targetPixelPos;
		_interpolationElapsed = INTERPOLATION_DURATION; // 완료된 것으로 처리
		
		// 몬스터의 경우 위치 동기화 디버그 로그 (텔레포트만)
		if (info.objecttype() == Protocol::OBJECT_TYPE_MONSTER && distance > 2)
		{
			static uint64 lastLogTime = 0;
			uint64 currentTime = GetTickCount64();
			if (currentTime - lastLogTime >= 1000) // 1초마다만 로그
			{
				cout << "Monster " << info.objectid() << " teleported (distance=" << distance << ") to cell(" << cellPos.x << ", " << cellPos.y << ")" << endl;
				lastLogTime = currentTime;
			}
		}
	}
	else
	{
		// 부드러운 이동을 위한 보간 설정
		if (_targetCellPos.x != cellPos.x || _targetCellPos.y != cellPos.y)
		{
			_interpolationStartPos = _visualPosition;
			_targetCellPos = cellPos;
			_interpolationElapsed = 0.0f;
			
			// 논리적 목적지 설정
			_destPos = targetPixelPos;
			
			// 몬스터 부드러운 이동 로그
			if (info.objecttype() == Protocol::OBJECT_TYPE_MONSTER)
			{
				static uint64 lastLogTime = 0;
				uint64 currentTime = GetTickCount64();
				if (currentTime - lastLogTime >= 2000) // 2초마다만 로그
				{
					cout << "Monster " << info.objectid() << " smooth move to cell(" << cellPos.x << ", " << cellPos.y << ")" << endl;
					lastLogTime = currentTime;
				}
			}
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

void GameObject::UpdateInterpolation(float deltaTime)
{
	DevScene* scene = dynamic_cast<DevScene*>(GET_SINGLE(SceneManager)->GetCurrentScene());
	if (scene == nullptr)
		return;

	Vec2 targetPixelPos = scene->ConvertPos(_targetCellPos);
	
	// 목표에 거의 도달했는지 확인
	const float epsilon = 1.0f;
	if (abs(_visualPosition.x - targetPixelPos.x) < epsilon && abs(_visualPosition.y - targetPixelPos.y) < epsilon)
	{
		// 보간 완료
		_visualPosition = targetPixelPos;
		_pos = targetPixelPos;
		return;
	}

	// 보간 진행
	_interpolationElapsed += deltaTime;
	float alpha = _interpolationElapsed / INTERPOLATION_DURATION;
	alpha = min(1.0f, max(0.0f, alpha)); // [0, 1] 범위로 클램핑

	// 선형 보간
	_visualPosition.x = _interpolationStartPos.x + (targetPixelPos.x - _interpolationStartPos.x) * alpha;
	_visualPosition.y = _interpolationStartPos.y + (targetPixelPos.y - _interpolationStartPos.y) * alpha;

#if DEBUG_INTERPOLATION
	cout << "Interpolating: " << _interpolationStartPos << " -> " << targetPixelPos << " | Alpha: " << alpha << endl;
#endif

	if (alpha >= 1.0f)
	{
		// 보간 완료
		_visualPosition = targetPixelPos;
		_pos = targetPixelPos;
	}
}

void GameObject::StartMove(Vec2Int targetCellPos)
{
	DevScene* scene = dynamic_cast<DevScene*>(GET_SINGLE(SceneManager)->GetCurrentScene());
	if (scene == nullptr)
		return;

	// 이동 불가능한 위치인지 확인
	if (!scene->CanGo(targetCellPos))
		return;

	// 현재 위치에서 목표 위치로 즉시 논리적 이동 시작
	SetCellPos(targetCellPos, false);
	SetState(MOVE);
	
	cout << "Started move to (" << targetCellPos.x << ", " << targetCellPos.y << ")" << endl;
}
