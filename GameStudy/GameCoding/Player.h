#pragma once

#include "FlipbookActor.h"
#include "Creature.h"

class Flipbook;
class Collider;
class BoxCollider;

class Player : public Creature
{
	using Super = Creature;
public:
	Player();
	virtual ~Player() override;

	virtual void BeginPlay() override;
	virtual void Tick() override;
	virtual void Render(HDC hdc) override;

private:
	virtual void TickIdle() override;
	virtual void TickMove() override;
	virtual void TickSkill() override;

	virtual void UpdateAnimation() override;

	void SetWeaponType(WeaponType weaponType) { _weaponType = weaponType; }
	WeaponType GetWeaponType() { return _weaponType; }

private:
	Flipbook* _flipbookIdle[4] = {};
	Flipbook* _flipbookMove[4] = {};
	Flipbook* _flipbookAttack[4] = {};
	Flipbook* _flipbookBow[4] = {};
	Flipbook* _flipbookStaff[4] = {};	bool _keyPressed = false;
	WeaponType _weaponType = WeaponType::Sword;
	
	// 이동 지연 관련 변수
	float _moveDelayTimer = 0.0f;    // 현재 지연 시간
	float _moveDelayMax = 0.15f;     // 방향 전환 후 이동까지의 지연 시간
	Dir _lastPressedDir = DIR_DOWN;  // 마지막으로 눌린 방향키
	
	// 활 공격 쿨다운 관련 변수
	float _bowCooldown = 0.0f;       // 현재 쿨다운 시간
	float _bowCooldownMax = 0.5f;    // 최대 쿨다운 시간(연사 간격)
	
	// 스태프(파이어볼) 공격 쿨다운 관련 변수
	float _staffCooldown = 0.0f;     // 현재 쿨다운 시간
	float _staffCooldownMax = 1.0f;  // 최대 쿨다운 시간(연사 간격, 파이어볼은 더 강력해서 쿨다운이 김)
};

