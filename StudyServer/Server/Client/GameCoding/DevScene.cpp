#include "pch.h"
#include "DevScene.h"
#include "Utils.h"
#include "InputManager.h"
#include "TimeManager.h"
#include "ResourceManager.h"
#include "Texture.h"
#include "Sprite.h"
#include "Actor.h"
#include "SpriteActor.h"
#include "Player.h"
#include "Flipbook.h"
#include "UI.h"
#include "Button.h"
#include "TestPanel.h"
#include "TilemapActor.h"
#include "Tilemap.h"
#include "SoundManager.h"
#include "Sound.h"
#include "Monster.h"
#include "MyPlayer.h"
#include "SceneManager.h"

DevScene::DevScene()
{

}

DevScene::~DevScene()
{
}

void DevScene::Init()
{
	GET_SINGLE(ResourceManager)->LoadTexture(L"Stage01", L"Sprite\\Map\\Stage01.bmp");
	GET_SINGLE(ResourceManager)->LoadTexture(L"Tile", L"Sprite\\Map\\Tile.bmp", RGB(128, 128, 128));
	GET_SINGLE(ResourceManager)->LoadTexture(L"Sword", L"Sprite\\Item\\Sword.bmp");
	GET_SINGLE(ResourceManager)->LoadTexture(L"Arrow", L"Sprite\\Item\\Arrow.bmp", RGB(128, 128, 128));
	GET_SINGLE(ResourceManager)->LoadTexture(L"Potion", L"Sprite\\UI\\Mp.bmp");
	GET_SINGLE(ResourceManager)->LoadTexture(L"PlayerDown", L"Sprite\\Player\\PlayerDown.bmp", RGB(128, 128, 128));
	GET_SINGLE(ResourceManager)->LoadTexture(L"PlayerUp", L"Sprite\\Player\\PlayerUp.bmp", RGB(128, 128, 128));
	GET_SINGLE(ResourceManager)->LoadTexture(L"PlayerLeft", L"Sprite\\Player\\PlayerLeft.bmp", RGB(128, 128, 128));
	GET_SINGLE(ResourceManager)->LoadTexture(L"PlayerRight", L"Sprite\\Player\\PlayerRight.bmp", RGB(128, 128, 128));
	GET_SINGLE(ResourceManager)->LoadTexture(L"Snake", L"Sprite\\Monster\\Snake.bmp", RGB(128, 128, 128));
	GET_SINGLE(ResourceManager)->LoadTexture(L"Hit", L"Sprite\\Effect\\Hit.bmp", RGB(0, 0, 0));

	GET_SINGLE(ResourceManager)->LoadTexture(L"Start", L"Sprite\\UI\\Start.bmp");
	GET_SINGLE(ResourceManager)->LoadTexture(L"Edit", L"Sprite\\UI\\Edit.bmp");
	GET_SINGLE(ResourceManager)->LoadTexture(L"Exit", L"Sprite\\UI\\Exit.bmp");

	GET_SINGLE(ResourceManager)->CreateSprite(L"Stage01", GET_SINGLE(ResourceManager)->GetTexture(L"Stage01"));
	GET_SINGLE(ResourceManager)->CreateSprite(L"TileO", GET_SINGLE(ResourceManager)->GetTexture(L"Tile"), 0, 0, 48, 48);
	GET_SINGLE(ResourceManager)->CreateSprite(L"TileX", GET_SINGLE(ResourceManager)->GetTexture(L"Tile"), 48, 0, 48, 48);
	GET_SINGLE(ResourceManager)->CreateSprite(L"Start_Off", GET_SINGLE(ResourceManager)->GetTexture(L"Start"), 0, 0, 150, 150);
	GET_SINGLE(ResourceManager)->CreateSprite(L"Start_On", GET_SINGLE(ResourceManager)->GetTexture(L"Start"), 150, 0, 150, 150);
	GET_SINGLE(ResourceManager)->CreateSprite(L"Edit_Off", GET_SINGLE(ResourceManager)->GetTexture(L"Edit"), 0, 0, 150, 150);
	GET_SINGLE(ResourceManager)->CreateSprite(L"Edit_On", GET_SINGLE(ResourceManager)->GetTexture(L"Edit"), 150, 0, 150, 150);
	GET_SINGLE(ResourceManager)->CreateSprite(L"Exit_Off", GET_SINGLE(ResourceManager)->GetTexture(L"Exit"), 0, 0, 150, 150);
	GET_SINGLE(ResourceManager)->CreateSprite(L"Exit_On", GET_SINGLE(ResourceManager)->GetTexture(L"Exit"), 150, 0, 150, 150);

	LoadMap();
	LoadPlayer();
	LoadMonster();
	LoadProjectiles();
	LoadEffect();
	LoadTilemap();
	
	GET_SINGLE(ResourceManager)->LoadSound(L"BGM", L"Sound\\BGM.wav");
	GET_SINGLE(ResourceManager)->LoadSound(L"Attack", L"Sound\\Sword.wav");

	//SpawnObjectAtRandomPos<MyPlayer>();
	//SpawnObjectAtRandomPos<Monster>();
	//SpawnObject<Monster>(Vec2Int{7, 7});

	Super::Init();
}

