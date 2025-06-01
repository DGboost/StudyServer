#pragma once
#include "Projectile.h"

class Fireball : public Projectile
{
    using Super = Projectile;
public:
    Fireball();

    virtual ~Fireball() override;
    virtual void BeginPlay() override;
    virtual void Tick() override;
    virtual void Render(HDC hdc) override;
    virtual void TickIdle() override;
    virtual void TickMove() override;
    virtual void UpdateAnimation() override;

protected:
    Flipbook* _flipbookMove[4] = {};
    float _lifeTime = 0.0f; // 파이어볼의 수명을 추적하는 멤버 변수
};

