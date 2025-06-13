#include "pch.h"
#include "Game.h"
#include "TimeManager.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "SoundManager.h"
#include "NetworkManager.h"

Game::Game()
{

}

Game::~Game()
{
	// ��� �������� ..
	GET_SINGLE(SceneManager)->Clear();
	GET_SINGLE(ResourceManager)->Clear();

	_CrtDumpMemoryLeaks();
}

void Game::Init(HWND hwnd)
{
	// 콘솔 창 활성화 (디버그 로그 확인용)
	AllocConsole();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
	freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
	
	// 콘솔 창 제목 설정
	SetConsoleTitle(L"GameCoding Client Console");
	
	cout << "========================================" << endl;
	cout << "GameCoding Client Started" << endl;
	cout << "Console logging enabled" << endl;
	cout << "========================================" << endl;

	_hwnd = hwnd;
	hdc = ::GetDC(hwnd);

	::GetClientRect(hwnd, &_rect);

	hdcBack = ::CreateCompatibleDC(hdc); // hdc�� ȣȯ�Ǵ� DC�� ����
	_bmpBack = ::CreateCompatibleBitmap(hdc, _rect.right, _rect.bottom); // hdc�� ȣȯ�Ǵ� ��Ʈ�� ����
	HBITMAP prev = (HBITMAP)::SelectObject(hdcBack, _bmpBack); // DC�� BMP�� ����
	::DeleteObject(prev);

	GET_SINGLE(TimeManager)->Init();
	GET_SINGLE(InputManager)->Init(hwnd);
	GET_SINGLE(SceneManager)->Init();
	GET_SINGLE(ResourceManager)->Init(hwnd, fs::path(L"C:\\Users\\IUBOO\\source\\repos\\StudyServer\\Server\\Client\\Resources"));
	GET_SINGLE(SoundManager)->Init(hwnd);

	GET_SINGLE(SceneManager)->ChangeScene(SceneType::DevScene);

	GET_SINGLE(NetworkManager)->Init();
}

void Game::Update()
{
	GET_SINGLE(TimeManager)->Update();
	GET_SINGLE(InputManager)->Update();
	GET_SINGLE(SceneManager)->Update();
	GET_SINGLE(NetworkManager)->Update();
}

void Game::Render()
{
	// 백 버퍼 클리어 (검은색으로) - 더 부드러운 클리어
	HBRUSH blackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
	RECT fullRect = { 0, 0, _rect.right, _rect.bottom };
	::FillRect(hdcBack, &fullRect, blackBrush);
	
	GET_SINGLE(SceneManager)->Render(hdcBack);

	uint32 fps = GET_SINGLE(TimeManager)->GetFps();
	float deltaTime = GET_SINGLE(TimeManager)->GetDeltaTime();

	{
		POINT mousePos = GET_SINGLE(InputManager)->GetMousePos();
		wstring str = std::format(L"Mouse({0}, {1})", mousePos.x, mousePos.y);
		// 텍스트 배경 투명화로 더 깔끔한 UI
		::SetBkMode(hdcBack, TRANSPARENT);
		::SetTextColor(hdcBack, RGB(255, 255, 255));
		::TextOut(hdcBack, 20, 10, str.c_str(), static_cast<int32>(str.size()));
	}
	
	{
		wstring str = std::format(L"FPS({0}), DT({1:.3f})", fps, deltaTime);
		::SetBkMode(hdcBack, TRANSPARENT);
		::SetTextColor(hdcBack, RGB(255, 255, 255));
		::TextOut(hdcBack, 550, 10, str.c_str(), static_cast<int32>(str.size()));
	}

	// Double Buffering - 더 부드러운 복사
	::BitBlt(hdc, 0, 0, _rect.right, _rect.bottom, hdcBack, 0, 0, SRCCOPY);
}
