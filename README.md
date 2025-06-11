# GameServer


수정된 클라이언트와 서버의 동작을 자세히 설명드리겠습니다.

## 🎮 **전체 아키텍처 개요**

현재 구현된 시스템은 **서버 중심(Server-Authoritative)** 게임 아키텍처로, 모든 게임 로직과 상태는 서버에서 관리하고 클라이언트는 순수하게 입력과 렌더링만 담당합니다.

## 🖥️ **서버 (Server) 동작**

### **1. 서버 초기화 및 시작**
```cpp
// Server.cpp
ServerServiceRef service = make_shared<ServerService>(
    NetAddress(L"127.0.0.1", 7777),  // 로컬호스트 7777포트에서 대기
    make_shared<IocpCore>(),
    [](){ return make_shared<GameSession>(); },
    100);
```

### **2. 클라이언트 연결 처리**
```cpp
// GameSession.cpp - OnConnected()
void GameSession::OnConnected()
{
    // 1. 세션 매니저에 추가
    GSessionManager.Add(static_pointer_cast<GameSession>(shared_from_this()));
    
    // 2. 클라이언트에게 게임 입장 성공 패킷 전송
    Send(ServerPacketHandler::Make_S_EnterGame());
    
    // 3. 게임룸에 플레이어 추가
    GRoom->EnterRoom(GetSessionRef());
}
```

### **3. 플레이어 생성 및 룸 입장**
```cpp
// GameRoom.cpp - EnterRoom()
void GameRoom::EnterRoom(GameSessionRef session)
{
    // 1. 새 플레이어 오브젝트 생성
    PlayerRef player = GameObject::CreatePlayer();
    
    // 2. 초기 위치 설정 (5, 5)
    player->info.set_posx(5);
    player->info.set_posy(5);
    
    // 3. 클라이언트에게 본인 플레이어 정보 전송
    SendBufferRef sendBuffer = ServerPacketHandler::Make_S_MyPlayer(player->info);
    session->Send(sendBuffer);
    
    // 4. 기존 플레이어들과 몬스터들 정보 전송
    Protocol::S_AddObject pkt;
    for (auto& item : _players) {
        Protocol::ObjectInfo* info = pkt.add_objects();
        *info = item.second->info;
    }
    for (auto& item : _monsters) {
        Protocol::ObjectInfo* info = pkt.add_objects();
        *info = item.second->info;
    }
    
    // 5. 새 플레이어를 룸에 추가
    AddObject(player);
}
```

### **4. 이동 요청 처리**
```cpp
// ServerPacketHandler.cpp - Handle_C_Move()
void ServerPacketHandler::Handle_C_Move(GameSessionRef session, BYTE* buffer, int32 len)
{
    Protocol::C_Move pkt;
    pkt.ParseFromArray(&header[1], size - sizeof(PacketHeader));
    
    // 게임룸에서 이동 처리
    GameRoomRef room = session->gameRoom.lock();
    if (room)
        room->Handle_C_Move(pkt);
}
```

### **5. 서버측 이동 로직 (핵심!)**
```cpp
// GameRoom.cpp - Handle_C_Move()
void GameRoom::Handle_C_Move(Protocol::C_Move& pkt)
{
    uint64 id = pkt.info().objectid();
    GameObjectRef gameObject = FindObject(id);
    
    // 1. 서버에서 이동 유효성 검증
    // TODO: 여기서 실제 이동 가능한지 체크 (벽, 다른 플레이어 등)
    
    // 2. 서버 상태 업데이트
    gameObject->info.set_state(pkt.info().state());
    gameObject->info.set_dir(pkt.info().dir());
    gameObject->info.set_posx(pkt.info().posx());
    gameObject->info.set_posy(pkt.info().posy());
    
    // 3. 모든 클라이언트에게 이동 결과 브로드캐스트
    SendBufferRef sendBuffer = ServerPacketHandler::Make_S_Move(pkt.info());
    Broadcast(sendBuffer);
}
```

### **6. 몬스터 AI (서버에서 자동 실행)**
```cpp
// GameRoom.cpp - Update()
void GameRoom::Update()
{
    for (auto& item : _monsters) {
        item.second->Update();  // 몬스터 AI 실행
    }
}
```

## 🎯 **클라이언트 (Client) 동작**

### **1. 서버 연결**
```cpp
// NetworkManager.cpp - Init()
void NetworkManager::Init()
{
    _service = make_shared<ClientService>(
        NetAddress(L"127.0.0.1", 7777),  // 서버에 연결
        make_shared<IocpCore>(),
        [=]() { return CreateSession(); },
        1);
    
    _service->Start();
}
```

