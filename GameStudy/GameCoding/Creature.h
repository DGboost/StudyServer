#pragma once
#include "GameObject.h"

class Creature : public GameObject
{
public:
	using Super = GameObject;

public:

	virtual ~Creature() override;

	virtual void BeginPlay() override;
	virtual void Tick() override;
	virtual void Render(HDC hdc) override;

private:

	virtual void TickIdle() override {}
	virtual void TickMove() override {}
	virtual void TickSkill() override {}
	virtual void UpdateAnimation() override {}

	void SetStat(Stat stat) { _stat = stat; }
	Stat GetStat() const { return _stat; }

protected:
	Stat _stat;
};

