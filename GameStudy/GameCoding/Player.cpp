#include "pch.h"
#include "Player.h"
#include "InputManager.h"
#include "TimeManager.h"
#include "ResourceManager.h"
#include "Flipbook.h"
#include "CameraComponent.h"
#include "SceneManager.h"
#include "DevScene.h"
#include "Arrow.h"
#include "Fireball.h"
#include "HitEffect.h"

Player::Player()
{
	_flipbookIdle[DIR_UP] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_IdleUp");
	_flipbookIdle[DIR_DOWN] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_IdleDown");
	_flipbookIdle[DIR_LEFT] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_IdleLeft");
	_flipbookIdle[DIR_RIGHT] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_IdleRight");
	
	_flipbookMove[DIR_UP] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_MoveUp");
	_flipbookMove[DIR_DOWN] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_MoveDown");
	_flipbookMove[DIR_LEFT] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_MoveLeft");
	_flipbookMove[DIR_RIGHT] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_MoveRight");

	_flipbookAttack[DIR_UP] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_AttackUp");
	_flipbookAttack[DIR_DOWN] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_AttackDown");
	_flipbookAttack[DIR_LEFT] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_AttackLeft");
	_flipbookAttack[DIR_RIGHT] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_AttackRight");

	_flipbookBow[DIR_UP] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_BowUp");
	_flipbookBow[DIR_DOWN] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_BowDown");
	_flipbookBow[DIR_LEFT] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_BowLeft");
	_flipbookBow[DIR_RIGHT] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_BowRight");

	_flipbookStaff[DIR_UP] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_StaffUp");
	_flipbookStaff[DIR_DOWN] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_StaffDown");
	_flipbookStaff[DIR_LEFT] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_StaffLeft");
	_flipbookStaff[DIR_RIGHT] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_StaffRight");

	CameraComponent* camera = new CameraComponent();
	AddComponent(camera);

	_stat.attack = 100;
}

Player::~Player()
{

}

void Player::BeginPlay()
{
	Super::BeginPlay();

	SetState(ObjectState::Move);
	SetState(ObjectState::Idle);

	SetCellPos({5, 5}, true);
}

void Player::Tick()
{
	Super::Tick();
	
	float deltaTime = GET_SINGLE(TimeManager)->GetDeltaTime();
	
	// 이동 지연 타이머 업데이트
	if (_moveDelayTimer > 0)
		_moveDelayTimer = max(0.0f, _moveDelayTimer - deltaTime);
	
	// 활 쿨다운 업데이트
	if (_bowCooldown > 0)
		_bowCooldown = max(0.0f, _bowCooldown - deltaTime);
	
	// 스태프(파이어볼) 쿨다운 업데이트
	if (_staffCooldown > 0)
		_staffCooldown = max(0.0f, _staffCooldown - deltaTime);
}

void Player::Render(HDC hdc)
{
	Super::Render(hdc);

}

