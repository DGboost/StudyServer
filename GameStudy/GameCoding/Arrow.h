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

private:
	virtual void UpdateAnimation() override;
};

