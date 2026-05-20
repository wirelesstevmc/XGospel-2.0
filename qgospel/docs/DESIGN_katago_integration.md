# Design: KataGo AI Engine Integration
## XGospel2 — Local Play vs KataGo (Phase 1) and IGS Bot Mode (Phase 2)

**Version:** Draft 1.1
**Date:** 2026-05-08
**Status:** Pre-implementation design — no code changes until this document is approved

---

## 1. Overview

This document describes the integration of the KataGo AI engine into xgospel2, a Qt5-based IGS Go client. The work is split into two phases:

**Phase 1 (this sprint):** A self-contained local game mode modelled on Sabaki's engine workflow.
The user registers one or more GTP engines in a persistent **Engine Manager** (an "Engines" tab
in Preferences, styled after Sabaki's Engines config panel). From the "Engines" menu the user
attaches a registered engine to the current board, then selects "Play vs Engine" to start a
local game with color, handicap, and komi. No IGS connection is required.

Key Sabaki concepts to replicate (reference: `/home/cahill/Claude_Projects/Sabaki_screenshots/`
and `/home/cahill/Claude_Projects/github/Sabaki`):

- **Engine registry** — persistent list of named engine profiles stored in `Settings`.
  Each profile stores: display name, path to executable (or wrapper script), and optional
  extra arguments (e.g. `-config gtp.cfg`). The path field accepts both direct binaries
  and shell scripts, enabling remote engines via SSH:
  ```sh
  #!/bin/sh
  # SSH wrapper — passwordless keys required; script must be executable
  ssh -t -t cahill@t490s '/home/cahill/.local/bin/KataGo/katago gtp'
  ```
- **Engines tab in Preferences** — a scrollable list of engine profiles with Add / Edit / Remove
  buttons. Matches Sabaki's "Engines" tab layout: each row shows name + path; a checkbox at the
  top enables GTP communication logging to a user-specified directory.
- **"Engines" menu** — mirrors Sabaki's `Engines` menu:
  - *Manage Engines…* — opens Preferences on the Engines tab
  - *Attach Engine* → submenu of registered engines → attaches selected engine to active board
  - *Detach Engine* — stops the current engine process
  - *Play vs Engine…* — opens the game setup dialog (color, handicap, komi) and starts a local game
  - *Engine vs Engine…* — starts self-play between two attached engines (deferred, Phase 1b)
  - *Toggle Analysis* — request best-move hints from engine for current position (deferred, Phase 1b)
- **Games are saveable** — local engine games use the existing SGF save infrastructure; the
  "Save Game" button in `BoardWindow` works identically to observed IGS games.

**Phase 2 (deferred):** IGS bot mode. When logged in under a registered bot account, xgospel2 accepts incoming IGS challenges, pipes IGS move traffic through KataGo's GTP interface, and sends KataGo's responses back to the IGS server — emulating how PandaBots operate.

### Environment Facts

| Item | Value |
|------|-------|
| KataGo binary | `/home/cahill/.local/bin/KataGo/katago` |
| KataGo version | v1.12.4, OpenCL backend |
| Neural network model | `/home/cahill/.local/bin/KataGo/default_model.bin.gz` (kata1-b18c384nbt-s5832081920) |
| GTP config | `/home/cahill/.local/bin/KataGo/default_gtp.cfg` |
| Config: maxVisits | 700 |
| Config: numSearchThreads | 16 |
| Config: rules | Japanese |
| Config: ponderingEnabled | true |
| Config: logDir | `gtp_logs` (relative path — **must cd to KataGo dir before launch**) |
| KataGo install dir | `/home/cahill/.local/bin/KataGo` |

The `logDir = gtp_logs` setting in `default_gtp.cfg` is a relative path. KataGo resolves it relative to its working directory. Therefore the QProcess working directory must be set to the KataGo install directory before launch; otherwise the log directory lookup fails at startup.

---

## 2. Architecture

### 2.1 Component Diagram

```
FixedXGospelWindow
  ├── Engine menu → "Play vs KataGo"
  │       │
  │       ▼
  │   LocalGameDialog (QDialog)
  │       │  (color, komi, handicap)
  │       ▼
  │   KataGoEngine (QObject)          ←── katago gtp process (QProcess)
  │       │  engineReady()                  stdin  ← GTP commands
  │       │  moveReady(int x, int y)       stderr → "GTP ready..."
  │       │  engineResigned()             stdout  → GTP responses
  │       │  engineError(QString)
  │       │
  │       ▼ (wired by FixedXGospelWindow)
  │   BoardWindow  (local_play_mode = true)
  │       │  moveRequested(game_id, x, y)  → FixedXGospelWindow::onLocalMove()
  │       │  resignRequested(game_id)      → FixedXGospelWindow::onLocalResign()
  │       │  passRequested(game_id)        → FixedXGospelWindow::onLocalPass()
  │       │  boardClosed(game_id)          → FixedXGospelWindow::onLocalBoardClosed()
  └───────┘
```

In local play mode, `BoardWindow` emits the same signals it uses for IGS games (`moveRequested`, `resignRequested`, `boardClosed`) plus a new `passRequested`. `FixedXGospelWindow` receives these signals and routes them to `KataGoEngine` rather than the IGS socket. This approach requires no architectural changes to `BoardWindow` beyond a `local_play_mode` flag that enables Pass and Resign controls unconditionally without requiring an IGS connection.

### 2.2 State Machine — KataGoEngine

```
IDLE ──start()──► STARTING ──"GTP ready"──► READY
                                               │
                             ┌─────────────────┤
                             │                 │
                          sendPlay()       requestGenmove()
                             │                 │
                             └────────► THINKING ──response──► READY
                                                              │
                                                         (pondering)
                                                         PONDERING
```

| State | Meaning |
|-------|---------|
| `IDLE` | Engine not started |
| `STARTING` | `QProcess` launched; waiting for `"GTP ready, beginning main protocol loop"` on stderr |
| `READY` | Engine initialized; no command in-flight |
| `THINKING` | `genmove` command sent; waiting for response |
| `PONDERING` | Engine pondering between moves (ponderingEnabled=true); ready to accept next command |

### 2.3 Command Queue

GTP is strictly serial: exactly one command may be in-flight at a time. `KataGoEngine` maintains an internal `QQueue<QString>` of pending GTP commands. When `READY`, the front of the queue is dequeued and written to stdin. When a response arrives (line starting with `=` or `?`), the queue is checked for the next pending command. This prevents the `sendPlay()` + `requestGenmove()` pair from being interleaved incorrectly.

---

## 3. File Inventory

### 3.1 New Files

#### `katago_engine.h` / `katago_engine.cpp`

`KataGoEngine` — a `QObject` subclass that owns a `QProcess` and implements the GTP client protocol.

```cpp
class KataGoEngine : public QObject {
    Q_OBJECT

public:
    enum State { IDLE, STARTING, READY, THINKING, PONDERING };

    explicit KataGoEngine(QObject *parent = nullptr);
    ~KataGoEngine();

    void start(double komi, int handicap);
    void stop();
    State state() const { return m_state; }

public slots:
    void sendPlay(StoneColor color, int x, int y);   // queues "play B/W <vertex>"
    void sendPass(StoneColor color);                  // queues "play B/W pass"
    void requestGenmove(StoneColor color);            // queues "genmove B/W"
    void requestFinalScore();                         // queues "final_score"

signals:
    void engineReady();
    void moveReady(int x, int y);
    void passMoveReady();
    void engineResigned();
    void scoreReady(const QString &result);
    void engineError(const QString &msg);

private slots:
    void onStderrReady();
    void onStdoutReady();
    void onProcessError(QProcess::ProcessError error);
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onStartupTimeout();

private:
    void setState(State s);
    void enqueue(const QString &cmd);
    void dispatchNext();
    void handleResponse(const QString &line);
    QString colorToGtp(StoneColor c) const;
    QString coordToGtp(int x, int y) const;
    bool    gtpToCoord(const QString &vertex, int &x, int &y) const;

    QProcess        *m_process;
    State            m_state;
    QQueue<QString>  m_cmd_queue;
    QString          m_pending_cmd;
    QTimer          *m_startup_timer;
    QString          m_stdout_buf;   // partial line accumulator
    double           m_komi;
    int              m_handicap;

    static const QString KATAGO_BIN;
    static const QString KATAGO_MODEL;
    static const QString KATAGO_CFG;
    static const QString KATAGO_DIR;
};
```

Static path constants (defined in `.cpp`):

```cpp
const QString KataGoEngine::KATAGO_BIN   = "/home/cahill/.local/bin/KataGo/katago";
const QString KataGoEngine::KATAGO_MODEL = "/home/cahill/.local/bin/KataGo/default_model.bin.gz";
const QString KataGoEngine::KATAGO_CFG   = "/home/cahill/.local/bin/KataGo/default_gtp.cfg";
const QString KataGoEngine::KATAGO_DIR   = "/home/cahill/.local/bin/KataGo";
```

---

#### `engine_manager.h` / `engine_manager.cpp`

`EngineProfile` — a plain struct stored in `Settings` (serialised as a `QList` of `QVariantMap`):

```cpp
struct EngineProfile {
    QString id;       // UUID — stable identifier across renames
    QString name;     // Display name, e.g. "KataGo (local)"
    QString path;     // Path to binary or wrapper script
    QString args;     // Extra command-line arguments, e.g. "-config gtp.cfg"
};
```

`EngineManager` — a singleton `QObject` (owned by `FixedXGospelWindow`) that:
- Loads and saves `QList<EngineProfile>` to/from `Settings` under key `engine_profiles`
- Provides `attach(profile, komi, handicap)` → creates and starts a `KataGoEngine` instance
- Provides `detach()` → stops current engine gracefully
- Exposes `currentEngine()` → `KataGoEngine*` (nullptr if none attached)

`EnginesPrefsWidget` — a `QWidget` embedded in the existing `PreferencesDialog` as a new
"Engines" tab:
- `QListWidget` showing registered engine profiles (name + path, one per row)
- Add / Edit / Remove buttons below the list
- "Enable GTP logging" checkbox + directory path field (matching Sabaki's layout)
- Add/Edit opens `EngineEditDialog` (name field, path field with Browse button, args field)

#### `local_game_dialog.h` / `local_game_dialog.cpp`

`LocalGameDialog` — a `QDialog` presented when the user chooses "Play vs Engine…". Shows the
selected engine name at the top (read-only), then:

```cpp
class LocalGameDialog : public QDialog {
    Q_OBJECT
public:
    explicit LocalGameDialog(const QString &engine_name, QWidget *parent = nullptr);
    StoneColor userColor() const;   // BLACK_STONE, WHITE_STONE, or EMPTY (random)
    double     komi()      const;
    int        handicap()  const;
private:
    QLabel          *m_engine_label;  // "Engine: KataGo (local)"
    QRadioButton    *m_black_btn;
    QRadioButton    *m_white_btn;
    QRadioButton    *m_random_btn;
    QDoubleSpinBox  *m_komi_spin;     // 0.0–9.5, step 0.5, default 6.5
    QSpinBox        *m_handicap_spin; // 0–9, default 0
    QPushButton     *m_ok_btn;
    QPushButton     *m_cancel_btn;
};
```

When handicap > 0 is selected, komi automatically resets to 0.5 (Japanese handicap convention)
via a `QSpinBox::valueChanged` connection.

---

### 3.2 Modified Files

#### `xgospel2_fixed.cpp`

1. **New "Engines" menu** inserted between "Settings" and "Help":

```cpp
QMenu *engines_menu = menu->addMenu("Engines");
engines_menu->addAction("Manage Engines...",  this, &FixedXGospelWindow::openEnginesPrefs);
engines_menu->addSeparator();
// "Attach Engine" submenu is rebuilt dynamically from EngineManager::profiles()
// whenever the menu is about to show (connect QMenu::aboutToShow)
attach_submenu = engines_menu->addMenu("Attach Engine");
engines_menu->addAction("Detach Engine",      this, &FixedXGospelWindow::detachEngine);
engines_menu->addSeparator();
engines_menu->addAction("Play vs Engine...",  this, &FixedXGospelWindow::launchLocalEngineGame);
engines_menu->addAction("Engine vs Engine...",this, &FixedXGospelWindow::launchSelfPlay);  // deferred
engines_menu->addSeparator();
engines_menu->addAction("Toggle Analysis",    this, &FixedXGospelWindow::toggleAnalysis);  // deferred
```

2. **New private members:**

```cpp
EngineManager *engine_manager  = nullptr;
BoardWindow   *engine_board    = nullptr;
StoneColor     engine_color;           // Which color the engine plays
int            engine_game_id  = -2;  // Sentinel — negative avoids IGS game ID collision
```

3. **New slots:** `openEnginesPrefs()`, `attachEngine(QString profile_id)`, `detachEngine()`,
   `launchLocalEngineGame()`, `onEngineReady()`, `onEngineMove(int x, int y)`, `onEnginePass()`,
   `onEngineResigned()`, `onEngineScore(QString)`, `onEngineError(QString)`,
   `onLocalMove(int, int, int)`, `onLocalPass(int)`, `onLocalResign(int)`, `onLocalBoardClosed(int)`.

4. **`closeBoardWindow()`** must skip the `socket->write("unobserve N\n")` call when
   `game_id == engine_game_id` (negative ID, no IGS connection).

5. **New includes:** `#include "katago_engine.h"`, `#include "engine_manager.h"`,
   `#include "local_game_dialog.h"`, `#include <QtCore/QRandomGenerator>`.

---

#### `board_window.h` / `board_window.cpp`

1. New private member: `bool local_play_mode = false;`
2. New public method: `void setLocalPlayMode(bool enabled);`
3. New signal: `void passRequested(int game_id);`
4. New accessor: `int getCurrentMove() const { return current_move; }`
5. New accessor: `int getConsecutivePasses() const { return consecutive_passes; }`
6. Pass button added to `setupUI()` layout, hidden by default, shown by `setLocalPlayMode(true)`.
7. `onPassClicked()` branches: if `local_play_mode && is_playing` → emit `passRequested(observed_game_id)`.

---

#### `Makefile`

Add `katago_engine.cpp` and `local_game_dialog.cpp` to `SOURCES`, add their headers to `MOC_HEADERS`, add dependency lines for `.o` and `.moc` targets.

---

## 4. Coordinate System

### 4.1 xgospel2 Internal Coordinates

- `x = 0` → leftmost column (A in GTP)
- `y = 0` → **top** row (row 19 in GTP)
- `y = 18` → bottom row (row 1 in GTP)

### 4.2 GTP Column Letters

GTP uses A–T **skipping I**:

| x | GTP | x | GTP |
|---|-----|---|-----|
| 0 | A | 9  | K |
| 1 | B | 10 | L |
| 2 | C | 11 | M |
| 3 | D | 12 | N |
| 4 | E | 13 | O |
| 5 | F | 14 | P |
| 6 | G | 15 | Q |
| 7 | H | 16 | R |
| 8 | J | 17 | S |
|   |   | 18 | T |

### 4.3 Conversion Functions

```cpp
QString KataGoEngine::coordToGtp(int x, int y) const {
    char col = (x < 8) ? ('A' + x) : ('A' + x + 1);  // skip I
    int  row = 19 - y;
    return QString("%1%2").arg(col).arg(row);
}

bool KataGoEngine::gtpToCoord(const QString &vertex, int &x, int &y) const {
    QString v = vertex.toUpper().trimmed();
    if (v == "PASS")   { x = -1; y = -1; return true; }
    if (v == "RESIGN") { x = -2; y = -2; return true; }
    if (v.length() < 2) return false;
    char col = v[0].toLatin1();
    if (col == 'I') return false;
    x = (col >= 'J') ? (col - 'A' - 1) : (col - 'A');
    bool ok;
    int row = v.mid(1).toInt(&ok);
    if (!ok || row < 1 || row > 19) return false;
    y = 19 - row;
    return true;
}
```

These conversions match the existing transformation in `FixedXGospelWindow::sendMove()`.

---

## 5. GTP Protocol Notes

### 5.1 Response Format

```
= <body>\n\n    # success — double newline terminates multi-line responses
? <msg>\n\n     # error
```

Parse by accumulating stdout bytes; treat blank line as response terminator.

### 5.2 Startup Sequence

```
1. QProcess::setWorkingDirectory(KATAGO_DIR)
2. QProcess::start("katago", ["gtp", "-model", KATAGO_MODEL, "-config", KATAGO_CFG])
3. Start 30-second watchdog QTimer
4. onStderrReady(): scan for "GTP ready, beginning main protocol loop"
5. When found: cancel watchdog, enqueue init sequence:
     boardsize 19
     komi <komi>
     clear_board
     [place_free_handicap N]   ← only if handicap > 0
6. emit engineReady() after init ACKs complete
```

### 5.3 In-Game GTP Commands

| Action | GTP | Response |
|--------|-----|----------|
| Place user stone | `play B D4` | `= ` |
| User passes | `play B pass` | `= ` |
| Request KataGo move | `genmove W` | `= D16` / `= pass` / `= resign` |
| Final score | `final_score` | `= B+2.5` |
| Shutdown | `quit` | (process exits) |

### 5.4 Pondering

`ponderingEnabled = true` in `default_gtp.cfg`. KataGo ponders on the opponent's turn automatically. This is transparent to the command queue — any new command interrupts pondering. No special handling required.

### 5.5 Response Dispatch

`m_pending_cmd` records what command is in-flight so the parser interprets `= \n` correctly:

- Pending `genmove`: parse vertex → `moveReady(x,y)` / `passMoveReady()` / `engineResigned()`
- Pending `final_score`: emit `scoreReady(result)`
- Pending `play` / `boardsize` / `komi` / `clear_board`: ACK, call `dispatchNext()`
- Pending `place_free_handicap`: parse vertex list → render handicap stones, call `dispatchNext()`

---

## 6. Phase 1 Implementation Plan

### Step 1: Implement `KataGoEngine`
1. Class skeleton, static path constants
2. `start()`: set working dir, launch process, start watchdog
3. `onStderrReady()`: scan for "GTP ready", kick init sequence
4. `onStdoutReady()`: accumulate bytes, call `handleResponse()` on blank line
5. `handleResponse()`: parse `=`/`?`, dispatch on `m_pending_cmd`
6. `enqueue()` / `dispatchNext()`
7. `stop()`: send `quit`, 2-second grace period, then `QProcess::kill()`
8. Coordinate converters

### Step 2: Implement `EngineManager` + `EnginesPrefsWidget`
1. `EngineProfile` struct + serialisation to/from `Settings` (`QList<QVariantMap>`)
2. `EngineManager`: load/save profiles, `attach()`, `detach()`, `currentEngine()`
3. `EnginesPrefsWidget`: list widget + Add/Edit/Remove + logging checkbox/path
4. `EngineEditDialog`: name, path (with Browse), args fields
5. Wire into existing `PreferencesDialog` as new "Engines" tab

### Step 3: Implement `LocalGameDialog`
1. Shows engine name at top, color picker, komi spin, handicap spin
2. Handicap → komi auto-reset connection
3. `userColor()` returns EMPTY for random

### Step 4: Modify `BoardWindow`
1. `local_play_mode` member + `setLocalPlayMode()`
2. `passRequested` signal
3. Pass button in `setupUI()` (hidden by default)
4. `onPassClicked()` branch for local play mode
5. `getCurrentMove()` and `getConsecutivePasses()` accessors

### Step 5: Wire `FixedXGospelWindow`
1. Includes, private members
2. Engine menu + `launchLocalKataGoGame()`
3. All local-game slots
4. Guard `closeBoardWindow()` against IGS unobserve for local games

### Step 6: Update `Makefile`

### Step 6: Game Flow

**User plays Black:**
```
engineReady() → wait for user click
User clicks (3,3) → onLocalMove → sendPlay(B,3,3) + requestGenmove(W)
KataGo: "= D4" → onKataGoMove(3,15) → processMove(WHITE, 3, 15)
```

**User plays White:**
```
engineReady() → requestGenmove(B)
KataGo: "= Q16" → onKataGoMove(15,2) → processMove(BLACK, 15, 2)
User clicks → onLocalMove → sendPlay(W,x,y) + requestGenmove(B)
```

**Two-pass game end:**
```
onLocalPass() → sendPass(user_color) + requestGenmove(katago_color)
onKataGoPass() → if consecutive_passes >= 2 → requestFinalScore()
onKataGoScore("B+2.5") → katago_board->updateGameResult("B+2.5")
```

**Resign:**
- User resigns: `onLocalResign()` → `katago_engine->stop()` → `updateGameResult("W+Resign")`
- KataGo resigns: `onKataGoResign()` → `updateGameResult("B+Resign")`

---

## 7. Phase 2 Design Notes (Deferred)

### 7.1 IGS Challenge Detection
Parse `"N <player> wants to play..."` or nmatch protocol challenge lines in `onDataReceived()`. Gate on new `bot_mode_enabled` setting.

### 7.2 Accept Criteria (configurable)
- Rank range, board size (19x19 only), free/rated preference, max handicap

### 7.3 Move Relay Loop
```
IGS move → processMove() + katago_engine->sendPlay() →
requestGenmove() → onKataGoMove() → socket->write("<vertex>\n")
```

### 7.4 Time Control
Send `time_settings` and `time_left` commands to KataGo before each `genmove`, sourced from IGS clock data already present in `BoardWindow`.

### 7.5 Concurrency
Accept at most one active KataGo game at a time. Auto-decline second challenge while a game is in progress.

---

## 8. Open Questions

1. **Pass button placement:** Pass currently appears only in `setupEditUI()`. A separate Pass button in `setupUI()` (hidden by default) is cleanest. Confirm it doesn't conflict with the edit window's Pass button.

2. **`katago_game_id = -2` sentinel:** Verify `closeBoardWindow()`, `board_windows` list management, and `startObserving()` all tolerate a negative game ID. The `unobserve` IGS write must be skipped when `game_id < 0`.

3. **Handicap stone rendering:** `place_free_handicap N` returns a vertex list. Parse the response in `handleResponse()`, convert to `(x, y)` pairs, and emit a new signal `handicapStonesReady(QList<QPair<int,int>>)` for `FixedXGospelWindow` to render via `processMove()` with color BLACK.

4. **KataGo path hardcoding:** Acceptable for Phase 1. Phase 2 should move paths to `Settings` + a new "Engine" tab in `PreferencesDialog`.

5. **Pondering shutdown:** `katago_engine->stop()` sends `quit` to stdin; KataGo stops pondering and exits. If process does not exit within 2 seconds, `QProcess::kill()`. No other action needed.

6. **Japanese rules end-of-game:** After two consecutive passes, show the `final_score` result alongside the board's interactive scoring UI. Both can coexist — `final_score` is advisory; user-driven dead stone marking (existing infrastructure) remains primary.

7. **`QRandomGenerator` availability:** Requires Qt 5.10+. Fallback: `static_cast<StoneColor>((qrand() % 2) + 1)`.

---

## 9. File Change Summary

| File | Type | Description |
|------|------|-------------|
| `katago_engine.h` | New | `KataGoEngine` QObject class — GTP process wrapper |
| `katago_engine.cpp` | New | GTP engine implementation |
| `engine_manager.h` | New | `EngineProfile` struct, `EngineManager` QObject |
| `engine_manager.cpp` | New | Profile load/save, attach/detach, `EnginesPrefsWidget`, `EngineEditDialog` |
| `local_game_dialog.h` | New | `LocalGameDialog` QDialog header |
| `local_game_dialog.cpp` | New | Game setup dialog (engine name, color, komi, handicap) |
| `xgospel2_fixed.cpp` | Modified | Engines menu, all engine/local-game slots, includes |
| `preferences_dialog.h/.cpp` | Modified | New "Engines" tab wired to `EnginesPrefsWidget` |
| `board_window.h` | Modified | `local_play_mode`, `passRequested` signal, new accessors |
| `board_window.cpp` | Modified | Pass button in `setupUI()`, `onPassClicked()` branch, `setLocalPlayMode()` |
| `settings.h/.cpp` | Modified | `saveEngineProfiles()` / `loadEngineProfiles()` using `QVariantList` |
| `Makefile` | Modified | New source files, MOC headers, dependency lines |

### Deferred to Phase 1b
- Engine vs Engine (self-play) mode
- Toggle Analysis / best-move hints overlay
- SSH wrapper engine support (works automatically — path field accepts any executable/script)
