#pragma once
#include "GameObject.h"

class Creature; // 전방 선언

class Projectile : public GameObject
{
    using Super = GameObject;
public:
    Projectile();
    virtual ~Projectile() override;
    virtual void BeginPlay() override;
    virtual void Tick() override;
    virtual void Render(HDC hdc) override;
    virtual void TickIdle() override {}
    virtual void TickMove() override {}
    virtual void TickSkill() override {}
    virtual void UpdateAnimation() override {}

    // 크리쳐의 attack과 동일한 역할
    void SetAttack(int32 attack) { _attack = attack; }
    int32 GetAttack() const { return _attack; }

    // 소유자 설정 (투사체를 발사한 크리쳐)
    void SetOwner(Creature* owner) { _owner = owner; }
    Creature* GetOwner() const { return _owner; }

    void SetType(ProjectileType type) { _type = type; }
    ProjectileType GetType() const { return _type; }

    // 충돌 처리
    virtual void OnCollision(GameObject* other);

protected:
    int32 _attack = 0;              // _damage 대신 _attack 사용 (크리쳐와 통일)
    ProjectileType _type;
    Creature* _owner = nullptr;     // 투사체 소유자
};