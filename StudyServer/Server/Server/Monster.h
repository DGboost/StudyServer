#pragma once
#include "GameObject.h"

class Monster : public GameObject
{
	using Super = GameObject;

public:
	Monster();
	virtual ~Monster() override;

	virtual void Update();

private:
	virtual void UpdateIdle();
	virtual void UpdateMove();
	virtual void UpdateSkill();

private:
	uint64 _waitUntil = 0;
	uint64 _lastTargetUpdate = 0;
	const uint64 TARGET_UPDATE_INTERVAL = 500; // 500ms마다 타겟 업데이트 (더 빠르게)
	weak_ptr<Player> _target;
};

