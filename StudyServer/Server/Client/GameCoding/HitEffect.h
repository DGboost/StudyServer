#pragma once
#include "GameObject.h"

class HitEffect : public GameObject
{
	using Super = GameObject;

public:
	HitEffect();
	virtual ~HitEffect() override;

	virtual void BeginPlay() override;
	virtual void Tick() override;
	virtual void Render(HDC hdc) override;
	virtual void UpdateAnimation() override;

	// 타겟 추적 기능 추가
	void SetTarget(GameObject* target) { _target = target; }

protected:
	GameObject* _target = nullptr;  // 추적할 타겟
};

