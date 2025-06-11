#pragma once
#include "Player.h"

class MyPlayer : public Player
{
	using Super = Player;
public:
	MyPlayer();
	virtual ~MyPlayer() override;

	virtual void BeginPlay() override;
	virtual void Tick() override;
	virtual void Render(HDC hdc) override;

private:
	void TickInput();
	void TryMove(Dir dir);

	virtual void TickIdle() override;
	virtual void TickMove() override;
	virtual void TickSkill() override;

private:
	// 필요시 추가할 수 있는 멤버 변수들
};
