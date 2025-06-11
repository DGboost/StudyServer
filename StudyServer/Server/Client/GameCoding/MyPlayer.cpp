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

	SyncToServer();
}

void MyPlayer::Render(HDC hdc)
{
	Super::Render(hdc);
}

void MyPlayer::TickInput()
{
	// 키 다운 이벤트로 변경 - 한 번 누르면 한 번만 이동
	if (GET_SINGLE(InputManager)->GetButtonDown(KeyType::W))
	{
		cout << "W key pressed - moving UP" << endl;
		SetDir(DIR_UP);
		TryMove();
	}
	else if (GET_SINGLE(InputManager)->GetButtonDown(KeyType::S))
	{
		cout << "S key pressed - moving DOWN" << endl;
		SetDir(DIR_DOWN);
		TryMove();
	}
	else if (GET_SINGLE(InputManager)->GetButtonDown(KeyType::A))
	{
		cout << "A key pressed - moving LEFT" << endl;
		SetDir(DIR_LEFT);
		TryMove();
	}
	else if (GET_SINGLE(InputManager)->GetButtonDown(KeyType::D))
	{
		cout << "D key pressed - moving RIGHT" << endl;
		SetDir(DIR_RIGHT);
		TryMove();
	}

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

void MyPlayer::TryMove()
{
	Vec2Int currentPos = GetCellPos();
	Vec2Int nextPos = currentPos;

	// 방향에 따른 다음 위치 계산
	switch (info.dir())
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
		
		// 서버에 이동 패킷 전송
		SendBufferRef sendBuffer = ClientPacketHandler::Make_C_Move();
		GET_SINGLE(NetworkManager)->SendPacket(sendBuffer);
		
		cout << "Move packet sent to server" << endl;
	}
	else
	{
		// 이동 불가능한 경우 방향만 변경
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

void MyPlayer::SyncToServer()
{
	// 이제 TryMove()에서 직접 패킷을 전송하므로 여기서는 별도 처리 불필요
}