void DevScene::Update()
{
	Super::Update();

	float deltaTime = GET_SINGLE(TimeManager)->GetDeltaTime();

	// 서버 권위적 시스템 - 클라이언트에서는 몬스터를 직접 스폰하지 않음
	// 서버에서 S_AddObject 패킷으로 몬스터 정보를 받아서 생성
	// TickMonsterSpawn();
}

void DevScene::Render(HDC hdc)
{
	Super::Render(hdc);

}

void DevScene::AddActor(Actor* actor)
{
	Super::AddActor(actor);

	Monster* creature = dynamic_cast<Monster*>(actor);
	if (creature)
	{
		_monsterCount++;
	}
}

void DevScene::RemoveActor(Actor* actor)
{
	Super::RemoveActor(actor);

	Monster* creature = dynamic_cast<Monster*>(actor);
	if (creature)
	{
		_monsterCount--;
	}
}

void DevScene::LoadMap()
{
	Sprite* sprite = GET_SINGLE(ResourceManager)->GetSprite(L"Stage01");

	SpriteActor* background = new SpriteActor();
	background->SetSprite(sprite);
	background->SetLayer(LAYER_BACKGROUND);
	const Vec2Int size = sprite->GetSize();
	background->SetPos(Vec2(size.x / 2, size.y / 2));

	AddActor(background);
}

void DevScene::LoadPlayer()
{
	// IDLE
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"PlayerUp");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_IdleUp");
		fb->SetInfo({ texture, L"FB_MoveUp", {200, 200}, 0, 9, 0, 0.5f });
	}
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"PlayerDown");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_IdleDown");
		fb->SetInfo({ texture, L"FB_MoveDown", {200, 200}, 0, 9, 0, 0.5f });
	}
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"PlayerLeft");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_IdleLeft");
		fb->SetInfo({ texture, L"FB_MoveLeft", {200, 200}, 0, 9, 0, 0.5f });
	}
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"PlayerRight");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_IdleRight");
		fb->SetInfo({ texture, L"FB_MoveRight", {200, 200}, 0, 9, 0, 0.5f });
	}
	// MOVE
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"PlayerUp");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_MoveUp");
		fb->SetInfo({ texture, L"FB_MoveUp", {200, 200}, 0, 9, 1, 0.5f });
	}
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"PlayerDown");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_MoveDown");
		fb->SetInfo({ texture, L"FB_MoveDown", {200, 200}, 0, 9, 1, 0.5f });
	}
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"PlayerLeft");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_MoveLeft");
		fb->SetInfo({ texture, L"FB_MoveLeft", {200, 200}, 0, 9, 1, 0.5f });
	}
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"PlayerRight");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_MoveRight");
		fb->SetInfo({ texture, L"FB_MoveRight", {200, 200}, 0, 9, 1, 0.5f });
	}
	// SKILL
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"PlayerUp");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_AttackUp");
		fb->SetInfo({ texture, L"FB_MoveUp", {200, 200}, 0, 7, 3, 0.5f, false });
	}
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"PlayerDown");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_AttackDown");
		fb->SetInfo({ texture, L"FB_MoveDown", {200, 200}, 0, 7, 3, 0.5f, false });
	}
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"PlayerLeft");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_AttackLeft");
		fb->SetInfo({ texture, L"FB_MoveLeft", {200, 200}, 0, 7, 3, 0.5f, false });
	}
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"PlayerRight");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_AttackRight");
		fb->SetInfo({ texture, L"FB_MoveRight", {200, 200}, 0, 7, 3, 0.5f, false });
	}
	// BOW
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"PlayerUp");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_BowUp");
		fb->SetInfo({ texture, L"FB_BowUp", {200, 200}, 0, 7, 5, 0.5f, false });
	}
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"PlayerDown");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_BowDown");
		fb->SetInfo({ texture, L"FB_BowDown", {200, 200}, 0, 7, 5, 0.5f, false });
	}
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"PlayerLeft");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_BowLeft");
		fb->SetInfo({ texture, L"FB_BowLeft", {200, 200}, 0, 7, 5, 0.5f, false });
	}
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"PlayerRight");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_BowRight");
		fb->SetInfo({ texture, L"FB_BowRight", {200, 200}, 0, 7, 5, 0.5f, false });
	}
	// STAFF
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"PlayerUp");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_StaffUp");
		fb->SetInfo({ texture, L"FB_StaffUp", {200, 200}, 0, 10, 6, 0.5f, false });
	}
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"PlayerDown");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_StaffDown");
		fb->SetInfo({ texture, L"FB_StaffDown", {200, 200}, 0, 10, 6, 0.5f, false });
	}
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"PlayerLeft");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_StaffLeft");
		fb->SetInfo({ texture, L"FB_StaffLeft", {200, 200}, 0, 10, 6, 0.5f, false });
	}
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"PlayerRight");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_StaffRight");
		fb->SetInfo({ texture, L"FB_StaffRight", {200, 200}, 0, 10, 6, 0.5f, false });
	}

}

