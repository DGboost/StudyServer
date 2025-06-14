#include "pch.h"
#include "GameRoom.h"
#include "Player.h"
#include "Monster.h"
#include "GameSession.h"

GameRoomRef GRoom = make_shared<GameRoom>();

GameRoom::GameRoom()
{
	
}

GameRoom::~GameRoom()
{

}

void GameRoom::Init()
{
	MonsterRef monster = GameObject::CreateMonster();
	// SetCellPos 메서드를 사용하여 일관성 있게 위치 설정
	monster->SetCellPos(Vec2Int{8, 8}, false);
	// 초기 상태와 방향 설정
	monster->info.set_state(IDLE);
	monster->info.set_dir(DIR_DOWN);
	AddObject(monster);
	
	cout << "Monster created at position (8, 8) with ID: " << monster->info.objectid() << endl;

	_tilemap.LoadFile(L"C:\\Users\\IUBOO\\source\\repos\\StudyServer\\Server\\Client\\Resources\\Tilemap\\Tilemap_01.txt");
}

void GameRoom::Update()
{
	// 서버 권위 구조: 모든 게임 로직을 서버에서 처리
	
	// 플레이어 업데이트
	for (auto& item : _players)
	{
		item.second->Update();
	}

	// 몬스터 업데이트 (AI 로직 실행)
	for (auto& item : _monsters)
	{
		item.second->Update();
	}
	
	// 주기적으로 모든 오브젝트 상태를 클라이언트에 브로드캐스트 (30Hz)
	static uint64 lastBroadcastTime = 0;
	uint64 currentTime = GetTickCount64();
	if (currentTime - lastBroadcastTime >= 33) // 약 30FPS (33ms) - 네트워크 부하 고려
	{
		BroadcastGameState();
		lastBroadcastTime = currentTime;
	}
}

void GameRoom::EnterRoom(GameSessionRef session)
{
	PlayerRef player = GameObject::CreatePlayer();

	// 세션과 플레이어 연결 설정
	session->gameRoom = GetRoomRef();
	session->player = player;
	player->session = session;
	// TEMP
	player->info.set_posx(5);
	player->info.set_posy(5);
	// 픽셀 위치도 설정 (타일 중앙)
	player->info.set_worldx(5 * 48.0f + 24.0f);
	player->info.set_worldy(5 * 48.0f + 24.0f);

	// 플레이어 정보 설정
	{
		SendBufferRef sendBuffer = ServerPacketHandler::Make_S_MyPlayer(player->info);
		session->Send(sendBuffer);
	}
	{
		Protocol::S_AddObject pkt;

		for (auto& item : _players)
		{
			Protocol::ObjectInfo* info = pkt.add_objects();
			*info = item.second->info;
		}

		for (auto& item : _monsters)
		{
			Protocol::ObjectInfo* info = pkt.add_objects();
			*info = item.second->info;
		}

		SendBufferRef sendBuffer = ServerPacketHandler::Make_S_AddObject(pkt);
		session->Send(sendBuffer);
	}

	AddObject(player);
}

void GameRoom::LeaveRoom(GameSessionRef session)
{
	if (session == nullptr)
		return;
	if (session->player.lock() == nullptr)
		return;

	uint64 id = session->player.lock()->info.objectid();
	RemoveObject(id);
}

GameObjectRef GameRoom::FindObject(uint64 id)
{
	{
		auto findIt = _players.find(id);
		if (findIt != _players.end())
			return findIt->second;
	}
	{
		auto findIt = _monsters.find(id);
		if (findIt != _monsters.end())
			return findIt->second;
	}

	return nullptr;
}

