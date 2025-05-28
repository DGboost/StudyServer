#pragma once
#include "FLipbookActor.h"
#include "Enums.h"

class GameObject : public FlipbookActor
{
	using Super = FlipbookActor;
public:

	virtual ~GameObject() override;

	virtual void BeginPlay() override;
	virtual void Tick() override;
	virtual void Render(HDC hdc) override;

private:

	virtual void TickIdle() {}
	virtual void TickMove() {}
	virtual void TickSkill() {}

	void SetState(ObjectState state);
	void SetDir(Dir dir);

	virtual void UpdateAnimation() {}

	bool HasReachedDest();
	bool CanGo(Vec2Int cellPos);
	void SetCellPos(Vec2Int cellPos, bool teleport = false);

private:
	Vec2Int _cellPos = {};
	Vec2 _speed = {};
	Dir _dir = DIR_DOWN;
	ObjectState _state = ObjectState::Idle;
};

