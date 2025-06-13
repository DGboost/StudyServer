#include "pch.h"
#include "CameraComponent.h"
#include "Actor.h"
#include "SceneManager.h"
#include "GameObject.h"
#include "TimeManager.h"

// 카메라 부드러움 계수 (값이 클수록 빠르게 따라감, 0.0~1.0)
const float CameraComponent::CAMERA_SMOOTH_FACTOR = 8.0f;

CameraComponent::CameraComponent()
{

}

CameraComponent::~CameraComponent()
{
}

void CameraComponent::BeginPlay()
{
	if (_owner != nullptr)
	{
		// GameObject인 경우 시작 위치 설정
		GameObject* gameObject = dynamic_cast<GameObject*>(_owner);
		if (gameObject != nullptr)
		{
			_currentCameraPos = gameObject->GetVisualPos();
		}
		else
		{
			_currentCameraPos = _owner->GetPos();
		}
		_initialized = true;
	}
}

void CameraComponent::TickComponent()
{
	if (_owner == nullptr)
		return;

	// GameObject인 경우 부드러운 시각적 위치 사용
	GameObject* gameObject = dynamic_cast<GameObject*>(_owner);
	Vec2 targetPos;
	
	if (gameObject != nullptr)
	{
		// 부드럽게 보간된 위치 사용
		targetPos = gameObject->GetVisualPos();
	}
	else
	{
		// 일반 Actor인 경우 기본 위치 사용
		targetPos = _owner->GetPos();
	}

	// 첫 프레임이거나 초기화되지 않은 경우 즉시 설정
	if (!_initialized)
	{
		_currentCameraPos = targetPos;
		_initialized = true;
	}
	else
	{
		// 부드러운 카메라 이동 (exponential smoothing)
		float deltaTime = GET_SINGLE(TimeManager)->GetDeltaTime();
		float smoothFactor = 1.0f - exp(-CAMERA_SMOOTH_FACTOR * deltaTime);
		
		_currentCameraPos.x += (targetPos.x - _currentCameraPos.x) * smoothFactor;
		_currentCameraPos.y += (targetPos.y - _currentCameraPos.y) * smoothFactor;
	}

	// 카메라 경계 제한
	Vec2 finalPos = _currentCameraPos;
	finalPos.x = ::clamp(finalPos.x, 400.f, 3024.f - 400.f);
	finalPos.y = ::clamp(finalPos.y, 300.f, 2064.f - 300.f);

	GET_SINGLE(SceneManager)->SetCameraPos(finalPos);
}

void CameraComponent::Render(HDC hdc)
{

}
