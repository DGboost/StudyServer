#pragma once
#include "Component.h"

class CameraComponent : public Component
{
public:
	CameraComponent();
	virtual ~CameraComponent() override;

	virtual void BeginPlay() override;
	virtual void TickComponent() override;
	virtual void Render(HDC hdc) override;

private:
	Vec2 _currentCameraPos = {0, 0};    // 현재 카메라 위치
	bool _initialized = false;          // 초기화 상태
	static const float CAMERA_SMOOTH_FACTOR; // 카메라 부드러움 계수

};

