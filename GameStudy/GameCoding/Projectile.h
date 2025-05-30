#pragma once
#include "GameObject.h"

class Projectile : public GameObject
{
	using Super = GameObject;

public:
	Projectile();
	virtual ~Projectile() override;

	virtual void BeginPlay() override;
	virtual void Tick() override;
	virtual void Render(HDC hdc) override;

private:
	virtual void UpdateAnimation() override;
};

