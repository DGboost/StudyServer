#pragma once
#include "Tilemap.h"
#include <shared_mutex>
#include <mutex>

struct PQNode
{
	PQNode(int32 cost, Vec2Int pos) : cost(cost), pos(pos) { }

	bool operator<(const PQNode& other) const { return cost < other.cost; }
	bool operator>(const PQNode& other) const { return cost > other.cost; }

	int32 cost;
	Vec2Int pos;
};

class GameRoom : public enable_shared_from_this<GameRoom>
{
public:
	GameRoom();
	virtual ~GameRoom();

	void Init();
	void Update();

	void EnterRoom(GameSessionRef session);
	void LeaveRoom(GameSessionRef session);
	GameObjectRef FindObject(uint64 id);
	GameRoomRef GetRoomRef() { return shared_from_this(); }

public:
	// PacketHandler
	void Handle_C_Move(Protocol::C_Move& pkt);
	void Handle_C_Attack(Protocol::C_Attack& pkt);

public:
	void AddObject(GameObjectRef gameObject);
	void RemoveObject(uint64 id);
	void Broadcast(SendBufferRef& sendBuffer);
	void BroadcastGameState(); // 추가: 주기적 상태 브로드캐스트

public:
	PlayerRef FindClosestPlayer(Vec2Int pos);
	bool FindPath(Vec2Int src, Vec2Int dest, vector<Vec2Int>& path, int32 maxDepth = 10);
	bool CanGo(Vec2Int cellPos);
	bool CanGo(int32 x, int32 y);
	Vec2Int GetRandomEmptyCellPos();
	GameObjectRef GetGameObjectAt(Vec2Int cellPos);

private:
	map<uint64, PlayerRef> _players;
	map<uint64, MonsterRef> _monsters;
	Tilemap _tilemap;
	
	// 멀티 스레드 동기화
	mutable shared_mutex _roomMutex;  // 읽기/쓰기 분리된 뮤텍스
	mutable recursive_mutex _updateMutex;  // Update 함수용 재귀 뮤텍스
	
public:
	// 스레드 안전성을 위한 헬퍼 함수들
	template<typename Func>
	auto ReadLock(Func func) const -> decltype(func()) {
		shared_lock<shared_mutex> lock(_roomMutex);
		return func();
	}
	
	template<typename Func>
	auto WriteLock(Func func) -> decltype(func()) {
		unique_lock<shared_mutex> lock(_roomMutex);
		return func();
	}
};

extern GameRoomRef GRoom;