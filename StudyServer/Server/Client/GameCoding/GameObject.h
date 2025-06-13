#pragma once
#include "FlipbookActor.h"

class GameObject : public FlipbookActor
{
	using Super = FlipbookActor;

public:
	GameObject();
	virtual ~GameObject() override;

	virtual void BeginPlay() override;
	virtual void Tick() override;
	virtual void Render(HDC hdc) override;

	virtual void TickIdle() {}
	virtual void TickMove() {}
	virtual void TickSkill() {}

	void SetState(ObjectState state);
	void SetDir(Dir dir);

	virtual void UpdateAnimation() {}

	bool HasReachedDest();
	bool CanGo(Vec2Int cellPos);
	Dir GetLookAtDir(Vec2Int cellPos);

	void SetCellPos(Vec2Int cellPos, bool teleport = false);
	Vec2Int GetCellPos();
	Vec2Int GetFrontCellPos();

	// 부드러운 렌더링을 위한 시각적 위치 반환
	Vec2 GetVisualPos() const { return _visualPosition; }

	int64 GetObjectID() { return info.objectid(); }
	void SetObjectID(int64 id) { info.set_objectid(id); }
	// 클라이언트 측 예측 지원
	void UpdateInterpolation(float deltaTime);
	void StartMove(Vec2Int targetCellPos);

public:
	Protocol::ObjectInfo info;
	
	// 보간 관련 변수들 (public으로 변경하여 ClientPacketHandler에서 접근 가능)
	Vec2 _visualPosition;           // 실제 화면에 그려지는 위치
	Vec2Int _targetCellPos;         // 서버로부터 받은 목표 타일 위치
	Vec2 _interpolationStartPos;    // 보간 시작 시점의 위치
	float _interpolationElapsed;    // 보간 경과 시간
	
	static const float INTERPOLATION_DURATION; // 보간 지속 시간 (0.15초)
	
protected:
	bool _dirtyFlag = false;
};

