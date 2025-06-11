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
	monster->info.set_posx(8);
	monster->info.set_posy(8);
	AddObject(monster);

	_tilemap.LoadFile(L"C:\\Users\\IUBOO\\source\\repos\\StudyServer\\Server\\Client\\Resources\\Tilemap\\Tilemap_01.txt");
}

void GameRoom::Update()
{
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
	
	// 추가적인 게임 로직 처리
	// 예: 아이템 스폰, 이벤트 처리 등
}

void GameRoom::EnterRoom(GameSessionRef session)
{
	PlayerRef player = GameObject::CreatePlayer();

	// ������ ���縦 ����
	session->gameRoom = GetRoomRef();
	session->player = player;
	player->session = session;

	// TEMP
	player->info.set_posx(5);
	player->info.set_posy(5);

	// ������ Ŭ�󿡰� ������ �����ֱ�
	{
		SendBufferRef sendBuffer = ServerPacketHandler::Make_S_MyPlayer(player->info);
		session->Send(sendBuffer);
	}
	// ��� ������Ʈ ���� ����
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
		return;

	// 클라이언트에서 받은 상태 확인
	ObjectState clientState = pkt.info().state();
	Dir moveDir = pkt.info().dir();
	
	if (clientState == IDLE)
	{
		// 클라이언트가 정지 상태 - 즉시 반영
		gameObject->info.set_state(IDLE);
		gameObject->info.set_dir(moveDir);
		
		SendBufferRef sendBuffer = ServerPacketHandler::Make_S_Move(gameObject->info);
		Broadcast(sendBuffer);
		return;
	}
	
	// 이동 요청 처리
	int32 currentX = gameObject->info.posx();
	int32 currentY = gameObject->info.posy();
	
	// 방향에 따른 이동 계산
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
	
	// 맵 경계 및 충돌 검사
	if (CanGo(newX, newY))
	{
		// 이동 허용 - 오브젝트 정보 업데이트
		gameObject->info.set_state(MOVE);
		gameObject->info.set_dir(moveDir);
		gameObject->info.set_posx(newX);
		gameObject->info.set_posy(newY);
		
		// 모든 클라이언트에게 이동 정보 브로드캐스트
		SendBufferRef sendBuffer = ServerPacketHandler::Make_S_Move(gameObject->info);
		Broadcast(sendBuffer);
	}
	else
	{
		// 이동 불가 - 방향만 변경
		gameObject->info.set_dir(moveDir);
		gameObject->info.set_state(IDLE);
		
		SendBufferRef sendBuffer = ServerPacketHandler::Make_S_Move(gameObject->info);
		Broadcast(sendBuffer);
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
	Vec2Int ret = { -1, -1 };

	Vec2Int size = _tilemap.GetMapSize();

	// �� �� �õ�?
	while (true)
	{
		int32 x = rand() % size.x;
		int32 y = rand() % size.y;
		Vec2Int cellPos{ x, y };

		if (CanGo(cellPos))
			return cellPos;
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