void GameRoom::Handle_C_Move(Protocol::C_Move& pkt)
{
	uint64 id = pkt.info().objectid();
	GameObjectRef gameObject = FindObject(id);
	if (gameObject == nullptr)
	{
		cout << "Object not found for move request: " << id << endl;
		return;
	}

	// 서버 권위 구조: 클라이언트의 입력을 받아 서버에서 이동 로직 수행
	Dir moveDir = pkt.info().dir();
	ObjectState clientState = pkt.info().state();
	
	cout << "Received move request from player " << id << " - Dir: " << moveDir << " State: " << clientState << endl;
	
	// 정지 요청 처리 (IDLE 상태)
	if (clientState == IDLE)
	{
		// 정지 요청 - 현재 방향 유지하고 IDLE 상태로
		gameObject->SetState(IDLE, false);
		cout << "Player " << id << " stop request processed" << endl;
		return;
	}
	
	// 현재 서버상 위치
	int32 currentX = gameObject->info.posx();
	int32 currentY = gameObject->info.posy();
	
	// 이동 요청 처리 - 서버에서 다음 위치 계산
	int32 newX = currentX;
	int32 newY = currentY;
	
	switch (moveDir)
	{
	case DIR_UP:
		newY -= 1;
		break;
	case DIR_DOWN:
		newY += 1;
		break;
	case DIR_LEFT:
		newX -= 1;
		break;
	case DIR_RIGHT:
		newX += 1;
		break;
	}
	
	cout << "Server calculating move: (" << currentX << ", " << currentY << ") -> (" << newX << ", " << newY << ")" << endl;
	
	// 서버에서 이동 가능 여부 검증
	if (CanGo(newX, newY))
	{
		// 이동 허용 - 서버에서 오브젝트 상태 업데이트
		gameObject->SetState(MOVE, false);
		gameObject->SetDir(moveDir, false);
		gameObject->SetCellPos(Vec2Int{newX, newY}, false);
		
		cout << "Server APPROVED move for player " << id << ": (" << newX << ", " << newY << ")" << endl;
	}
	else
	{
		// 이동 불가 - 방향만 변경하고 현재 위치 유지
		gameObject->SetDir(moveDir, false);
		gameObject->SetState(IDLE, false);
		
		cout << "Server REJECTED move for player " << id << ": position corrected to (" << currentX << ", " << currentY << ")" << endl;
	}
	
	// _dirtyFlag는 이미 Set 메서드들에서 설정됨
	// BroadcastGameState()에서 주기적으로 전송됨
}

void GameRoom::Handle_C_Attack(Protocol::C_Attack& pkt)
{
	uint64 attackerId = pkt.attackerinfo().objectid();
	uint64 targetId = pkt.targetid();

	// 공격자 오브젝트 찾기
	GameObjectRef attacker = FindObject(attackerId);
	if (attacker == nullptr)
	{
		cout << "Attack failed: Invalid attacker" << endl;
		return;
	}

	// 타겟이 없는 경우 (공중 공격)
	if (targetId == 0)
	{
		cout << "Air attack by " << attackerId << " (no target)" << endl;
		
		// 공격 모션만 브로드캐스트 (데미지는 0)
		SendBufferRef sendBuffer = ServerPacketHandler::Make_S_Attack(attacker->info, 0, 0);
		Broadcast(sendBuffer);
		return;
	}

	// 타겟 오브젝트 찾기
	GameObjectRef target = FindObject(targetId);
	if (target == nullptr)
	{
		cout << "Attack failed: Invalid target" << endl;
		return;
	}

	// 공격 거리 체크 (인접한 셀만 공격 가능)
	Vec2Int attackerPos = attacker->GetCellPos();
	Vec2Int targetPos = target->GetCellPos();
	int32 distance = abs(attackerPos.x - targetPos.x) + abs(attackerPos.y - targetPos.y);
	
	if (distance > 1)
	{
		cout << "Attack failed: Target too far (distance: " << distance << ")" << endl;
		return;
	}

	// 데미지 계산
	int32 attackerAttack = attacker->info.attack();
	int32 targetDefence = target->info.defence();
	int32 damage = max(1, attackerAttack - targetDefence); // 최소 1 데미지

	// 타겟 HP 감소
	int32 currentHp = target->info.hp();
	int32 newHp = max(0, currentHp - damage);
	target->info.set_hp(newHp);

	cout << "Attack successful: " << attackerId << " -> " << targetId 
		 << " (Damage: " << damage << ", HP: " << currentHp << " -> " << newHp << ")" << endl;

	// 공격 정보를 모든 클라이언트에 브로드캐스트
	{
		SendBufferRef sendBuffer = ServerPacketHandler::Make_S_Attack(attacker->info, targetId, damage);
		Broadcast(sendBuffer);
	}

	// 타겟이 죽었는지 확인
	if (newHp <= 0)
	{
		cout << "Object " << targetId << " died, killed by " << attackerId << endl;
		
		// 사망 정보를 모든 클라이언트에 브로드캐스트
		{
			SendBufferRef sendBuffer = ServerPacketHandler::Make_S_Die(targetId, attackerId);
			Broadcast(sendBuffer);
		}

		// 몬스터인 경우 재생성 처리
		if (target->info.objecttype() == Protocol::OBJECT_TYPE_MONSTER)
		{
			// 몬스터 제거
			RemoveObject(targetId);
			
			// 일정 시간 후 새 몬스터 생성 (간단한 구현을 위해 즉시 생성)
			MonsterRef newMonster = GameObject::CreateMonster();
			Vec2Int randomPos = GetRandomEmptyCellPos();
			newMonster->SetCellPos(randomPos, false);
			newMonster->info.set_state(IDLE);
			newMonster->info.set_dir(DIR_DOWN);
			newMonster->info.set_hp(100);
			newMonster->info.set_maxhp(100);
			newMonster->info.set_attack(10);
			newMonster->info.set_defence(2);
			AddObject(newMonster);
			
			cout << "New monster spawned at (" << randomPos.x << ", " << randomPos.y << ")" << endl;
		}
		else if (target->info.objecttype() == Protocol::OBJECT_TYPE_PLAYER)
		{
			// 플레이어가 죽은 경우 HP 복구 후 초기 위치로 이동
			target->info.set_hp(target->info.maxhp());
			target->SetCellPos(Vec2Int{5, 5}, false); // 초기 위치
			cout << "Player " << targetId << " respawned at (5, 5)" << endl;
		}
	}
}

