#pragma once

#include "CorePch.h"
#include "Types.h"
#include "Values.h"
#include "Defines.h"
#include "Enums.h"
#include "Utils.h"

#include "Protocol.pb.h"
#include "Enum.pb.h"
#include "Struct.pb.h"
#include <assert.h>
#include "ClientPacketHandler.h"
#include "NetworkManager.h"

// C++20
#include <format>
#include <filesystem>
namespace fs = std::filesystem;

#define _CRTDEBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#pragma comment(lib, "msimg32.lib")

// 클라이언트 디버그 로그 제어
#define ENABLE_CLIENT_DEBUG_LOG 0  // 0: 비활성화, 1: 활성화

#if ENABLE_CLIENT_DEBUG_LOG == 0
// 모든 콘솔 출력을 무효화하는 null stream 클래스
class NullStream {
public:
    template<typename T>
    NullStream& operator<<(const T&) { return *this; }
    NullStream& operator<<(std::ostream&(*)(std::ostream&)) { return *this; }
};

// cout과 wcout을 null stream으로 대체
#define cout NullStream()
#define wcout NullStream()
#endif

// ����
#include <MMSystem.h>
#include <dsound.h>
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "dsound.lib")

#ifdef _DEBUG
#pragma comment(lib, "ServerCore\\Debug\\ServerCore.lib")
#pragma comment(lib, "Protobuf\\Debug\\libprotobufd.lib")
#else
#pragma comment(lib, "ServerCore\\Release\\ServerCore.lib")
#pragma comment(lib, "Protobuf\\Release\\libprotobuf.lib")
#endif