void Player::TickIdle()
{
	float deltaTime = GET_SINGLE(TimeManager)->GetDeltaTime();

	_keyPressed = false;
	Vec2Int deltaXY[4] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };

	// 방향키 입력 처리
	if (GET_SINGLE(InputManager)->GetButton(KeyType::Up))
	{
		_keyPressed = true;
		if (_dir != DIR_UP)
		{
			// 방향이 다르면 방향만 변경하고 이동 지연 타이머 설정
			SetDir(DIR_UP);
			_lastPressedDir = DIR_UP;
			_moveDelayTimer = _moveDelayMax;
		}
		else if (_moveDelayTimer <= 0 && _lastPressedDir == DIR_UP)
		{
			// 같은 방향이고 지연 시간이 끝났으면 이동
			Vec2Int nextPos = _cellPos + deltaXY[DIR_UP];
			if (CanGo(nextPos))
			{
				SetCellPos(nextPos);
				SetState(ObjectState::Move);
			}
		}
	}
	else if (GET_SINGLE(InputManager)->GetButton(KeyType::Down))
	{
		_keyPressed = true;
		if (_dir != DIR_DOWN)
		{
			// 방향이 다르면 방향만 변경하고 이동 지연 타이머 설정
			SetDir(DIR_DOWN);
			_lastPressedDir = DIR_DOWN;
			_moveDelayTimer = _moveDelayMax;
		}
		else if (_moveDelayTimer <= 0 && _lastPressedDir == DIR_DOWN)
		{
			// 같은 방향이고 지연 시간이 끝났으면 이동
			Vec2Int nextPos = _cellPos + deltaXY[DIR_DOWN];
			if (CanGo(nextPos))
			{
				SetCellPos(nextPos);
				SetState(ObjectState::Move);
			}
		}
	}
	else if (GET_SINGLE(InputManager)->GetButton(KeyType::Left))
	{
		_keyPressed = true;
		if (_dir != DIR_LEFT)
		{
			// 방향이 다르면 방향만 변경하고 이동 지연 타이머 설정
			SetDir(DIR_LEFT);
			_lastPressedDir = DIR_LEFT;
			_moveDelayTimer = _moveDelayMax;
		}
		else if (_moveDelayTimer <= 0 && _lastPressedDir == DIR_LEFT)
		{
			// 같은 방향이고 지연 시간이 끝났으면 이동
			Vec2Int nextPos = _cellPos + deltaXY[DIR_LEFT];
			if (CanGo(nextPos))
			{
				SetCellPos(nextPos);
				SetState(ObjectState::Move);
			}
		}
	}
	else if (GET_SINGLE(InputManager)->GetButton(KeyType::Right))
	{
		_keyPressed = true;
		if (_dir != DIR_RIGHT)
		{
			// 방향이 다르면 방향만 변경하고 이동 지연 타이머 설정
			SetDir(DIR_RIGHT);
			_lastPressedDir = DIR_RIGHT;
			_moveDelayTimer = _moveDelayMax;
		}
		else if (_moveDelayTimer <= 0 && _lastPressedDir == DIR_RIGHT)
		{
			// 같은 방향이고 지연 시간이 끝났으면 이동
			Vec2Int nextPos = _cellPos + deltaXY[DIR_RIGHT];
			if (CanGo(nextPos))
			{
				SetCellPos(nextPos);
				SetState(ObjectState::Move);
			}
		}
	}
	else
	{
		// 키가 눌리지 않으면 지연 타이머 리셋
		_moveDelayTimer = 0;
	}
	
	// 애니메이션 업데이트
	if (_state == ObjectState::Idle)
	{
		UpdateAnimation();
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
	
	if (GET_SINGLE(InputManager)->GetButton(KeyType::A))
	{
		// 검은 일반 스킬 사용
		if (_weaponType == WeaponType::Sword)
			SetState(ObjectState::Skill);
		// 활은 쿨다운이 0일 때만 바로 화살 발사
		else if (_weaponType == WeaponType::Bow && _bowCooldown <= 0)
		{			
			DevScene* scene = dynamic_cast<DevScene*>(GET_SINGLE(SceneManager)->GetCurrentScene());
			if (scene)
			{
				Arrow* arrow = scene->SpawnObject<Arrow>(_cellPos);
				arrow->SetDir(_dir);
				arrow->SetOwner(this);
				
				// 플레이어의 공격력을 화살에 부여
				int32 arrowAttack = _stat.attack / 2; // 플레이어 공격력의 절반을 화살에 부여
				arrow->SetAttack(arrowAttack);
				
				// 화살 발사 후 쿨다운 적용
				_bowCooldown = _bowCooldownMax;
				
				// 발사 애니메이션 재생
				SetState(ObjectState::Skill);
			}
		}
		// 스태프는 쿨다운이 0일 때만 바로 파이어볼 발사
		else if (_weaponType == WeaponType::Staff && _staffCooldown <= 0)
		{
			DevScene* scene = dynamic_cast<DevScene*>(GET_SINGLE(SceneManager)->GetCurrentScene());
			if (scene)
			{
				Fireball* fireball = scene->SpawnObject<Fireball>(_cellPos);
				fireball->SetDir(_dir);
				fireball->SetOwner(this);
				
				// 플레이어의 공격력을 파이어볼에 부여
				int32 fireballAttack = _stat.attack; // 플레이어 공격력 그대로 부여 (화살보다 강함)
				fireball->SetAttack(fireballAttack);
				
				// 파이어볼 발사 후 쿨다운 적용
				_staffCooldown = _staffCooldownMax;
				
				// 발사 애니메이션 재생
				SetState(ObjectState::Skill);
			}
		}
	}
}

