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
	float deltaTime = GET_SINGLE(TimeManager)->GetDeltaTime();
	_moveRequestTimer -= deltaTime;
	
	// 연속 이동을 위해 GetButton 사용 (키를 누르고 있는 동안 계속 true)
	bool movementInput = false;
	Dir inputDir = DIR_NONE;
	
	if (GET_SINGLE(InputManager)->GetButton(KeyType::Up))
	{
		inputDir = DIR_UP;
		movementInput = true;
	}
	else if (GET_SINGLE(InputManager)->GetButton(KeyType::Down))
	{
		inputDir = DIR_DOWN;
		movementInput = true;
	}
	else if (GET_SINGLE(InputManager)->GetButton(KeyType::Left))
	{
		inputDir = DIR_LEFT;
		movementInput = true;
	}
	else if (GET_SINGLE(InputManager)->GetButton(KeyType::Right))
	{
		inputDir = DIR_RIGHT;
		movementInput = true;
	}
	
	// 이동 입력이 있고, 타이머가 만료되었을 때만 이동 요청 전송
	if (movementInput && _moveRequestTimer <= 0.0f)
	{
		TryMove(inputDir);
		_moveRequestTimer = MOVE_REQUEST_INTERVAL;
	}
	
	// 모든 이동 키가 떼졌을 때 정지 요청 전송
	if (!movementInput && 
		(GET_SINGLE(InputManager)->GetButtonUp(KeyType::Up) ||
		 GET_SINGLE(InputManager)->GetButtonUp(KeyType::Down) ||
		 GET_SINGLE(InputManager)->GetButtonUp(KeyType::Left) ||
		 GET_SINGLE(InputManager)->GetButtonUp(KeyType::Right)))
	{
		TryStop();
	}

	// 무기 변경 - 로컬에서만 처리 (시각적 변경)
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
	// 공격/스킬 사용 - 서버에 공격 패킷 전송
	if (GET_SINGLE(InputManager)->GetButtonDown(KeyType::A))
	{
		TryAttack();
	}
}

void MyPlayer::TryMove(Dir dir)
{
	// 클라이언트 측 예측: 즉시 이동을 시작하고 서버에 요청 전송
	// cout << "Input received: attempting to move in direction " << dir << endl;
	
	// 현재 위치에서 목표 위치 계산
	Vec2Int currentPos = GetCellPos();
	Vec2Int targetPos = currentPos;
	
	switch (dir)
	{
	case DIR_UP:
		targetPos.y -= 1;
		break;
	case DIR_DOWN:
		targetPos.y += 1;
		break;
	case DIR_LEFT:
		targetPos.x -= 1;
		break;
	case DIR_RIGHT:
		targetPos.x += 1;
		break;
	}
	
	// 방향 즉시 변경
	SetDir(dir);
	
	// 클라이언트에서 즉시 이동 시작 (예측)
	StartMove(targetPos);
	
	// 서버에 이동 요청 패킷 전송
	SendBufferRef sendBuffer = ClientPacketHandler::Make_C_Move(dir);
	GET_SINGLE(NetworkManager)->SendPacket(sendBuffer);
	
	// cout << "Move prediction started and request sent to server for direction " << dir << endl;
}

void MyPlayer::TryStop()
{
	// 클라이언트에서 즉시 정지 (예측)
	// cout << "Stop input received" << endl;
	
	// 클라이언트에서 즉시 IDLE 상태로 변경
	SetState(IDLE);
	
	// 서버에 정지 패킷 전송 (현재 방향과 IDLE 상태)
	Dir currentDir = static_cast<Dir>(info.dir());
	SendBufferRef sendBuffer = ClientPacketHandler::Make_C_Move(currentDir, true); // isStop = true
	GET_SINGLE(NetworkManager)->SendPacket(sendBuffer);
	
	// cout << "Stop request sent to server" << endl;
}

void MyPlayer::TryAttack()
{
	// 서버 권위 구조: 클라이언트는 공격 입력만 서버로 전송
	cout << "Attack input received" << endl;
	
	// 현재 바라보는 방향의 앞 셀에 있는 오브젝트를 공격 대상으로 설정
	Vec2Int frontPos = GetFrontCellPos();
	
	// 장면에서 해당 위치의 오브젝트 찾기
	DevScene* scene = GET_SINGLE(SceneManager)->GetDevScene();
	if (scene)
	{
		GameObject* target = nullptr;
		// 해당 위치에 있는 오브젝트 찾기 (몬스터 우선)
		for (Actor* actor : scene->_actors[LAYER_OBJECT])
		{
			GameObject* gameObject = dynamic_cast<GameObject*>(actor);
			if (gameObject && gameObject != this)
			{
				Vec2Int objPos = gameObject->GetCellPos();
				if (objPos.x == frontPos.x && objPos.y == frontPos.y)
				{
					// 몬스터를 우선 타겟으로 설정
					if (gameObject->info.objecttype() == Protocol::OBJECT_TYPE_MONSTER)
					{
						target = gameObject;
						break;
					}
					else if (target == nullptr)
					{
						target = gameObject;
					}
				}
			}
		}
		
		// 타겟이 있든 없든 항상 공격 패킷을 전송 (공격 모션을 위해)
		uint64 targetId = target ? target->info.objectid() : 0; // 타겟이 없으면 0
		SendBufferRef sendBuffer = ClientPacketHandler::Make_C_Attack(targetId);
		GET_SINGLE(NetworkManager)->SendPacket(sendBuffer);
		
		if (target)
		{
			cout << "Attack request sent to server for target " << target->info.objectid() << endl;
		}
		else
		{
			cout << "Attack request sent to server with no target (air attack)" << endl;
		}
	}
}

void MyPlayer::TickIdle()
{
	// 서버 권위 구조: 입력만 처리, 상태 변경은 서버 응답에 의존
	TickInput();
}

void MyPlayer::TickMove()
{
	// 입력 처리
	TickInput();
	
	// 보간은 GameObject::UpdateInterpolation에서 처리됨
	// 목표에 도달했는지 확인
	if (HasReachedDest())
	{
		// 도달 완료 - 서버에서 IDLE 상태 업데이트를 받을 때까지 대기
		// 상태 변경은 서버 권위에 따라 처리됨
	}
}

void MyPlayer::TickSkill()
{
	Super::TickSkill();
}
