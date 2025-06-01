// Arrow.h
#pragma once
#include "Projectile.h"

class Arrow : public Projectile
{
    using Super = Projectile;
public:
    Arrow();

    virtual ~Arrow() override;
    virtual void BeginPlay() override;
    virtual void Tick() override;
    virtual void Render(HDC hdc) override;
    virtual void TickIdle() override;
    virtual void TickMove() override;
    virtual void UpdateAnimation() override;

protected:
    Flipbook* _flipbookMove[4] = {};
    float _lifeTime = 0.0f; // 화살의 수명을 추적하는 멤버 변수
};