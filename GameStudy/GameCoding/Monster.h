#pragma once
#include "Creature.h"

class Monster : public Creature
{
public:
	using Super = Creature;

public:
	virtual ~Monster() override;

	virtual void BeginPlay() override;
	virtual void Tick() override;
	virtual void Render(HDC hdc) override;

private:
	virtual void TickIdle() override {}
	virtual void TickMove() override {}
	virtual void TickSkill() override {}
	virtual void UpdateAnimation() override {}
};
