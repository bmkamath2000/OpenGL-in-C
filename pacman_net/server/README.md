# Pacman over RabbitMQ + Protobuf

## Directory layout

```
pacman_net/
├── proto/
│   └── pacman.proto          # shared message schema
├── shared/
│   ├── Constants.h           # BOARD_X / BOARD_Y
│   ├── Ghost.h
│   └── Ghost.cpp             # AI logic (used by server only)
├── server/
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── PacmanServer.h
│   └── PacmanServer.cpp
└── client/
    ├── CMakeLists.txt
    ├── main.cpp
    ├── ClientWidget.h
    └── ClientWidget.cpp
```

## Dependencies

### RabbitMQ broker
```bash
# Ubuntu/Debian
sudo apt install rabbitmq-server
sudo systemctl start rabbitmq-server

# Windows: download installer from https://rabbitmq.com/install-windows.html
```

### rabbitmq-c (C client library)
```bash
# Ubuntu/Debian
sudo apt install librabbitmq-dev

# Windows (vcpkg)
vcpkg install rabbitmq-c
```

### Google Protobuf
```bash
# Ubuntu/Debian
sudo apt install libprotobuf-dev protobuf-compiler

# Windows (vcpkg)
vcpkg install protobuf
```

### Qt 6.4+
Download from https://www.qt.io/download or install via your package manager.

---

## Build — Server (no Qt needed)

```bash
cd server
mkdir build && cd build
cmake ..
cmake --build .
```

## Build — Client (Qt required)

```bash
cd client
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt/6.4.2/gcc_64
cmake --build .
```

---

## Run

### 1. Start RabbitMQ (if not already running)
```bash
sudo systemctl start rabbitmq-server
```

### 2. Start the server (on the game machine)
```bash
./pacman_server [rabbitmq-host] [port] [user] [password]
# Defaults: localhost 5672 guest guest
```

### 3. Start the client (on the viewer machine)
```bash
./pacman_client [rabbitmq-host] [port] [user] [password]
# Point rabbitmq-host at the server's IP if on a different machine
```

Use arrow keys to control Pacman from the client.

---

## How it works

```
Server                        RabbitMQ                 Qt Client
──────                        ────────                 ─────────
Game loop (60fps)
  │
  ├─ run physics/AI
  ├─ serialize GameState
  │   (protobuf)
  └──────────────────► game.state ──────────────────► deserialize
                        (fanout exchange)               │
                                                        └─ paintGL()
                                                            renders scene

Key press (client)
  │
  ├─ serialize KeyInput
  │   (protobuf)
  └──────────────────► input.keys ──────────────────► server reads
                        (direct queue)                  applyKey()
```

- `game.state` is a **fanout exchange** — multiple clients can connect
  simultaneously and all receive the same state.
- `input.keys` is a **single queue** — only one client controls Pacman
  at a time (first writer wins).
- All messages are binary protobuf (`application/octet-stream`).
