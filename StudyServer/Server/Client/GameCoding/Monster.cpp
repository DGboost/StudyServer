#include "pch.h"
#include "Monster.h"
#include "InputManager.h"
#include "TimeManager.h"
#include "ResourceManager.h"
#include "Flipbook.h"
#include "CameraComponent.h"
#include "SceneManager.h"
#include "DevScene.h"
#include "Player.h"
#include "HitEffect.h"

Monster::Monster()
{
	_flipbookMove[DIR_UP] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_SnakeUp");
	_flipbookMove[DIR_DOWN] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_SnakeDown");
	_flipbookMove[DIR_LEFT] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_SnakeLeft");
	_flipbookMove[DIR_RIGHT] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_SnakeRight");

	// 레이어 설정
	SetLayer(LAYER_OBJECT);
	
	// 초기 애니메이션 설정
	SetFlipbook(_flipbookMove[DIR_DOWN]); // 기본 방향으로 설정
	cout << "Monster created with flipbooks - Down: " << (_flipbookMove[DIR_DOWN] ? "OK" : "NULL") << " Layer: " << GetLayer() << endl;
}

Monster::~Monster()
{

}

void Monster::BeginPlay()
{
	Super::BeginPlay();

	// 초기 상태 설정
	SetState(IDLE);
	SetDir(DIR_DOWN);
	
	// 애니메이션 업데이트 강제 호출
	UpdateAnimation();
	
	cout << "Monster BeginPlay - State: " << info.state() << " Dir: " << info.dir() << " Flipbook: " << (_flipbook ? "OK" : "NULL") << endl;
}

void Monster::Tick()
{
	Super::Tick();
	
	// 몬스터가 Tick되고 있는지 확인
	static uint64 lastLogTime = 0;
	uint64 currentTime = GetTickCount64();
	if (currentTime - lastLogTime >= 3000) // 3초마다만 로그
	{
		cout << "Monster Tick - ID: " << info.objectid() << " State: " << info.state() << " Pos: (" << _pos.x << ", " << _pos.y << ")" << endl;
		lastLogTime = currentTime;
	}
}

void Monster::Render(HDC hdc)
{
	Super::Render(hdc);
	
	// 몬스터가 렌더링되고 있는지 확인
	static uint64 lastLogTime = 0;
	uint64 currentTime = GetTickCount64();
	if (currentTime - lastLogTime >= 3000) // 3초마다만 로그
	{
		Vec2 cameraPos = GET_SINGLE(SceneManager)->GetCameraPos();
		cout << "Monster Render - ID: " << info.objectid() << " Flipbook: " << (_flipbook ? "OK" : "NULL") 
			<< " Pos: (" << _pos.x << ", " << _pos.y << ")" 
			<< " Camera: (" << cameraPos.x << ", " << cameraPos.y << ")"
			<< " Layer: " << GetLayer() << endl;
		lastLogTime = currentTime;
	}
}

void Monster::TickIdle()
{
	// 클라이언트에서는 몬스터 AI 로직을 실행하지 않음
	// 서버에서 S_Move 패킷으로 몬스터의 위치와 상태를 전달받아 처리
	return;
}

void Monster::TickMove()
{
	// 이동 로직은 GameObject::UpdateInterpolation에서 처리
	// 기존의 수동 이동 코드 제거
}

void Monster::TickSkill()
{
	if (_flipbook == nullptr)
		return;

	if (_waitSeconds > 0)
	{
		float deltaTime = GET_SINGLE(TimeManager)->GetDeltaTime();
		_waitSeconds = max(0, _waitSeconds - deltaTime);	
		return;
	}

	// 클라이언트에서는 스킬 이펙트만 표시하고, 실제 데미지 처리는 서버에서 담당
	// 스킬 애니메이션이 끝나면 IDLE 상태로 전환
	SetState(IDLE);
}

void Monster::UpdateAnimation()
{
	if (_flipbookMove[info.dir()] != nullptr)
	{
		SetFlipbook(_flipbookMove[info.dir()]);
		
		// 디버그 로그
		static uint64 lastLogTime = 0;
		uint64 currentTime = GetTickCount64();
		if (currentTime - lastLogTime >= 2000) // 2초마다만 로그
		{
			cout << "Monster animation updated - Dir: " << info.dir() << " Flipbook: " << (_flipbook ? "OK" : "NULL") << endl;
			lastLogTime = currentTime;
		}
	}
	else
	{
		cout << "Warning: Monster flipbook is NULL for direction " << info.dir() << endl;
	}
}
