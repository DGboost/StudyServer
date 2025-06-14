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
	void TryStop(); // 정지 요청 메서드 추가
	void TryAttack(); // 공격 요청 메서드 추가

	virtual void TickIdle() override;
	virtual void TickMove() override;
	virtual void TickSkill() override;

private:
	// 이동 요청 제한을 위한 타이머
	float _moveRequestTimer = 0.0f;
	const float MOVE_REQUEST_INTERVAL = 0.1f; // 100ms 간격으로 이동 요청
	
	// 필요시 추가할 수 있는 멤버 변수들
};
