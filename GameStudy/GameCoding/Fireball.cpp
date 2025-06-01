#include "pch.h"
#include "Fireball.h"
#include "TimeManager.h"
#include "ResourceManager.h"
#include "Flipbook.h"
#include "SceneManager.h"
#include "DevScene.h"
#include "Creature.h"
#include "HitEffect.h"

Fireball::Fireball()
{    // 플립북 배열 설정
    _flipbookMove[DIR_UP] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_FireballUp");
    _flipbookMove[DIR_DOWN] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_FireballDown");
    _flipbookMove[DIR_LEFT] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_FireballLeft");
    _flipbookMove[DIR_RIGHT] = GET_SINGLE(ResourceManager)->GetFlipbook(L"FB_FireballRight");

    // 프로젝타일 타입 및 속성 설정
    _type = ProjectileType::Fireball;
    _attack = 35; // 파이어볼 기본 공격력 (화살보다 강함)
    _lifeTime = 0.0f; // 생성 시 수명 초기화
}

Fireball::~Fireball()
{
}

void Fireball::BeginPlay()
{
    Super::BeginPlay();    SetState(ObjectState::Move); // 초기 상태를 이동으로 설정
    UpdateAnimation();
}

void Fireball::Tick()
{
    Super::Tick();

    // 파이어볼 수명 체크 (너무 오래 돌아다니지 않도록)
    _lifeTime += GET_SINGLE(TimeManager)->GetDeltaTime();

    if (_lifeTime > 5.0f) // 5초 후 자동 소멸
    {
        Scene* scene = GET_SINGLE(SceneManager)->GetCurrentScene();
        if (scene)
            scene->RemoveActor(this);
    }
}

void Fireball::Render(HDC hdc)
{
    Super::Render(hdc);
}

void Fireball::TickIdle()
{
    DevScene* scene = dynamic_cast<DevScene*>(GET_SINGLE(SceneManager)->GetCurrentScene());
    if (!scene)
        return;

    Vec2Int currentPos = GetCellPos();
    Vec2Int nextPos = currentPos;

    switch (GetDir())
    {
    case DIR_UP:
        nextPos.y -= 1;
        break;
    case DIR_DOWN:
        nextPos.y += 1;
        break;
    case DIR_LEFT:
        nextPos.x -= 1;
        break;
    case DIR_RIGHT:
        nextPos.x += 1;
        break;
    }    // 다음 위치에 크리처가 있는지 체크
    Creature* creature = scene->GetCreatureAt(nextPos);
    if (creature && creature != GetOwner())
    {
        // 히트 이펙트 생성
        scene->SpawnObject<HitEffect>(nextPos);

        // 명시적으로 Projectile*로 캐스팅하여 OnDamaged 호출
        Projectile* projectile = static_cast<Projectile*>(this);
        creature->Creature::OnDamaged(projectile);

        // 파이어볼 제거
        scene->RemoveActor(this);
        return;
    }    // 다음 위치로 이동 가능 여부 체크
    if (scene->CanGo(nextPos))
    {
        SetCellPos(nextPos);
        SetState(ObjectState::Move);
    }
    else
    {
        // 이동 불가능 - 파이어볼 제거
        scene->RemoveActor(this);
    }
}

void Fireball::TickMove()
{
    float deltaTime = GET_SINGLE(TimeManager)->GetDeltaTime();
    Vec2 moveDir = (_destPos - _pos);    if (moveDir.Length() < 5.0f)
    {
        // 목표 위치에 도달
        SetState(ObjectState::Idle);
        _pos = _destPos;
    }
    else
    {
        // 목표 위치로 이동
        float speed = 600.0f;
        switch (_dir)
        {
        case DIR_UP:
            _pos.y -= speed * deltaTime;
            break;
        case DIR_DOWN:
            _pos.y += speed * deltaTime;
            break;
        case DIR_LEFT:
            _pos.x -= speed * deltaTime;
            break;
        case DIR_RIGHT:
            _pos.x += speed * deltaTime;
            break;
        }
    }
}

void Fireball::UpdateAnimation()
{
    if (_flipbookMove[_dir] != nullptr)
        SetFlipbook(_flipbookMove[_dir]);
}

