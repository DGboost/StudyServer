#include "pch.h"
#include "Creature.h"
#include "InputManager.h"
#include "TimeManager.h"
#include "ResourceManager.h"
#include "Flipbook.h"
#include "CameraComponent.h"
#include "SceneManager.h"
#include "DevScene.h"
#include "Projectile.h"

Creature::Creature()
{
	
}

Creature::~Creature()
{

}

void Creature::BeginPlay()
{
	Super::BeginPlay();
}

void Creature::Tick()
{
	Super::Tick();

}

void Creature::Render(HDC hdc)
{
	Super::Render(hdc);
}

void Creature::OnDamaged(Creature* attacker)
{
	if (attacker == nullptr)
		return;

	Stat& attackerStat = attacker->GetStat();
	Stat& stat = GetStat();

	int32 damage = attackerStat.attack - stat.defence;
	if (damage <= 0)
		return;

	stat.hp = max(0, stat.hp - damage);
	if (stat.hp == 0)
	{
		Scene* scene = GET_SINGLE(SceneManager)->GetCurrentScene();
		if (scene)
			scene->RemoveActor(this);
	}
}

void Creature::OnDamaged(Projectile* projectile)
{
    if (projectile == nullptr)
        return;

    // 공격력 계산
    int32 attack = projectile->GetAttack();
    int32 damage = attack - _stat.defence;
    if (damage <= 0)
        return;

    // 체력 감소
    _stat.hp = max(0, _stat.hp - damage);
    
    // 죽었는지 확인
    if (_stat.hp == 0)
    {
        Scene* scene = GET_SINGLE(SceneManager)->GetCurrentScene();
        if (scene)
            scene->RemoveActor(this);
    }
}