void DevScene::LoadMonster()
{
	cout << "Loading Monster flipbooks..." << endl;
	
	// MOVE
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"Snake");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_SnakeUp");
		fb->SetInfo({ texture, L"FB_SnakeUp", {100, 100}, 0, 3, 3, 0.5f });
		cout << "Created FB_SnakeUp: " << (fb ? "OK" : "FAILED") << endl;
	}
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"Snake");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_SnakeDown");
		fb->SetInfo({ texture, L"FB_SnakeDown", {100, 100}, 0, 3, 0, 0.5f });
		cout << "Created FB_SnakeDown: " << (fb ? "OK" : "FAILED") << endl;
	}
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"Snake");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_SnakeLeft");
		fb->SetInfo({ texture, L"FB_SnakeLeft", {100, 100}, 0, 3, 2, 0.5f });
		cout << "Created FB_SnakeLeft: " << (fb ? "OK" : "FAILED") << endl;
	}
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"Snake");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_SnakeRight");
		fb->SetInfo({ texture, L"FB_SnakeRight", {100, 100}, 0, 3, 1, 0.5f });
		cout << "Created FB_SnakeRight: " << (fb ? "OK" : "FAILED") << endl;
	}
	
	cout << "Monster flipbooks loading completed." << endl;
}

void DevScene::LoadProjectiles()
{
	// MOVE
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"Arrow");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_ArrowUp");
		fb->SetInfo({ texture, L"FB_ArrowUp", {100, 100}, 0, 0, 3, 0.5f });
	}
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"Arrow");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_ArrowDown");
		fb->SetInfo({ texture, L"FB_ArrowDown", {100, 100}, 0, 0, 0, 0.5f });
	}
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"Arrow");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_ArrowLeft");
		fb->SetInfo({ texture, L"FB_ArrowLeft", {100, 100}, 0, 0, 1, 0.5f });
	}
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"Arrow");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_ArrowRight");
		fb->SetInfo({ texture, L"FB_ArrowRight", {100, 100}, 0, 0, 2, 0.5f });
	}
}

void DevScene::LoadEffect()
{
	{
		Texture* texture = GET_SINGLE(ResourceManager)->GetTexture(L"Hit");
		Flipbook* fb = GET_SINGLE(ResourceManager)->CreateFlipbook(L"FB_Hit");
		fb->SetInfo({ texture, L"FB_Hit", {50, 47}, 0, 5, 0, 0.5f, false });
	}
}

void DevScene::LoadTilemap()
{
	TilemapActor* actor = new TilemapActor();
	AddActor(actor);

	_tilemapActor = actor;
	{
		auto* tm = GET_SINGLE(ResourceManager)->CreateTilemap(L"Tilemap_01");
		tm->SetMapSize({ 63, 43 });
		tm->SetTileSize(48);

		GET_SINGLE(ResourceManager)->LoadTilemap(L"Tilemap_01", L"Tilemap\\Tilemap_01.txt");

		_tilemapActor->SetTilemap(tm);
		_tilemapActor->SetShowDebug(false);
	}
}

void DevScene::Handle_S_AddObject(Protocol::S_AddObject& pkt)
{
	uint64 myPlayerId = GET_SINGLE(SceneManager)->GetMyPlayerId();

	const int32 size = pkt.objects_size();
	for (int32 i = 0; i < size; i++)
	{
		const Protocol::ObjectInfo& info = pkt.objects(i);
		if (myPlayerId == info.objectid())
			continue;

		if (info.objecttype() == Protocol::OBJECT_TYPE_PLAYER)
		{
			Player* player = SpawnObject<Player>(Vec2Int{info.posx(), info.posy()});
			player->SetDir(info.dir());
			player->SetState(info.state());
			player->info = info;
		}		else if (info.objecttype() == Protocol::OBJECT_TYPE_MONSTER)
		{
			Monster* monster = SpawnObject<Monster>(Vec2Int{ info.posx(), info.posy() });
			
			// 서버에서 받은 정보로 설정
			monster->info = info;
			monster->SetDir(info.dir());
			monster->SetState(info.state());
			
			// 위치 설정
			monster->SetCellPos(Vec2Int{ info.posx(), info.posy() }, true);
			
			// 애니메이션 강제 업데이트
			monster->UpdateAnimation();
			
			cout << "Monster spawned on client: ID=" << info.objectid() << " at (" << info.posx() << ", " << info.posy() << ") State=" << info.state() << " Dir=" << info.dir() << endl;
		}
	}
}