void GameRoom::AddObject(GameObjectRef gameObject)
{
	uint64 id = gameObject->info.objectid();

	auto objectType = gameObject->info.objecttype();

	switch (objectType)
	{
		case Protocol::OBJECT_TYPE_PLAYER:
			_players[id] = static_pointer_cast<Player>(gameObject);
			break;
		case Protocol::OBJECT_TYPE_MONSTER:
			_monsters[id] = static_pointer_cast<Monster>(gameObject);
			break;
		default:
			return;
	}

	gameObject->room = GetRoomRef();

	// �ű� ������Ʈ ���� ����
	{
		Protocol::S_AddObject pkt;

		Protocol::ObjectInfo* info = pkt.add_objects();
		*info = gameObject->info;
	
		SendBufferRef sendBuffer = ServerPacketHandler::Make_S_AddObject(pkt);
		Broadcast(sendBuffer);
	}
}

void GameRoom::RemoveObject(uint64 id)
{
	GameObjectRef gameObject = FindObject(id);
	if (gameObject == nullptr)
		return;

	switch (gameObject->info.objecttype())
	{
	case Protocol::OBJECT_TYPE_PLAYER:
		_players.erase(id);
		break;
	case Protocol::OBJECT_TYPE_MONSTER:
		_monsters.erase(id);
		break;
	default:
		return;
	}

	gameObject->room = nullptr;

	// ������Ʈ ���� ����
	{
		Protocol::S_RemoveObject pkt;
		pkt.add_ids(id);

		SendBufferRef sendBuffer = ServerPacketHandler::Make_S_RemoveObject(pkt);
		Broadcast(sendBuffer);
	}
}

void GameRoom::Broadcast(SendBufferRef& sendBuffer)
{
	for (auto& item : _players)
	{
		item.second->session->Send(sendBuffer);
	}
}

PlayerRef GameRoom::FindClosestPlayer(Vec2Int pos)
{
	float best = FLT_MAX;
	PlayerRef ret = nullptr;

	for (auto& item : _players)
	{
		PlayerRef player = item.second;
		if (player)
		{
			Vec2Int dir = pos - player->GetCellPos();
			float dist = dir.LengthSquared();
			if (dist < best)
			{
				best = dist;  // 버그 수정: dist = best -> best = dist
				ret = player;
			}
		}
	}

	return ret;
}

