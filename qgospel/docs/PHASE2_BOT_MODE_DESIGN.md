
# XGospel2 Phase 2 — IGS Bot Mode Design

**Date:** 2026-05-12  
**Status:** Design / Pre-implementation  
**Depends on:** Phase 1 complete (v89) — KataGo local engine play fully working

---

## 1. Goal

Allow a logged-in xgospel2 account to act as a Go bot on IGS: KataGo plays on behalf of
the user account, accepting incoming match requests automatically, managing time, and
sending moves to the server in real time.

The human operator monitors the game via the existing board window and can intervene
(resign, pass, or send a manual GTP command) if needed.

---

## 2. Scope

**In scope:**
- Bot mode toggle (on/off) in the main window
- Auto-accept incoming `match` and `nmatch` requests within configured constraints
- Canadian byoyomi time management (`kgs-time_settings canadian`)
- Move submission to IGS via `sendMove()` after KataGo responds
- Post-move `time_left` parsing and forwarding to KataGo
- Engine rules configuration (`kata-set-rules japanese`)
- Bot status display in the UI (active/idle, current game, time remaining)
- Minimum byoyomi period: 60 seconds (1 minute), as common on IGS

**Out of scope for Phase 2:**
- Multiple simultaneous bot games (one game at a time)
- Rank-based accept/decline filtering (Phase 2.5 if needed)
- Handicap games as bot (may add if straightforward)
- Analysis/hints overlay on the board (Phase 3)

---

## 3. IGS Protocol Constraints

### 3.1 Rules
IGS uses Japanese rules by default. KataGo must be told explicitly:
```
kata-set-rules japanese
```
This must be sent during engine initialization, after `clear_board`, before the first move.

### 3.2 Time System — Canadian Byoyomi
IGS uses Canadian byoyomi as its primary time control. The GTP command to configure it:
```
kgs-time_settings canadian <main_time_seconds> <byoyomi_period_seconds> <stones_per_period>
```
Example for a 10-minute game with 25 stones in 5 minutes:
```
kgs-time_settings canadian 600 300 25
```

After each move IGS sends a `time_left` line (CMD 35 or embedded in CMD 15):
```
15 <game_id> <move_num> <color> <vertex> <white_time> <black_time>
```
The time fields are in seconds. After each of KataGo's moves the client must send:
```
time_left <color> <seconds_remaining> <stones_remaining_in_period>
```
to keep KataGo's internal clock synchronized with the server.

### 3.3 Minimum Byoyomi Period
IGS enforces a minimum byoyomi period of 60 seconds. The bot must not request or accept
games with a byoyomi period shorter than 60 seconds.

### 3.4 Komi
Standard IGS rated games use komi 6.5 (Japanese rules). The bot should use 6.5 by default.
Handicap games adjust komi to 0.5 per IGS convention.

---

## 4. Architecture

### 4.1 Bot Mode Flag
A new `bool bot_mode_active` member on `FixedXGospelWindow`. When true:
- Incoming match/nmatch requests are auto-accepted (subject to constraints below)
- The engine is attached to the IGS game board (not a local board)
- Move signals from KataGo route to `sendMove()` instead of `onLocalMove()`

### 4.2 Match Auto-Accept Flow
```
IGS sends: "9 NMatch requested with opponent(B 0 19 300 600 25 0 0 0)."
    ↓
Parse: board size, main time, byoyomi time, byoyomi stones, color
    ↓
Validate constraints (19x19 only, byoyomi >= 60s, no active bot game)
    ↓
Send accept command: "nmatch opponent B 0 19 <main> <byoyomi> <stones> 0 0 0"
    ↓
Wait for CMD 67 (game created) → game_id known
    ↓
Attach engine_manager to new game:
  - kgs-time_settings canadian <main> <byoyomi> <stones>
  - kata-set-rules japanese
  - boardsize <N>
  - komi <komi>
  - clear_board
    ↓
If bot plays Black → send genmove black immediately
If bot plays White → wait for opponent's first move (CMD 15)
```