void DevScene::Handle_S_RemoveObject(Protocol::S_RemoveObject& pkt)
{
	const int32 size = pkt.ids_size();
	for (int32 i = 0; i < size; i++)
	{
		int32 id = pkt.ids(i);

		GameObject* object = GetObject(id);
		if (object)
			RemoveActor(object);
	}
}

GameObject* DevScene::GetObject(uint64 id)
{
	for (Actor* actor : _actors[LAYER_OBJECT])
	{
		GameObject* gameObject = dynamic_cast<GameObject*>(actor);
		if (gameObject && gameObject->info.objectid() == id)
			return gameObject;
	}

	return nullptr;
}

Player* DevScene::FindClosestPlayer(Vec2Int pos)
{
	float best = FLT_MAX;
	Player* ret = nullptr;

	for (Actor* actor : _actors[LAYER_OBJECT])
	{
		Player* player = dynamic_cast<Player*>(actor);
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

// A* -> Dijikstra -> BFS -> Graph
// PQ
bool DevScene::FindPath(Vec2Int src, Vec2Int dest, vector<Vec2Int>& path, int32 maxDepth)
{
	// F = G + H
	// F = cost + heuristic
	// G = cost (src -> dest)
	// H = heuristic (dest -> src)
	int32 depth = abs(src.y - dest.y) + abs(src.x - dest.x);
	if (depth >= maxDepth)
		return false;

	priority_queue<PQNode, vector<PQNode>, greater<PQNode>> pq;
	map<Vec2Int, int32> best;
	map<Vec2Int, Vec2Int> parent;

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
		// PQNode extractMin = pq.top();
		PQNode node = pq.top();
		pq.pop();

		if (best[node.pos] < node.cost)
			continue;

		if (node.pos == dest)
		{
			found = true;
			break;
		}

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
				// existing cost is better than current cost?
				if (bestValue <= cost)
					continue;
			}

			// cost = G + H
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

			// dest -> src
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

		// If we reached the source, break
		if (pos == parent[pos])
			break;

		pos = parent[pos];
	}

	std::reverse(path.begin(), path.end());
	return true;
}

bool DevScene::CanGo(Vec2Int cellPos)
{
	if (_tilemapActor == nullptr)
		return false;

	Tilemap* tm = _tilemapActor->GetTilemap();
	if (tm == nullptr)
		return false;

	Tile* tile = tm->GetTileAt(cellPos);
	if (tile == nullptr)
		return false;

	// Check if the tile is walkable (value != 1 means walkable)
	if (GetCreatureAt(cellPos) != nullptr)
		return false;

	return tile->value != 1;
}

Vec2 DevScene::ConvertPos(Vec2Int cellPos)
{
	Vec2 ret = {};

	if (_tilemapActor == nullptr)
		return ret;

	Tilemap* tm = _tilemapActor->GetTilemap();
	if (tm == nullptr)
		return ret;

	int32 size = tm->GetTileSize();
	Vec2 pos = _tilemapActor->GetPos();

	ret.x = pos.x + cellPos.x * size + (size / 2);
	ret.y = pos.y + cellPos.y * size + (size / 2);

	return ret;
}

Vec2Int DevScene::GetRandomEmptyCellPos()
{
	Vec2Int ret = {-1, -1};

	if (_tilemapActor == nullptr)
		return ret;

	Tilemap* tm = _tilemapActor->GetTilemap();
	if (tm == nullptr)
		return ret;

	Vec2Int size = tm->GetMapSize();

	// 무작위로 빈 셀을 찾기
	while (true)
	{
		int32 x = rand() % size.x;
		int32 y = rand() % size.y;
		Vec2Int cellPos{x, y};

		if (CanGo(cellPos))
			return cellPos;
	}
}

void DevScene::TickMonsterSpawn()
{
	// 서버 권위적 시스템 - 클라이언트에서는 몬스터를 직접 스폰하지 않음
	// 서버에서만 몬스터를 생성하고 S_AddObject 패킷으로 클라이언트에 전달
	return;

	// 이하 코드는 사용하지 않음 (서버에서만 몬스터 스폰)
	if (_monsterCount < DESIRED_COUNT)
		SpawnObjectAtRandomPos<Monster>();
}

