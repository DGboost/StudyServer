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
	// 서버 권위 구조: 입력을 서버로 전송만
	// GetButtonDown: 키를 누르는 순간에만 true
	if (GET_SINGLE(InputManager)->GetButtonDown(KeyType::Up))
	{
		TryMove(DIR_UP);
	}
	else if (GET_SINGLE(InputManager)->GetButtonDown(KeyType::Down))
	{
		TryMove(DIR_DOWN);
	}
	else if (GET_SINGLE(InputManager)->GetButtonDown(KeyType::Left))
	{
		TryMove(DIR_LEFT);
	}
	else if (GET_SINGLE(InputManager)->GetButtonDown(KeyType::Right))
	{
		TryMove(DIR_RIGHT);
	}
	
	// 키를 떼면 정지 요청 전송
	if (GET_SINGLE(InputManager)->GetButtonUp(KeyType::Up) ||
		GET_SINGLE(InputManager)->GetButtonUp(KeyType::Down) ||
		GET_SINGLE(InputManager)->GetButtonUp(KeyType::Left) ||
		GET_SINGLE(InputManager)->GetButtonUp(KeyType::Right))
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
	// 서버 권위 구조: 클라이언트는 입력만 서버로 전송
	// 로컬 상태 변경 제거
	
	cout << "Input received: attempting to move in direction " << dir << endl;
	
	// 시각적 피드백을 위한 방향 변경도 제거 - 서버 응답만 신뢰
	
	// 서버에 이동 요청 패킷 전송 (방향 정보만)
	SendBufferRef sendBuffer = ClientPacketHandler::Make_C_Move(dir);
	GET_SINGLE(NetworkManager)->SendPacket(sendBuffer);
	
	cout << "Move request sent to server for direction " << dir << endl;
	
	// 실제 이동은 서버 응답(S_Move)을 받았을 때만 수행
}

void MyPlayer::TryStop()
{
	// 서버에 정지 요청 전송
	cout << "Stop input received" << endl;
	
	// 서버에 정지 패킷 전송 (현재 방향과 IDLE 상태)
	Dir currentDir = static_cast<Dir>(info.dir());
	SendBufferRef sendBuffer = ClientPacketHandler::Make_C_Move(currentDir, true); // isStop = true
	GET_SINGLE(NetworkManager)->SendPacket(sendBuffer);
	
	cout << "Stop request sent to server" << endl;
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
	// 서버 권위 구조: 클라이언트에서 자동 이동 로직 제거
	// 이동 애니메이션은 서버에서 받은 상태 변경에 의해서만 수행
	TickInput();
	
	// 부드러운 이동 애니메이션 (서버 위치로 보간)
	float deltaTime = GET_SINGLE(TimeManager)->GetDeltaTime();
	Vec2 dir = (_destPos - _pos);
	
	if (dir.Length() < 5.f)
	{
		_pos = _destPos;
		// 상태 변경은 서버에서만 - 로컬에서 IDLE로 변경하지 않음
	}
	else
	{
		// 목적지로 부드럽게 이동
		Vec2 moveDir = dir;
		moveDir.Normalize();
		_pos += moveDir * 300 * deltaTime;
	}
}

void MyPlayer::TickSkill()
{
	Super::TickSkill();
}
