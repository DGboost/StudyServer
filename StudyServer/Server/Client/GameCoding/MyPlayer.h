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
	void TryMove();

	virtual void TickIdle() override;
	virtual void TickMove() override;
	virtual void TickSkill() override;
	void SyncToServer();

private:
	// 이제 사용하지 않는 변수들 제거
	//bool _keyPressed = false;
	//bool _prevKeyPressed = false;
	//uint64 _lastSyncTime = 0;
	//const uint64 SYNC_INTERVAL = 33;
};