void Player::TickMove()
{
	float deltaTime = GET_SINGLE(TimeManager)->GetDeltaTime();

	// 목적지에 충분히 가까이 도달했는지 체크
	Vec2 dirVec = (_destPos - _pos);	
	if (dirVec.Length() < 5.f)
	{
		SetState(ObjectState::Idle);
		_pos = _destPos;
		return;
	}
	
	// 이동해야 할 방향 결정
	Dir moveDir;
	
	// 상하 움직임이 더 크면 상하 방향 우선
	if (abs(dirVec.y) > abs(dirVec.x))
	{
		if (dirVec.y < 0)
			moveDir = DIR_UP;
		else
			moveDir = DIR_DOWN;
	}
	// 좌우 움직임이 더 크거나 같으면 좌우 방향 우선
	else
	{
		if (dirVec.x < 0)
			moveDir = DIR_LEFT;
		else
			moveDir = DIR_RIGHT;
	}
	
	// 현재 방향과 이동 방향이 다르면 방향만 변경하고 위치는 변경하지 않음
	if (_dir != moveDir)
	{
		SetDir(moveDir);
		return; // 여기서 함수 종료 - 방향만 바꾸고 이동하지 않음
	}
	
	// 방향이 같으면 실제 이동 처리
	switch (_dir)
	{
	case DIR_UP:
		_pos.y -= 200 * deltaTime;
		break;
	case DIR_DOWN:
		_pos.y += 200 * deltaTime;
		break;
	case DIR_LEFT:
		_pos.x -= 200 * deltaTime;
		break;
	case DIR_RIGHT:
		_pos.x += 200 * deltaTime;
		break;
	}
}

void Player::TickSkill()
{
	if (_flipbook == nullptr)
		return;

	// TODO : Damage?
	if (IsAnimationEnded())
	{
		DevScene* scene = dynamic_cast<DevScene*>(GET_SINGLE(SceneManager)->GetCurrentScene());
		if (scene == nullptr)
			return;

		if (_weaponType == WeaponType::Sword)
		{
			Creature* creature = scene->GetCreatureAt(GetFrontCellPos());
			if (creature)
			{
				scene->SpawnObject<HitEffect>(GetFrontCellPos());
				creature->OnDamaged(this);
			}
		}				
		else if (_weaponType == WeaponType::Bow)
		{
			// 애니메이션 종료 시 Idle 상태로만 전환
			// 실제 화살 발사는 TickIdle에서 처리
		}
		else if (_weaponType == WeaponType::Staff)
		{
			// 애니메이션 종료 시 Idle 상태로만 전환
			// 실제 파이어볼 발사는 TickIdle에서 처리
		}

		SetState(ObjectState::Idle);
	}
}

void Player::UpdateAnimation()
{
	switch (_state)
	{
	case ObjectState::Idle:
		SetFlipbook(_flipbookIdle[_dir]); // _keyPressed 여부와 관계없이 항상 Idle 애니메이션 표시
		break;
	case ObjectState::Move:
		SetFlipbook(_flipbookMove[_dir]);
		break;
	case ObjectState::Skill:
		if (_weaponType == WeaponType::Sword)
			SetFlipbook(_flipbookAttack[_dir]);
		else if (_weaponType == WeaponType::Bow)
			SetFlipbook(_flipbookBow[_dir]);
		else
			SetFlipbook(_flipbookStaff[_dir]);
		break;
	}
}