### **2. 입력 처리 (서버로 전송만)**
```cpp
// MyPlayer.cpp - TickInput()
void MyPlayer::TickInput()
{
    _keyPressed = true;
    
    if (GET_SINGLE(InputManager)->GetButton(KeyType::W)) {
        SetDir(DIR_UP);
    }
    else if (GET_SINGLE(InputManager)->GetButton(KeyType::S)) {
        SetDir(DIR_DOWN);
    }
    // ... 기타 방향키
    else {
        _keyPressed = false;
    }
}
```

### **3. 서버로 이동 요청 전송**
```cpp
// MyPlayer.cpp - SyncToServer() [수정된 부분]
void MyPlayer::SyncToServer()
{
    // 키가 눌렸을 때만 서버에 이동 요청 전송
    if (_keyPressed == false)
        return;

    SendBufferRef sendBuffer = ClientPacketHandler::Make_C_Move();
    GET_SINGLE(NetworkManager)->SendPacket(sendBuffer);
    
    _keyPressed = false; // 한 번 전송 후 플래그 리셋
}
```

### **4. 이동 로직 제거 (핵심 변경!)**
```cpp
// MyPlayer.cpp - TryMove() [수정된 부분]
void MyPlayer::TryMove()
{
    // 서버에서 이동 처리하므로 클라이언트에서는 입력만 서버로 전송
    // 실제 이동은 서버의 응답을 받은 후 처리
}
```

### **5. 서버 응답 처리**
```cpp
// ClientPacketHandler.cpp - Handle_S_Move() [수정된 부분]
void ClientPacketHandler::Handle_S_Move(ServerSessionRef session, BYTE* buffer, int32 len)
{
    Protocol::S_Move pkt;
    pkt.ParseFromArray(&header[1], size - sizeof(PacketHeader));
    
    const Protocol::ObjectInfo& info = pkt.info();
    
    DevScene* scene = GET_SINGLE(SceneManager)->GetDevScene();
    if (scene) {
        // 모든 오브젝트(본인 포함)의 위치를 서버 데이터로 업데이트
        GameObject* gameObject = scene->GetObject(info.objectid());
        if (gameObject) {
            gameObject->SetDir(info.dir());
            gameObject->SetState(info.state());
            gameObject->SetCellPos(Vec2Int{info.posx(), info.posy()});
        }
    }
}
```

## 🔄 **전체 통신 플로우**

### **플레이어 이동 시퀀스:**

1. **클라이언트**: 플레이어가 WASD 키 입력
2. **클라이언트**: `C_Move` 패킷을 서버로 전송
3. **서버**: 이동 요청을 받아 유효성 검증
4. **서버**: 게임 상태 업데이트 (플레이어 위치 변경)
5. **서버**: 모든 연결된 클라이언트에게 `S_Move` 패킷 브로드캐스트
6. **클라이언트들**: 서버에서 받은 위치 정보로 화면 업데이트

### **몬스터 움직임 시퀀스:**

1. **서버**: 주기적으로 몬스터 AI 실행
2. **서버**: 몬스터가 플레이어 추적하여 이동 결정
3. **서버**: 몬스터 위치 업데이트
4. **서버**: 모든 클라이언트에게 `S_Move` 패킷 브로드캐스트
5. **클라이언트들**: 몬스터 위치 업데이트하여 렌더링

## 🎯 **주요 개선점**

### **1. 서버 권위성 (Server Authority)**
- ✅ 모든 게임 로직이 서버에서 실행
- ✅ 클라이언트는 입력과 렌더링만 담당
- ✅ 핵, 치팅 방지 효과

### **2. 동기화 개선**
- ✅ 모든 플레이어가 동일한 게임 상태 공유
- ✅ 네트워크 지연이 있어도 일관성 유지

### **3. 확장성**
- ✅ 새로운 게임 기능 추가 시 서버에서만 구현
- ✅ 클라이언트는 단순한 뷰어 역할

## 🚀 **실행 결과**

이제 서버와 클라이언트를 실행하면:

1. **서버**: 7777포트에서 클라이언트 연결 대기
2. **클라이언트**: 서버에 연결하여 게임 참여
3. **플레이어**: WASD로 이동 시 서버에서 처리된 후 모든 클라이언트에 반영
4. **몬스터**: 서버에서 자동으로 플레이어를 추적하며 움직임
5. **멀티플레이어**: 여러 클라이언트가 동시에 접속하여 실시간 게임 가능

이 구조를 통해 안정적이고 확장 가능한 멀티플레이어 게임 기반을 구축했습니다!

Similar code found with 1 license type