### 4.3 Move Loop
```
Opponent move arrives (CMD 15)
    ↓
board->processMove() updates board state
    ↓
engine->enqueue("play <color> <vertex>")   ← inform KataGo of opponent move
engine->enqueue("genmove <bot_color>")     ← ask for bot's response
    ↓
KataGoEngine::moveReady(vertex) signal
    ↓
onBotEngineMove(vertex):
  sendMove(vertex)                          ← write to IGS socket
  engine->enqueue("time_left <bot_color> <seconds> <stones>")
```

### 4.4 Time Tracking
Parse time from CMD 15 move lines. IGS embeds remaining time per color after each move.
Store `bot_time_remaining` and `bot_stones_in_period` as member variables, updated after
each move. Send `time_left` to KataGo immediately after each `genmove` response.

### 4.5 Engine Reuse
Phase 1 already has `engine_manager` and `KataGoEngine`. Bot mode reuses the same
`engine_manager`. Key difference: in bot mode the engine is attached to an IGS game, not
a local board — so `onBotEngineMove` routes to `sendMove()` instead of `engine_board`.

---

## 5. UI Changes

### 5.1 Bot Mode Toggle
A **"Bot Mode"** toggle button or checkbox in the main console window toolbar.
- When OFF: normal human play, incoming match requests show the existing dialog
- When ON: auto-accept indicator visible; incoming requests are silently accepted and logged

### 5.2 Bot Status Panel
A small status area (could be in the existing console or a dock widget) showing:
- Bot mode: ON / OFF
- Current game: opponent name, game ID, color
- Time remaining: white / black
- Last move: vertex, move number

