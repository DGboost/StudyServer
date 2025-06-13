#include "pch.h"
#include "Scene.h"
#include "Actor.h"
#include "Creature.h"
#include "Monster.h"
#include "UI.h"
#include "TimeManager.h"
#include "SceneManager.h"

Scene::Scene()
{

}

Scene::~Scene()
{
	for (const vector<Actor*>& actors : _actors)
		for (Actor* actor : actors)
			SAFE_DELETE(actor);

	_actors->clear();

	for (UI* ui : _uis)
		SAFE_DELETE(ui);

	_uis.clear();
}

void Scene::Init()
{
	for (const vector<Actor*>& actors : _actors)
		for (Actor* actor : actors)
			actor->BeginPlay();

	for (UI* ui : _uis)
		ui->BeginPlay();
}

void Scene::Update()
{
	float deltaTime = GET_SINGLE(TimeManager)->GetDeltaTime();

	// ����
	for (const vector<Actor*> actors : _actors)
		for (Actor* actor : actors)
			actor->Tick();

	for (UI* ui : _uis)
		ui->Tick();
			// 주기적으로 액터 수 로그 (필요시 주석 해제)
	/*
	static uint64 lastLogTime = 0;
	uint64 currentTime = GetTickCount64();
	if (currentTime - lastLogTime >= 5000) // 5초마다만 로그
	{
		cout << "Scene Update - Total actors in LAYER_OBJECT: " << _actors[LAYER_OBJECT].size() << endl;
		lastLogTime = currentTime;
	}
	*/
}

void Scene::Render(HDC hdc)
{
	vector<Actor*>& actors = _actors[LAYER_OBJECT];
	std::sort(actors.begin(), actors.end(), [=](Actor* a, Actor* b)
	{
		return a->GetPos().y < b->GetPos().y;
	});

	for (const vector<Actor*>& actors : _actors)
		for (Actor* actor : actors)
			actor->Render(hdc);

	for (UI* ui : _uis)
		ui->Render(hdc);
}

void Scene::AddActor(Actor* actor)
{
	if (actor == nullptr)
		return;

	_actors[actor->GetLayer()].push_back(actor);
		// 디버그 로그 추가 (필요시 주석 해제)
	/*
	Monster* monster = dynamic_cast<Monster*>(actor);
	if (monster)
	{
		cout << "Scene::AddActor - Monster added to layer " << actor->GetLayer() << " Total actors in LAYER_OBJECT: " << _actors[LAYER_OBJECT].size() << endl;
	}
	*/
}

void Scene::RemoveActor(Actor* actor)
{
	if (actor == nullptr)
		return;

	vector<Actor*>& v = _actors[actor->GetLayer()];
	v.erase(std::remove(v.begin(), v.end(), actor), v.end());
}

Creature* Scene::GetCreatureAt(Vec2Int cellPos)
{
	for (Actor* actor : _actors[LAYER_OBJECT])
	{
		// GameObjectType
		Creature* creature = dynamic_cast<Creature*>(actor);
		if (creature && creature->GetCellPos() == cellPos)
			return creature;
	}

	return nullptr;
}
