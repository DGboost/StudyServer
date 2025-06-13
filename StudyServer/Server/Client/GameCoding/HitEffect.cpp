#include "pch.h"
#include "HitEffect.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "Scene.h"

HitEffect::HitEffect()
{
	SetLayer(LAYER_EFFECT);
	UpdateAnimation();
}

HitEffect::~HitEffect()
{

}

void HitEffect::BeginPlay()
{
	Super::BeginPlay();	
}

void HitEffect::Tick()
{
	Super::Tick();

	// 타겟이 있으면 타겟의 위치를 따라가기
	if (_target != nullptr)
	{
		_pos = _target->_visualPosition;
		_visualPosition = _target->_visualPosition;
	}

	if (IsAnimationEnded())
	{
		Scene* scene = GET_SINGLE(SceneManager)->GetCurrentScene();
		scene->RemoveActor(this);
	}
}

void HitEffect::Render(HDC hdc)
{
	Super::Render(hdc);

}

void HitEffect::UpdateAnimation()
{
	SetFlipbook(GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_Hit"));
}