### 5.3 Bot Player Dialog
When another player sends a tell to the bot account, a `PlayerStatsDialog` should still
open (existing behavior). The bot can send a canned auto-reply (e.g. "I am a bot powered
by KataGo. GG!") — configurable in Preferences. This is optional for Phase 2.

### 5.4 Board Window
The existing `BoardWindow` is reused. In bot mode it shows the live IGS game exactly as it
would for a human playing. The engine console panel (Analysis pane) is already always
visible and shows the GTP I/O log so the operator can monitor KataGo's activity.

The **Resign** button remains functional so the operator can resign on KataGo's behalf if
something goes wrong.

---

## 6. Constraints and Validation

| Constraint | Rule |
|-----------|------|
| Board size | Accept 19x19 only (9x9 / 13x13 can be added later) |
| Byoyomi period | Must be >= 60 seconds; decline otherwise |
| Simultaneous games | Only one bot game at a time; decline if already in a game |
| Time per move (local) | Not used — server time controls apply via `kgs-time_settings` |
| Rules | Always `kata-set-rules japanese` |
| Komi | Use server-negotiated komi (from nmatch params or CMD 7) |
| Handicap | Phase 2: accept only even games (handicap 0); defer handicap handling |

---

## 7. IGS Commands Reference

| Direction | Command | Purpose |
|-----------|---------|---------|
| → IGS | `nmatch <opp> <color> <hc> <size> <main> <byoyomi> <stones> 0 0 0` | Accept nmatch |
| → IGS | `match <opp> <color> <size> <main> <byoyomi>` | Accept old match (xgospel1, classic clients) |
| → IGS | `tell <player> <message>` | Bot auto-reply to incoming tell |
| → IGS | `<vertex>\n` (e.g. `K10\n`) | Send move |
| → IGS | `pass\n` | Send pass |
| → IGS | `resign\n` | Resign |
| ← IGS | CMD 15 | Move received (with time fields) |
| ← IGS | CMD 67 | Game created confirmation |
| ← IGS | CMD 35 | Time update |
| → KataGo | `kgs-time_settings canadian <main> <byoyomi> <stones>` | Set time control |
| → KataGo | `kata-set-rules japanese` | Set rules |
| → KataGo | `play <color> <vertex>` | Feed opponent move |
| → KataGo | `genmove <color>` | Request bot move |
| → KataGo | `time_left <color> <seconds> <stones>` | Sync remaining time |

---

## 8. Phase 2 Implementation Steps (Ordered)

1. **Bot mode toggle** — add `bot_mode_active` flag + UI button; no logic yet
2. **Auto-accept nmatch** — when bot mode on, intercept incoming nmatch, validate
   constraints, send accept command, suppress the manual dialog
3. **Auto-accept old match** — same flow for old `match` protocol; use IGS defaults
   (25 stones / 300s byoyomi) when stones count is not present in request line;
   the server's suggested command (`9 Use <match ...>`) is already parsed and stored
   in `pending_match_dialog` — reuse that path without showing the dialog
4. **Engine attach to IGS game** — on CMD 67 (game created in bot mode), attach
   `engine_manager`, send `kgs-time_settings canadian` + `kata-set-rules japanese`
   + `boardsize` + `komi` + `clear_board`
5. **Move loop** — wire `moveReady` → `onBotEngineMove` → `sendMove()`; wire incoming
   CMD 15 → `play <opp_color> <vertex>` → `genmove <bot_color>`
6. **Time sync** — parse time fields from CMD 15; send `time_left` after each bot move
7. **Resign/pass handling** — operator Resign button works; KataGo pass → IGS pass
8. **Auto-reply to tells** — when bot mode active and a tell arrives, pick a random
   entry from the canned response dictionary and send it via `tell <sender> <response>`
9. **Game end cleanup** — on resign/result, detach engine, reset bot state, ready for
   next game
10. **Testing** — fast games (60s byoyomi) and standard (300s byoyomi) with BusyBee
    (xgospel1, old match protocol) and GoPanda (nmatch protocol) as test opponents

---

## 9. Open Questions

- **Old `match` protocol vs `nmatch`**: Old `match` doesn't carry byoyomi stones count
  in the request line. **Decision: support both.** Old `match` is essential because older
  clients (xgospel1, and many classic IGS clients) cannot send nmatch. For old `match`
  requests, use IGS defaults for byoyomi: 25 stones / 5 minutes (300 seconds). The server
  always sends the suggested accept command (`9 Use <match opp B 19 5 5> or <decline opp>`)
  which we already parse — so the accept flow is identical; only the time defaults differ.
  Implementation cost is low since the match parsing path already exists.

- **Color preference**: Should the bot advertise a color preference, or accept either?
  Recommendation: accept either, let the challenger choose.

- **Auto-reply to tells**: A dictionary of canned responses adds personality and informs
  players they are facing a bot. **Decision: implement a small built-in response set.**
  Responses are chosen randomly from the dictionary on each incoming tell so the bot
  doesn't feel mechanical. The dictionary will cover common situations:
  - Greeting / game start (triggered on first tell in a game)
  - Good game / end of game
  - General / unknown (fallback for anything else)
  Example entries:
  - "Hello! I am a KataGo bot. Good luck and have fun!"
  - "Thanks for the game! I hope it was enjoyable."
  - "I am an automated bot powered by KataGo. I cannot read tells but wish you a good game!"
  - "Good game! Feel free to review it with me afterwards."
  - "I am just a bot — I cannot chat, but I enjoy playing! GL HF."
  The dictionary will be stored as a `QStringList` constant and can later be made
  user-configurable via Preferences. Auto-reply is enabled only when bot mode is active.

- **Byoyomi period parsing from CMD 15**: Need to confirm exact field positions in the
  CMD 15 time data for byoyomi stones remaining vs. main time remaining. Will verify
  against live server during implementation.

---

## 10. Success Criteria

- Bot accepts an nmatch request from BusyBee (xgospel1) without human intervention
- KataGo plays a complete game to completion (resign or counting)
- Time never exceeds server limit (no timeout forfeit)
- Operator can resign via the board window Resign button at any point
- Bot handles fast games (60s byoyomi, 25 stones) without timing out