bool GameRoom::FindPath(Vec2Int src, Vec2Int dest, vector<Vec2Int>& path, int32 maxDepth /*= 10*/)
{
	int32 depth = abs(src.y - dest.y) + abs(src.x - dest.x);
	if (depth >= maxDepth)
		return false;

	priority_queue<PQNode, vector<PQNode>, greater<PQNode>> pq;
	map<Vec2Int, int32> best;
	map<Vec2Int, Vec2Int> parent;

	// �ʱⰪ
	{
		int32 cost = abs(dest.y - src.y) + abs(dest.x - src.x);

		pq.push(PQNode(cost, src));
		best[src] = cost;
		parent[src] = src;
	}

	Vec2Int front[4] =
	{
		{0, -1},
		{0, 1},
		{-1, 0},
		{1, 0},
	};

	bool found = false;

	while (pq.empty() == false)
	{
		// ���� ���� �ĺ��� ã�´�
		PQNode node = pq.top();
		pq.pop();

		// �� ª�� ��θ� �ڴʰ� ã�Ҵٸ� ��ŵ
		if (best[node.pos] < node.cost)
			continue;

		// �������� ���������� �ٷ� ����
		if (node.pos == dest)
		{
			found = true;
			break;
		}

		// �湮
		for (int32 dir = 0; dir < 4; dir++)
		{
			Vec2Int nextPos = node.pos + front[dir];

			if (CanGo(nextPos) == false)
				continue;

			int32 depth = abs(src.y - nextPos.y) + abs(src.x - nextPos.x);
			if (depth >= maxDepth)
				continue;

			int32 cost = abs(dest.y - nextPos.y) + abs(dest.x - nextPos.x);
			int32 bestValue = best[nextPos];
			if (bestValue != 0)
			{
				// �ٸ� ��ο��� �� ���� ���� ã������ ��ŵ
				if (bestValue <= cost)
					continue;
			}

			// ���� ����
			best[nextPos] = cost;
			pq.push(PQNode(cost, nextPos));
			parent[nextPos] = node.pos;
		}
	}

	if (found == false)
	{
		float bestScore = FLT_MAX;

		for (auto& item : best)
		{
			Vec2Int pos = item.first;
			int32 score = item.second;

			// �����̶��, ���� ��ġ���� ���� �� �̵��ϴ� ������
			if (bestScore == score)
			{
				int32 dist1 = abs(dest.x - src.x) + abs(dest.y - src.y);
				int32 dist2 = abs(pos.x - src.x) + abs(pos.y - src.y);
				if (dist1 > dist2)
					dest = pos;
			}
			else if (bestScore > score)
			{
				dest = pos;
				bestScore = score;
			}
		}
	}

	path.clear();
	Vec2Int pos = dest;

	while (true)
	{
		path.push_back(pos);

		// ������
		if (pos == parent[pos])
			break;

		pos = parent[pos];
	}

	std::reverse(path.begin(), path.end());
	return true;
}

void GameRoom::BroadcastGameState()
{
	// 서버 권위 구조: 모든 오브젝트의 상태를 주기적으로 브로드캐스트
	
	// 플레이어 상태 브로드캐스트
	for (auto& pair : _players)
	{
		PlayerRef player = pair.second;
		if (player->_dirtyFlag) // 상태가 변경된 경우만
		{
			SendBufferRef sendBuffer = ServerPacketHandler::Make_S_Move(player->info);
			Broadcast(sendBuffer);
			player->_dirtyFlag = false;
		}
	}
	
	// 몬스터 상태 브로드캐스트
	for (auto& pair : _monsters)
	{
		MonsterRef monster = pair.second;
		if (monster->_dirtyFlag) // 상태가 변경된 경우만
		{
			SendBufferRef sendBuffer = ServerPacketHandler::Make_S_Move(monster->info);
			Broadcast(sendBuffer);
			monster->_dirtyFlag = false;
		}
	}
}

GameObjectRef GameRoom::GetGameObjectAt(Vec2Int cellPos)
{
	for (auto& item : _players)
	{
		if (item.second->GetCellPos() == cellPos)
			return item.second;
	}

	for (auto& item : _monsters)
	{
		if (item.second->GetCellPos() == cellPos)
			return item.second;
	}

	return nullptr;
}

bool GameRoom::CanGo(Vec2Int cellPos)
{
	Tile* tile = _tilemap.GetTileAt(cellPos);
	if (tile == nullptr)
		return false;

	// 다른 플레이어나 몬스터가 있는지 확인
	if (GetGameObjectAt(cellPos) != nullptr)
		return false;

	return tile->value != 1;
}

bool GameRoom::CanGo(int32 x, int32 y)
{
	return CanGo(Vec2Int{x, y});
}

Vec2Int GameRoom::GetRandomEmptyCellPos()
{
	// 간단한 구현: 무작위로 빈 위치 찾기
	for (int attempts = 0; attempts < 100; attempts++)
	{
		int32 x = rand() % 20 + 5; // 5~24 범위
		int32 y = rand() % 20 + 5; // 5~24 범위
		Vec2Int pos{x, y};
		
		if (CanGo(pos))
		{
			return pos;
		}
	}
	
	// 찾지 못한 경우 기본 위치 반환
	return Vec2Int{10, 10};
}