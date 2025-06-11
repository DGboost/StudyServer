#include "pch.h"
#include "MyPlayer.h"
#include "InputManager.h"
#include "TimeManager.h"
#include "ResourceManager.h"
#include "Flipbook.h"
#include "CameraComponent.h"
#include "SceneManager.h"
#include "DevScene.h"
#include "Arrow.h"
#include "HitEffect.h"

MyPlayer::MyPlayer()
{
	CameraComponent* camera = new CameraComponent();
	AddComponent(camera);
}

MyPlayer::~MyPlayer()
{

}

void MyPlayer::BeginPlay()
{
	Super::BeginPlay();

}

void MyPlayer::Tick()
{
	Super::Tick();
	
	// 주기적 동기화 제거 - 이동은 TryMove()에서만 처리
}

void MyPlayer::Render(HDC hdc)
{
	Super::Render(hdc);
}

void MyPlayer::TickInput()
{
	// 키 다운 이벤트로 한 번 누르면 한 번만 이동
	if (GET_SINGLE(InputManager)->GetButtonDown(KeyType::W))
	{
		SetDir(DIR_UP);
		TryMove(DIR_UP);
	}
	else if (GET_SINGLE(InputManager)->GetButtonDown(KeyType::S))
	{
		SetDir(DIR_DOWN);
		TryMove(DIR_DOWN);
	}
	else if (GET_SINGLE(InputManager)->GetButtonDown(KeyType::A))
	{
		SetDir(DIR_LEFT);
		TryMove(DIR_LEFT);
	}
	else if (GET_SINGLE(InputManager)->GetButtonDown(KeyType::D))
	{
		SetDir(DIR_RIGHT);
		TryMove(DIR_RIGHT);
	}

	// 무기 변경 및 스킬 입력
	if (GET_SINGLE(InputManager)->GetButtonDown(KeyType::KEY_1))
	{
		SetWeaponType(WeaponType::Sword);
	}
	else if (GET_SINGLE(InputManager)->GetButtonDown(KeyType::KEY_2))
	{
		SetWeaponType(WeaponType::Bow);
	}
	else if (GET_SINGLE(InputManager)->GetButtonDown(KeyType::KEY_3))
	{
		SetWeaponType(WeaponType::Staff);
	}

	if (GET_SINGLE(InputManager)->GetButton(KeyType::SpaceBar))
	{
		SetState(SKILL);
	}
}

void MyPlayer::TryMove(Dir dir)
{
	Vec2Int currentPos = GetCellPos();
	Vec2Int nextPos = currentPos;

	// 방향에 따른 다음 위치 계산
	switch (dir)
	{
	case DIR_UP:
		nextPos.y -= 1;
		break;
	case DIR_DOWN:
		nextPos.y += 1;
		break;
	case DIR_LEFT:
		nextPos.x -= 1;
		break;
	case DIR_RIGHT:
		nextPos.x += 1;
		break;
	}

	cout << "TryMove: from (" << currentPos.x << ", " << currentPos.y << ") to (" << nextPos.x << ", " << nextPos.y << ")" << endl;

	// 이동 가능한지 확인
	if (CanGo(nextPos))
	{
		// 클라이언트에서 즉시 위치 업데이트 (예측 이동)
		SetCellPos(nextPos);
		SetState(MOVE);
		
		// 서버에 이동 패킷 전송 (즉시 전송)
		SendBufferRef sendBuffer = ClientPacketHandler::Make_C_Move();
		GET_SINGLE(NetworkManager)->SendPacket(sendBuffer);
		
		cout << "Move packet sent to server for position (" << nextPos.x << ", " << nextPos.y << ")" << endl;
	}
	else
	{
		// 이동 불가능한 경우 방향만 변경하고 IDLE 상태로
		SetState(IDLE);
		cout << "Cannot move to (" << nextPos.x << ", " << nextPos.y << ")" << endl;
	}
}

void MyPlayer::TickIdle()
{
	TickInput();
}

void MyPlayer::TickMove()
{
	// 이동 중에도 입력 처리 가능하도록 변경
	TickInput();
	
	// 이동 애니메이션이 끝나면 IDLE 상태로 변경
	float deltaTime = GET_SINGLE(TimeManager)->GetDeltaTime();
	Vec2 dir = (_destPos - _pos);
	
	if (dir.Length() < 5.f)  // 허용 오차를 늘림
	{
		SetState(IDLE);
		_pos = _destPos;
	}
	else
	{
		// 부드러운 이동 애니메이션 - 속도를 줄임
		switch (info.dir())
		{
		case DIR_UP:
			_pos.y -= 300 * deltaTime; // 300으로 조정
			break;
		case DIR_DOWN:
			_pos.y += 300 * deltaTime;
			break;
		case DIR_LEFT:
			_pos.x -= 300 * deltaTime;
			break;
		case DIR_RIGHT:
			_pos.x += 300 * deltaTime;
			break;
		}
	}
}

void MyPlayer::TickSkill()
{
	Super::TickSkill();
}
