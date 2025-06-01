#pragma once
#include "GameObject.h"

class Creature; // ���� ����

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

    // ũ������ attack�� ������ ����
    void SetAttack(int32 attack) { _attack = attack; }
    int32 GetAttack() const { return _attack; }

    // ������ ���� (����ü�� �߻��� ũ����)
    void SetOwner(Creature* owner) { _owner = owner; }
    Creature* GetOwner() const { return _owner; }

    void SetType(ProjectileType type) { _type = type; }
    ProjectileType GetType() const { return _type; }

    // �浹 ó��
    virtual void OnCollision(GameObject* other);

protected:
    int32 _attack = 0;              // _damage ��� _attack ��� (ũ���Ŀ� ����)
    ProjectileType _type;
    Creature* _owner = nullptr;     // ����ü ������
};