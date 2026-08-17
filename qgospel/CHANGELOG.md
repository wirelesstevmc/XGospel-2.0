# XGospel2 Changelog

All notable changes to the XGospel2 Qt5-based IGS Go client will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

---
### 2026-08-12 The below refenced mapping/encoding mistakes appear to be a recurring issue with Claude!
### For this reason I insisted that he create a reference document to avoid such mistakes in the future.
### We should probably make a comment in the code that illustrates the existing (correct) mapping/encoding
### Here is the document that Claude created: IGS_CMD22_TERRITORY_ENCODING.md

## 2026-08-09 (v286): Revert v285 coordinate/encoding changes — repeat of v270 mistake

### Revert — Restore v266/v275-consistent QPair(row, col) convention throughout

v285 changed `receiveScoreLine` to store `QPair(col, 18-row)` and changed black territory detection from `digit==5` to `digit==3`. This was a repeat of the v270 mistake that was deliberately reverted at v272. The CHANGELOG documents at v272:

> "Reverted all changes from v270 and v271-debug. The QPair storage convention change caused cascading regressions: incorrect territory overlays on all scored games, dead stone detection errors."

And at v275:

> "v274 swapped boardToScreen and getStoneAt arguments... This broke the editor scoring overlay. Root cause: the QPair (row, col) convention is used self-consistently throughout. Changing only some of the call sites breaks the self-consistency. All four sites restored to pos.first, pos.second matching v266."

**v285's encoding claim was also wrong.** The claim was that IGS CMD22 value 3 = black territory. Cross-checking game 417's CMD22 against CMD20 arithmetic proved definitively: value 4 = white territory (58 cells + 1 cap − 5.5 komi = 53.5 ✓ matches CMD20), and therefore **value 5 = black territory** (the original v266 code was correct all along).

**Reverts applied:**

`board_window.cpp` — `receiveScoreLine`:
- `QPair<int, int> pos(col, 18 - row)` → `QPair<int, int> pos(row, col)`
- `digit == 4 || digit == 3` debug guard → `digit == 4 || digit == 5`

`board_window.cpp` — `receiveScoreEnd` territory_map building:
- `digit == 3` for black territory → `digit == 5`
- dame: `digit == 0 || digit == 2 || digit == 5` → `digit == 2 || digit == 3`

`board_window.cpp` — `receiveScoreEnd` dead stone detection:
- `digit == 3 && actual_stone == WHITE_STONE` → `digit == 5 && actual_stone == WHITE_STONE`

`board_window.cpp` — SGF TW/TB generation:
- `char col = 'a' + pos.first; char row = 'a' + pos.second` → `char col = 'a' + pos.second; char row = 'a' + pos.first`
- `ownership == 3` for black territory → `ownership == 5`

`xgospel2_fixed.cpp` — inactive-slot CMD22 buffer:
- `QPair<int,int>(col, 18 - row_number)` → `QPair<int,int>(row_number, col)`

`xgospel2_fixed.cpp` — inactive-slot territory decode:
- `digit == 3` for black territory → `digit == 5`
- dame: `digit == 0 || digit == 2 || digit == 5` → `digit == 2 || digit == 3`

`xgospel2_fixed.cpp` — inactive-slot prisoner counting:
- `digit == 3` for black prisoner → `digit == 5`

**Established invariant (DO NOT CHANGE):**

The self-consistent convention is:
- `receiveScoreLine` stores `QPair(row, col)` — row first, col second
- IGS CMD22: 0=dame, 1=W-stone, 2=B-stone, 3=dame/neutral, **4=W-territory, 5=B-territory**
- All draw functions use `boardToScreen(pos.first, pos.second)` — matching row/col order
- SGF generation: `col = 'a' + pos.second; row = 'a' + pos.first`

v284 anomaly detection code is retained unchanged.

---

## 2026-08-09 (v285): Fix CMD22 territory overlay — coordinate inversion and encoding [REVERTED in v286]

*(This was a repeat of the v270 mistake. See v286 for revert details.)*

---

## 2026-08-09 (v284): IGS scoring anomaly detection and persistent log

### Feature — CMD20 vs local-score discrepancy detection

When a bot game ends and the server's CMD20 score disagrees with our local KataGo-based territory calculation (from the Pass 2 overlay) by 10 or more points, xgospel2 now:

1. **Emits a `*SYSTEM*` warning** in Comments & Kibitz identifying the game number, server score, local score, gap, and whether the winner disagrees.
2. **Appends a timestamped entry** to `~/xgospel2_scoring_anomalies.log` with full details (player names, raw CMD20 values, local raw values) for documentation and potential reporting to IGS administrators.

The Pass 2 local scores (`bot_local_white_score` / `bot_local_black_score`) are captured in `applyBotTerritoryOverlay` when both players have typed done. The comparison happens in the CMD20 handler immediately after score delivery. The 10-point threshold absorbs komi rounding and minor positional disagreements while flagging genuine algorithmic failures like the game 350 F16 incident (37-point gap, winner wrong: B+21.5 server vs W+20.5 local).

---

## 2026-08-09 (v283): Dead stone markers survive CMD22 board replacement

### Bug — Dead stone X markers erased when CMD22 territory map arrives

`drawDeadStoneMarkers` called `getStoneAt(pos)` to determine the stone color for the X marker. After CMD22 arrives, `receiveScoreEnd` replaces the board display with the territory map — stones the server already removed (e.g. F16, a dead black stone toggled via CMD49) become empty (`5`/dame) in the CMD22 encoding. So `getStoneAt` returned EMPTY → `continue` → marker silently suppressed.

Fix: added `dead_stone_colors: QMap<QPair<int,int>, StoneColor>` to `GoBoardWidget`. `setDeadStones()` populates this map by reading `getBoardState()` for each position **at mark time** (while the stone is still on the board). `drawDeadStoneMarkers` falls back to `dead_stone_colors` when `getStoneAt` returns EMPTY, so the X marker persists correctly after CMD22 replaces the board.

---

## 2026-08-08 (v282): Three fixes: bot dead stone visual marking, server score in C&K, board geometry persistence

### Fix A — Bot's own dead stones not visually marked in docked mode
The 1500ms pre-removal timer marked dead stones only on `engine_board`, which is null in normal docked bot games (non-Eve mode). The CMD49 echo handler was correctly skipping bot's own removals to avoid double-toggle, leaving them permanently unmarked. Restored the v273 fallback: when `engine_board` is null, fall back to `shared_board_window` (docked) or the matching `board_windows` entry (non-docked). Added already-marked guard to prevent double-toggle.

### Fix B — Comments & Kibitz showed locally calculated score instead of server score
`updateGameResult()` computed `white_territory + white_captures + komi` locally for the "Game finished" comment line, ignoring the server-provided CMD20 values (`server_white_score`, `server_black_score`). Now uses server values when `has_server_score` is true. Core principle: never recalculate what the server already computed.

### Fix C — Board window geometry not persisted across sessions in docked mode
`closeEvent` saves geometry correctly, but in docked mode the shared board window is hidden (not destroyed) between sessions — `closeEvent` never fires. Added `hideEvent` override that saves geometry, splitter sizes, and calls `settings->save()` whenever the window is hidden. Constructor already restores from settings on first show; now the saved values are always up to date.

---

## 2026-08-07 (v281): Fix SIGSEGV — getBotBlacklist() dangling iterator in match/nmatch handler

### Bug — Crash in bot match/nmatch accept handler when blacklist is non-empty

`settings->getBotBlacklist()` returns `QStringList` by value. The two calls `settings->getBotBlacklist().constBegin()` and `settings->getBotBlacklist().constEnd()` in `std::any_of(...)` each created a separate temporary `QStringList`. The iterators from the first temporary were dangling by the time `std::any_of` iterated — undefined behavior → SIGSEGV. Reproduced when qzg sent an nmatch request to the bot. Fixed by binding the result to a local `const QStringList bl` in both the `match` (line ~7418) and `nmatch` (line ~7456) suggested-command handlers before calling `any_of`.

---

## 2026-08-07 (v280): Board window not re-shown after close + re-observe

### Bug — Board window stays hidden after user closes it and re-observes a game

`closeBoardWindow` correctly calls `shared_board_window->hide()` when the last slot is closed and sets `active_slot_game_id = -1`. When the user then clicks a game to observe, `observeGame` creates a new slot, calls `loadSlot`, but never calls `show()` on the already-existing (hidden) `shared_board_window` — the `if (!shared_board_window)` creation block is skipped because the window already exists. Fix: after the creation block, added `if (!shared_board_window->isVisible()) shared_board_window->show()` so the window re-appears whenever a game is loaded into a previously-hidden board window.

---

## 2026-08-07 (v279): SIGSEGV crash handler — backtrace to session log

Added a `SIGSEGV` signal handler in `main()` that calls `backtrace()` and `backtrace_symbols_fd()` to write a stack trace to stderr (redirected to the session log) before re-raising the signal. This makes previously silent UI-thread crashes (e.g. splitter drag crash in v278) produce an actionable call chain in the session log.

---

## 2026-08-07 (v278): closeBoardWindow — detachSlotClockTimer before slot delete (all branches)

### Bug — shared_board_window->clock_timer dangled after slot delete causing segfault on re-observe

`closeBoardWindow` deleted a `GameSlot` without first detaching its `clock_timer` from `shared_board_window`. On the next `loadSlot` call, `disconnect(clock_timer, ...)` operated on the freed QTimer pointer → segfault. Fixed by calling `shared_board_window->detachSlotClockTimer()` before `delete slot` in both the active-slot and non-active-slot branches of `closeBoardWindow`.

---

## 2026-08-07 (v277): closeBoardWindow — stop clock_timer before delete (non-active-slot path)

### Bug — clock_timer fire on freed GameSlot causing segfault on board close

Non-active-slot path in `closeBoardWindow` deleted the slot without stopping its `clock_timer` first. The timer could fire between `delete slot` and GC, invoking a callback on freed memory. Fixed by adding `slot->clock_timer->stop()` before `delete slot`.

---

## 2026-08-07 (v276): Full revert to v266 baseline + re-apply v267/v268/v269/v271 fixes

Reverted all source files from v266 backup (`xgospel2_v266-DEAD-STONE-MARKERS-AND-SAVE-GAME-RETENTION-FIXES_07142026.tar.gz`) after v275 left scoring overlay, editor/score function, and SGF generation in broken states. Re-applied v267 (observed-game ID recycle eviction + blacklist case sensitivity), v268 (bot desync recovery + Refresh Board button), v269 (systematic game-ID reuse hardening + "Board is restored" handler), and v271 (editGame guard) cleanly on top.

---

## 2026-08-07 (v275): Revert v274 draw-function swap; restore v266-consistent coordinate convention

v274 swapped `boardToScreen` and `getStoneAt` arguments in the three draw functions and in `receiveScoreEnd` dead stone detection. This broke the editor scoring overlay and made the observed-game overlay worse. Root cause: the QPair `(row, col)` convention is used self-consistently throughout — draw functions, stone lookup, and dead stone detection all use `pos.first` as the first board_state index. Changing only some of the call sites breaks the self-consistency. All four sites restored to `pos.first, pos.second` matching v266.

v273 fixes (double-toggle prevention, bot dead stone pre-marking) are retained.

---

## 2026-08-07 (v274): Fix territory/dead stone overlay screen position transposition

### Bug — All overlay markers drawn at transposed board positions

**Root cause:** `GoBoardWidget::drawTerritoryMarkers`, `drawDeadStoneMarkers`, and `drawDisputedMarkers` all called `boardToScreen(pos.first, pos.second)` where QPair stores `(row_number, col)`. `boardToScreen(x, y)` maps x→horizontal (column axis) and y→vertical (row axis). With `pos.first=row_number` as x, the horizontal and vertical axes were swapped — a 90° transposition of the entire overlay. Territory markers landed at wrong positions. Dead stone markers additionally called `getStoneAt(pos.first, pos.second)` with the same transposition: for most dead stone positions the transposed cell is empty, hitting the `continue` guard and silently suppressing the marker entirely.

**Fix:** Swapped to `boardToScreen(pos.second, pos.first)` and `getStoneAt(pos.second, pos.first)` in all three draw functions. `pos.second=col` maps correctly to the horizontal axis and `pos.first=row_number=row_from_top` maps correctly to the vertical axis. Change is self-contained in the paint path — no QPair storage convention changed.

---

## 2026-08-07 (v273): Dead stone overlay — double-toggle fix + bot's own stones marked

### Bug A — Opponent's second CMD49 for same group un-marks all dead stone markers

**Symptom:** When an opponent sent multiple CMD49 stone-removal messages for the same connected group (e.g., carlson sending B16 then C17 for a group of 4 white stones), the first CMD49 correctly flood-filled and marked the entire connected group. The second CMD49 found the seed coordinate already in `dead_stones`, triggered `markStoneAsDead`'s toggle-off branch, and un-marked the entire group. Net result: no dead stone markers visible for the opponent's removed group.

**Root cause:** `markStoneAsDead` uses a toggle: if the seed is already marked, it un-marks the full connected group. This is correct for interactive user clicks (allowing players to unmark stones), but wrong when called programmatically from CMD49 — where each message is an independent confirmation of the same agreed removal, not a reversal.

**Fix:** In the CMD49 handler (both docked active-slot path and non-docked path), check if the seed coordinate is already in `dead_stones` before calling `markStoneAsDead`. If already marked, skip — the group was fully captured by the first CMD49 flood-fill. Same guard added to the inactive-slot flood-fill path.

### Bug B — Bot's own dead stones never shown with dead stone markers

**Symptom:** Dead stone overlay showed no markers on the bot's own removed groups (M14, E12, Q10, L4 in game #41). Only the opponent's removals (when not double-toggled) showed markers.

**Root cause:** The 1500ms scoring timer (which fires before the bot sends its first removal) was supposed to pre-mark the bot's dead groups visually. The code called `engine_board->markStoneAsDead()` — but `engine_board` is only non-null in Eve mode (KataGo vs KataGo). In normal bot games it is null, so the if-guard silently skipped all marking. The CMD49 echo handler skipped bot's own removes (`is_bot_own_remove_docked` guard) to prevent double-toggle — but since the pre-marking never ran, nothing marked the stones.

**Fix:** Extended the 1500ms timer to fall back to `shared_board_window` (docked mode) or the active `BoardWindow` (non-docked) when `engine_board` is null. Added `already_marked` guard before calling `markStoneAsDead` to prevent double-toggle if the CMD49 echo fires anyway.

---

## 2026-08-07 (v272): Revert v270 territory QPair transposition fix

Reverted all changes from v270 and v271-debug. The QPair storage convention change (`QPair(row,col)` → `QPair(col,row)`) caused cascading regressions: incorrect territory overlays on all scored games, dead stone detection errors, and suspected game ID corruption. Root cause of the original S7/T7/R8/T8 green-dame issue in game #55 was confirmed as an IGS server mis-encoding (S7 coded as digit `1` alive white stone instead of digit `4` white territory), not a client rendering bug. The overlay transposition was a pre-existing cosmetic issue that is visually tolerable; the regressions from the fix were not.

Restored: `QPair(row,col)` storage in `receiveScoreLine()` and inactive-slot CMD22 buffer; `getStone(pos.second, pos.first)` in dead stone detection (both active and inactive slot paths); `pos.second/pos.first` swap in SGF generation; all draw functions left unchanged at `boardToScreen(pos.first, pos.second)`.

---

## 2026-08-04 (v271): Edit Game available for SGF files opened from disk

### Bug — Edit Game button rejected SGF files loaded via Open SGF

**Symptom:** Opening a saved SGF file via File → Open SGF and then clicking Edit Game showed "No game currently being observed. Edit Game is available when observing a game." — making it impossible to edit or annotate any saved game record.

**Root cause:** `BoardWindow::editGame()` guarded entry with `!is_observing && !is_playing && !game_finished`. A board loaded via Open SGF has all three flags false (it is not a live IGS game), so the guard fired. The condition was written only with live-game states in mind and never accounted for the SGF-from-disk case.

**Fix:** Added `&& !game_root` to the guard. `game_root` is non-null whenever a game tree has been loaded (either from a live game or from an SGF file). A board with no game tree at all (`game_root == nullptr`) is genuinely empty and has nothing to edit — the message is appropriate there. Updated the message text to mention SGF files as a valid context. One-line change in `board_window.cpp`.

---

## 2026-08-02 (v270): Territory overlay row/column transposition fix

### Bug — Scoring overlay rendered at wrong positions (90° rotated)

**Symptom:** After scoring, the territory markers (white/black territory boxes and dame/dead stone green squares) appeared at completely wrong intersections. Dead stone markers from near the bottom-centre of the board (CMD22 digit `3`) rendered as green squares in the wrong quadrant — in game #55 (got2go vs Raul, B+1.5) the dead-stone cells around L-N rows 1-4 appeared as green dame at S7/T7/R8/T8, causing white's territory count to show as 74 instead of the correct 78. The visual result misled players into thinking valid territory was uncounted dame.

**Root cause:** `territory_ownership` stores `QPair<int,int>` keys. Both insertion paths (active-slot `receiveScoreLine()` and inactive-slot CMD22 buffer in `xgospel2_fixed.cpp`) stored `QPair(cmd22_row_number, cmd22_col)` — row index first. But all rendering functions call `boardToScreen(pos.first, pos.second)` and all stone-lookup calls use `board_state[pos.first][pos.second]` / `getStone(pos.first, pos.second)`, where the first argument is always the column (x-axis) and the second is the row from top (y-axis). The entire territory overlay was transposed 90° — row rendered as column and column as row.

The same transposition affected the SGF generation code, but that code accidentally compensated: the comment said `pos.second = col` while pos.second was actually `cmd22_row`, so the labels were wrong but the variable usage happened to produce correct SGF output anyway. After the QPair insertion fix, the SGF code needed its comments updated to reflect the new order.

**Fix:** Corrected `QPair` insertion order at both CMD22 storage sites to store `QPair(col, row_from_top)`:
- `receiveScoreLine()` in `board_window.cpp`: `QPair<int,int>(col, row)` (was `QPair(row, col)`)
- Inactive-slot CMD22 buffer in `xgospel2_fixed.cpp`: `QPair<int,int>(col, row_number)` (was `QPair(row_number, col)`)

All rendering functions (`drawTerritoryMarkers`, `drawDeadStoneMarkers`, `drawDisputedMarkers`) and all stone-lookup calls (`receiveScoreEnd`, inactive-slot dead stone detection) remain unchanged — they already use `pos.first` as column and `pos.second` as row, which is now correct. SGF generation comments updated to reflect the new QPair convention.

**Impact:** Every game that showed a CMD22 scoring overlay was displaying territory and dead stone markers at transposed positions. The score_engine overlay (from KataGo analysis) was always correct — it stored QPairs as `(col, row)` from the start. The server score (CMD20) was also always correct — only the CMD22 visual overlay was wrong.

---

## 2026-07-28 (v269): Systematic game ID reuse hardening across all protocol handlers

### Root cause (recurring)
IGS aggressively recycles game IDs. When a game ends its slot is remapped to a negative ID (`-N`, `game_finished=true`). If a new game is assigned ID `N` before a late server message for the old game arrives, `findSlot(N)` returns the NEW game's slot — corrupting it. Similarly, player-name loops that skip `game_finished` but don't check `is_scoring_mode` can match a new active game when looking for a just-finished one. This class of bug caused the v268 territory overlay corruption (CMD22 from an unrelated game painted over a finished game's board).

### Bug — Bot fails to re-send dead stone removals after IGS "Board is restored"

**IGS scoring protocol:** After both players pass, IGS opens a scoring negotiation phase. Each player independently marks dead stones in their own territory and types `done`. However, if either player continues marking stones they missed AFTER already typing `done`, IGS resets the entire scoring board for BOTH players with `9 Board is restored to what it was when you started scoring` / `9 Please type 'done again.` Both players must then re-submit all their dead stone removals from scratch before typing `done` again.

**Root cause:** The bot's scoring re-entry handler only triggered on `check your score with the score command` — the initial scoring trigger. The "Board is restored" message uses entirely different text and was never caught. When IGS fired the restore (as happened in game 23 vs happyman77, who disputed the bot's correct removals of J18/H17/K16/H8+H7+G7), the bot silently re-sent `done` via the "has typed done" re-send path — without re-submitting any removals. All six dead opponent stones were treated as alive, flipping the result from W+1.5 to B+37.5 — a 39-point swing.

**Fix:** Added a dedicated handler for the "Board is restored" line that fully resets all scoring state (`bot_done_sent=false`, `bot_scoring_pending=true`, clears `bot_pending_removes/positions/groups`, resets retry counters) and re-requests KataGo ownership analysis from scratch. The 1500ms timer then re-identifies dead groups and re-sends all removals before sending `done` again — exactly as if scoring had just started.

**Testing required:** Must be verified with a mock game where the opponent disputes the bot's removals after already typing done, forcing a board restore. The console log should show `[BOT] Board restored by IGS — re-running ownership analysis and re-sending removals` followed by a full new set of removal commands.

---

### Fixes — seven handlers hardened

**CMD22 territory header (docked loop, line ~6422):** Added `if (slot->is_playing && !slot->is_scoring_mode) continue;` after the `game_finished` guard. This precisely targets only the bot's own playing slot (`is_playing=true`): it must have entered scoring mode before receiving CMD22. Purely observed game slots (`is_playing=false`) are always eligible and pass through unconditionally. A pure `!is_scoring_mode` guard was first attempted but caused a regression (v269 patch): observed games never have `is_scoring_mode` set before CMD22 arrives, so that guard silently blocked all territory data for observed games.

**CMD20 counting result (docked loop, line ~7657):** Added `if (!slot->is_scoring_mode) continue;` alongside the existing `game_finished` guard. CMD20 follows CMD9/CMD22 so the matching slot must already be in scoring mode; this prevents a new rematch (same two players, not yet in scoring) from being falsely matched.

**Counting result / `{Game N: W score B score}` handler (line ~6720):** Changed `findSlot(game_id)` to `findSlot(game_id); slot && !slot->game_finished`. A late counting result for old game N can no longer corrupt a newly started game N.

**Resign result (structured `{Game N: ...}` format, line ~6835):** Same `!slot->game_finished` guard added.

**Simple resign (`has resigned the game`, line ~6894):** Same guard.

**No-move time loss (line ~6944) and time forfeit (line ~6975) and adjourned result (line ~7000):** Same `!slot->game_finished` guard added to all three.

**CMD49 dead stone removal (line ~7286):** Added `!slot->game_finished` guard. A late CMD49 for a finished game can no longer mark stones dead on a recycled new game's slot.

### Regression caught and fixed (v269 patch)
The initial CMD22 guard (`!slot->is_scoring_mode`) blocked territory data for all observed game slots because observed slots never have `is_scoring_mode` set before CMD22 arrives — only the CMD22 inactive path sets it, which runs AFTER the match. The refined guard (`slot->is_playing && !slot->is_scoring_mode`) fixes this: observed slots (`is_playing=false`) always receive CMD22; only bot playing slots are gated on `is_scoring_mode`.

### Invariant established
Every docked-mode handler that calls `findSlot(game_id)` to modify slot state or route data to `shared_board_window` now verifies `!slot->game_finished` before proceeding. Every player-name loop that produces scoring output (`CMD22`, `CMD20`) also requires `slot->is_scoring_mode`. CMD22 additionally gates bot playing slots on `is_scoring_mode` while leaving observed slots unconditionally eligible.

---

## 2026-07-27 (v268): Bot engine desync recovery; Refresh Board button

### Bug — Territory overlay from a new game painted onto a just-finished game's display

**Symptom:** After a bot game ended, the board display (still showing the finished game) was overwritten with territory squares and dead-stone markers from a completely different game. The Comments & Kibitz panel showed a different score total. Live black groups appeared marked dead. The server score (CMD20) was correct; only the visual overlay was corrupted.

**Root cause:** CMD22 territory data is matched to a slot by player name in the docked-mode loop (line ~6422). The loop skipped `game_finished` slots (correct), but did **not** require the slot to be in `is_scoring_mode`. When the finished game's slot was remapped to a negative ID, and IGS issued CMD22 for an unrelated game that happened to include `got2go` as a player, the loop found the new active slot and routed `territory_board = shared_board_window`. `shared_board_window` was still displaying the finished game's board position → the new game's CMD22 territory grid was painted over the wrong board.

**Fix:** Added `if (!slot->is_scoring_mode) continue;` to the CMD22 docked-mode slot lookup (one line, after the `game_finished` guard). CMD22 territory data is now only routed to slots that have explicitly entered scoring mode via the `9 ... check your score` trigger. Slots in normal play, finished games, and unrelated observers are all skipped correctly.

---

## 2026-07-27 (v268): Bot engine desync recovery; Refresh Board button

### Bug 1 — Bot engine board desyncs from IGS when server rejects a move with "5 It is not your turn"

**Root cause (race condition):** When a warm KataGo engine cleared its board almost instantly after a new game started, the "engine ready late" path in `onBotEngineReady` immediately called `onBotOpponentMove` and requested `genmove`. KataGo returned a move (e.g. C4) in milliseconds. The bot sent it to IGS before the server had finished committing the opponent's first move server-side. IGS rejected the bot's move with `< 5 It is not your turn`. KataGo had already placed the stone on its internal board; there was no handler to undo it. The boards diverged silently.

**Cascade failure:** When the opponent later played the same intersection (C4) as a legal move on the IGS board, the bot sent `play black C4` to KataGo. KataGo already had White at C4, so it replied `? illegal move`. `KataGoEngine` cleared its command queue (cancelling the pending genmove) and emitted `illegalMove`. There was no `illegalMove` handler connected in bot mode, so the engine went permanently silent and the bot stopped responding.

**Fix — Part A (prevention): 500ms delay in "engine ready late" path.** When `onBotEngineReady` detects a missed opponent move and the warm engine path responds faster than IGS can settle, a 500ms `QTimer::singleShot` is inserted before calling `onBotOpponentMove`. This gives the IGS server time to complete its internal state transition after the opponent's move.

**Fix — Part B (IGS rejection recovery): CMD5 "not your turn" handler.** When IGS sends any `5 ...` error and the bot just sent a move (`bot_last_sent_vertex` is set), the handler sends GTP `undo` to KataGo (removing the rejected stone from KataGo's board), then re-requests `genmove` after an additional 500ms pause. `bot_last_sent_vertex` is set in `onBotMoveReady` and cleared on rejection or game end.

**Fix — Part C (KataGo desync recovery): `onBotIllegalMove` handler.** Connected to `KataGoEngine::illegalMove` in both warm and cold bot engine setups. When KataGo rejects a `play` command with `? illegal move`, the handler calls `botReplayMovesIntoEngine()`: clears KataGo's board, restores game parameters (komi, time settings, rules, handicap stones), replays all IGS-confirmed moves from `slot->move_history` in order, then requests `genmove` if it is the bot's turn. This fully resyncs KataGo's internal board to the IGS server's authoritative state.

### Feature — Refresh Board button on board window

Added a "Refresh Board" button to the board window info panel, positioned between "Edit Game" and "Close Board". Clicking it sends `moves <game_id>` to the IGS server, which responds with the full move history for the current game — useful when a board opens empty mid-game. Uses the black-on-gold xgospel1-style button (matching the Refresh Players and Refresh Games buttons). Signal: `BoardWindow::refreshRequested(int game_id)`.

**Bug fix during implementation:** `shared_board_window` has two creation paths in `xgospel2_fixed.cpp`: the CMD15 playing-game path (line ~5342) and the observe/cross-session-resume path (line ~8529). The `refreshRequested` signal connection (and `doneRequested`) were only wired in the first path. When the session started via the second path, the button click emitted the signal but no handler was connected, so nothing happened. Fixed by adding the missing connections to the second creation path. Three non-dock board creation paths were also audited; the CMD15 first-move path (line ~5876) and the manual-observe path (line ~8634) were missing the connection and were fixed as well.

---

## 2026-07-26 (v267): Fix observed game ID recycle corrupting bot game slot; fix blacklist/greylist case sensitivity

### Bug 1 — Observed game slot not evicted on ID recycle when game_finished is false

**Root cause:** The OBSERVE-MATCH eviction guard (`if (slot->game_finished)`) only fires for slots whose game ended via `untrackFinishedGame()`. That function is only called for the bot's own playing games. When a purely observed third-party game ends on IGS, the slot stays in `game_slots` with `game_finished=false`. When IGS silently recycles the ID for a new game, the OBSERVE-MATCH handler falls through to the player-name update path, overwriting `white_player`/`black_player` on the stale slot without clearing board state, move history, ranks, or handicap. After multiple recyclings, rank and handicap from an earlier game (e.g. a 10d player in a -5.5 komi game) corrupt the new bot game slot — producing wrong `WR[]` in the saved SGF and wrong handicap display.

**Evidence:** Session log showed game ID 188 recycled twice for observed games (komi=-5.5 → komi=6.5 → komi=0.5 bot game). The SGF for the bot game showed `WR[10d]` from the original observed game 188 player. The `untrackFinishedGame` log for game 188 only appeared once, confirming the intermediate recyclings were never evicted.

**Fix:** Added player-mismatch detection before the name-update path in the OBSERVE-MATCH handler. When the slot is a pure observation (`is_observing=true, !is_playing, !adjourned`), the slot has known players, and both white and black in the OBSERVE-MATCH line differ from the slot's current players, the slot is remapped to `-game_id` (same as the `game_finished` eviction path) and `game_finished` is set to true on the synthetic slot. The new game proceeds with a clean slate.

### Bug 2 — Blacklist and greylist player names forced to lowercase, breaking case-sensitive IGS accounts

**Root cause:** `settings.cpp` `getBotGreylist()` and `setBotGreylist()` applied `.toLower()` when loading and saving entries. Same for `getBotBlacklist()`/`setBotBlacklist()`. The match-time comparisons used `opponent.toLower()` / `QStringList::contains(opp.toLower())`. IGS account names are case-sensitive — a name entered as "K831114831" was stored as "k831114831" and never matched incoming challenges that used the original casing.

**Fix:** Removed all `.toLower()` calls from the settings load/save paths for both blacklist and greylist. Updated match-time comparisons to use `Qt::CaseInsensitive` (`QString::compare()` for greylist, `std::any_of` with case-insensitive compare for blacklist). Updated preferences dialog help text to say "case-sensitive" instead of "case-insensitive".

---

## 2026-07-14 (v266): Fix dead stone display for observed games; fix save confirmation lost on slot switch

### Bug 1 — Observed game dead stones not shown when user is at a non-final move position

**Root cause:** `receiveScoreEnd()` detected dead stones by calling `board_widget->getStoneAt()` — the widget's current visual position. When the user had navigated the slider to an earlier move (e.g. move 280 of 298), the widget showed that mid-game position, so stones that are dead at the final board position were absent or in different locations, yielding zero dead stones detected. Territory squares (digit 4/5 → green overlay) still appeared correctly because they are stored independently of the board widget.

**Fix:** Walk `game_root` forward via `nextMove()` to the final leaf node and read stone colors from `leaf->getBoard().getStone(r,c)` instead of the widget. Falls back to `board_widget->getStoneAt()` if the game tree is empty.

### Bug 2 — "✓ Game saved to: ..." message vanishes from Comments & Kibitz after switching slots

**Root cause:** `saveGame()` appended the save confirmation directly to the `comment_display` widget. `loadSlot()` rebuilds `comment_display` from `slot->comments` (the structured SAY/KIBITZ list), which never included the save message. Every slot switch discarded it.

**Fix:** Added `QStringList system_messages` to `GameSlot`. `saveGame()` emits `gameSaved(game_id, filename)`; the signal handler in `xgospel2_fixed.cpp` appends the confirmation to `slot->system_messages`. `loadSlot()` appends all `system_messages` entries after rebuilding from the structured comment list. `slot->reset()` clears `system_messages` with the rest of the slot state.

---

## 2026-07-13 (v265): Fix segfault when background finished-game slot ends while a different slot is active

### Bug 1 — `refreshPlayers()` sends `all <id>` for finished games (noisy + IGS errors)

**Root cause:** The observer auto-refresh in `refreshPlayers()` guarded with `active_slot_game_id > 0`, which is true for finished positive-ID slots (e.g. a taROU game that ended but kept its original ID). IGS replied `< 5 Invalid game number.` for every 120s timer tick while such a slot was the active view.

**Fix:** Added `!active_s->game_finished` to the guard. Observer refresh is now suppressed when the active slot is finished.

### Bug 2 — `untrackFinishedGame()` unconditionally calls `detachSlotClockTimer()` (segfault path)

**Root cause:** `detachSlotClockTimer()` reverts the board window's `clock_timer` pointer from a specific slot's timer back to `own_clock_timer`. When a background finished game (not the currently displayed slot) triggered `untrackFinishedGame`, the call would erroneously detach the *active* slot's clock timer instead. This left the board window's `clock_timer` pointing at `own_clock_timer` while the active slot's timer was disconnected. Subsequent `switchActiveGame` calls then connected and disconnected wrong timers, eventually leaving a dangling pointer that caused a segfault during the next paint event (`drawDeadStoneMarkers`).

**Reproducer:** Browse finished taROU game (e.g. #384) with slider, switch to another finished game (#-51), then game #384's `untrackFinishedGame` fires asynchronously while #-51 is active, corrupting the clock timer chain, and the next slot switch (#-302) crashes.

**Fix:** Added `game_id == active_slot_game_id` guard in `untrackFinishedGame` before the `detachSlotClockTimer()` call. Background finished games stop their own clock timer as before; only the active slot detaches from the board window.

---

## 2026-07-11 (v264): Fix new-game slot skipped on game ID reuse; freeze ranks at game start

### Bug 1 & 2 — New slot not created when IGS recycles a game ID to the same sequence opponent

**Root cause:** The stale CMD15 guard at the dock-mode playing-board creation path reads:

```cpp
if (white_byo_moves == -1 || black_byo_moves == -1) goto skip_playing_board_creation;
```

This guard was designed to skip phantom post-game CMD15 echoes (which arrive with `byo=-1` as a cleanup side-effect). However, the **first** CMD15 for any brand-new game also has `byo_moves=-1` because the clocks have not started yet. When IGS assigned game ID #21 to a new opponent (pickletenn) immediately after the previous game #21 (Taco3) ended, the guard fired on the first genuine CMD15, skipping new-slot creation entirely. Consequences:

- `switchActiveGame(21)` was never called → `active_slot_game_id` stayed on the finished game 55
- SAY delivery in dock mode routes to `findSlot(active_slot_game_id)` and checks `!game_finished` — the finished slot's guard blocked it → "0 playing board(s)"
- `botStartGame(21)` still fired (it is after the `skip` label) but `findSlot(21)` returned null → greeting comment lost, engine board pointed at wrong slot
- Comments & Kibitz widget continued showing the old game's history rather than opening a fresh board

**Fix:** Added `int newly_confirmed_game_id = -1` member variable. When `9 Creating match [N]` arrives, `newly_confirmed_game_id` is set to `N`. The stale guard now requires BOTH `byo_moves == -1` AND `game_id != newly_confirmed_game_id`. A genuine new game that IGS has just confirmed is exempt; stale post-game echoes (which arrive without a preceding "Creating match" line) still get skipped. `newly_confirmed_game_id` is reset to `-1` once the new slot is created.

**Why "Creating match [N]" is the right discriminator:** IGS always sends `9 Creating match [N] with opponent.` immediately before the CMD15 flood for a newly accepted game, including when it recycles an ID. Stale post-game CMD15 lines never have a preceding "Creating match" line.

### Bug 3 — SGF WR[]/BR[] used ranks at game end, not game start

**Root cause:** `slot->white_rank` and `slot->black_rank` are updated during the game whenever the bot fetches `stats <opponent>` (the response triggers `upsertPlayerRank` → `bslot->white_rank = stats_rank`). The SGF `WR[]/BR[]` properties were written from these live-updated fields, so they could reflect rank changes that occurred after the game started rather than the rank at the moment the game began.

**Fix:**
- Added `white_rank_at_start` / `black_rank_at_start` fields to `GameSlot` (game_slot.h) and `BoardWindow` (board_window.h)
- Set once at slot creation (`slot->white_rank_at_start = slot->white_rank`) alongside the existing `white_rank` / `black_rank` assignment — never overwritten
- Copied into `BoardWindow` via `loadSlot` and `startObserving`
- SGF `WR[]/BR[]` output now uses `*_at_start` (falls back to live rank if empty, e.g. for SGFs loaded from file)
- The `*SYSTEM*` "IGS Game #N started" comment now includes player names and ranks at start, so the record is visible in Comments & Kibitz even for games that are never saved

---

## 2026-07-10 (v263): Observer count in panel title; observer names as clickable buttons

*(v181 plan items)*

**Feature — observer count in panel title:** The Observers panel header label now shows the live count: "Observers (N)" where N updates with each observer list refresh.

**Feature — observer name buttonization:** Each observer name in the Observers panel is rendered as a clickable flat button. Clicking opens the player stats dialog for that observer (same as double-clicking a name in the Players list).

---

## 2026-07-09 (v262): Fix segfault when closing ~10 slots — stale moves_dispatch_queue IDs + recursive drain

**Bug:** Closing an observation slot removed it from `game_slots` but did not remove its ID from `moves_dispatch_queue`. When `dispatchNextMovesRequest()` later ran, it found the dead ID, called itself recursively to skip it, and left the next dead ID for another recursive call. After ~10 slot closures, enough dead IDs had accumulated that the recursive chain exhausted the call stack and segfaulted.

**Fix 1:** `closeBoardWindow` now calls `moves_dispatch_queue.removeAll(game_id)` immediately when a slot is closed, so stale IDs never accumulate in the queue.

**Fix 2:** `dispatchNextMovesRequest` replaced its tail-recursive skip (`dispatchNextMovesRequest(); return;`) with an iterative `while` loop that drains all stale entries in one pass before dispatching the first live slot. Eliminates the stack growth entirely regardless of queue length.

---

## 2026-07-07 (v261): Fix crash when closing an active observation slot — delete after switchActiveGame

**Bug:** In `closeBoardWindow`, after snapshotting and removing the closing slot from `game_slots`, the code called `delete slot` and then `switchActiveGame()`. `switchActiveGame` calls `loadSlot(new_slot)` on the `BoardWindow`, but `shared_board_window` still held an internal pointer into the deleted slot's `game_root` tree. Accessing it inside `loadSlot` (via `getTotalMoves()` etc.) caused a use-after-free crash.

**Fix:** Moved `delete slot` to after `switchActiveGame()` completes. `switchActiveGame → loadSlot` redirects the board window's `game_root` to the new slot first; only then is the old slot's memory freed. In the no-remaining-slots path, `delete slot` happens before `clearBoard()`/`hide()` since there is no `loadSlot` call to worry about.

---

## 2026-07-07 (v260): Tighten slot close lock — applies to all playing games, not just bot games

**Change:** The close button lock (`!slot->is_observing && !slot->game_finished`) now applies to any playing game slot, not only bot-mode games. Observation slots (`is_observing = true`) remain freely closeable at all times — the user may have clicked the wrong game. Playing slots have the × hidden and close attempts are rejected with a `*SYSTEM*` message until `untrackFinishedGame` re-enables the button.

---

## 2026-07-07 (v259): Per-slot × close button in dock; board window no longer destroyed on close

**Bug:** The only way to close a game slot was to close the entire `shared_board_window`, which destroyed all slots and set `shared_board_window = nullptr`. Subsequent `observeGame()` calls could not reopen the window. Closing during an active bot game terminated all game tracking.

**Fix — per-slot close button:** Added a small red × button to the top-right corner of each `GameButtonWidget`. Clicking × emits `closeRequested(game_id)` → `GameSelectionDock::gameCloseRequested(game_id)` → `closeBoardWindow(game_id)`, closing only that one slot.

**Fix — close lock for playing games:** Playing slots (`is_observing = false, game_finished = false`) have their × hidden (`setCloseable(false)`) when the slot is created. `untrackFinishedGame` re-enables it (`setCloseable(true)`) when the game ends. `closeBoardWindow` rejects close attempts on live playing slots with a `*SYSTEM*` message.

**Fix — board window survival:** `closeBoardWindow` no longer destroys `shared_board_window`. When the last slot is closed, the window is hidden (`hide()`) and can be re-shown by a subsequent `observeGame()` call. When a non-last slot is closed and it was the active slot, the code switches to the next available slot (preferring the active bot game).

**New files/methods:** `GameButtonWidget::setCloseable()`, `GameButtonWidget::closeRequested` signal, `GameSelectionDock::gameCloseRequested` signal, `GameSelectionDock::setGameCloseable()`, `GameSelectionDock::updateGameId()`.

---

## 2026-07-07 (v258): Retain finished slot history on ID recycle; print IGS game ID at game start

**Bug (v257):** When IGS recycled a finished slot's game ID, v257 deleted the slot — erasing the completed game's board history and move record from the dock entirely.

**Fix — history preservation:** Instead of deleting the finished slot, remap it to a synthetic negative ID (`-game_id`). `dock->updateGameId(game_id, -game_id)` keeps the dock button visible and the slot browsable. The new game for the recycled ID proceeds normally through the observe/play path without interference.

**Fix — game ID in Comments & Kibitz:** When a bot/playing game slot is created, `processComment("*SYSTEM*", "IGS Game #N started.")` is written immediately. This ensures the original IGS game number is visible in the kibitz log even after the server recycles the ID and the slot is remapped.

---

## 2026-07-07 (v257): Fix v256 regression — evict finished slot without sending spurious observe command

**Bug (v256 regression):** The v256 OBSERVE-MATCH fix called `observeGame(game_id, ...)` to evict a finished slot. However, the OBSERVE-MATCH handler fires for **every game in the IGS server game list** (periodic refresh lines like "9 Observing game N"), not only for games the bot explicitly observes. Calling `observeGame()` from this handler sends `socket->write("observe N\n")`, inadvertently subscribing to a random active game whose ID happened to be recycled. This caused unexpected move data floods and likely contributed to the segfault.

**Fix:** Replaced the `observeGame()` call with a direct inline eviction: remove the finished slot from `game_slots`, remove its dock button, clear the board widget if it was the active slot, detach the clock timer, and delete the slot. No `observe N` command is sent. The new game will be observed only if the user or bot explicitly requests it via the normal observe path.

**Why this is safe:** The OBSERVE-MATCH line that triggers this code is a server push — it tells us IGS has recycled the game ID. We only need to clean up our stale slot. We do not need to re-subscribe; that happens later when the bot plays or the user clicks the game.

---

## 2026-07-06 (v256): Fix board corruption when IGS recycles a finished slot's game ID for a new observed game

**Bug:** When IGS reused a game ID that belonged to a finished slot (e.g. game ID 16 finishes, then IGS assigns ID 16 to an entirely different observed game), the OBSERVE-MATCH handler in dock mode found the slot via `findSlot(game_id)` and entered the `else if` branch. Inside that branch, the only path that did any work required `!slot->game_finished && slot->adjourned_player.isEmpty()`. Since the slot was finished (`game_finished=true`), the condition was false and the entire block was skipped silently. The stale finished slot remained in `game_slots`, the board retained all stones from the old game, and the new game's moves were overlaid on top — producing phantom-stone corruption (e.g. game #168: gogame1 vs woodnstone stones mixed with chentom vs taROU stones).

**Fix:** Added a `slot->game_finished` guard at the top of the `else if (findSlot)` branch. When the slot is finished, `observeGame()` is called directly with the new game's players. `observeGame()` already handles the finished-slot case: it removes the stale slot from `game_slots`, removes its dock button, clears the board widget if it was the active slot, creates a fresh slot, and sends `observe N` + deferred `moves N`. A `*SYSTEM*` console message announces the eviction with both old and new player names. After `observeGame()` returns, the handler returns immediately (the slot is already fully set up).

**Verified:** Compiles cleanly (no new warnings). Covers all three sub-cases: same players recycled (e.g. same opponent re-challenges immediately), different players recycled (the common case), and recycled-to-observed game that was never a bot game.

---

## 2026-07-04 (v255): Fix board corruption when new game starts with same ID as just-finished game

**Bug:** When IGS reused a game ID immediately (e.g. tanukida resigns game 67 then instantly re-challenges as game 67), the board showed stale stones from the previous game. Two related failure modes:

1. **ID-recycle eviction without visual clear:** When the finished slot was evicted and a new empty slot was created, the board widget was cleared by `loadSlot()`. However, as a defensive measure, the explicit `clearBoard()` call was missing from the eviction path itself — so if `loadSlot()` was not reached (e.g. due to `switchActiveGame()` early-out returning before calling `loadSlot()`), the board retained old stones.

2. **Spurious `moves N` replay on live game:** If `moves N` was sent for a game already in progress (manually via console, or as a side effect of the ID-recycle sequence), the history reply arrived as move_number=0 on a LIVE slot. The LIVE path appended moves without clearing first, replaying all moves (including captures) on top of the existing position. Captured stones were re-placed, creating phantom stones at positions that should be empty.

**Fix 1 (eviction path):** Added `shared_board_window->clearBoard()` immediately when the finished-slot eviction also resets `active_slot_game_id`. This ensures the visual board is blank before the new slot's `loadSlot()` call. Added `BoardWindow::clearBoard()` inline method to the header as a thin wrapper over `board_widget->clearBoard()`.

**Fix 2 (spurious replay detection):** In the dock-mode LIVE active-slot move path, when `move_number == 0` arrives while the slot already has move history, the code now recognizes this as a spurious `moves N` replay. It resets all slot state (`board_state`, `move_history`, `server_move_count`, captures, `game_root`), clears the board widget, sets `replay_state = REPLAYING` with `catchup_high = server_move_count - 1`, and falls through to `replaying_case` so the history replays cleanly from a blank board. The `*SYSTEM*` log message `[BOARD] Spurious moves-N reply for live game N — resetting board for clean replay` identifies when this path fires.

**Verified:** Both fixes compile cleanly. Fix 1 is active whenever an ID-recycle eviction happens with the finished game as the active slot. Fix 2 covers the general case where `moves N` is accidentally sent for an already-live game.

---

## 2026-06-30 (v254): Fix area-map dead stone detection — opponent stone cells always -1, not +1

**Bug:** The v253 area-map algorithm was a complete no-op. After building the flood-fill areaMap, the group-scan loop tested `areaMap[gy][gx] != 1` on the opponent stone's own cell. But opponent stone cells are assigned `-1` by the flood-fill (only empty regions get `+1` for bot territory). The gate was therefore always true and every opponent group was skipped before any analysis ran. Result: area-map always reported 0 dead groups.

**Fix:** BFS the full connected opponent group first, then check whether any stone in the group has a 4-neighbour empty cell with `areaMap == +1`. This correctly identifies groups enclosed in bot territory — including groups fully surrounded by other opponent stones (no direct empty-cell neighbour) since the full group extent is collected before the territory check.

**Also fixed:** Game ID recycling guard (v253): when a game ends and IGS assigns the same ID to a new game before the games list refreshes, clicking the stale dock button now posts a `*SYSTEM*` warning in the Comments & Kibitz panel identifying the expected vs actual players.

**Also fixed (v253):** Segfault from stale `.o` files — `game_slot.h` dependency added to `board_window.o` and `xgospel2_fixed.o` rules in Makefile so header changes trigger recompilation of all dependents.

---

## 2026-06-26 (v253): Two-phase Sabaki-style area-map dead stone detection (experimental)

**Feature:** Adds a configurable alternative dead stone detection algorithm for bot end-game scoring, selectable via Preferences → Bot Settings → "Dead Stone Detection" checkbox. Default off — classic algorithm remains active for overnight rated play.

**Problem with classic algorithm:** KataGo ownership values are probabilistic near the dame boundary. Stones adjacent to dame are in contested zones; the engine assigns them low ownership, causing them to fall below the 0.70 threshold and be missed. The liberty enclosure BFS also misses them because their liberties expand into dame points which are not in `bot_territory`.

**New algorithm (two phases):**
- Phase 1 — Geometric flood-fill (deterministic, no KataGo thresholds): flood-fill every contiguous empty region on a working board copy. A region bordered exclusively by bot stones is bot territory. Any opponent stone inside a bot-territory region is a dead candidate. Candidates are removed and the pass repeats until stable — handles cascading chains where removing one group exposes another.
- Phase 2 — KataGo ownership confirmation (safety check): ownership > 0.80 (clearly bot-owned) OR |ownership| < 0.40 (unresolved aji — KataGo hedging on an obvious dead group) → confirmed dead. Groups where fewer than 50% of stones pass confirmation are left alive (conservative).

**Algorithm source:** Mirrors the `@sabaki/influence` `areaMap()` function (45 lines of pure geometry): flood-fill empty regions, single-color border → that color's territory, mixed border → dame. KataGo demoted to confirming role rather than primary detector.

**Configuration:** `bot_use_area_map_detection` setting (bool, default false). Saved in preferences file. UI checkbox with descriptive help text explaining both phases and the testing requirement.

**Console output:** Tagged `[BOT] [area-map]` — shows pass number, group size, KataGo confirmation count, and DEAD/alive verdict per group.

---

## 2026-06-24 (v252r): Fix CMD22 capture counts for inactive slots — observed game scoring fully aligned with q5go

**Root cause:** When CMD22 arrives for a game that is not the currently active slot (user is viewing a different game), the territory data is buffered in the slot rather than the shared board window. The v249 fix that reads authoritative capture counts from the CMD22 header only applied the `updateCaptures()` call when `territory_board` (active path) was set — the `territory_slot` (inactive path) branch was missing. Result: inactive slot games retained replay-counted captures which were inflated by scoring-phase stone removals being replayed as regular moves.

**Fix (v252):** Added `else if (territory_slot)` branch in the CMD22 second-header handler. Writes CMD22 header capture counts directly into `slot->white_captures` / `slot->black_captures`. The counting result handler already reads `slot->white_captures` for its prisoner calculation, so no further changes needed downstream.

**All three capture fixes together (v249–v252):**
- v249: CMD22 header regex expanded to extract prisoner count (field 3, e.g. `22 maiakari 1k  17 ...`). On second header line, `updateCaptures()` called with server values — replaces inflated replay-counted captures for active slot.
- v250: `updateLabels()` stops adding `dead_black_stones` / `dead_white_stones` on top of captures when `territory_ownership` is non-empty (CMD22 data present). CMD22 header prisoner counts already include dead stones — adding them again was double-counting.
- v251: `updateGameResult()` Comments & Kibitz "Game finished" line now computes totals using `white_captures` / `black_captures` directly (same source as stats panel) instead of stale `white_prisoners` / `black_prisoners`.
- v252: Inactive slot CMD22 path also writes server capture counts into slot (was active-slot only in v249).

**Verified:** Stats panel (Stones/Cap/Terr/Total) and Comments & Kibitz totals now match q5go exactly for both active and inactive slots across multiple scored games.

---

## 2026-06-20 (v248): Fix scoring markers not showing at final position after game ID recycling

**Root cause:** `is_final_position` in `displayNode()` tested `node->moveNumber() == getTotalMoves()`. After a game ends and IGS recycles the same game ID for a new game, `getTotalMoves()` returns the new game's move count, so the final node of the old game never matches and scoring markers are never rendered.

**Fix:** Use leaf-node test: `node->childCount() == 0`. A node with no children is definitionally the final position regardless of what `getTotalMoves()` returns. Retains the `moveNumber() == total_moves` fallback for non-leaf cases.

---

## 2026-06-21 (v247): Fix segfault in loadSlot() clock_timer swap when slot deleted during navigation

**Root cause:** User navigating move history slider on a finished game while a new game starts. `loadSlot()` called `disconnect(clock_timer, ...)` on a dangling pointer — the finished slot's clock_timer had already been deleted by `detachSlotClockTimer()` + slot eviction. The `clock_timer` member was left pointing to freed memory.

**Fix:** Guard clock_timer disconnect with null check and `!= own_clock_timer`; fall back to `own_clock_timer` if `slot->clock_timer` is null.

---

## 2026-06-21 (v246): Fix scoring markers persisting during move history navigation; disable geometric enclosure

**Scoring marker fix:** Dead stone, territory, and dame markers were visible at all positions when navigating the move history slider. The `!is_scoring_mode` guard in `displayNode()` prevented clearing markers during scoring phase, but this caused them to persist at historical positions after scoring completed. Fix: clear all scoring markers unconditionally at any non-final position — they are only meaningful at the final board position.

**Geometric enclosure disabled:** The v241 geometric enclosure BFS treated opponent stones as walls, flagging live groups with two eyes as dead. Mass false-positive dead stone removals corrupted scoring in nearly every game. Disabled pending a correct algorithm. Liberty enclosure test (v219) remains active.

**Note:** Game 110 SGF appeared corrupt but was correctly recorded — Yoda (10d) passed 40+ times from move 52, producing sequential W[] passes while got2go bot continued playing.

---

**Root cause:** The v241 geometric enclosure BFS treated opponent stones as walls, causing it to flag live groups with two eyes as dead. Any opponent group whose liberty space happened to be bounded by a mix of bot stones and other opponent stones (including its own eyes) was incorrectly declared enclosed. This produced mass false-positive dead stone removals, corrupting scoring in nearly every game.

**Fix:** Geometric enclosure test disabled. The liberty enclosure test (v219, BFS through bot_territory) is sufficient and correct. Geometric enclosure needs a fundamentally different algorithm (e.g. must verify the pocket has no two-eye potential) before it can be re-enabled safely.

**Note:** Game 110 SGF appeared corrupt but was correctly recorded — Yoda resigned by passing repeatedly after move 51, producing 40+ sequential W[] passes in the SGF.

---

## 2026-06-19 (v245): Fix post-reconnect game resume — use stored filename + restore board + restart engine

**Root cause:** Four separate failures in the cross-session reconnect resume path:
1. `load <opponent>` was sent speculatively — IGS needs the full game filename (e.g. `load woodnstone-weakkyu`) not just the opponent name
2. Fresh slot created for cross-session resume had no move history — `moves N` was never requested
3. KataGo engine was not restarted after reconnect, so bot could not generate moves
4. `bot_game_id` was never updated to the new game ID — opponent moves arrived but bot did not recognize them as its own game

**Fix:**
- Removed speculative `load <opponent>` timer; replaced with CMD18 parser that captures the game filename from the `stored` response and sends `load <filename>` immediately
- Cross-session CMD67 path now sets `cross_session_resume_game_id` and sends `moves <game_id>` after fresh slot creation (instead of suppressing it)
- When `moves N` replay completes for a reconnect game (`bot_restart_after_replay_game_id`), engine is cleared, all history moves replayed into KataGo, and `genmove` requested if it is the bot's turn
- Removed `load_retry_timer` and `load_retry_count` entirely — no longer needed

---

## 2026-06-19 (v244): Fix auto-reconnect socket state — abort stale socket + log errors

**Root cause:** After `ss --kill` drops the TCP connection, QTcpSocket may remain in a non-`UnconnectedState`. Previous code waited 5s and retried without forcing the socket closed, so `connectToHost` never fired.

**Fix:**
- `attemptReconnect()` now calls `socket->abort()` if socket is not already in `UnconnectedState`
- `errorOccurred` handler now logs the socket error string to the console
- Removed redundant `!connected_to_igs` guard from `errorOccurred` handler

---

## 2026-06-19 (v243): Fix post-reconnect "load" timing — retry until IGS 5-minute window closes

**Root cause:** v242 sent `load <opponent>` immediately in the 1s setup timer after reconnect. But if the TCP drop and reconnect happen within IGS's 5-minute "disconnected game" window, IGS still considers the game live and rejects `load` with `"You can not load this game."` The game was never resumed.

**Fix:** Replaced the immediate `load` with a staged retry system:
- First `load` attempt deferred 10s after login (gives IGS time to settle post-reconnect)
- `"You can not load this game"` and `"You are in disconnected game"` responses schedule a 30s retry via `load_retry_timer`
- Up to 9 retries × 30s = 4.5 minutes — covers the full IGS 5-minute disconnected window
- When CMD67 arrives (game successfully resumed by either path), `load_retry_timer` is stopped and `reconnect_opponent` cleared in both resume paths (OBSERVE-MATCH and CMD67)
- If all retries exhausted, logs a warning that the game may be forfeited

---

## 2026-06-19 (v242): Auto-reconnect to IGS on unexpected disconnect + post-reconnect game resume

**Feature:** When the TCP connection to IGS drops unexpectedly (network hiccup, server blip, interface drop), the client now automatically attempts to reconnect and restore the session.

**Reconnect logic:**
- `onDisconnected()` detects unexpected drops (vs. user-initiated disconnect via Connection > Disconnect or File > Exit)
- Schedules reconnect with exponential back-off: 5s → 10s → 20s → 30s cap, max 10 attempts (~3.5 min total)
- Each attempt calls `connectToHost()` — `onConnected()` fires on success and runs the full login + setup sequence automatically
- Failed connection attempts (socket error) schedule the next retry via `errorOccurred` signal
- User-initiated disconnect sets `manual_disconnect = true` to suppress auto-reconnect

**Bot game resume:**
- At disconnect time, if a bot game is active (`bot_game_id != -1`), the opponent name is saved to `reconnect_opponent`
- After successful reconnect and login setup, `load <opponent>` is sent automatically
- IGS adjournment machinery (CMD67 + existing v237/v239 resume logic) handles the rest — board state, move history, and engine restart are all recovered

---

## 2026-06-19 (v241): Add geometric enclosure test for dead stone detection (KataGo-independent guard)

**Root cause:** In game 131 vs so2828427, KataGo rated the area around K17 (a lone black stone deep in white territory) at 0% white ownership — likely due to low visit count uncertainty. Both the ownership test (0% < 65% threshold) and the existing KataGo-based enclosure test (which uses `bot_territory` built from 70% ownership threshold) failed to mark it dead. The server's flood-fill scoring was then disrupted by unremoved dead stones on both sides, producing a badly wrong result (B+61.5 instead of the correct ~B+15.5).

**Fix:** Added a third detection pass — a **geometric enclosure test** fully independent of KataGo. For each opponent group not already marked dead, a BFS flood-fills from the group's liberties through empty cells, blocked only by bot stones (actual board state, not ownership estimates). If the entire reachable empty region fits within 40 cells (a genuine dead pocket is always small), the group is geometrically walled in and marked dead regardless of KataGo's ownership values. The 40-cell cap prevents falsely marking large live groups.

The three-tier detection now is: ownership (KataGo ≥65%) → KataGo-based enclosure → geometric enclosure (board-state only).

---

## 2026-06-18 (v240): Increase AYT heartbeat frequency during active bot game

**Problem:** The AYT keep-alive was sent every 15 minutes regardless of game state. During an active bot game a silent TCP drop could go undetected for up to 15 minutes, causing the game to be lost on time with no warning.

**Fix:** During an active bot game (`bot_mode_active && bot_game_id != -1`), the heartbeat interval is reduced to 60 seconds. Outside of an active bot game the 15-minute interval is unchanged. Both the send path and `resetHeartbeatCounter()` (called on AYT response and user command) use the same context-aware interval.

---

## 2026-06-17 (v239): Use stored-game list to prime cross-session resume without requiring restore signal

**Feature:** The `stored_adjourned_opponents` set (populated at login from the `stored` command response) is now used as a fallback in the CMD67 handler. If `pending_resume_player` is empty when CMD67 arrives but the opponent is a known stored-game opponent, the client immediately treats the CMD67 as a cross-session resume and creates a fresh board — without needing the `"9 X has restored your old game"` signal to arrive first. The opponent is removed from the set once the resume is handled to prevent stale matches.

This makes cross-session resume reliable even if the `"restored"` message is missed, delayed, or arrives after CMD67.

---

## 2026-06-17 (v238): Send "stored" at login and display adjourned game notice in console

**Feature:** At login (in the 1s setup timer), the `stored` command is sent to IGS to query any games saved on the server from previous sessions. The response is parsed and displayed in the console as a prominent notice:
- If stored games exist: `>>> [STORED] N adjourned game(s) waiting on server — use 'load <player>' to resume:` followed by each game entry
- If none: `>>> [STORED] No adjourned games on server.`

This ensures the user is immediately aware of cross-session adjourned games at startup rather than discovering them only if the opponent reconnects.

---

## 2026-06-17 (v237): Fix cross-session adjournment — no board appearing for game adjourned in prior session

**Root cause:** When a game is adjourned and the client is restarted, there is no `GameSlot` in memory for that game. On reconnect, IGS sends `"9 X has restored your old game."` (sets `pending_resume_player`) followed by `CMD67` with the new game ID. The CMD67 resume path searched `game_slots` for a slot with `adjourned_player == X` — found nothing — and silently did nothing. The `pending_resume_player` flag then stayed set, blocking new slot creation for all subsequent CMD67s until cleared. Additionally, if the opponent ultimately failed to reconnect, `"Disconnected game with X was removed"` never cleared `pending_resume_player`, leaving it poisoned for the rest of the session.

**Fix 1:** In the CMD67 resume path, when no adjourned slot is found (cross-session case), log a diagnostic message and clear `pending_resume_player` immediately, allowing the standard slot-creation path to run and build a fresh board. Move history is fetched from the server via `moves N` as normal.

**Fix 2:** In the "Disconnected game with X was removed" handler, clear `pending_resume_player` if it names the dropped player, so a failed cross-session resume never poisons subsequent game setups.

---

## 2026-06-17 (v236): Fix resumed playing game not highlighted in games list + fix ranks in CMD67 resume path

**Root cause:** There are two adjournment/resume code paths. The v235 fix covered the "Observing game" path (non-dock or observer-initiated resume). The CMD67 path — which handles the bot's own playing game resumption — was missing both fixes: it still passed `"?"` for ranks to `addGame`, and never updated `observed_game_ids`, so the resumed game row was never highlighted in the Games list.

**Fix:** In the CMD67 resume path, replaced hardcoded `"?"` with `adjourned->black_rank`/`adjourned->white_rank`, and added the `observed_game_ids` remove/insert + `updateObservedGames()` call, matching the v235 fix in the other path.

---

## 2026-06-17 (v235): Fix stale games list highlight after adjournment/resume

**Root cause:** On adjournment/resume the game ID changes (e.g. game 1 → game 114). The ADJOURN RESUME handler correctly updated `active_slot_game_id`, `bot_game_id`, and the dock button, but never updated `observed_game_ids` — the set that drives bold/highlighted rendering in the Games list window. The old ID remained in the set and the new ID was never inserted, leaving the old game row permanently highlighted and the new one unhighlighted even after Refresh Games.

**Fix:** In the ADJOURN RESUME handler, swap the old ID for the new ID in `observed_game_ids` and call `games_window->updateObservedGames()` to repaint immediately.

---

## 2026-06-17 (v234): Fix rank not showing on resumed game dock button

**Root cause:** After adjournment/resume, IGS assigns a new game ID (e.g. game 6 → game 90). The ADJOURN RESUME handler removed the old dock button and created a new one, but always passed `"?"` as the rank strings for both players. The slot's `black_rank` and `white_rank` fields were already populated from the stats response that arrived earlier during the original game — those stored ranks were simply not being passed to `addGame`.

**Fix:** In the ADJOURN RESUME handler, replaced the hardcoded `"?"` strings with `adjourned_slot->black_rank` and `adjourned_slot->white_rank`. The resumed game dock button now shows the correct rank (e.g. "7k+") immediately rather than "?" indefinitely.

---

## 2026-06-17 (v233): Fix board corruption during scoring from mid-game observer refresh

**Root cause:** The auto-refresh players timer can fire while the bot is in the scoring phase (after `done` is sent, before CMD20 is received). The players list refresh causes IGS to send `"9 Observing game N (got2go vs. kagg) :"` as part of the observer count update. This line was processed by the OBSERVE-MATCH handler which updated player names and potentially triggered board state changes, corrupting the territory overlay display. The board was left showing "Black to Play" with incorrect territory counts rather than the scoring result.

**Fix:** Added a guard to skip `"9 Observing game"` processing for the bot's own active game while `bot_done_sent=true`. The line is matched by game ID and suppressed during the scoring window so the OBSERVE-MATCH handler cannot interfere with the board display.

**Note:** This fix is believed to resolve the corruption but requires live testing to confirm — CMD22 territory data arriving mid-scoring is a separate interaction that may warrant further investigation if corruption persists.

---

## 2026-06-17 (v232): Fix false positive ?+T time forfeit result after scored games

**Root cause:** IGS sends `"9 Removed game file <login>-<opponent> from database."` after every game end — including normal scored games (after CMD20 + `"9 game completed."`). The time forfeit handler fired on this line whenever `bot_game_id != -1`, producing a spurious `[BOT] Game N ended via time forfeit — ?+T` log entry even for games that ended cleanly by scoring. In game 258, the correct result was B+4.5 but the log showed `?+T`.

**Fix:** Added `!bot_cmd20_received` guard to the time forfeit path. If CMD20 was already received for this game, the "Removed game file" line is ignored by the forfeit handler — the game ended by scoring and cleanup was already handled by `botEndGame()`.

---

## 2026-06-16 (v231): Greylist negative HC Limit for stronger opponents

**Feature:** The greylist "Max HC" column is renamed "HC Limit" and now accepts negative values, enabling the bot to decline matches from consistently stronger opponents (sandbagged ratings, AI-assisted players, etc.) who offer even or unfavorable conditions.

- **Positive limit (e.g. 8)**: decline if offered handicap > limit — same behaviour as before (weaker opponent asking too many stones)
- **Zero**: decline any handicap game; only accept even games from this opponent
- **Negative limit (e.g. -1)**: decline even games and any game where the bot would receive handicap stones — used for opponents who are stronger than their listed rank suggests

Example: yaonie9d listed at 6d+ but consistently defeats the bot — set HC Limit = -1 to decline all match requests. When they offer an even game (handicap=0), `0 > -1` triggers the decline with the custom tell.

The `> 0` validation guards that previously silently dropped zero and negative entries have been removed from `settings.cpp` (load and save paths) and `preferences_dialog.cpp` (`greylistFromTable`). The comparison logic in `botAcceptMatch()` (`effective_handicap > entry.max_hc`) already handled negatives correctly — only the storage layer needed fixing.

---

## 2026-06-16 (v230): Fix corrupt scoring + sequential removal + File > Exit menu item

### File > Exit
Added "Exit" item to the File menu (below a separator after "Open SGF..."). Calls `QApplication::quit()` which sends close events to all windows and exits the event loop cleanly — board windows, players list, games list, shout window, and all stats dialogs are all properly destroyed. Workaround for cases where closing the main console window left orphan child windows running.

### Fix corrupt scoring when opponent removes same dead groups simultaneously

**Root cause:** Dead stone removal commands were sent in a tight fire-and-forget loop followed immediately by `done`, with no confirmation waiting and no error recovery. When both the bot and opponent independently identified the same dead groups and sent removal commands concurrently, IGS rejected the bot's commands with "5 You cannot remove a liberty." — the group stones were already gone, leaving the coordinate as an empty point. With 3 of 9 removal commands rejected in game 91, IGS was left in an inconsistent scoring state: 52 white stones remained on the board but the CMD22 territory grid showed W:0 territory and B:117 territory, producing a corrupt result of B+134.5 instead of the correct ~B+7.5.

**Fix:** Removals are now sent one at a time with server confirmation required before the next is sent:
1. Send seed coordinate for first dead group
2. Wait for CMD49 confirmation → advance index, send next group's seed
3. On "5 You cannot remove a liberty" rejection → retry with the next stone in the group as alternate seed (full group membership now stored alongside the seed)
4. If all stones in a group exhausted → opponent already removed it; skip and continue
5. After all groups confirmed or skipped → send `done`

Full group membership (`bot_pending_remove_groups`) is stored at identification time so retry candidates are always available. State variables `bot_remove_index` and `bot_remove_retry_offset` track progress through the sequential send.

**Policy confirmed:** Bot marks only dead opponent stones within its own rightful territory. It never touches stones in the opponent's territory — that is the opponent's responsibility. The retry/skip logic is purely to handle concurrent removals gracefully, not to expand the bot's marking scope.

---

## 2026-06-16 (v229): Fix intermittent duplicate player entries in players list

**Root cause:** The `userlist_player_names` dedup set is populated by parsing each `< 42` line with the q5Go regex (`parseIGSPlayer`). When the regex fails for an unusual line (wide characters, non-standard field widths, special info strings), the name is never inserted into the set. The WHO supplement phase then adds a second row for that player — identifiable by empty Won/Lost/Rated/Country columns (WHO format provides no extended stats).

**Fix:** Added a model-level dedup guard in `addPlayerToTable()`: before appending a new row, scan column 1 (Name) for a case-insensitive match. If found, silently drop the duplicate. This is O(n) per player but runs only during list population. It catches all cases regardless of which parser path was taken, including any future regex failures.

---

## 2026-06-15 (v228): Greylist max_hc enforcement for old match protocol clients

**Problem:** The old `match` protocol (used by xgospel1 and clients like BusyBee) always sends `handicap=0` in the match request — the actual handicap is assigned by IGS server-side at CMD67. The greylist check compared `handicap > entry.max_hc`, so it always evaluated `0 > max_hc` and never fired for old-protocol opponents.

**Fix:** Before the greylist loop, compute `effective_handicap`: if `handicap==0` and the suggested command starts with `"match "` (old protocol), estimate the handicap from rank difference using `expectedHandicap()`. The greylist check then uses `effective_handicap` instead of the raw (always-zero) handicap. The decline log message notes when the value is estimated from ranks vs. explicitly stated by the opponent.

**Limitation (documented):** For old-protocol clients, the estimate is based on the players-list rank snapshot and may differ from what IGS actually assigns at boundary rank values. This is an inherent IGS protocol constraint — old `match` clients cannot communicate handicap in the request. To be noted in the user manual.

---

## 2026-06-15 (v227): Fix bot incorrectly declining old match protocol requests

**Root cause:** The fairness check in `botAcceptMatch()` compared `|handicap - exp_hc| > 1` and declined if the difference exceeded 1 stone. For old `match` protocol requests, `handicap` is always 0 (IGS assigns it server-side), so any opponent with a rank difference of 2+ stones would be declined with "unfair handicap: offered 0, expected N".

**Fix:** Detect old match protocol via `handicap == 0 && suggested_cmd.startsWith("match ")`. When detected, skip the handicap comparison entirely (log a note instead) and only enforce the color check where the rank gap makes color unambiguous. The nmatch path is unaffected — handicap is always explicit there.

---

## 2026-06-15 (v226): Fix players list and game observation broken by early toggle open

**Root cause (v225 regression):** The v225 fix called `toggleBotMode(true)` synchronously at the `"1 1"` login line. `toggleBotMode` sends `toggle open true` to the socket immediately, injecting a server response (`"1 Setting open to be True"`) into the middle of the `userlist`/`who` data stream. The end-of-list detection for both the players list and games list triggers on lines matching `"1 1"` with length < 10 — the server's response to `toggle open` was close enough in format to trip this guard, terminating the lists prematurely and leaving the board observation path broken.

**Fix:** At login, set `bot_mode_active = true` and update the menu checkmark directly without touching the socket. Defer `toggle open true` into the existing 1-second protocol-setup timer (alongside `id`, `toggle newrating`, `nmatchrange`, `toggle nmatch`) where it can't interfere with the data stream. Bot mode is active from the moment login completes, catching early match requests; socket state stays clean until the timer fires.

---

## 2026-06-15 (v225): Fix bot not accepting matches after client restart (reverted — caused regression)

Attempted to restore bot mode before the 1-second protocol timer by calling `toggleBotMode(true)` synchronously at login. This caused the players list and game observation to break (see v226). The correct fix is in v226.

---

## 2026-06-15 (v224): Fix greylist not persisting after client restart

**Root cause:** The greylist was stored in `m_params["bot_greylist"]` as tab-delimited records joined by `\n` (newline). The settings file format writes one `KEY [value]` per line — embedded newlines in a value span multiple lines without key prefixes, so on reload only the text up to the first embedded newline was associated with the key. Subsequent records were read as orphan lines and discarded, leaving the greylist empty after every restart.

**Fix:** Changed the storage format to use `||` as the record separator and `|` as the field separator within each record. The entire greylist now fits on a single line in the settings file and survives the read/write round-trip correctly. Any pipe characters in a custom tell are silently replaced with spaces to avoid breaking the separator scheme.

---

## 2026-06-13 (v223): Persist selected engine profile across sessions

Selected engine profile ID is now saved to settings whenever the user picks an engine via Engines → Attach Engine. On startup, `m_pending_engine_profile_id` is restored from settings before the menu is built, so the checkmark reflects the previous selection immediately and bot mode launches with the correct engine automatically.

---

## 2026-06-13 (v222): Checkmark on currently selected engine in Attach Engine submenu

Each engine listed under Engines → Attach Engine is now a checkable action. The currently selected engine (matching `m_pending_engine_profile_id`) shows a checkmark when the submenu opens. The submenu is rebuilt on every show so the checkmark always reflects the current selection.

---

## 2026-06-13 (v221): Persist bot mode across sessions

**Root cause:** Bot mode was initialised to off every startup (hardcoded `setChecked(false)`) and never saved to settings. After a restart, match requests arrived but `bot_mode_active` was false so the nmatch handler silently ignored them — no accept, no decline.

**Fix:** `toggleBotMode()` now saves `bot_mode_enabled` to settings immediately on each toggle. After login completes (post-nmatch setup), the setting is read back and `toggleBotMode(true)` is called automatically if it was on in the previous session. The menu checkmark is also initialised from settings at startup so the UI reflects the restored state.

---

## 2026-06-12 (v220): Bot opponent greylist — per-opponent handicap limits with custom tells

**Feature:** New "Opponent Greylist" table in Preferences → Bot Settings tab. Each row specifies a player name, a maximum acceptable handicap (inclusive), and an optional custom tell message. When a match or nmatch is offered with a handicap exceeding the limit, the bot declines and sends the custom tell to the opponent explaining the reason. The greylist is checked after the blacklist and before the fairness check in botAcceptMatch(). Stored as tab-delimited records in the settings file (newline-separated), preserving commas freely in the tell text.

---

## 2026-06-08 (v219): Add liberty-enclosure test for dead stone detection

**Root cause:** The bot's dead stone detection used only KataGo ownership percentage: a group was marked dead only if ≥65% of its stones fell on intersections with ≥0.70 KataGo ownership. At low visit counts (~50-200) KataGo's ownership signal for enclosed groups can be below the threshold (e.g. 60%) even when the group is geometrically enclosed with no escape. In game 170, a white group near H5 was genuinely enclosed in black territory but only scored 60% — below the 65% cutoff — so the bot failed to remove it, causing a scoring discrepancy of the entire group's territory.

**Fix:** Add a liberty flood-fill enclosure test as a second dead-stone criterion alongside the ownership percentage check. For each opponent group, BFS-expand all empty intersections reachable from the group's liberties without crossing bot stones. If every reachable empty cell is inside bot territory (KataGo ownership ≥0.70), the group has no escape path and is declared dead regardless of ownership percentage. Either test alone is sufficient to flag a group as dead. The log now reports the reason: "ownership", "enclosed", or "ownership+enclosed".

---

## 2026-06-07 (v218): Fix bot scoring silent no-op on repeated opponent disputes

**Root cause:** When an opponent disputes dead stone removal ("Board is restored"), IGS re-sends the "check your score with the score command" line, re-triggering the bot's scoring entry path. On the first entry `bot_done_sent=false`, so the 1500ms timer lambda (guarded by `!bot_done_sent`) fires correctly. However `bot_done_sent` and `bot_pass2_rendered` were only reset in `botEndGame()` — never on a mid-game "Board is restored" re-entry. On the second and subsequent rounds, the timer lambda returned immediately without sending stone removal commands or `done` to IGS, leaving the server waiting indefinitely and producing incorrect scoring.

**Fix:** At the top of the "check your score" handler, if `bot_done_sent` is already true (indicating a re-entry after "Board is restored"), reset `bot_done_sent=false` and `bot_pass2_rendered=false` before setting `bot_scoring_pending=true`. This allows the 1500ms timer lambda to proceed normally on every re-negotiation round.

---

## 2026-06-06 (v217): Fix dead stone markers cleared by CMD15 during scoring phase

**Root cause:** `displayNode()` unconditionally clears dead stone markers (`setDeadStones({})`) whenever the current node is not the final position. During the IGS scoring phase, any CMD15 line (e.g. the "Board is restored" re-scoring sequence) advances the game tree to a new node and triggers `displayNode` — which treats it as a non-final position and wipes the 6 correctly-set dead stone markers. The scoring result then shows territory without dead stone removal.

**Fix:** Guard the dead stone clear in `displayNode` with `!is_scoring_mode` — if scoring is already active, do not clear the overlay when a non-final node is displayed. Dead stone state is preserved through the re-scoring sequence.

---

## 2026-06-06 (v216): Fix live moves skipped after inactive-slot replay completes

**Root cause:** When a slot replays history via the inactive path (serialized `moves N` queue), the board window's `mv_counter` is set to `-1` by `clearMoveHistoryBeforeMovesCommand()` before the request is sent. After replay finishes and `loadSlot()` reloads the board from slot state, `mv_counter` was never reset — it stayed at `-1`. The `[q5Go]` live-move guard in `processMove()` skips any move with `move_number > 0` while `mv_counter == -1`, so all live moves arriving after replay were silently dropped. Result: the board and `board_state` were missing every move played after the initial history replay caught up.

**Fix:** At the end of `loadSlot()`, if `mv_counter == -1` and `slot->replay_state == LIVE`, set `mv_counter = slot->server_move_count` to release the guard. This correctly initialises the counter to the replay endpoint so subsequent live moves are accepted without skipping.

---

## 2026-06-06 (v215): Fix CMD22 dead stone detection wrong for active slot

**Root cause:** For the active (currently viewed) slot in `LIVE` state, CMD15 moves are routed to the board widget via `target_board = shared_board_window` — the board widget renders them correctly on screen. However `applyMoveToSlotBoard()` was never called for active-slot live moves, so `slot->board_state` (the flat array) was only current up to the end of the initial history replay (e.g. move 7 out of 284). When CMD22 territory data arrived at game end, the inactive-slot dead-stone detection checked `slot->board_state[pos]` against territory digit 4/5 — but `board_state` was 277 moves stale, identifying only a few stones as dead instead of the correct 19.

**Fix:** In the active-slot LIVE path, call `applyMoveToSlotBoard(slot, move)` in addition to routing to the board widget, keeping `slot->board_state` current throughout the game so CMD22 dead-stone detection is accurate at game end.

---

## 2026-06-05 (v213): Fix corrupt CMD22 scoring — active slot history replay bypassed board_state

**Root cause:** When the first observed game is the active (currently viewed) slot, CMD15 history moves routed through the fast active-board path (`current_game_context == active_slot_game_id`) which updated `move_history` and the board widget but **never wrote to `board_state`** and never advanced `replay_state` out of `WAITING_FOR_MOVES0`. The slot remained stuck in `WAITING_FOR_MOVES0` indefinitely with an empty `board_state`, so CMD22 territory scoring read all-empty intersections and marked the board incorrectly.

**Fix:** The active-slot fast path now checks `slot->replay_state == GameSlot::LIVE` before routing directly to the board window. During `WAITING_FOR_MOVES0` or `REPLAYING`, even the active slot is routed through the per-slot state machine (via `goto active_slot_replaying`) so `board_state` is built correctly. When replay completes and the slot transitions to `LIVE`, `loadSlot()` redraws the board window with the complete position.

---

## 2026-06-05 (v212): Fix corrupt CMD22 scoring when multiple games observed in rapid succession

**Root cause:** When the user observes several games in quick succession, all outstanding `moves N` history requests were dispatched simultaneously (all at once in the "Adding game to observation list" handler). IGS replies to each `moves N` with a stream of CMD15 moves tagged by game ID; when multiple streams are in-flight concurrently, the `current_game_context` tracking can route history moves to the wrong slot, causing some slots to remain forever in `WAITING_FOR_MOVES0` with their `board_state` never updated. At game end, CMD22 territory scoring reads the stale (empty/partial) `board_state` and marks the wrong intersections, producing a visually corrupt territory display.

**Fix 1 — serialized `moves N` dispatch:** A new `moves_dispatch_queue` (ordered list) replaces the old set-based simultaneous flush. When "Adding game to observation list" arrives, all pending game IDs are enqueued but only the first is dispatched immediately. Each time a slot transitions to `LIVE` (replay complete), `dispatchNextMovesRequest()` is called to send the next `moves N`. Only `REPLAYING` state counts as "in-flight"; `WAITING_FOR_MOVES0` (request not yet sent) does not block dispatch. This ensures at most one history stream is in-flight at any time, eliminating the interleaving problem.

**Fix 2 — active slot replay routed through slot state machine:** When the active (currently viewed) slot is in `WAITING_FOR_MOVES0` or `REPLAYING` state, CMD15 moves are now routed through the per-slot state machine (same path as inactive slots) instead of the fast active-board path. Previously the fast path bypassed `board_state` updates entirely for the active slot, leaving `board_state` empty for CMD22 scoring. The board window is refreshed via `loadSlot()` once replay completes and the slot transitions to `LIVE`.

---

## 2026-06-05 (v211): Keep completed games on dock for review/save; fix clock_timer ownership

**Root cause (regression introduced in v207):** `untrackFinishedGame()` was evicting the `GameSlot` and removing the dock button immediately when a game ended. This prevented the user from reviewing or saving the completed game. The original intent of the v207 immediate eviction was only to prevent recycled-ID crashes — but that is already handled by the v206/v208/v209 guards at the CMD15/CMD67 recycle paths, which check `game_finished=true` and evict lazily when the ID is actually reused.

**Fix:** `untrackFinishedGame()` now only: stops the slot's clock timer, calls `detachSlotClockTimer()` to prevent a dangling pointer in `BoardWindow`, removes from `observed_game_ids`, and updates the Games window highlighting. The `GameSlot` and dock button remain intact. Lazy eviction at ID-recycle time (v206/v208/v209 paths) is sufficient to prevent the recycled-ID crash.

---

## 2026-06-05 (v210): Fix dangling BoardWindow::clock_timer after slot eviction

**Root cause:** `BoardWindow::clock_timer` is swapped to point at the active `GameSlot`'s `clock_timer` during `loadSlot()`. When `untrackFinishedGame()` (or any eviction path) calls `delete slot`, the slot's `clock_timer` is destroyed — but `BoardWindow::clock_timer` still holds the now-dangling pointer. The next time a new game starts and `loadSlot()` calls `disconnect(clock_timer, ...)`, it dereferences freed memory and crashes. This explains the step-E crash in the debug instrumentation.

**Fix:** Added `own_clock_timer` member to `BoardWindow` (the timer created in the constructor, never deleted). Added `detachSlotClockTimer()` method that safely reverts `clock_timer` back to `own_clock_timer` when a slot is being evicted. Called before every `delete slot` in: `untrackFinishedGame()`, the v206 CMD67 recycle path, and the v208/v209 CMD15 playing-path evictions.

---

## 2026-06-05 (v209): Fix crash on duplicate CMD15 for new playing game

**Root cause:** IGS sends CMD15 twice when a new nmatch/match is created — once immediately on acceptance, and again after the "Creating match [N]" server confirmation. The v208 eviction guard unconditionally deleted any pre-existing slot for the game ID, including the live playing slot that was correctly set up by the first CMD15. The second CMD15 evicted that slot (freeing its `GameSlot` and its `clock_timer`), then called `loadSlot` on a new fresh slot — but `BoardWindow::clock_timer` was still pointing to the just-freed timer. The `disconnect(clock_timer, ...)` call at line E of `loadSlot` then dereferenced a dangling pointer and crashed.

**Fix:** In the eviction guard, if the existing slot is already a live playing slot (`is_playing=true`, `!game_finished`), treat the CMD15 as a duplicate confirmation and skip entirely via `goto skip_playing_board_creation`. Only evict observation slots and finished slots (genuine ID recycles).

---

## 2026-06-05 (v208): Fix crash when IGS recycles an observed game ID for a new playing game

**Root cause:** When IGS assigned a new match to a game ID that the client was already observing (e.g. got2go accepted an nmatch from zmb413 and IGS reused game 76 which we were watching), the `we_are_playing` path in CMD15 found an existing **observation** slot for that ID with `game_finished=false`. The eviction guard at the time only removed slots where `game_finished==true`, so the live observation slot survived. `findSlot(game_id)` then returned non-null, suppressing new slot creation, and the code fell through to `loadSlot` on the stale observation slot — which called `setDeadStones` on an inconsistent board and crashed.

**Fix:** In the `we_are_playing` / `pending_resume_player.isEmpty()` path, evict **any** pre-existing slot for the game ID unconditionally (whether it is a finished slot, an active observation slot, or any other state). When `we_are_playing=true`, IGS has told us this is our game — no prior slot for that ID is valid. Also clears `observed_game_ids` for the evicted ID so the observation tracking stays consistent.

---

## 2026-06-05 (v207): Fix segfault on game-ID recycle — evict finished slot in untrackFinishedGame

**Root cause:** The v205/v206 guards tried to detect the stale-vs-recycle distinction at CMD15/CMD67 arrival time, but this was fundamentally fragile — the `match` protocol (as opposed to `nmatch`) can bring a new CMD15 with valid byoyomi values before CMD67 arrives, bypassing both guards. The underlying problem was that finished slots were never removed from `game_slots` — they sat there with `game_finished=true` until something evicted them, creating a window where any incoming CMD15 for a recycled ID could call `loadSlot`/`setDeadStones` on a dead slot and crash.

**Fix:** Evict the finished `GameSlot` immediately inside `untrackFinishedGame()` — the earliest reliable point where we know the game is truly over. This removes the slot from `game_slots`, removes its dock button, and resets `active_slot_game_id`. Any subsequent CMD15/CMD67 for a recycled ID finds no slot and creates a fresh one cleanly. The v205/v206 guards remain as a secondary safety net.

---

## 2026-06-02 (v206): Fix new game slot not created when IGS recycles a finished game ID

**Root cause:** The v205 `goto skip_playing_board_creation` guard correctly blocked stale post-game CMD15 lines from recreating a just-finished slot, but it was too broad — it also blocked the legitimate case where IGS immediately recycles the same game ID for the very next game (observed repeatedly with Blanks: game 40 ended then game 40 restarted, game 49 three times, etc). The new game's CMD67 hit the guard, found the finished slot, and skipped slot creation entirely — leaving the bot playing into the old board.

**Fix:** The two cases are now distinguished by `white_byo_moves == -1 || black_byo_moves == -1` — IGS only sends `-1` byoyomi in the stale post-game CMD15 cleanup line, never in a real CMD67. Stale CMD15 → `goto skip`. Real CMD67 recycling a finished ID → evict the finished slot first, then proceed with fresh slot creation.

---

## 2026-06-01 (v205): Fix crash after time forfeit — stale CMD15 triggers board recreation

**Root cause:** When an opponent runs out of time, IGS sends a scoring-phase cleanup CMD15 (`"15 Game N I: got2go (0 60 -1) vs Blanks (0 60 -1)"`) a few milliseconds after the `"9 Removed game file"` game-over message. By this point `slot->game_finished = true` and `untrackFinishedGame` has already run. However, the CMD15 handler's `we_are_playing` check sees `got2go` in the player list and enters the board-creation path, calling `loadSlot` on the just-finished slot — which calls `board_widget->setDeadStones` on an inconsistent board state, crashing.

**Fix:** Added a `game_finished` guard at the top of the dock-mode `if (!board_exists)` branch. If a finished slot already exists for the game_id, the stale CMD15 is skipped via `goto skip_playing_board_creation` without touching the board widget.

---

## 2026-05-31 (v204): Fix kibitz routed to wrong game slot (bot game)

**Root cause:** When a kibitz header `"11 Kibitz user [rank]: Game X vs Y [N]"` arrived, the game ID was parsed into `pending_kibitz_game`. But the continuation line routing used `active_slot_game_id` instead — so when the bot game became active (slot 443), all subsequent kibitzes for the observed game (slot 326) were incorrectly displayed in the bot game's comment pane.

**Fix 1 — Correct routing in first kibitz block:** Changed the docked-mode routing to use `pending_kibitz_game` as the target slot ID (falling back to `active_slot_game_id` only when no game context is known). Comments are stored in the correct slot and only displayed immediately if that slot is currently visible.

**Fix 2 — Rebuild comments from authoritative list on slot switch:** `loadSlot` previously restored from a `comment_html` snapshot taken when the slot was last active — missing any kibitzes appended while the slot was in the background. Changed to always rebuild from the `slot->comments` list, which is always up to date regardless of which slot is displayed.

---

## 2026-05-31 (v203): Fix CMD21 unintended observations; fix dock button ranks showing "?"

**Bug 1 — Unintended game observations from CMD21:**
- CMD21 `"21 {Game N: X vs Y @ Move M}"` is a server-wide broadcast announcing any game resumption; it was unconditionally calling `observeGame()` for every such broadcast, auto-subscribing to random games
- Fix: added `findSlot(game_id)` guard — only call `observeGame()` if we already have a live slot for that game (i.e. we were already observing it before it adjourned)

**Bug 2 — Dock button ranks showing "?" for all games:**
- `addGame()` is called at observe/play time, but the players list (CMD5/who) may not have loaded yet — `findPlayerRank()` returns "?" at that moment
- Fix: after the players list finishes loading (`waiting_for_players = false`), iterate all live game slots, look up ranks from the now-populated players list, update `slot->black_rank`/`white_rank`, and call `dock->updateGameRanks()` to refresh the button labels

---

## 2026-05-31 (v202): toggle open true only sent when bot mode is enabled

**Change:** `toggle open true` was unconditionally sent at login, overriding whatever open/closed state the user had last set on the IGS server. IGS remembers this preference server-side, so forcing it open on every login was wrong for human-player sessions
- Removed the unconditional `toggle open true` from the login sequence
- The `toggle open true` in the bot-mode activation path (when bot mode is explicitly enabled) is kept — the bot needs to be open to receive challenges

---

## 2026-05-31 (v201): Fix resume dock button not appearing / slot not becoming active

**Root cause:** `active_slot_game_id` was updated to the new game ID before `switchActiveGame(game_id)` was called. `switchActiveGame` checks `if (game_id == active_slot_game_id) return` at entry — since the IDs already matched, it returned immediately without calling `loadSlot`, `dock->setActiveGame`, or raising the window. The dock button was added but never highlighted; the board was never loaded

**Fix:** Remove the premature `active_slot_game_id = game_id` assignment. `switchActiveGame` now sees `active_slot_game_id` as the old game ID, executes the full switch (snapshot old slot, load new slot, highlight dock button, raise window). `active_slot_game_id` is updated inside `switchActiveGame` as normal. Dock `addGame` is called before `switchActiveGame` so the button exists when `setActiveGame` fires

---

## 2026-05-31 (v200): Fix resume — use existing board state, restart bot from move history

**Root cause of v199 failure:** `moves N` sent to IGS for a playing game is silently dropped — IGS only honours it for observed games. No CMD15 history lines ever arrived, so `replay_state` was stuck in `WAITING_FOR_MOVES0` permanently, blocking all bot genmove triggers including the first live opponent move after resume

**Fix: Don't request history from server — it's already in the slot**
- `slot->board_cells` and `slot->move_history` are fully intact from before the adjournment
- Set `replay_state = LIVE` immediately on resume — no server replay needed
- `switchActiveGame` + `loadSlot` renders the existing board state into the shared window

**Fix: Restart bot by replaying move history into KataGo via GTP**
- Send `clear_board`, `komi`, `set_free_handicap` (if applicable) to reset the engine
- Replay all `move_history` entries as GTP `play` commands to reconstruct engine state
- If `current_player == bot_color`, immediately request `genmove` — bot was to move next
- If waiting for opponent, log and wait for the next live move to trigger normal genmove flow

---

## 2026-05-31 (v199): Fix resume board display and bot genmove storm during history replay

**Bug fix: Resumed game board not visible — active slot not switched on resume**
- Root cause: `switchActiveGame()` was never called after slot reassignment. The shared board window remained on whatever slot was previously active; the resumed slot existed but was not displayed and could not be clicked in the dock
- Fix: Call `switchActiveGame(game_id)` + `setPlayingMode(true)` immediately after slot reassignment in the CMD67 resume block, matching the same sequence used for new playing games

**Bug fix: Bot sent genmove for every history move replayed on resume**
- Root cause: The CMD15 bot trigger (`onBotOpponentMove`) had no guard for replay state. During `WAITING_FOR_MOVES0`/`REPLAYING`, all history moves routed through the normal live-move bot path, sending a `genmove` for each replayed opponent move — 9 consecutive genmoves in v198 test
- Fix: Check `bslot->replay_state` before calling `onBotOpponentMove`. Suppress the entire bot trigger block while the slot is in `WAITING_FOR_MOVES0` or `REPLAYING` state. The bot will respond normally to the first live opponent move after replay transitions to `LIVE`

---

## 2026-05-31 (v198): Fix resume move history — send "moves N" directly, not "observe N"

**Root cause of v197 failure:** On resume the bot and game routing worked correctly (slot reassigned, bot responded to moves), but the board position was not restored. The resume block sent `observe N` + deferred `moves N`. IGS rejected the observe with `"5 You cannot observe your own game."` — meaning the `"Adding game to observation list"` confirmation never arrived, so `games_pending_moves_request` was never drained and `moves N` was never sent
- Confirmed in v197 log: `5 You cannot observe your own game.` immediately after the resume

**Fix:** For a playing game resume, send `moves N` directly (same as the normal playing-game history path). No `observe` needed — the slot is already playing. `replay_state` set to `WAITING_FOR_MOVES0` before the send so the history handler clears the board and replays from move 1

---

## 2026-05-31 (v197): Fix adjourned game resume — intercept at CMD67, not "Observing game"

**Root cause of v195/v196 failure:** CMD67 (`67 N ...`) fires *before* `"9 Observing game N"` and creates a fresh playing slot for the resumed game ID. By the time `"9 Observing game N"` arrived, `findSlot(N)` returned the newly-created slot, bypassing the adjourned-slot search entirely
- Observed in v196 diagnostics: `OBSERVE-MATCH: game=66 findSlot=FOUND` — slot 66 already existed because CMD67 had just created it

**Fix:** Resume interception moved to the CMD67 playing-slot creation path (dock mode). When `pending_resume_player` is set:
1. Search `game_slots` for an adjourned slot matching `adjourned_player == pending_resume_player`
2. If found: reassign `game_id`, restore player names, clear `adjourned_player`/`pending_resume_player`, restart clock, set `replay_state = WAITING_FOR_MOVES0`, update dock button/`active_slot_game_id`/`bot_game_id`, send `observe N` with deferred `moves N`
3. Skip the normal new-slot creation entirely — existing slot with board position, history, and comments is reused

**Diagnostics kept:** `>>> CMD48 ADJOURN`, `>>> ADJOURN RESTORE`, `>>> ADJOURN RESUME (CMD67)`, `>>> OBSERVE-MATCH` remain always-on for now

---

## 2026-05-31 (v196): Add always-on diagnostics to adjourn/resume path for debugging

**Diagnostic: CMD48, ADJOURN RESTORE, ADJOURN RESUME, and OBSERVE-MATCH debug lines now always printed**
- Previously guarded by `suppress_server_console` — invisible during normal operation, making the resume path impossible to trace
- `>>> CMD48 ADJOURN: Game N adjourned by X` — confirms CMD48 fired and adjourned_player was set
- `>>> ADJOURN RESTORE: X reconnected` — confirms "restored/restarted your game" parser fired
- `>>> OBSERVE-MATCH: game=N w=X b=Y findSlot=FOUND/null pending_resume=X` — shows state at the decision point in the "Observing game" handler
- `>>> ADJOURN RESUME: Game N resumed as game M` — confirms slot reassignment executed

---

## 2026-05-31 (v195): Enforce slot identity contract on adjourned game resumption

**Refactor: Game ID is the only mutable field during adjourn/resume — all other slot state is preserved**
- On resume, only `game_id` is updated; board position, move history, comments, and clocks survive unchanged
- Player names are restored from the authoritative IGS `"9 Observing game N (white vs. black) :"` line (not from potentially-corrupted slot state left by a recycled ID)
- Dock button: old ID removed, new ID registered atomically in the same block
- `active_slot_game_id` and `bot_game_id` updated to new ID in the same pass

---

## 2026-05-31 (v194): Fix adjourned game resumption — player name pollution + board restore

**Bug fix: Player names overwritten on adjourned slot when IGS immediately recycles its game ID**
- Root cause: After CMD48, IGS can reassign the same game ID to a completely different game within seconds (before the disconnected player reconnects). The `"9 Observing game N (latke vs. ssmu) :"` for the recycled ID passed the `adjourned_player.isEmpty()` guard in v193 and overwrote white/black player names on the still-adjourned slot. When the original player then reconnected under a new game ID (e.g. 76), the player-name match in the adjourned slot lookup failed because the names were now latke/ssmu
- Fix: Guard in the `"9 Observing game"` handler now skips the name-update path entirely if `adjourned_player` is set on the slot — the slot belongs to the original game until explicitly resumed or removed

**Bug fix: Adjourned slot not found for resumption when ID was recycled**
- Root cause: Even with the name-pollution guard, if the slot ID was recycled first, `findSlot(new_game_id)` returns nothing and the player-name match fails because names were already corrupted before the guard was added
- Fix: Two-path lookup for adjourned slot: (a) player names still intact — match on white/black, (b) fallback — match on `adjourned_player` field against `pending_resume_player` (set when `"9 X has restored/restarted your game"` arrives). Path (b) survives name corruption

**Enhancement: Parse `"9 X has restored your old game."` / `"9 X has restarted your game."` as reconnect signal**
- Set `pending_resume_player` to X; next `"9 Observing game N"` uses it as a fallback identity for the slot reassignment

**Enhancement: Restore board position after resumption**
- On successful slot reassignment: send `observe N` + deferred `moves N` (via `games_pending_moves_request`) so the server replays the full game history into the existing slot. Player names and `bot_game_id` are also corrected to the new game ID

---

## 2026-05-31 (v193): Handle adjourned game resumption with reassigned game ID

**Enhancement: Adjourned game slot survives reconnect even if IGS assigns a new game ID**
- When a disconnected player reconnects within the 5-minute window, IGS may resume the game under a different game ID than the one that was adjourned
- Previously: the new game ID would not match any slot, causing `observeGame()` to create a duplicate slot or silently do nothing
- Fix: When `"9 Observing game N (white vs. black) :"` arrives for an unknown game ID, the slot list is searched for an adjourned (unfinished + `adjourned_player` set) slot whose white/black player names match. If found, the slot's `game_id` is updated to N, the dock button is re-registered under the new ID, `adjourned_player` is cleared, the clock is restarted, and "Opponent reconnected — game resumed." is appended to the comments panel
- `active_slot_game_id` is also updated if the adjourned slot was the active slot

---

## 2026-05-31 (v192): Fix CMD48 adjourn — suspend on disconnect, resign on removal

**Bug fix: Adjourned game incorrectly marked finished immediately on disconnect (dock mode)**
- Root cause: v191 CMD48 handler set `game_finished=true` immediately when IGS sent `"48 Game N has been adjourned by <player>"`. This was wrong because the disconnected player has a 5-minute window to reconnect and resume the game. Premature `game_finished=true` caused the slot to be evicted, allowing game ID reuse to overwrite player names — exactly the bug that was being fixed
- Correct IGS behaviour: CMD48 suspends the game; `"*SYSTEM*: Disconnected game with <player> was removed."` is the definitive end signal if the player does not reconnect within 5 minutes, at which point IGS awards the opponent a win by resign
- Fix: Two-stage handling:
  - CMD48 (`"48 Game N has been adjourned by <player>"`) — stops clock, records `adjourned_player` on the slot, posts "Game adjourned — waiting for opponent to reconnect..." to comments panel. `game_finished` remains false; slot stays active for potential resumption
  - Disconnect-removed (`"*SYSTEM*: Disconnected game with X was removed."`) — finds slot by player name, computes result: `B+R` if white dropped, `W+R` if black dropped, calls `updateGameResult` and `untrackFinishedGame`. Slot is now protected from game ID reuse
- Non-dock mode: same B+R/W+R result logic applied to board_windows lookup
- `adjourned_player` field added to `GameSlot` (full clean rebuild required)

---

## 2026-05-31 (v191): Fix handicap SGF coordinate swap

**Bug fix: Handicap stones written with swapped coordinates in generated SGF**
- Root cause: `generateSGF()` encoded `AB[]` handicap setup stones using `col='a'+pos.second`, `row='a'+pos.first` — but `getHandicapPositions()` returns `QPair<col,row>` (first=col, second=row). For symmetric corner positions the swap was invisible, but asymmetric side points (D10, Q10 in 6-stone handicap; top/bottom in 8-stone) were written to wrong coordinates. E.g. D10 was written as K4 (`[jd]` instead of `[dj]`) and Q10 as K16 (`[jp]` instead of `[pj]`)
- Affected handicap counts: 6 (side points), 7 (side points + center), 8 (all sides), 9 (all sides + center). Counts 2-5 use only corner points which are symmetric and appeared correct
- Fix: swapped to `col='a'+pos.first`, `row='a'+pos.second` to match the pair convention
- Impact: edit board launched from a 6+ stone handicap game was missing the side handicap stones; saved SGF had incorrect handicap stone placement

---

## 2026-05-30 (v190): Fix move history never arriving due to premature "moves N" send

**Bug fix: Complete move history silently dropped when observing a game (dock mode)**
- Root cause: `observe N` and `moves N` were sent back-to-back in the same Qt event loop tick. IGS requires the observation to be fully registered server-side before it will honour a `moves N` request. When both commands arrived at IGS in the same burst, IGS silently dropped `moves N` — producing no response. The slot remained stuck in `WAITING_FOR_MOVES0` forever, buffering live moves but never receiving the history reply. Result: SGF saved with only the live moves seen after joining (e.g. 56 moves instead of 766 for a long game), with corrupted board position and wrong scoring
- Diagnosed from session log: `[SLOT] game 69 moves N sent -> WAITING_FOR_MOVES0` appeared **before** `"9 Adding game to observation list."` — proving the moves request was sent before IGS confirmed the observation
- Fix: `moves N` is now deferred into a `games_pending_moves_request` set. When IGS sends `"9 Adding game to observation list."` confirming the observation is registered, all pending `moves N` commands are sent. This guarantees IGS honours the request
- Confirmed in session log for other games (25, 67, 32, 131): their `moves N` was already sent after the confirmation and worked correctly — only games where the confirmation arrived after the send were affected

---

## 2026-05-30 (v189): Game result duplicate fix + catch-up move buffer fix

**Bug fix: Game result duplicated in comments panel after slot switch (dock mode)**
- When switching away from a finished game slot then switching back, `loadSlot` restored the HTML snapshot (which already contained the "Game finished:" line) and then called `updateGameResult` again — appending a second copy
- Fix: `loadSlot` now skips the `updateGameResult` call if "Game finished:" is already present in the restored comment panel text

**Bug fix: Catch-up moves lost when joining a nearly-finished game (dock mode)**
- Root cause: When IGS floods live moves before the `moves N` history reply, those moves were discarded in `WAITING_FOR_MOVES0` state — only `catchup_high` was tracked. For a 310-move game joined near the end, IGS might flood moves 293-310 before the history, then deliver history only up to move 292. With `catchup_high=310` but history ending at 292, the slot was stuck in REPLAYING forever, never transitioning to LIVE — leaving the board frozen at move 292 with 18 moves missing
- Fix: Pre-history catch-up moves are now buffered in `pending_catchup_moves` instead of discarded. After history replay reaches its natural end, buffered moves with `move_number > last_history_move` are sorted and drained in order, filling the gap
- Confirmed against reference SGF: game 335 (guyome vs takasi2007, 310 moves) now replays completely with correct final position and W+2.5 score

**Bug fix: Object layout mismatch segfault on fresh start**
- Adding `pending_catchup_moves` (QList<GameMove>) to GameSlot changed its memory layout; the previous partial rebuild left board_window.o and xgospel2_fixed.o compiled against the old layout, causing a segfault in `loadSlot` at the `time_control` QString assignment
- Fix: full `make clean && make` required after any GameSlot member additions; documented as mandatory procedure

---

## 2026-05-29 (v188): Three-clock bar + critical move-history replay fix

**New: Three-clock bar (Local / GMT / Server)**
- Clock bar added between the status label and the console, ticking every second from startup
- Local and GMT clocks derived from the system clock
- Server clock seeded from UTC+9 (IGS joyjoy.net = JST) on login; self-corrects if IGS sends `9 The current time (GMT) is:` during a session
- Green-on-black monospace styling with border, matching console color scheme

**Bug fix: Move history replay truncated when joining a game mid-stream (dock mode)**
- Root cause: `observeGame` set `replay_state = REPLAYING` immediately. When IGS sent the live catch-up move (e.g. move 101) before the `moves N` history reply, the REPLAYING→LIVE threshold (`move_number > catchup_high`) fired instantly because `catchup_high` was still `-1`. The slot transitioned to LIVE before the history arrived, so all 100+ history moves were dropped as "duplicates" and the board record started from the catch-up move — producing a truncated SGF with the wrong first move
- Fix: `observeGame` now sets `replay_state = WAITING_FOR_MOVES0`. Pre-history live catch-up moves are tracked into `catchup_high`. When the `moves N` reply arrives (move 0), the state transitions to REPLAYING and the full history is replayed cleanly. REPLAYING→LIVE transition threshold changed from `>` to `>=` so the last replay move correctly completes the transition
- Confirmed against reference SGF from independent client: `B[qd]` (R16) is now correctly the first recorded move

**Bug fix: Finished BoardWindow could receive moves from a recycled game ID (non-dock mode)**
- The CMD15 board-lookup loop did not check `!board->isFinished()`, so a finished board still open for review could be found and have moves from a new game (same recycled ID) routed into it
- Fix: added `!board->isFinished()` guard to the board-lookup loop

---

## 2026-05-26 (v187): Shout window, BR column sort fix, player dialog rank title fix

**New: Shout (CMD21 broadcast) window**
- Dedicated Shout window shows all IGS broadcast messages with clickable sender names
- Sender names open the Player Stats dialog (same as clicking a name anywhere else)
- Send bar at bottom lets you broadcast your own shout to IGS; own message echoed locally since IGS does not reflect it back
- Shout messages are cached from login regardless of whether the window is open — open via `Windows > Show Shouts` to read accumulated messages
- New `Windows > Show Shouts` menu item to open/raise the Shout window
- New preference in Application Settings tab: "Auto-launch Shout window on login" (default off) — when on, window opens minimized at login

**Bug fix: Games list BR column sort was broken**
- `FixedRankSortProxyModel::lessThan()` was checking `column == 2` (hardcoded) before using the UserRole sort key, so every other column (including BR at column 3) fell back to default lexicographic sort, clustering all rank strings around "10kyu"
- Fixed: UserRole sort key is now used for any column when both sides have a key, regardless of column index

**Bug fix: Player Stats dialog title showed `[NR]` even when rank was known**
- Title was set once at construction from the initial (empty) rank string and never updated
- Fixed: `updateStatsData()` now calls `setWindowTitle` when a non-empty rank arrives from the server

---

## 2026-05-27 (v186): Fix new game not launching board when IGS recycles a finished game ID

**Bug fix: Playing or observed game failed to create a board when game ID was reused**

When IGS assigned a new game the same number as a recently-finished game, both the
playing-game slot creation path and the observe-game `observeGame()` path found the
old finished slot via `findSlot(game_id)` and skipped creating a new one — leaving the
dock with no board for the new game. Manually closing the old board and clicking the
game again worked around it, confirming the stale slot was blocking creation.

Fixed at both slot creation sites: before creating a new slot, check whether an existing
slot with the same ID has `game_finished == true`. If so, remove it from `game_slots`,
remove its button from the `GameSelectionDock`, and reset `active_slot_game_id` if needed
— then proceed to create the fresh slot normally.

This is the same recycled-game-ID class of bug as v183 (player name corruption), now
addressed at the slot creation level.
Location: `xgospel2_fixed.cpp` CMD15 playing-game path (~line 4658) / `observeGame()` (~line 7312).

---

## 2026-05-26 (v185): Fix observer list not updating correctly on slot switch

**Bug fix: Slot switch showed wrong game's observer list**

When switching the active slot in docked mode, the observer panel displayed the previous
game's observers instead of the newly-selected game's. A manual Refresh also had no effect.

Two fixes applied:
1. **`snapshotToSlot`** now writes the widget's live `observers_raw` list back into
   `slot->observers` before switching away, so each slot always holds exactly what was
   on screen — not a stale snapshot from the last server refresh cycle.
2. **`loadSlot`** clears the observer panel for finished games (no further server updates
   will arrive) and restores from the freshly-snapshotted slot data for live games.

Location: `board_window.cpp` `snapshotToSlot()` / `loadSlot()`.

---

## 2026-05-26 (v184): Fix observer list pollution from concurrent player stats responses

**Bug fix: Idle times and stats lines appearing as fake observer entries**

Player stats responses arriving concurrently with an observer list refresh (e.g. `9 Idle Time:  3s`,
`9 Playing in game: 15`) were being parsed as name/rank pairs, inflating the observer count
with nonsense entries like "3s 1p", "0s 8d".

Three fixes applied:
1. **Colon guard**: Any `9 ` line containing `:` is now rejected during observer parsing —
   all IGS stats lines contain colons; genuine observer rows never do.
2. **Rank token validation**: Each parsed pair's rank token must match `\d+[kdp][+*?]?` or
   equal `"BC"`. Pairs that fail this check are silently skipped.
3. **Alternate parsing block removed**: A vestigial fallback path (`waiting_for_observer_response
   && !parsing_observers`) matched rank-like patterns in any `9 ` line, adding single fake
   entries from stats interleaved between observer requests. Removed entirely.

Location: `xgospel2_fixed.cpp` observer line parser (~line 4088).

---

## 2026-05-26 (v183): Fix game ID reuse corrupting finished slot player names

**Bug fix: Finished slot player names overwritten when IGS recycles a game ID**

When a bot game ended and IGS immediately reused that game number for a different game,
the `9 Observing game N (PlayerA vs. PlayerB) :` response for the new game matched the
old finished slot by ID and overwrote its `white_player`/`black_player` fields, causing
the board display to show the wrong players when switching back to the finished slot.

Fixed by adding a `game_finished` guard in the "update player names" branch of the
`Observing game` parser — both dock mode (`GameSlot::game_finished`) and non-dock mode
(`BoardWindow::isFinished()`). A finished slot is never updated with recycled-ID player names.
Location: `xgospel2_fixed.cpp` `Observing game` line parser (~line 4022).

---

## 2026-05-24 (v182): Player dialog chat font, observer auto-refresh, comment history, player name buttons, bot crash resign

**Fix 1: Player dialog chat font increased to 11px**

The `message_display` QTextEdit in the player tell/chat dialog was styled at `font-size: 10px`.
Increased to 11px to match match preferences and other chat areas. The `message_input` text
field also receives `font-size: 11px` styling.
Location: `xgospel2_fixed.cpp` player dialog construction.

**Fix 2: Remove debug message from Refresh Observers button**

The "*** BUTTON CLICKED - Test observers added! ***" debug string logged to the console
whenever the Refresh Observers button was clicked has been removed. The handler now
cleanly calls `clearObservers()` and `requestObservers()`.
Location: `board_window.cpp` `requestObservers()`.

**Fix 3: Observer list auto-refreshed at players refresh rate**

The observer list now refreshes automatically on the same timer as the players list
(every 30 seconds). In docked mode the active slot's observer list is refreshed; in
non-docked mode every open board window with a live game refreshes its own list.
Location: `xgospel2_fixed.cpp` `refreshPlayers()`.

**Fix 4: Comment & Kibitz history no longer changes on slot switch**

When switching the active game in docked mode, the Comments panel now correctly
restores the full chat history of the newly-selected game. A snapshot of the rendered
HTML is stored in `GameSlot::comment_html` on `snapshotToSlot()` and restored via
`setHtml()` in `loadSlot()`, preserving formatting and scroll position.
Location: `game_slot.h` `comment_html` field / `board_window.cpp` `snapshotToSlot()` / `loadSlot()`.

**Fix 5: Player names on board window are now clickable**

The white and black player name labels in the board window have been converted from
`QLabel` to `QPushButton` with transparent background, no border, left-aligned text,
hover underline, and a pointing-hand cursor. Clicking opens the player's stats/tell
dialog via `openStatsDialogForPlayer()`. Signals `whitePlayerClicked(QString)` and
`blackPlayerClicked(QString)` are wired at all five board-window connection sites.
Location: `board_window.h` signals / `board_window.cpp` construction / `xgospel2_fixed.cpp` connect sites.

**Fix 6: Bot sends resign when KataGo crashes mid-game**

When KataGo crashes (e.g. SIGSEGV exit code 11) during an active bot game, xgospel2
now immediately sends `resign` to IGS so the game ends cleanly rather than expiring on
time. The `onEngineError` handler detects `bot_mode_active && bot_game_id != -1` and
writes `resign\n` directly to the IGS socket, logging the crash message.
Location: `xgospel2_fixed.cpp` `onEngineError()`.

---

## 2026-05-25 (v181): Observer list buttonization, count display, and rank sorting

**Feature 1: Observer names are now clickable**

Clicking any name in the Observers list opens that player's stats/tell dialog, exactly
as double-clicking a name in the Players list does. The `observerClicked(QString)` signal
is emitted by `BoardWindow` and wired to `players_window->openStatsDialogForPlayer()` at
all five board-window connection sites (docked and non-docked paths, both the initial
setup and the observe-specific path).
Location: `board_window.cpp` `addObserver` / `board_window.h` signal / `xgospel2_fixed.cpp` connect sites.

**Feature 2: Observer count in panel title**

The Observers panel title now reads "Observers (N)" where N is the number of observers
currently in the list. The count updates on each `addObserver()` call and resets to
"Observers" on `clearObservers()`. `observers_title` is promoted from a local variable
to a member pointer so it can be updated dynamically.
Location: `board_window.h` member / `board_window.cpp` `clearObservers()` / `addObserver()`.

**Feature 3: Sort by rank (default) with toggle**

Observers are sorted by strength (strongest at top) by default, matching xgospel1
behavior. A small "Rank" button to the right of the title toggles between rank order
and join order; the button label changes to "Order" when join order is active. The raw
join-order list is preserved in `observers_raw` so toggling is lossless.
Rank parsing handles p/d/k suffixes with +/* modifiers correctly.
Location: `board_window.cpp` `rankToStrength()` / `addObserver()` / `toggleObserverSort()`.

---

## 2026-05-24 (v180): Player dialog stats font size increase, remove redundant name label

**Fix 1: Player name duplicated in dialog**

The player stats box displayed the player name and rank in a centered label at the top
of the info panel, redundant with the dialog title bar which already shows the same
information. Removed `player_name_label` from the stats box. The member pointer is
retained and initialized to `nullptr` so the existing `if (player_name_label)` guard
in `updateDynamicLabels()` safely skips it.

**Fix 2: Player stats font too small**

All stats labels (Wins, Losses, Rated, Country, Info, Idle, Playing, Observing, last
log) were styled at `font-size: 9px`. Increased to `font-size: 11px`. Vertical spacing
between grid rows increased from 0 to 2px for readability.
Location: `xgospel2_fixed.cpp` `PlayerStatsDialog` constructor.

---

## 2026-05-24 (v179): Observer list font size increase, match prefs font size increase

**Fix 1: Observers list font too small**

Observers list was rendered at `font-size: 10px` — smaller than the Comments & Kibitz
panel (`font-size: 12px`). Increased observers list to `font-size: 12px` for consistency.
Location: `board_window.cpp` observers_list stylesheet.

**Fix 2: Match preferences text almost unreadable in player dialog**

`dyn_matchprefs_label` was styled at `font-size: 8px`. Increased to `font-size: 11px`.
Word wrap was already enabled so longer match preference strings wrap to a second line.
Location: `xgospel2_fixed.cpp` PlayerStatsDialog constructor.

---

## 2026-05-24 (v178): Bot opponent blacklist in Preferences

**Feature: Bot Opponent Blacklist**

Added a "Bot Settings" tab to the Preferences dialog containing an "Opponent Blacklist"
field. Comma-separated IGS usernames entered here will have their match and nmatch
requests automatically declined by the bot.

- Names are stored lowercase and matched case-insensitively
- Both old `match` protocol and `nmatch` protocol requests are checked
- Setting persists across sessions via the `bot_blacklist` key in settings file
- Console logs `[BOT] Declining match/nmatch from X — player is blacklisted`

New code:
- `Settings::getBotBlacklist()` / `setBotBlacklist()` — `settings.h` / `settings.cpp`
- `PreferencesDialog` — new "Bot Settings" tab with `m_bot_blacklist_edit` QLineEdit
- `xgospel2_fixed.cpp` match/nmatch accept handlers — blacklist check before accepting

---

## 2026-05-24 (v177): Rebase onto v154 working overlay; restore v155–v176 bot/feature improvements

**Background**

Versions v155–v176 introduced a series of CMD22 territory overlay changes that
progressively broke the scoring overlay for observed games in docked-pane mode. The
root cause was a combination of:
1. Incorrect CMD22 digit encoding (0/1 inverted vs server reality)
2. Coordinate axis swap in `drawTerritoryMarkers` applied in wrong direction
3. CMD22 routing rewrite (`cmd22_streams` map) that discarded data when games were
   not yet in scoring mode at the time CMD22 arrived (before the result line)

Rather than continuing to patch the broken state, v177 reverts `board_window.cpp` and
`xgospel2_fixed.cpp` to the v154 baseline (confirmed working overlay) and surgically
re-applies only the non-overlay improvements from v155–v176.

**Bot improvements restored from v155–v176:**

- `bot_time_forfeit_loser` — tracks who ran out of time from "9 X has run out of time."
  so the "Removed game file" path can emit the correct `B+T` / `W+T` result
- KataGo lag buffer — subtracts 2s from `time_left` sent to KataGo to account for IGS
  network round-trip latency; floor at 1s so KataGo always gets a non-zero budget
- Bot re-send `done` on re-negotiation — when opponent types `done` a second time after
  additional stone removals, bot re-sends `done` to avoid getting stuck waiting for score
- `bot_time_forfeit_loser.clear()` added to bot reset path

**Other improvements restored:**

- Console dump filename now includes version string (e.g. `xgospel2_console_dump_v177_...`)
- Prompt suppression extended to cover IGS `"2"` and `"2 ..."` lines
- `current_node->setTerritoryMap(tmap)` added in `loadSlot` territory restore so node
  has territory for subsequent `displayNode` calls
- `getScoringModeEnabled()` accessor added to `GoBoardWidget`
- Leaf-node test for auto-follow `at_end` (`!current_node->nextMove()` instead of
  move-count comparison) for consistency with game trees that have extra nodes

**Omitted from restoration (requires Settings infrastructure not in v154):**

- Bot opponent blacklist (added separately in v178)

---

## 2026-05-24 (v176): Bot re-send done on scoring re-negotiation

**Fix: Bot stuck waiting for score when opponent removes additional stones**

During IGS scoring, if the opponent removes a stone after both sides have typed `done`,
IGS sends another "has typed done." message. The bot was logging "waiting for score"
but not re-sending `done`, leaving the game stuck until manual intervention.

Fix: when opponent types `done` a third (or subsequent) time after `bot_pass2_rendered`
is already set, the bot immediately re-sends `done`.
Location: `xgospel2_fixed.cpp` "has typed done." handler.

---

## 2026-05-21 (v154): Game-end result display fixes and kibitz console flood mute

**Fix 1: Handicap shows 0 at game end (bot games)**

`updateGameSetup()` was called from the CMD22 slot path using `slot->handicap`, which
comes from `game_handicap_map` and may be stale (0) for a second game vs the same opponent
if CMD7 hasn't updated the map yet. The authoritative `bot_handicap` and `bot_komi` values
set during match acceptance were never propagated to the board display.

Fix: in `onBotEngineReady()`, after confirming `bot_komi` and logging the engine-ready
message, call `engine_board->updateGameSetup(bot_handicap, bot_komi, "")` so the board's
handicap/komi label always reflects the match-acceptance values.

**Fix 2: CMD20 result not shown in Comments & Kibitz (bot game not active slot)**

In docked mode, the CMD20 score-result handler was skipping `updateGameResult()` when
the bot game wasn't the currently displayed slot. The board window showed the other game's
position without the result overlay.

Fix: unconditionally call `switchActiveGame(slot->game_id)` and `updateGameResult()` when
CMD20 matches the bot game, regardless of which slot is active.

**Fix 3: Race between CMD20 and "9 game completed." double-cleanup**

When both lines arrive in the same socket read cycle (as observed in game 567), CMD20
fired `botEndGame()` and then `"9 game completed."` fired it again, resetting bot state
twice and causing a log duplicate.

Fix: new `bot_cmd20_received` flag — set in the CMD20 handler when the score is parsed,
checked in the `"9 game completed."` fallback so it only runs when CMD20 was absent.

**Fix 4: Kibitz from observed pro games floods the output console**

Kibitz lines from observed games were always appended to the output console (the `>>> [OK]
KIBITZ SUCCESS` summary line was unconditional). Kibitz content is already stored in the
Comments & Kibitz pane and the slot comment list, making the console output redundant.

Fix: gate the `KIBITZ SUCCESS` console append behind `!suppress_server_console`.

---

## 2026-05-21 (v153): Bot match fairness validation — decline unfair handicap/color requests

When bot mode is open to all challengers, players can send manually crafted nmatch
requests with incorrect handicap or wrong color assignment, exploiting the bot and
damaging its rating.

Added fairness validation in `botAcceptMatch()`:

- **Rank conversion**: new `rankToStones()` helper converts IGS rank strings to a linear
  stone scale (30k=-30 … 1k=-1, 1d=0 … 9d=8, 1p=9 … 9p=17), matching IGS server convention
  where 1d vs 1k = 1 stone difference.

- **Expected handicap**: `expectedHandicap()` computes the correct handicap from the rank
  difference (capped at 9), and determines which player should be Black.

- **Handicap check**: decline if offered handicap differs from expected by more than ±1
  stone (±1 tolerance for borderline rank cases).

- **Color check**: when rank diff ≥ 2 stones, color is not negotiable — decline if the
  offered color assignment puts the stronger player as Black.

- **Fallback**: if either rank is unknown ("?"), log a warning and accept anyway rather
  than blocking legitimate games where ranks haven't loaded yet.

Decline reasons are logged with full detail: offered vs expected values and both ranks.
Also added missing `socket->flush()` calls to the existing boardsize and byoyomi decline
paths.

---

## 2026-05-21 (v152): Bot resign handling — correct game_id, early farewell, result in Comments

**Bug 1: Resignation not routed to bot game (wrong game_id in docked mode)**

The simple resignation handler used `active_slot_game_id` to identify which game ended.
When the user was also observing other games, the active slot was a different game, so
`untrackFinishedGame` was called with the wrong ID — `botEndGame()` never fired, leaving
`bot_game_id` set and causing the bot to decline the next match challenge as "game already
in progress."

Fix: in docked mode, when `bot_mode_active` and the resigner matches `bot_opponent`,
use `bot_game_id` directly instead of `active_slot_game_id`.

**Bug 2: Farewell `tell` hit "5 Cannot find recipient" — sent too late**

`botEndGame()` sent the farewell after `"9 Removed game file"`, by which time the opponent
had often already logged off. Added early farewell in the resignation handler (before
`untrackFinishedGame`), guarded by a new `bot_farewell_sent` flag so `botEndGame()` skips
it if already sent. Scored games (CMD20 path) still send via `botEndGame()` as before.

**Bug 3: Resignation result not shown in Comments & Kibitz**

When the bot game was not the active displayed slot, `updateGameResult()` was skipped.
Fix: call `switchActiveGame(game_id)` then `updateGameResult()` unconditionally so the
result always appears on the board regardless of which slot was active.

---

## 2026-05-21 (v151): Bot time management — send time_left before genmove

**Bug: KataGo ignoring IGS time controls, moving too quickly**

`onBotOpponentMove()` was calling `play` then `genmove` with no `time_left` in between.
KataGo had no time context when starting to think, so it fell back to the `time_settings`
sent during engine init (`time_settings 0 5 1` — 5 seconds per move, absolute). This
capped thinking time to ~5s regardless of how much time was actually available on IGS.

Fix: send `time_left <our_color> <seconds> <stones>` between `play` and `genmove` so
KataGo knows the full remaining budget before it starts searching. With a canadian time
control of 600s/25 stones, KataGo will now spread ~24s per move instead of 5s.

Also fixed: in non-docked mode `bot_time_remaining` was never refreshed during the game
(it stayed at the initial `bot_main_time` value). Both modes now pull current time from
`findSlot(bot_game_id)` on every opponent move.

---

## 2026-05-21 (v150): UI polish — Close button fix, post-game tell, font sizes, format string

**Fix 1: Player stats dialog — Close button duplicated on stats refresh**

`updateStatsData()` previously tore down the entire layout and called `setupUI()` on every
stats refresh, causing a new Close button to be added each time. After viewing a player's
stats then re-requesting them, N stacked Close buttons appeared.

Fix: added 10 `QLabel*` member pointers (`dyn_wins_label` … `dyn_matchprefs_label`);
`setupUI()` now creates them once and calls `updateDynamicLabels()` at the end;
`updateStatsData()` calls `updateDynamicLabels()` in-place instead of tearing down and
rebuilding the layout. No layout teardown ever occurs after first construction.

**Fix 2: Post-game discussion — Comments & Kibitz uses `tell` after game ends**

After a game finishes, IGS rejects `say` commands. The send button in Comments & Kibitz
now routes outgoing messages through `tell <opponent>` when `game_finished` is true, using
the correct opponent name derived from `white_player`/`black_player` vs `my_username`.

Added `tellRequested(player, message)` signal to `BoardWindow`; connected to
`FixedXGospelWindow::sendTell` at all four board-creation sites (docked×2, standalone×2).

**Fix 3: Font size increases — player info pane and Comments & Kibitz**

- `game_info_label` (Game # | Color to Play): 11px → 13px
- `white_player_label` / `black_player_label`: 12px → 14px
- `white_captures_label` / `black_captures_label`: 10px → 12px
- `handicap_komi_label`: 10px → 12px
- `comment_display` (QTextEdit): 10px → 12px
- `comment_input` (QLineEdit): 10px → 12px
- Comments send button: 10px → 12px

**Fix 4: Format string bug in bot territory log line**

`QString("[BOT] Bot territory cells (threshold %.2f): %1")` — `%.2f` is C printf syntax;
Qt ignores it and prints a literal `%.2f`. Changed to use `.arg(BOT_TERRITORY_THRESHOLD, 0, 'f', 2)`.

**Fix 5: Bot mode board title in docked pane shows "No game" at game start**

`botStartGame()` now calls `engine_board->updatePlayerNames(wname, bname)` for the docked
case, ensuring the player name labels and game_info_label refresh immediately when the
bot game begins (previously relied entirely on loadSlot having run first).

---

## 2026-05-20 (v149): CMD22 wrong-slot routing fix; bot cleanup on missing CMD20

**Bug 1: CMD22 territory data routed to finished game slot (rematch scenario)**

When got2go plays multiple consecutive games against the same opponent, old finished game
slots stay in `game_slots`. The CMD22 handler matched by player name only, so it found
the first slot containing that player name — which was the already-finished previous game.
The active game's CMD22 was silently buffered into the dead slot, so Stage 3 territory
never appeared on the active board.

Fix: skip `slot->game_finished == true` slots in the CMD22 player-name search loop
(both docked and non-docked modes). Same fix applied to non-docked `BoardWindow` loop
using `board->isFinished()`.

**Bug 2: `botEndGame()` never called when IGS omits CMD20**

IGS sometimes closes a scored game with `"9 game completed."` + `"9 Removed game file..."
without sending a CMD20 result line. `untrackFinishedGame()` (which calls `botEndGame()`)
was only triggered by CMD20, so `bot_game_id` and `bot_overlay_active` were never reset,
leaving the bot in a half-finished state for the next game.

Fix: when `"9 game completed."` arrives and `bot_done_sent` is true (bot participated in
scoring) but `bot_game_id != -1` (botEndGame not yet called), call
`untrackFinishedGame(bot_game_id)` as a fallback cleanup path.

**Files changed**
- `xgospel2_fixed.cpp`: CMD22 loop adds `game_finished` skip; `"9 game completed."` fallback
  cleanup for bot games without CMD20

---

## 2026-05-19 (v148): Replace KataGo ownership with local flood-fill for territory overlay

**Root cause of cross-color territory at Stage 1 / Stage 2 — now fixed permanently**

`applyBotTerritoryOverlay()` was building the territory map from KataGo `kata-raw-nn`
`whiteOwnership` values. Despite multiple attempts to align GTP row order vs. display row
order, the rendering was consistently wrong (black and white boxes swapped).

The real problem: KataGo ownership is engine-side analysis with inherent noise, and the
coordinate mapping between KataGo GTP conventions and xgospel2's display system is fragile.

**Fix: replaced KataGo ownership with `ScoreEngine::estimate()` — the exact same
flood-fill algorithm used by the Score button and `markStoneAsDead()` in edit-position mode.**

The new implementation:
1. Reads the current board position from the board widget via `board->getStoneAt(x, y)`
2. Reads all dead-marked stones from the board widget via `board->getDeadStones()`
3. Builds a `GoBoard` with dead stones excluded (treating them as empty intersections)
4. Calls `ScoreEngine::estimate()` — flood-fill assigns territory correctly since dead
   stones are absent and flood-fill flows through their vacated positions
5. Calls `board->setScoringModeWithTerritory(territory_map)` with the result

This is coordinate-system-agnostic and matches Stage 3 (CMD22) rendering exactly.
No more KataGo ownership indexing, no more flip/flop confusion.

**Files changed**
- `xgospel2_fixed.cpp`: `applyBotTerritoryOverlay()` rewritten; added `#include "score_engine.h"`
- `board_window.h`: added `getDeadStones()` accessor on `BoardWindow` delegating to `board_widget`

---

## 2026-05-19 (v146): Three fixes from v145 test game

**1. Wrong territory colors at Stage 1 / Stage 2 (inverted black/white boxes)**

KataGo `kata-raw-nn whiteOwnership` is in GTP row order: row 0 = bottom of the board.
`applyBotTerritoryOverlay` was indexing `own[y * 19 + x]` where y=0 is the display top —
treating GTP-bottom as display-top, flipping the entire territory map vertically.
Fix: index as `own[(18 - y) * 19 + x]` to align GTP bottom-row 0 with display bottom-row 18.

**2. Score result not appearing in Comments & Kibitz box (bot scoring games)**

When IGS sends `20 <white> (W:O): X to <black> (B:#): Y` (CMD20) without the corresponding
`9 {Game N: ... : W X B Y}` counting result line, the board window never received
`updateGameResult()`. The CMD20 handler logged the score but didn't call `updateGameResult()`.
Fixed by: matching both white AND black player names (not OR — prevents false matches when a
player has multiple simultaneous games), calling `setServerScore()` + `updateGameResult()` on
the board/slot, stopping the clock timer, and calling `untrackFinishedGame()` so `botEndGame()`
fires correctly.
**Critically: did NOT add `enterScoringModeForResult()`** — that calls `calculateScore()` which
clobbers non-scoring games (e.g. W+R games would get a spurious territory overlay).

**3. Game record corruption — W+R games showing scoring overlay (CRITICAL BUG)**

The v145 CMD20 handler used `slot->white_player == white_player || slot->black_player == black_player`
(OR match). When got2go played multiple games, a CMD20 for one game's score would match a
different game's slot (e.g. a W+R game against cau) and call `enterScoringModeForResult()` on
it, overlaying a territory map on a game that ended by resignation. The fix: require BOTH players
to match (`&&`), and remove `enterScoringModeForResult()` from the CMD20 path entirely.

**4. Console Dump Folder moved to Application Settings tab**

The setting was placed at the bottom of the Server Connections tab where it was hidden below
the large host list. Moved into the "Console Settings" group on the Application Settings tab,
directly below Console Buffer Size, as a form row with Browse button.

**Files changed**
- `xgospel2_fixed.cpp`: `applyBotTerritoryOverlay` y-flip; CMD20 handler (both-player match,
  `updateGameResult`, `untrackFinishedGame`, no `enterScoringModeForResult`)
- `preferences_dialog.cpp`: removed consoledump block from Server tab; added as form row in
  Console Settings group of Application Settings tab

---

## 2026-05-19 (v145): Bot scoring — fix territory overlay wiped by CMD49 markStoneAsDead()

**Root cause identified and fixed for all-green territory at Stage 1 and Stage 2**

Each time IGS echoes a dead stone removal as CMD49 (format: `49 <game_id> <player> is removing @ <coord>`),
`BoardWindow::markStoneAsDead()` is called on the shared board window. That function always calls
`calculateScore()`, which uses a raw flood-fill on the current board state (dead stones still physically
present) and replaces our KataGo ownership territory map with an all-EMPTY (green dame) map.

This happened once for each remove command — typically 5 dead groups × 1 CMD49 each = 5 overwrites
of the correct KataGo overlay, leaving only the broken flood-fill result visible.

**Fix: `bot_overlay_active` flag + CMD49 restore + docked-mode double-toggle guard**

1. Added `bool bot_overlay_active` member (reset to false in `botEndGame()`).
2. `applyBotTerritoryOverlay()` sets `bot_overlay_active = true` after applying the map.
3. CMD49 handler (docked pane mode, active slot): after `markStoneAsDead()`, if
   `bot_overlay_active` is true, immediately re-applies the KataGo overlay via
   `applyBotTerritoryOverlay(bot_pass2_rendered ? "Pass 2 (restore)" : "Pass 1 (restore)")`.
4. CMD49 handler (non-docked mode): same re-apply guard added after the existing
   `is_bot_own_remove` check.
5. Added `is_bot_own_remove_docked` guard in docked mode CMD49 path — the bot's own
   remove echoes were calling `markStoneAsDead()` a second time, toggling the stones
   back to ALIVE. Non-docked mode already had this guard; docked mode was missing it.
6. `bot_overlay_active = false` is set when CMD22 `receiveScoreEnd()` fires so Stage 3
   (authoritative server result) takes control cleanly without being overwritten back.

**Also added in v144 (recorded here): Console dump folder preference**

- `Settings::getConsoleDumpDirectory()` / `setConsoleDumpDirectory()` — key `CONSOLEDUMPDIR`
- Default: `$HOME/Claude_Projects/64-bit/xgospel2_console_dumps`
- Preferences dialog: "Console Dumps" group box in Server Connections tab with browse button
- "Save Console" action now opens the file dialog defaulting to the configured folder with
  auto-generated timestamp filename (`xgospel2_console_dump_YYYYMMDD_hhmmss.txt`)

**Files changed**
- `xgospel2_fixed.cpp`: `bot_overlay_active` member; CMD49 docked guard; CMD49 non-docked guard;
  `applyBotTerritoryOverlay()` sets flag; `botEndGame()` clears flag; CMD22 clears flag
- `settings.h` / `settings.cpp`: `getConsoleDumpDirectory()` / `setConsoleDumpDirectory()`
- `preferences_dialog.h` / `preferences_dialog.cpp`: console dump folder UI

---

## 2026-05-19 (v143): Dock button rank fix; Pass 2 duplicate suppression

**Dock game pane button rank always showing "?"**

Root cause: `dock2->addGame()` is called at CMD15 game-creation time, before the IGS
`who` response arrives with the opponent's rank. `findPlayerRank()` returns "?" because
the player isn't yet in the players model. The rank was later written to `bslot->white_rank`
/ `bslot->black_rank` but the already-created button was never refreshed.

Fix: in the `who` response handler (after updating `bslot->*_rank`), call the new
`dock2->updateGameRanks(game_id, black_rank, white_rank)` which propagates to
`GameButtonWidget::updateRanks()` and triggers a repaint with the real rank.

New plumbing added:
- `GameButtonWidget::updateRanks(b_rank, w_rank)` — updates `m_b_rank`/`m_w_rank` and calls `update()`
- `GameSelectionDock::updateGameRanks(game_id, b_rank, w_rank)` — finds button and delegates

**Pass 2 territory overlay firing multiple times**

IGS sends `"<player> has typed done."` multiple times per scoring session. The "has typed
done" handler was calling `applyBotTerritoryOverlay()` on every occurrence, causing
redundant repaints.

Fix: added `bool bot_pass2_rendered` member. Set to `true` on first Pass 2 render;
cleared in `botEndGame()`. Guard in handler: `if (bot_done_sent && !bot_pass2_rendered)`.

---

## 2026-05-19 (v142): Bot scoring — three-pass progressive territory rendering

**Three-pass territory overlay sequence**

Territory now renders progressively in three stages rather than waiting 63 seconds
for the server's CMD22:

1. **Pass 1 — bot sends done** (1500ms after CMD9): territory rendered from KataGo
   ownership snapshot. Opponent's dead groups not yet removed so their territory
   may appear as dame at this stage.
2. **Pass 2 — opponent sends done** (`"<player> has typed done."` received): CMD49
   removals already applied to the board; `applyBotTerritoryOverlay()` re-renders
   with both sides' dead stones now removed. 99%+ of the time this matches the
   server result.
3. **Pass 3 — CMD22 arrives** (~63s later): server's authoritative territory data
   overwrites via `receiveScoreEnd()` as always.

**`bot_ownership_snapshot` member** — `QVector<float>` stored in `onBotOwnershipReady()`
before the 1500ms timer fires; persists until `botEndGame()` clears it. Allows the
"has typed done" handler to re-apply the same ownership map after opponent removals.

**`applyBotTerritoryOverlay(label)`** — shared helper that builds
`QMap<QPair<int,int>, StoneColor>` from `bot_ownership_snapshot` at threshold 0.35
(black-positive convention) and calls `engine_board->setScoringModeWithTerritory()`.
Console logs `"[BOT] Territory overlay — Pass N (...): M intersections"`.

---

## 2026-05-19 (v141): Bot scoring — fix territory overlay wiped by displayNode()

**Root cause: `displayNode()` clears scoring mode on every non-final node**

`setScoringModeWithTerritory()` set scoring mode and applied the territory map, but
every subsequent CMD15 move triggered `displayNode()` which called
`board_widget->setScoringMode(false)` for any node without territory attached to it
— immediately wiping the overlay.

Fix: `setScoringModeWithTerritory()` now calls `current_node->setTerritoryMap(tmap)`
before applying to the widget. `displayNode()` checks `node->hasTerritory()` first
(line 5377) and restores the map when true, so the overlay survives subsequent
CMD15 redraws.

---

## 2026-05-19 (v140): Bot scoring — fix territory overlay using KataGo ownership instead of ScoreEngine

**Root cause: `calculateScore()` ignores already-marked dead stones**

v136 called `engine_board->calculateScore()` after `markStoneAsDead()` to render territory
immediately. However `calculateScore()` calls `ScoreEngine::estimate()` on the raw
`current_node->getBoard()` position — which still has dead stones present as live stones.
`ScoreEngine` produces its own independent dead-stone set and territory map, completely
ignoring the stones already marked via `markStoneAsDead()`. Result: territory was computed
as if no stones were dead, producing incorrect (or missing) territory markers.

**Fix: build territory map directly from KataGo ownership vector**

The `ownership` parameter of `onBotOwnershipReady()` is now captured by value into the
1500ms timer lambda. After dead stones are marked and `done` is sent, the timer builds a
`QMap<QPair<int,int>, StoneColor>` directly from the ownership floats:
- `ownership[y*19+x] >= +0.35` → `BLACK_STONE` territory
- `ownership[y*19+x] <= -0.35` → `WHITE_STONE` territory
- Convention: values are black-positive (+1=black owns, -1=white owns) — already negated
  from KataGo's white-positive `whiteOwnership` in `katago_engine.cpp`

Threshold 0.35 is intentionally loose (vs 0.70 used for dead-stone detection) to colour
all clearly-owned intersections including the now-vacated dead-stone points.

**New public method: `BoardWindow::setScoringModeWithTerritory()`**

Added to `board_window.h` / `board_window.cpp`: sets `is_scoring_mode = true`,
calls `board_widget->setScoringMode(true)` and `board_widget->setTerritoryMap(tmap)`.
Allows the bot timer to apply a territory overlay without touching the dead stone set
or running ScoreEngine. CMD22 arriving ~63s later still overwrites via `receiveScoreEnd()`
as normal.

---

## 2026-05-19 (v139): Games list — fix false end-of-list trigger from IGS broadcast lines

**Root cause of intermittent games list blanking**

IGS periodically broadcasts server status lines of the form:
```
< 9 ******** 1246 Players 366 Total Games ********
```
The end-of-list detection condition `line.contains("**") && !line.contains("[")` matched
these broadcast lines — they contain `**` but no `[`. When one arrived while
`waiting_for_games` was true (i.e. mid-refresh), it set `waiting_for_games = false`
immediately, causing all subsequent real game lines to be silently discarded. The result
was a blank games window showing "0 games" in the title bar. A manual refresh recovered
because it reset `waiting_for_games = true` correctly.

Fix: added `&& !line.contains("Players")` to the `**` end-of-list guard in
`xgospel2_fixed.cpp`. IGS broadcast lines always contain "Players"; real end-of-list
markers (e.g. `**end**`) do not.

---

## 2026-05-19 (v138): Games list — defer clearGames() until first game line arrives

Previously `refreshGames()` called `clearGames()` immediately when sending the `"games"`
request to IGS, leaving the window blank and showing "0 games" during the entire
network round-trip. Now `clearGames()` is deferred to the moment the first game line
is received (`game_count == 0` check inside the line parser). The previous list remains
visible until new data actually starts arriving, so a delayed or interrupted server
response never leaves the window blank.

---

## 2026-05-19 (v137): Games list — fix synchronous repaint race causing occasional blanking

`updateObservedGames()` was calling `viewport()->repaint()` (synchronous) which could
fire mid-model-update and paint a stale or blank view. Changed to `viewport()->update()`
(deferred, coalesced by Qt's paint event queue).

Also removed a redundant `QTimer::singleShot(500)` that called `updateObservedGames()`
a second time 500ms after end-of-list — it was a workaround for the repaint race and
is no longer needed.

---

## 2026-05-19 (v136): Bot scoring — immediate territory overlay; games pane rank display fix

**Immediate territory overlay after bot sends done (v136)**

Previously xgospel2 showed green dame markers in dead-stone areas during the ~63-second
wait for the server's CMD22 territory broadcast. q5Go displayed full territory immediately.

Fix: after the 1500ms timer fires dead stone removes + `done`, call `engine_board->calculateScore()`
immediately. This runs `ScoreEngine::estimate()` on the current board + already-marked dead stones,
flood-fills territory, and renders the scoring overlay without waiting for CMD22. When CMD22 arrives
63 seconds later, `receiveScoreEnd()` overwrites with the authoritative server result as normal.

Single addition to the timer lambda in `onBotOwnershipReady()` in `xgospel2_fixed.cpp`:
```cpp
if (engine_board)
    engine_board->calculateScore();
```

**Games pane buttons show actual player ranks (v135)**

Bot game accept/challenge buttons in the docked games pane were displaying "?" for both
player ranks. Fixed in `xgospel2_fixed.cpp` CMD15 handler: `dock2->addGame()` now passes
`slot->black_rank` and `slot->white_rank` (populated by `findPlayerRank()`) instead of
hardcoded `"?"` strings.

---

## 2026-05-18 (v116): Bot mode — replace kata-analyze with kata-raw-nn for ownership; Save Console menu item

**Root cause of v112–v115 ownership failures**

`kata-analyze interval 500 ownership true` relies on KataGo emitting at least one
`info move ... ownership ...` line before the 1500ms collection window closes. Two
conditions prevented this in every tested game:

1. `searchFactorAfterTwoPass = 0.25` reduced max visits to 175 after two passes.
   With 16 search threads + OpenCL, 175 visits completed in <500ms — the 500ms
   interval never fired, producing zero output lines.
2. `ponderingEnabled = true` kept a warm search tree. Even with `searchFactorAfterTwoPass`
   corrected to 1.0 (v115 config change), KataGo reused ponder visits and completed
   all 700 visits in ~1 second — again faster than the 500ms interval.

**Fix: replace `kata-analyze` with `kata-raw-nn 0`**

`kata-raw-nn 0` is a synchronous GTP command: KataGo runs a single neural net
evaluation and returns a `\n\n`-terminated response block containing `whiteOwnership`
(361 floats). It does not depend on visit counts, interval timing, or pondering state.

Changes in `katago_engine.cpp`:
- `requestOwnership()` now enqueues `kata-raw-nn 0` instead of `kata-analyze interval 500 ownership true`
- Removed the entire `kata-analyze` streaming drain loop (single-`\n` line consumer)
- Removed `m_ownership_timer`, `m_ownership_stop_sent`, and the 1500ms timer machinery
- `handleResponse()` parses `whiteOwnership` from the `kata-raw-nn` response body;
  values are negated to match the bot's black-positive ownership convention
- Error path: `kata-raw-nn` failure emits `ownershipReady(QVector<float>())` → fallback `done`

Changes in `katago_engine.h`:
- Removed `m_ownership_stop_sent` and `m_ownership_timer` members
- Updated comments

Changes in `xgospel2_fixed.cpp`:
- Console log messages updated: "kata-analyze ownership (1.5s window)" → "kata-raw-nn ownership"
- Version bumped to v116

**Console → Save Console to File... menu item**

Added "Save Console to File..." as the first item in the Console menu (above the
suppression options, separated by a divider). Opens a native save dialog defaulting
to `~/xgospel2_console_dump.txt`. Writes the full current console text and logs the
saved path in the console. Requires no new dependencies (`QFile`, `QTextStream`,
`QFileDialog`, `QDir` were already available).

**Expected log output for a successful scored game (v116)**

```
[BOT] Engine passed — prefetching kata-raw-nn ownership
[BOT] Scoring phase — requesting kata-raw-nn ownership
<<< [kata-raw-nn ownership: 361 values]
[BOT] Scheduling remove for N dead group(s) + done in 500ms
[BOT] >>> remove <coord>
[BOT] >>> done
```

---

## 2026-05-15 (v101): Bot mode — proactive ownership prefetch eliminates scoring race condition

**Root cause of v100 failure (game #64, got2go W vs Xiaowu B)**

Session log confirmed that `requestOwnership()` was triggered reactively at the scoring
trigger ("check your score"), but the opponent typed `done` before KataGo finished
computing. By the time the ownership response arrived, the game was over.

**Fix: proactive ownership prefetch during the pass sequence**

Ownership is now requested proactively in two places:
1. `onBotOpponentMove`: when the opponent plays `pass`, `requestOwnership()` is enqueued
   immediately after `play <color> pass` (before `requestGenmove`).
2. `onBotPassReady`: after the engine passes and sends `pass` to IGS, `requestOwnership()`
   is queued so that the position after both passes is analysed.

The result is stored in `bot_cached_ownership` (a `QVector<float>` member). When
`onBotOwnershipReady` fires with `!bot_scoring_pending`, it simply caches the 361 floats
instead of acting on them.

At the scoring trigger, if `bot_cached_ownership.size() == 361`:
- The cached vector is consumed immediately via `QTimer::singleShot(0, ...)`.
- `requestOwnership()` is skipped — no KataGo round-trip needed.
- Dead stone detection and `done` are sent before the opponent can type `done`.

If the cache is empty (edge case: ownership not yet computed), the system falls back to
requesting it fresh from KataGo as in v100.

`bot_cached_ownership` is cleared in `botEndGame()` alongside other bot state.

---

## 2026-05-15 (v100): Bot mode — dead stone detection via kata-analyze ownership

**Root cause analysis of game 397 (got2go vs chaopingli)**

Session log confirmed two problems with v99:
1. IGS sends "check your score" after 2 passes (not 3 as assumed). The 3rd pass is
   the human opponent's response during the scoring phase itself.
2. CMD22 is sent by IGS to observers only — the active bot player never receives it.
   The v99 dual-gate (final_score + CMD22) therefore never fired because bot_cmd22_ready
   was never set, so done was never sent.

**New approach: kata-analyze ownership**

Replaced CMD22-based dead stone detection with KataGo's own ownership analysis.
When the scoring trigger fires, `requestOwnership()` sends
`kata-analyze interval 0 ownership true maxmoves 1` to KataGo.
KataGo responds with a GTP block containing 361 ownership floats (row-major,
top→bottom, left→right; +1.0=Black owns, -1.0=White owns).

`onBotOwnershipReady` algorithm:
1. For each board point, check if an opponent stone sits there AND KataGo's
   ownership exceeds ±0.75 threshold in the bot's favour.
2. BFS flood-fill groups those candidates into connected groups.
3. A group is confirmed dead only if every stone in it is above threshold —
   this prevents false positives at group boundaries.
4. `remove <coord>` sent for each confirmed dead stone (both bot=Black and
   bot=White cases handled correctly via sign of ownership value).
5. `done` sent after all removes.

New `KataGoEngine::requestOwnership()` public slot and `ownershipReady(QVector<float>)`
signal added. `m_awaiting_ownership` flag routes the kata-analyze response through
a dedicated parser path in `handleResponse`. `BoardWindow::getStoneAt(x,y)` wrapper
added for board state access from the main window.
CMD22 gate and dual-gate infrastructure (v99) removed entirely.

---

## 2026-05-14 (v99): Bot mode — dual-gate safeguard for scoring race condition

**Race condition: final_score could respond before CMD 22 arrives**

`onBotScoreReady` (v98) read `territory_ownership` immediately when KataGo responded.
On a fast engine with a slow network, CMD 22 might not have arrived yet, causing
`remove`+`done` to fire with an empty ownership map and no dead stone removals.

Fix: two independent gates — `bot_score_ready` (set by `onBotScoreReady`) and
`bot_cmd22_ready` (set by `onBotCmd22Ready`, called at CMD 22 row 18). The shared
`botTryExecuteScoring()` helper only runs when both flags are true, so whichever
event arrives second triggers the actual `remove`+`done` sequence. The race
condition is eliminated regardless of engine speed or network latency.
Both flags and `bot_final_score_str` are cleared in `botEndGame`.

---

## 2026-05-14 (v98): Bot mode — dead stone removal and done in scoring phase

**Bug: Bot loses games it should win — dead opponent stones not removed in scoring**

After three consecutive passes (b-w-b or w-b-w), IGS enters scoring mode and sends
"check your score with the score command" followed by CMD 22 territory data. The bot was
passing correctly but never sending `remove <coord>` or `done` — so IGS waited indefinitely
with its default territory estimate, leaving dead opponent stones alive and producing the
wrong score.

Fix: when the scoring trigger line is detected for a bot game, `requestFinalScore()` is sent
to KataGo. In `onBotScoreReady`, the CMD 22 `territory_ownership` map (already populated by
the time `final_score` responds) is scanned for dead opponent stones via BFS flood-fill:
a connected group of opponent stones whose only non-dame/non-neutral neighbours are all
enemy territory cells (digit 4 = white territory, digit 5 = black territory) is considered
dead. Each dead stone position is sent as `remove <coord>` to IGS, then `done` is sent to
agree on the score.

`scoreReady` signal now connected in both warm and cold bot startup paths. New
`bot_scoring_pending` flag prevents duplicate scoring requests. Flag is cleared in
`botEndGame`. `BoardWindow::getTerritoryOwnership()` accessor added for dock-mode slot
territory data access.

---

## 2026-05-14 (v97): Bot mode — opponent rank shows "?" when challenger was not online at login

**Bug: Board shows "?" for opponent rank if challenger came online after initial userlist/who**

`getRankForPlayer` searches `players_model`, which is populated only from the `userlist`/`who`
commands sent at login. Players who connect later are never added to the model. In game 133
vs sams1883, they were not in the initial dump so the board showed "sams1883 ?".

Fix (two parts):
1. `botStartGame` now sends `stats <opponent>` immediately when a bot game starts. The stats
   response parser already extracts `stats_rank`; it now also calls
   `players_window->upsertPlayerRank(name, rank)`. `upsertPlayerRank` updates the existing
   model row if present, or inserts a stub row (name + rank only) if not.
2. After upserting, if the arriving rank is for the current bot opponent, the bot board's
   `white_rank`/`black_rank` is updated and `updateLabels()` is called — so the name panel
   refreshes live without waiting for a board reload. New `BoardWindow::setWhiteRank` /
   `setBlackRank` methods handle this.

---

## 2026-05-14 (v96): Bot mode — engine stops mid-game when UI routing loses target_board

**Bug: KataGo stops responding mid-game (e.g. move 40 vs river, game 122)**

The bot handler in the CMD 15 processing block was nested inside `if (target_board)`.
`target_board` is only non-null when `current_game_context == active_slot_game_id`. A
transient mismatch — caused by a concurrent UI update or CMD 7 broadcast that momentarily
reset `active_slot_game_id` — made `target_board` null for one move. That skipped the entire
block, including both board routing *and* the bot handler that sends `play <color> <vertex>`
and `genmove <color>` to KataGo. KataGo received no command and sat idle for the rest of the
game.

Fix: the bot handler is now unconditionally outside the `if (target_board)` block. The two
responsibilities are now independent: UI routing (target_board path) and engine feeding (bot
handler path). The bot handler fires whenever
`bot_mode_active && bot_game_id != -1 && current_game_context == bot_game_id`, regardless of
whether the board window is currently rendering the game.

---

## 2026-05-14 (v95): Bot mode — board title, greeting timing, say/tell console echo

**Bug: Board title shows "No game" during bot games**

In dock mode the board header displayed "No game" instead of "Game #N | Black/White to Play"
throughout the game. `updateLabels` in `board_window.cpp` only showed the game number when
`is_observing` was true; playing games set `is_playing` but not `is_observing`. Fix: changed
the condition from `if (is_observing)` to `if (is_observing || is_playing)`. The dock slot
already sets `is_playing=true` and `observed_game_id` correctly via `loadSlot`.

Also removed an erroneous `startObserving` call added in this session for dock mode — it
reset `is_playing=false` and conflicted with the slot system. Non-dock mode still uses
`startObserving` as before.

**Bug: Greeting "say Hello…" silently dropped by IGS**

The greeting was sent to the socket inside `botStartGame`, which is called from the first
CMD 15 line — before IGS sends `"9 Creating match [N]"`. The game is not yet open on the
server at that point, so IGS silently discards the `say`. Fix: greeting moved to
`onBotEngineReady`, which fires only after KataGo finishes initialising — always well after
the match is created. Confirmed delivered to opponents.

**Enhancement: outgoing say/tell echoed to console and board Comments**

Greeting and farewell are now echoed as `[BOT] >>> say:` / `[BOT] >>> tell:` in the output
console and via `processComment` into the board's Comments pane, so the operator can confirm
delivery without watching the raw socket.

---

## 2026-05-14 (v94): Bot mode — color/handicap wrong for nmatch nigiri and game ID reuse

**Bug 1: Nigiri ("N") color assigned wrong — bot waits when it should move**

When an opponent sends `nmatch woodnstone N 0 19 60 480 25 0 0 0`, the `N` means nigiri
(server assigns colors). The bot parsed `N` as White (the `else` branch when color ≠ "B").
If the server actually assigned woodnstone as Black (moves first), the bot sat waiting for
Black's first CMD 15 move that would never arrive — because the bot *was* Black.
The CMD 67 correction relied on `game_white/black_player_map` from CMD 7, but fresh nmatch
games arrive in CMD 67 before the next CMD 7 broadcast, so the maps were empty and the
correction was skipped. Both river games failed this way.

Fix: at the CMD 67 botStartGame trigger, read color directly from CMD 67's own `white_name`
/ `black_name` fields (always present). CMD 7 maps are now the fallback rather than primary
source. This reliably handles nigiri for both new and existing game IDs.

**Bug 2: `bot_handicap` picked up stale value from previous game in same game ID slot**

`botAcceptMatch` had no `handicap` parameter; `onBotEngineReady` fell back to
`bslot->handicap` when `bot_handicap == 0`. If a previous game occupied the same game ID
(e.g., fukuadmin vs madonna H:3 in game 55, then river's even game also assigned game 55),
`bslot->handicap` was `3` from the previous game. Bot called `set_free_handicap Q16 D4`
and then `genmove W` (White-in-handicap branch) on what was actually an even game.
IGS replied `"It is not your turn"`.

Fix: `botAcceptMatch` now takes an explicit `int handicap` parameter (from `parts[3]` of
the nmatch string). `bot_handicap` is stored at acceptance time. The `bslot->handicap`
fallback is removed entirely — the `"0(B): Handicap N"` CMD 15 event still overrides
`bot_handicap` for old-protocol matches where handicap is server-allocated.

**Bug 3: `bot_komi` overwritten with stale slot value — wrong komi sent to engine**

`onBotEngineReady` unconditionally read `bot_komi` from `bslot->komi`. If CMD 7 hadn't
updated the slot yet for the new game (same game-ID-reuse problem), the slot held the
previous game's komi. Confirmed: wireless even game got `komi=0.5` (from prior hc game
in slot 12) instead of `6.5`. Fix: only apply `bslot->komi` when `game_white/black_player_map`
confirm the slot belongs to the current game (opponent name matches). Otherwise keep
`bot_komi` from match-acceptance (default: 6.5 for even games, 0.5 for hc≥2).
The nmatch call site now also computes the correct komi default from the `hc` field.

**Bug 4 (latent): handicap-stone history replayed to engine — `? illegal move`**

In handicap games with a warm engine, IGS sends each handicap stone as a separate CMD 15
`"0(B): Q16"` move after game creation. These arrive after `bot_engine_ready = true` and
would be forwarded to KataGo as `play black Q16`, which KataGo rejects because those
vertices are already placed via `set_free_handicap`. Now that `bot_handicap` is correctly
set to `0` for even games, the guard `move.move_number == 0 && bot_handicap >= 2` added
in this session will only fire in actual handicap games, preventing the illegal-move error.

---

## 2026-05-14 (v93+): Bot mode — warm engine keep-alive + late-trigger for bot=Black hc>=2

**Warm engine keep-alive between games**

Previously KataGo was killed after every game and relaunched cold for the next one. On this
hardware, cold startup takes long enough that a 60-second main-time game could expire before
the engine was ready, resulting in `no-move` forfeit. Fix: `botEndGame` now sends `clear_board`
to the running process and leaves it alive instead of calling `detach()`. The second and
subsequent games use a warm path in `botStartGame`: it detects `engine->isReady()`, connects
`boardCleared → onBotEngineReady`, and queues `clear_board` — the ack fires in milliseconds
and the engine is playing within a second. Cold start is still used for the first game of a
session or if the engine process has died. Turning bot mode OFF while idle now also detaches
the warm engine.

**Bug: late-trigger missing for bot=Black hc>=2**

In a handicap game where the bot plays Black, White (the opponent) moves first. If White's
first regular move arrived via CMD 15 before KataGo finished loading (`bot_engine_ready`
still false), the trigger was silently dropped and the bot never responded. The late-trigger
scan in `onBotEngineReady` only covered the `bot=White even game` case. Fix: added the
symmetric scan for `bot_color == BLACK_STONE && bot_handicap >= 2` — walks `move_history`
for the last White regular move and calls `onBotOpponentMove` directly if found. Confirmed
root cause of the domigo (game 42) `no-move` loss.

---

## 2026-05-13 (v93): Bot mode — handicap-White genmove, toggle open lifecycle, no-greeting cleanup

**Bug: bot=White in handicap game never played move 1**

In a handicap game where the bot plays White, the 9 (or N) handicap stones are placed by
the server as move 0. White moves first after that — there is no Black regular move to wait
for. The prior code fell through to the even-game "wait for Black's CMD 15 move" branch,
scanned `move_history` for a Black regular move (`x >= 0`), found only handicap entries
(`x == -2`), and sat idle forever. Fix: added a new `bot_color == WHITE_STONE && bot_handicap >= 2`
branch in `onBotEngineReady` that calls `genmove W` immediately after `set_free_handicap`
is enqueued — no opponent move needed.

Turn-order table (all four cases now correct):

| bot_color | handicap | Who moves first | `onBotEngineReady` action |
|-----------|----------|-----------------|--------------------------|
| BLACK     | 0 or 1   | Bot (Black)     | `genmove B` immediately  |
| WHITE     | ≥ 2      | Bot (White)     | `genmove W` immediately  |
| WHITE     | 0 or 1   | Opponent (Black)| scan move_history; play if missed |
| BLACK     | ≥ 2      | Opponent (White)| wait for White's CMD 15 move |

**`toggle open` lifecycle corrections**

- On login: always send `toggle open true` so the account can receive challenges.
- Bot mode ON (`toggleBotMode(true)`): send `toggle open true` — advertise availability.
- Bot mode OFF (`toggleBotMode(false)`): send `toggle open false` (only if no game active).
- Game end (`botEndGame`): send `toggle open true` — reopen to new challengers.

**`bot_game_id` cleanup on `no-move` / `no-greeting` time loss**

IGS sends `"9 X lost the game N due to no-move."` or `"9 X lost the game N due to no-greeting."`
when a player times out without moving or greeting. These were not caught by the existing
result handlers, leaving `bot_game_id` stuck at the finished game's ID and causing all
subsequent match offers to be declined. Fix: new `QRegExp no_move_re` handler catches all
`"due to no-<word>"` variants and calls `untrackFinishedGame(game_id)` → `botEndGame()`.

---

## Session Summary — 2026-05-12–13 (v90–v92): Phase 2 Bot Mode — Full Stabilisation

This session completed Phase 2 (IGS bot mode with KataGo), resolving all live-testing bugs
discovered during games with BusyBee, got2go (6d), Pantherati, MA, and cli442542.

**GTP parser fixes (katago_engine.cpp)**

| Fix | Detail |
|-----|--------|
| KataGo two-phase `genmove` response | KataGo sends empty `=\n\n` (thinking ack) then `= vertex\n\n` (actual move) as two separate GTP responses. Parser was consuming the empty `=` as the final response, leaving `= vertex` orphaned with no pending command. Fix: when body is empty and pending command starts with `genmove`, don't clear `m_pending_cmd` — treat it as a thinking ack and keep waiting. |
| GTP split-packet buffering | `=\n` could arrive in one TCP read and `vertex\n\n` in the next. The `blank == -1` path was consuming the bare `=\n` line as a complete response. Fix: if a `=` or `?` line is seen but no `\n\n` terminator yet, break and wait for more data. |
| Block continuation body | When `=` and `vertex` appear on separate lines within the same `\n\n` block, the block parser now collects continuation lines and appends them to the response. |

**Bot mode logic fixes (xgospel2_fixed.cpp)**

| Fix | Detail |
|-----|--------|
| Illegal move on handicap hoshi (Q4) | IGS sends `0(B): Q4` as Black's first real move (not a handicap stone) when hc=0 or 1. Old code special-cased `move_number==0` as handicap placement. Fix: treat all opponent moves identically via `onBotOpponentMove`; `set_free_handicap` only sent for hc≥2. |
| `0(B): Handicap N` — bot never responded (White) | IGS sends `"0(B): Handicap 9"` as a special move; parser sets `x=-2`, `y=N`. `onBotEngineReady` read `bslot->handicap` which was 0 (CMD 7 snapshot predates game). Fix: CMD 15 handler captures `bot_handicap = move.y` when `move.x == -2` before engine is ready; `onBotEngineReady` prefers this value, sends `set_free_handicap`, then calls `requestGenmove(WHITE)`. |
| `bot_engine_ready` race | Opponent moves arriving before `set_free_handicap` was enqueued could trigger a premature `genmove`. Added `bot_engine_ready = false` in `botStartGame`, set to `true` at end of `onBotEngineReady`. All CMD 15 bot triggers gated on `bot_engine_ready` (except the Handicap N count capture which is always active). |
| Mid-game second match accepted | Bot auto-accepted a new nmatch while a game was in progress, stomping the active engine. Fix: both match and nmatch handlers check `bot_game_id != -1` → send `decline <opp>` instead. |
| Wrong turn order — even game bot=White | Bot triggered `genmove W` immediately after `set_free_handicap` before Black had moved. Fix: all cases except bot=Black+hc=0 now wait for opponent's first CMD 15 move. |
| Wrong turn order — handicap game bot=Black | Bot triggered `genmove B` immediately after engine ready, but Black moves last after handicap. Fix: bot=Black+hc≥2 waits for White's first move. |
| nmatch color field misinterpreted | `nmatch <opp> B 0 19 …` — `parts[2]` is the bot's requested color. Was inverted in error. Fixed: `B` → `BLACK_STONE` for the bot. |
| CMD7 color correction timing | `bot_color` was corrected after `botStartGame()` was called, so `onBotEngineReady` saw the wrong color. Fix: apply CMD7 `game_white_player_map`/`game_black_player_map` correction **before** calling `botStartGame()`. |
| Bot game board ranks showing `?` | `findPlayerRank` was a stub returning `?` always. Fixed: added `getRankForPlayer(name)` to `FixedPlayersWindow` (searches live `players_model` col 1=name, col 2=rank); `findPlayerRank` now delegates to it. |

**Bot mode UX additions**

| Feature | Detail |
|---------|--------|
| Auto-greeting | On game start: `say Hello <opponent>! Good luck and have fun!` |
| Auto-farewell | On game end: `tell <opponent> Thank you for the game <opponent>!` |

---

## Session Summary — 2026-05-11 (v88–v89): Dock Mode Play + Communications

This session completed Phase 1 (local KataGo engine play) and fixed a series of dock mode
bugs discovered during live IGS play testing with xgospel1 (BusyBee) as the opponent.

**Phase 1 — Local engine play (v88)**

| Fix | Detail |
|-----|--------|
| `saveGame()` guard removed | `if (!is_observing) return` silently skipped saves during local play; fixed with `QDir::mkpath()` and `"local"` id_str |
| SIGABRT on Score during ponder | `requestFinalScore()` now sends `stop\n` out-of-band when state==THINKING, sets `m_stop_for_score` flag, prepends `final_score` to queue; interrupted move response is swallowed |
| Illegal move recovery | `? illegal move` from KataGo now emits `illegalMove(vertex)` signal instead of `engineError`; `onEngineIllegalMove` calls `stepBackOneMove()` + re-enables board without closing it |
| Engine dropdown in LocalGameDialog | `QComboBox` populated from `EngineProfile` list, same as EvE dialog |
| `TCP_NODELAY` + flush on move send | `LowDelayOption` set in `onConnected()`; `socket->flush()` added after each move write in `sendMove()` |

**Dock mode play (v88–v89)**

| Fix | Detail |
|-----|--------|
| Board not populating in dock mode | CMD15 `we_are_playing` path only created boards in `board_windows`; added dock branch creating `GameSlot` with `is_playing=true`, appended to `game_slots`, calls `switchActiveGame()` |
| Auto-switch to playing game | `switchActiveGame(game_id)` called on match accept — dock focus and blue border switch automatically from observed game to playing game |
| Say/tell not appearing in board comment panel | CMD19 dock handler had `if (!is_say && ...)` — removed `!is_say` condition; both say and kibitz now route correctly with proper `is_kibitz` flag |
| False MOVE SKIPPED warning | Inactive slot moves handled by `applyMoveToSlotBoard` don't set `target_board`; added `dock_move_handled` flag to suppress spurious warning |
| Post-game tells routed to finished board (CMD24) | CMD24 dock handler lacked `!slot->game_finished` guard; after resign, BusyBee's tells were delivered to the dead game's comment panel instead of the player dialog |
| Post-game tells routed to finished board (`tells you:`) | Same `game_finished` guard added to `tells you:` dock routing path |
| Post-game says routed to finished board (CMD19) | Same guard added to CMD19 dock routing |
| Player dialog not opening for incoming tell | `broadcastIncomingTell` had `players_model->rowCount() > 0` gate that silently dropped tells when player list not yet loaded; now opens dialog unconditionally |
| `"1 8"` prompt lines in console | IGS CMD1 server prompt lines now suppressed when "Suppress Debug Output" (or "Suppress All") is checked |

---

## Session Summary — 2026-05-06 (v82–v87): Per-Slot Replay Architecture

This session replaced the flawed global `history_replay_game_id` pin architecture with a
per-slot state machine, eliminating all cross-game contamination in concurrent observations.

**Root cause of all prior scoring bugs** (v82–v86): The global pin could only protect one
game's replay at a time.  When a second game's `moves N` reply arrived while the first was
still replaying, CMD15 headers from the second game hijacked `current_game_context`, routing
history moves to the wrong slot.  v51 was immune because it uses a separate `BoardWindow`
(and separate memory pool) per game — no shared global state.

**New architecture**: Each `GameSlot` owns a `ReplayState` enum
(`WAITING_FOR_MOVES0` → `REPLAYING` → `LIVE`) and a permanent `catchup_high` watermark.
CMD15 headers always update `current_game_context` freely.  Each slot independently filters
its own moves with no global coordination needed.

| Version | Fix |
|---------|-----|
| v82 | Post-replay re-send filter used `history_replay_game_id == -1` — failed when a concurrent game held the pin; changed to `!= current_game_context` |
| v83 | `catchup_high` was cleared on first new live move — if a re-sent catch-up arrived after clearance it passed through unfiltered; removed clearance entirely |
| v83 | `observeGame(383)` overwrote pin while game 359 was mid-replay — introduced `pending_moves_requests` deferred queue to serialize `moves N` sends |
| v84 | Scoring-phase stone removals injected into game tree — pass threshold corrected from 2 to 3 (IGS requires w-b-w or b-w-b triple-pass before scoring) |
| v85 | Deferred queue drain reset `history_replay_catchup_high = -1`, releasing pin at move 0; fixed by restoring from `slot->catchup_high` |
| v86 | `W[kc]` placed on existing black stone during replay (game 253) — occupied-square band-aid identified as symptom of the fundamental global-pin architecture flaw |
| v87 | **Complete architectural rewrite**: removed `history_replay_game_id`, `history_replay_catchup_high`, `pending_moves_requests` globals; replaced with per-slot `ReplayState` machine in `GameSlot`; `mv_counter` removed from `GameSlot` (lives only in `BoardWindow`); stale `board_window.o` with dangling `slot->mv_counter` reference fixed; `loadSlot()` refresh added at `REPLAYING→LIVE` transition for slots switched-to before replay completes |

Verified: 5 of 5 scored games matched v51 reference exactly (move count + all scoring parameters).

---

## Session Summary — 2026-05-05 (v79–v81): TCP Fix + Catch-up Duplicate Filtering

This session found and fixed three further bugs in the inactive slot pipeline during
extended live testing against v51 on the t490s reference node.

| Version | Fix |
|---------|-----|
| v79 | Stop skipping first catch-up flood move when sending `moves N` — IGS `moves N` does **not** re-include the most recent live move, so it was silently dropped |
| v80 | Replace `socket->readLine(buf, bytesAvailable()+1)` with `socket->readLine()` (no size limit) — old form computed buffer size before TCP burst fully arrived, truncating lines mid-burst and silently dropping moves |
| v81 | Add per-slot `catchup_high` persisting after global pin release — IGS re-sends catch-up flood moves as "current position" after `moves N` finishes; they were being re-injected as live moves. Also fixed null `game_root` segfault in `getTotalMoves()` and initialized `game_root` in `observeGame()` for new slots. |

Key architectural insight: after the global `history_replay_game_id` pin clears, IGS
sends a re-injection of the catch-up move(s) as "live current position." A per-slot
`catchup_high` watermark persists past pin release and drops any move with
`move_number <= catchup_high` until a genuinely new move arrives.

Verified: games 267, 285, 399 (v78); games 472, 564 (v79); game 308 (v80 fix); games 321, 47 (v81 fix).

---

## Session Summary — 2026-05-04 (v76–v78): Game Tree Integrity for Observed Slots

This session continued live testing of the docked game selection pane and found three
further bugs in the catch-up flood / history-replay machinery for non-viewed slots.
All three involved spurious moves being injected into the inactive slot's game tree,
corrupting the SGF record and causing off-by-N capture counts.

| Version | Fix |
|---------|-----|
| v76 | Reset `white_captures`/`black_captures` on move-0 reset (catch-up flood captures persisted) |
| v77 | Set `catchup_high = first_catch_up_move.move_number` when sending `moves N` (single-flood pin released too early) |
| v78 | Guard inactive slot move routing on `!slot->game_finished` (scoring-phase stone-removal lines injected into game tree) |

Verified 3 of 3 scored games matching v51 reference at end of session.

---

## Session Summary — 2026-05-04 (v65–v75): Scoring Accuracy for Observed Slots

This session focused entirely on making territory and prisoner counts correct for
games that are **observed but not currently viewed** (non-active docked slots).
A t490s node running v51 served as the reference throughout.

The root cause took the full session to isolate.  Key breakthrough: comparing the
SGF game records from v74 and v51 for the same game showed them **byte-for-byte
identical**, proving board reconstruction was correct and the bug was purely in
the territory decode.  CMD22 digit 4/5 marks all territory including dead stone
positions; dead stones are territory AND prisoners simultaneously (Japanese rules).
The v58 fix had wrongly excluded dead stone positions from the territory count,
understating territory by exactly the dead stone count every time.

| Version | Fix |
|---------|-----|
| v65 | `history_replay_game_id` pin — CMD15 headers hijacking history move routing |
| v66–v70 | Pin release timing refinements (sequential move guard) |
| v71 | memset inactive slot board_state on observe (broke board state — reverted in v72) |
| v72 | Territory counting uses CMD22 data only, never board_state |
| v73 | Reset board_state/move_history when move_number==0 arrives (start of moves N reply) |
| v74 | Pin release keyed on `mv_counter > catchup_high` (catch-up flood high-water mark) |
| v75 | **Root fix**: removed wrong `dead_stone_positions` exclusion from territory loop |

Verified correct on games #113 and #64 (non-viewed scored results matching v51).

---

## Session Summary — 2026-05-03 (v56–v64): Docked Pane Stabilization

This session completed the integration of the SGF editor and stabilized the docked
game selection pane feature (introduced in v51–v55).  Nine successive fixes were
landed across one day of live testing:

| Version | Fix |
|---------|-----|
| v56 | Game tree built for all docked slots; SGF editor launched from finished/live games |
| v57 | Dangling `slot->game_root` pointer after `clearMoveHistoryBeforeMovesCommand()` |
| v58 | Dead stones double-counted as both territory and prisoners in inactive-slot CMD9 |
| v59 | Segfault closing active docked game (delete before `loadSlot` redirect) |
| v60 | Dame (green squares) missing on slot switch; required slider round-trip to appear |
| v61 | Ghost hover cursor and clock display lag on slot switch |
| v62 | CMD7 broadcast overwrote `slot->komi` after scoring, corrupting score comment |
| v63 | Score comment W/B totals recalculated incorrectly; now use pre-computed prisoners |
| v64 | Segfault closing the last remaining docked game (empty `QList::first()` guard) |

All six scored games verified correct in live testing by end of session.

---

## [v78] - 2026-05-04

### Fixed
- **Scoring-phase stone-removal lines injected into inactive slot game tree** — after a
  game finishes, IGS continues sending CMD15-format lines for stone removal during the
  territory marking / counting phase.  These look identical to regular move lines and were
  being parsed and appended to the inactive slot's `move_history` and game tree, producing
  spurious moves (e.g. `W[hb] B[ha] W[hd]…`) after the real game ended.  The active slot
  path was already protected via `BoardWindow::isFinished()`; the inactive slot path had
  no equivalent guard.

  Fix: added `else if (slot->game_finished) { /* drop */ }` branch in the inactive slot
  move-routing block.  All CMD15 move lines for a finished inactive slot are silently
  discarded.

## [v77] - 2026-05-04

### Fixed
- **Single catch-up flood move caused pin to release after move 0** — when only one
  catch-up flood move arrived before `moves N` was sent, `history_replay_catchup_high`
  was left at -1.  After the history replay processed move 0, `mv_counter=0 > -1` was
  true and the pin released immediately.  IGS then re-sent the catch-up move as the
  "current" live position, which was treated as a new live move and added to the game tree
  as a duplicate.

  Fix: when sending `moves N` on the first catch-up flood move, record that move's
  `move_number` into `history_replay_catchup_high` instead of leaving it at -1.

## [v76] - 2026-05-04

### Fixed
- **Spurious capture from catch-up flood phase persisted into scoring** — `white_captures`
  and `black_captures` were zeroed on board_state reset (move-0 boundary) but the move-0
  reset block did not reset these counters.  If the catch-up flood applied a move that
  captured a stone in the incomplete `board_state`, the capture count was 1 too high at
  scoring time.  Fix: added `slot->white_captures = 0; slot->black_captures = 0;` to the
  move-0 reset block alongside the existing `board_state` and `move_history` resets.

## [v75] - 2026-05-04

### Fixed
- **Territory undercounted for observed (non-viewed) slots** — the core scoring bug.
  For games that finished while not being viewed, White and Black territory were each
  short by exactly the number of dead stones in their territory.

  Root cause: the v58 fix added a guard skipping `dead_stone_positions` entries from
  the CMD22 territory loop, on the assumption those positions were already counted
  as prisoners and should not also be territory.  This was wrong.  IGS CMD22 digit 4
  marks a cell as white territory whether it is empty or occupied by a dead black stone;
  dead stones are simultaneously territory (the point scores after removal) and prisoners
  (the stone is captured).  Japanese rules count both independently.  The SGF `TW`/`TB`
  markers confirm this: they include dead stone positions in the territory count.

  Fix: remove the `dead_stone_positions.contains(pos) → continue` line from the
  territory loop.  All digit-4 entries count as white territory; all digit-5 entries
  count as black territory.  Prisoner counting from `dead_stone_positions` is unaffected.

## [v74] - 2026-05-04

### Fixed
- **Pin release too early for observed (non-viewed) slots** — `history_replay_game_id`
  pin was released after only 1–2 moves of history were processed, causing the rest of
  the `moves N` replay to be routed to the wrong game context.

  Root cause: the sequential-move guard (`move_number == move_history.size()`) fired
  immediately on move 1 after the move-0 reset, since that condition is always true for
  sequential moves.  Pin released, other games' CMD15 headers hijacked context, all
  remaining history moves skipped.

  Fix: track `history_replay_catchup_high` — the highest move number seen in the IGS
  catch-up flood before `moves N` response arrives.  Release the pin only when
  `slot->mv_counter > history_replay_catchup_high`, meaning the clean history replay
  has processed past all catch-up flood moves.

## [v73] - 2026-05-04

### Fixed
- **Catch-up flood moves accumulated in board_state before `moves N` arrived** — when
  observing a game mid-stream, the IGS server sends a few current moves immediately
  (catch-up flood) before the `moves N` full history.  Both were applied to
  `board_state`, leaving stale partial moves that corrupted board reconstruction.

  Fix: when `move_number == 0` arrives for a non-viewed slot that already has moves
  in `move_history`, reset `board_state`, `move_history`, and the game tree root before
  processing.  Move 0 is the unambiguous start of the `moves N` history response.

## [v72] - 2026-05-04

### Fixed
- **Reverted v71 memset** — clearing `board_state` on `observeGame()` was too early
  (before any catch-up flood moves arrived) and caused blank boards on resign results.
- **Territory counting no longer uses board_state** — uses CMD22 `territory_ownership`
  digit directly to determine dead stone color, removing dependency on potentially
  incomplete board reconstruction.

## [v71] - 2026-05-04 (reverted in v72)

### Attempted Fix (reverted)
- memset inactive slot board_state on observeGame() — fired too early, before catch-up
  flood; caused blank boards on resign results.

## [v70] - 2026-05-04

### Fixed
- **Pin release too early (take 2)** — `!slot->move_history.isEmpty()` guard fired on
  the second catch-up flood move (history size 1), releasing pin before `moves N` arrived.
  Changed guard to `move_number == slot->move_history.size()` (sequential continuity).

## [v69] - 2026-05-04

### Fixed
- **Wrong W/B totals in score comment for viewed (active) slot** — `updateGameResult()`
  was called before `enterScoringModeForResult()`, so `white_prisoners`/`black_prisoners`
  only had raw captures (no dead stones added yet) when the comment was built.

  Fix: call `enterScoringModeForResult()` first in both the docked active-slot path and
  the non-docked path, so `calculateScore()` runs and sets correct prisoner totals before
  `updateGameResult()` reads them.

## [v68] - 2026-05-04

### Added
- Diagnostic logging for CMD22 inactive slot decode: per-row data, d4/d5/dame counts,
  dead stone breakdown, and territory loop skip counts.  Aided root-cause analysis.

## [v67] - 2026-05-04

### Fixed
- **Pin release too early (take 1)** — pin cleared immediately on first move of `moves N`
  reply because `slot->move_history` was non-empty (catch-up flood move was in it).
  Added guard: only release when `slot->move_history` is non-empty AND move number is
  sequential with history depth.

## [v66] - 2026-05-04

### Fixed
- **Pin clear added to non-docked active board dispatch path** — the `if (target_board)`
  block now also releases `history_replay_game_id` when a live move is dispatched,
  so non-docked games don't hold a stale pin forever.

## [v65] - 2026-05-04

### Fixed
- **Incorrect scoring for games joined mid-stream** — games observed after they were
  already in progress (e.g. game 102 joined at move 271) showed wrong territory and
  dead stone counts because the full move history was never applied to `board_state`.

  Root cause: when `moves N` is sent to fetch history, the response is a stream of
  bare `15 move_number(color): coord` lines with no game-number prefix.  During that
  replay, any other active game's CMD15 header resets `current_game_context` to the
  other game, causing all subsequent history move lines to be routed to the wrong slot
  (or dropped as "MOVE SKIPPED").  The slot's `board_state` was therefore never
  populated, so CMD22 dead-stone detection found nothing — dead stones were not
  counted as prisoners and territory was wrong.

  Fix: introduce `history_replay_game_id`.  When `moves N` is sent, pin
  `current_game_context` by recording N.  In the CMD15 header parser, suppress
  context switches to other games while a replay is in progress; clear the pin when
  a CMD15 header for the replay game itself arrives (signalling live-move resumption).

## [v64] - 2026-05-03

### Fixed
- **Segfault closing the last docked game** — closing game 247 when it was the only
  slot triggered a Qt assert `"!isEmpty()"` in `QList::first()`.

  Root cause: the v59 close-path fix checked `!game_slots.isEmpty()` before calling
  `removeOne(slot)`, so the condition was always true as long as the closing slot
  itself was in the list.  After `removeOne`, the list was empty, and the immediately
  following `game_slots.first()` crashed.

  Fix: change the guard to `game_slots.size() > 1` — there must be at least one
  *other* slot remaining after removal before attempting to switch to it.

## [v63] - 2026-05-03

### Fixed
- **Wrong W/B totals in score comment for games with dead stones** — the comment area
  showed correct territory/komi numbers but wrong totals because `updateGameResult()`
  was re-counting dead stones from `dead_stones` and adding them to raw `white_captures`/
  `black_captures`.  For inactive slots `dead_stones` may not be fully synced at the
  moment `updateGameResult()` runs, so dead-stone prisoners were under-counted (e.g.
  12-point shortfall for game 303 with 14 dead stones).

  Fix: use the already-correct `white_prisoners`/`black_prisoners` members directly
  (set by CMD9 for inactive slots, or by `calculateScore()` for the active slot).
  Both already include dead stones as prisoners; the ad-hoc recount is no longer needed.

## [v62] - 2026-05-03

### Fixed
- **Wrong komi in score comment for finished docked games** — the comment area showed
  an incorrect White total (e.g. "W 53.5" instead of "W 65.5") for finished games
  because CMD7 game-info broadcasts kept arriving after scoring and overwrote
  `slot->komi` with the wrong value (-5.5 instead of 6.5 for game 183).  The
  `updateGameResult()` comment line then used that stale komi.

  The non-docked path already had a `!board->isFinished()` guard for this exact
  reason; the docked CMD7 path was missing the equivalent check.

  Fix: both docked CMD7 komi-update paths now skip finished slots
  (`slot->game_finished`), preserving the komi value that was correct at scoring time.

## [v61] - 2026-05-03

### Fixed
- **Ghost cursor on slot switch** — after switching to a different docked game, a
  phantom hover stone briefly appeared at the previous slot's last mouse position.
  `GoBoardWidget::hover_x/y` was never reset on slot switch, so the old position
  remained until the next `mouseMoveEvent`.  Fixed by calling `clearHover()` in
  `loadSlot()` immediately after `clearBoard()`.

- **Clock rendering lag on slot switch** — the clock display showed stale time for
  up to one full timer tick (≤1 s) after switching to a new slot.  Fixed by calling
  `updateClockDisplay()` immediately after connecting the new slot's timer, rather
  than waiting for the next tick to fire.

## [v60] - 2026-05-03

### Fixed
- **Dame (green squares) not visible on initial slot switch** — dame markers only
  appeared after moving the slider back and forth to the final position.

  Root cause: the inactive-slot CMD22 decode was storing only digits 4 (white
  territory) and 5 (black territory) in `slot->territory_map`, skipping digits 2/3
  (dame/neutral).  `loadSlot()` restores the overlay directly from `territory_map`,
  so dame points were missing on first display.  `navigateToNode()` reconstructs
  the overlay from the raw `territory_ownership` digit map (which does include 2/3),
  so dame appeared only after a slider round-trip triggered that path.

  Fix: also store digit 2/3 positions in `territory_map` as `EMPTY` during the
  inactive-slot CMD22 decode, matching the `receiveScoreEnd()` / `navigateToNode()`
  behavior for active slots.

## [v59] - 2026-05-03

### Fixed
- **Segfault when closing the active docked game** — closing game 585 (the currently
  active slot) crashed immediately after `setDeadStones` was called during the
  subsequent `loadSlot` for the next slot.

  Root cause: the same class of dangling-pointer bug as v57, but in the close path.
  `closeBoardWindow` was deleting the closing slot (`delete slot`) before calling
  `switchActiveGame`, which calls `loadSlot`.  `shared_board_window->game_root` still
  pointed into the just-freed slot's `GameNode` tree.  `loadSlot` immediately touches
  `game_root` via `getTotalMoves()` (the rebuild-if-empty guard), causing a use-after-free.

  Fix: reorder the close sequence — snapshot the closing slot first, remove it from
  the list, call `switchActiveGame` (which now redirects `game_root` to the new slot),
  and only then `delete` the old slot.  `active_slot_game_id` is set to `-1` before
  `switchActiveGame` so its early-out and redundant-snapshot guards behave correctly.

## [v58] - 2026-05-03

### Fixed
- **Double-counting dead stones in inactive-slot score display** — when CMD9 (counting
  result) arrived for a game in an inactive docked slot that had already received CMD22
  territory data, the client displayed an incorrect Black total (e.g. B=109.0 instead
  of the server-correct B=144.0).

  Root cause: the CMD22 territory map for inactive slots includes dead stone positions
  (marked by the server with the opponent's color).  The CMD9 score calculation loop
  counted all `territory_map` entries as territory, including those occupied by dead
  stones — and then counted the same dead stones a second time as prisoners via
  `dead_stone_positions`.  The 35-point discrepancy for game 143 matched exactly the
  34 dead white stones detected.

  Fix: in the CMD9 inactive-slot territory counting loop, skip any `territory_map`
  entry whose board cell is non-empty (`cell != EMPTY`).  Dead stones on the board
  are correctly handled as prisoners in the loop that follows; counting them as
  territory a second time was the error.

## [v57] - 2026-05-03

### Fixed
- **Dangling `slot->game_root` pointer causing game record / scoring corruption** —
  `clearMoveHistoryBeforeMovesCommand()` resets the game tree by deleting `game_root`
  and creating a fresh `GameNode`.  In docked mode `game_root` and `slot->game_root`
  are the same pointer, so the old slot pointer became dangling after the reset.
  Any subsequent `loadSlot()` call for that game would alias the freed memory and
  corrupt whatever had been allocated there, producing wrong moves, wrong territory,
  or a crash.

  Fix: both call sites of `clearMoveHistoryBeforeMovesCommand()` in docked mode now
  immediately re-sync `slot->game_root` and `slot->current_node` to the freshly
  created root, and clear `slot->move_history` to match.  `snapshotToSlot()` also
  now always writes `slot->game_root = game_root` (previously omitted under the
  assumption the pointer never changed — that assumption was wrong).

  Added `BoardWindow::getGameRoot()` accessor so `xgospel2_fixed.cpp` can read the
  private `game_root` field after the reset without coupling the two classes further.

## [v56] - 2026-05-03

### Fixed
- **Game tree not built for inactive docked slots** — the slider, navigation arrows, and
  "Edit Game" all lost the game record for any game that was not the active slot when
  its moves arrived.  Using the navigation slider on a switched-to game showed "0/0"
  and immediately lost all position history.

  Root cause: inactive slots route all incoming moves through `applyMoveToSlotBoard()`
  which updated the flat `slot->board_state` array (used for the thumbnail preview) but
  never built the `slot->game_root` `GameNode` tree.  That tree is what the slider,
  navigation, SGF export, and edit window all read.

  Fix (three layers, each handles a different scenario):

  1. **`applyMoveToSlotBoard()` now builds the game tree** — each call appends a new
     `GameNode` to `slot->current_node`, computing the `GoBoard` state with correct
     Go-rules capture removal.  The `GoBoard` class gained `countLiberties()` and
     `removeGroup()` methods so the logic is self-contained.  Live moves to inactive
     slots are now tracked in real time; no catch-up needed.

  2. **`loadSlot()` rebuilds on switch** — when switching to a slot whose tree is still
     empty (games observed before this fix, or any missed edge case), the new
     `rebuildGameTreeFromMoveHistory()` method replays `slot->move_history` — which is
     always complete — and builds the full tree before the UI is updated.

  3. **`editGame()` defensive rebuild** — same guard as above, catches any path that
     arrives at Edit Game with an empty tree.

- **Slider not at last move on slot switch** — `loadSlot()` now advances `current_node`
  and `current_move_index` to the end of the active variation whenever `auto_follow_mode`
  is true.  Previously `slot->current_move_index` could be stale (snapshotted before
  subsequent moves arrived while the slot was inactive), causing the slider to sit at an
  earlier position even though the board showed the final state.

- **SGF Editor: territory overlay replacing stone view** — `loadSGF()` now clears
  `is_scoring_mode`, the territory map, dead stones, and `territory_ownership` before
  loading the new tree.  The edit window always opens in normal stone-view mode;
  the territory overlay (from a scored source game) no longer bleeds through.

- **Edit Game button guard** — previously blocked when `is_observing == false`.  Now
  allows editing of finished games (`game_finished`) and own live games (`is_playing`)
  in addition to observed games.

## [v55] - 2026-05-03

### Fixed
- **Dead stone marking for inactive docked slots** — the scoring pipeline now works
  correctly for every game in the dock, whether or not it was the actively displayed
  game when scoring concluded.  Previously only the active slot received dead stone
  markers; all other slots showed them as alive, producing wrong territory (green dame
  squares scattered through opponent territory) and incorrect score totals.

  Two complementary sources now populate `slot->dead_stone_positions` before
  `calculateTerritoryForSlot()` runs:

  1. **IGS "is removing @ X Y" messages** — when a player marks a dead group during
     the scoring phase, the handler flood-fills the full connected group from the
     seed coordinate in `slot->board_state` and inserts every stone into
     `slot->dead_stone_positions`.  Previously only the single seed coordinate was
     stored.

  2. **Command 22 territory data** — IGS sends a 19×19 grid of territory ownership
     digits before the final counted result.  The inactive-slot CMD22 decoder now
     cross-references each grid point against `slot->board_state`: a stone sitting on
     a point the server classified as opponent territory is definitionally dead and is
     added to `slot->dead_stone_positions`.  This catches all dead groups reliably even
     when no "is removing @" messages were received (e.g. the game reached its result
     before the client started observing the scoring phase).

  When the CMD9 counted result arrives, if Command 22 territory is already present the
  territory map and dead stone set from CMD22 are used directly (no flood-fill needed);
  otherwise `calculateTerritoryForSlot()` falls back to a plain flood-fill of
  `slot->board_state` using whatever dead stone positions are available.

- **Removed incorrect algorithmic dead stone detection** — `calculateScore()` no
  longer calls `detectDeadStones()` as a fallback.  Dead stones are provided by the
  server (via "is removing @" and CMD22); client-side heuristic detection was
  unreliable and produced false results.  The `detectDeadStones()` function is
  retained only for the manual click-to-toggle path during user-driven re-scoring.

### Verified
- 6 of 6 scored results in live testing showed correct territory marking and dead
  stone removal across both active and inactive docked slots.

---

## [v52_R2-DOCKABLE-GAME-PANE] - 2026-05-02

### Changed
- **Game dock buttons** — stone indicators upgraded from plain filled circles to
  board-quality `StoneRenderer` pixmaps, matching the look of stones on the board.
  Button height auto-sizes to accommodate the larger icons.  Rank text colour changed
  from dark grey to white for improved legibility against the button background.

### Fixed
- **Teaching game title routing** — `"Game is titled:"` arrives via IGS Command 9
  before any Command 15 move sets `current_game_context`.  Added
  `most_recently_observed_game_id` as a one-shot fallback: set when `observeGame`
  creates the slot, cleared immediately after the title is consumed, so the title
  always lands on the correct slot regardless of observe order.
- **Teaching title label persistence** — switching away from a teaching game in
  docked mode left the title banner visible on subsequent games.  `loadSlot` now
  explicitly shows or hides `teaching_title_label` based on whether the incoming
  slot carries a non-empty `custom_game_title`.

## [v51_R2-DOCKABLE-GAME-PANE] - 2026-05-02

### Added
- **Dockable multi-game observation pane** — full implementation of the design documented in
  [`docs/DESIGN_dockable_game_pane.md`](docs/DESIGN_dockable_game_pane.md).

  When `use_docked_game_pane` is enabled in Preferences → Game Pane, a single shared
  `BoardWindow` is used for all observed games; a collapsible dock panel on the left edge
  shows one button per game.  Clicking a button switches the main board to that game
  instantly, with all state (position, clock, captures, observers, comments, game tree)
  restored exactly as if each game had its own window.

#### New files
- **`game_slot.h` / `game_slot.cpp`** — `GameSlot` QObject: owns all per-game state for one
  observed game (board array, move history, game tree, clocks, captures, observers, comments,
  scoring overlays, audit trail, clock QTimer).
- **`game_button_widget.h` / `game_button_widget.cpp`** — `GameButtonWidget`: one clickable
  button per game in the dock; shows player names and rank; hover after 500 ms pops up a
  miniature board preview (frameless QLabel pixmap); active game highlighted with blue border.
- **`game_selection_dock.h` / `game_selection_dock.cpp`** — `GameSelectionDock`: QDockWidget
  wrapper containing a scrollable VBox of `GameButtonWidget`s; manages add/remove/setActive.
- **`game_types.h`** — shared `StoneGroup`, `GameMode`, `GameMove` structs extracted from
  `board_window.h` to break the circular include chain
  `game_slot.h → igs_move_parser.h → board_window.h → game_slot.h`.
- **`check_braces.py`** — development utility: string-aware Python brace-depth checker,
  callable as `python3 check_braces.py <file> [start] [end]` to verify net brace balance
  over any line range during editing.

#### New settings keys
| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `use_docked_game_pane` | bool | false | Enable docked multi-game mode |
| `hover_board_size` | int | 200 | Hover popup board size in pixels (100–600) |
| `board_window_state` | bytes | — | `QMainWindow::saveState()` — dock position/float state |

#### Preferences dialog additions
- New **Game Pane** group under Application Settings:
  - Checkbox: "Use docked game selection pane"
  - Spin box: "Board preview size" (100–600 px, step 25)

#### `GoBoardWidget` additions
- `renderToPixmap(size, board_state[], board_size, last_x, last_y, title_overlay)` —
  off-screen render of any board position into a `QPixmap`; used for hover previews.
  Tiles the live board texture, draws grid/star-points/stones/last-move marker, and
  optionally overlays a teaching-game title bar at the top edge.

#### `BoardWindow` additions
- `loadSlot(GameSlot*)` — restores all UI state from a slot (scalars, board widget,
  overlays, clock, observers, comments, game tree strip).
- `snapshotToSlot(GameSlot*)` — full snapshot of live UI back to slot before a game switch.
- `snapshotBoardStateToSlot(GameSlot*)` — lightweight board-only snapshot after each live
  move (does not disconnect the clock timer).
- `renderSlotToPixmap(...)` — public forwarder to `GoBoardWidget::renderToPixmap` (board
  widget is private).
- `getGameSelectionDock()` — accessor for the dock owned by the board window.

#### `FixedXGospelWindow` additions
- `game_slots` (`QList<GameSlot*>`) — all active observed games in docked mode.
- `shared_board_window` (`BoardWindow*`) — single shared window in docked mode.
- `active_slot_game_id` (int) — game ID of currently displayed slot.
- `findSlot(int game_id)` — O(n) lookup by game ID.
- `switchActiveGame(int game_id)` — snapshot old slot, load new slot, update dock highlight.
- `applyMoveToSlotBoard(GameSlot*, GameMove)` — applies a stone placement + captures
  directly to an inactive slot's `board_state[][]` array (no BoardWindow involved).
- `updateHoverPixmapForSlot(GameSlot*)` — renders a `hover_board_size`-px pixmap from the
  slot's current board state and pushes it to the dock button.

#### Message routing (docked-mode branches throughout `onDataReceived`)
All `for (BoardWindow* board : board_windows)` dispatch loops gained a docked-mode branch
that routes incoming IGS messages to the correct `GameSlot` by game ID:
- Moves (live and history), passes, handicap stones
- Kibitz and say messages
- Time updates and byo-yomi
- Capture counts
- Dead stone marking (removestones protocol)
- Territory data rows
- Observer list updates
- Game result, resign, forfeit, adjournment
- Teaching game title (`Game is titled:`)
- Game setup (`GAMERPROPS`, komi, handicap, type)
- Game resume (Command 21)

For the active slot the message is forwarded to `shared_board_window`; for inactive slots
the slot's data fields are updated in memory only.

#### Hover pixmap lifecycle
1. **On first observe** — empty-board pixmap pushed to new dock button.
2. **Each live move, active slot** — `snapshotBoardStateToSlot` then `updateHoverPixmapForSlot`.
3. **Each live move, inactive slot** — `applyMoveToSlotBoard` then `updateHoverPixmapForSlot`.
4. **On game switch** — full `snapshotToSlot` of old slot then `updateHoverPixmapForSlot`.

#### Dock geometry persistence
`BoardWindow::closeEvent` saves `QMainWindow::saveState()` as hex via
`settings->saveByteArray("board_window_state", ...)`.  The constructor calls
`restoreState()` on startup, restoring dock side, width, and float state.

### Technical Details
- `game_types.h`: `StoneGroup`, `GameMode`, `GameMove` — breaks circular include chain
- `igs_move_parser.h`: changed `#include "board_window.h"` to `#include "game_types.h"`
- `board_window.h`: removed duplicate `StoneGroup`/`GameMode`/`GameMove` definitions;
  added `#include "game_types.h"`, dock accessors, slot load/snapshot/render methods
- `settings.h/cpp`: added `saveByteArray()` / `loadByteArray()` (hex-encoded `QByteArray`);
  `use_docked_game_pane`, `hover_board_size` keys already present from Phase 2
- `Makefile`: added `game_slot.o`, `game_button_widget.o`, `game_selection_dock.o`

### Notes
- Non-docked mode (default) is completely unchanged — all existing window-per-game
  behaviour is preserved; the `docked_pane_mode` flag gates every new code path.
- The `use_docked_game_pane` preference defaults to `false`; users must opt in.
- Dock panel supports `DockWidgetMovable | DockWidgetFloatable` — can be undocked to float.
- No limit on simultaneously observed games; the dock's `QScrollArea` handles any count.

---

## [v51_R1-COMPLEX-SCORING] - 2026-04-30

### Changed
- **Major version bump to v51** — SGF editor, game tree navigation, complex scoring engine,
  and disputed territory visualisation represent a significant milestone in development.

### Fixed
- **Update button segfault**: The SGF edit window's "Update" button crashed when the source
  live game window had been closed before Update was clicked. `source_board_window` was a
  dangling pointer — the null check did not catch it. Fix: connect `QObject::destroyed` on
  the source window to null out the pointer in the edit window before memory is freed.
- **Disputed marker colour**: Changed from grey to red for better visibility when comparing
  complex scoring output against q5Go side-by-side.
- **Info panel height truncation**: Removed hard `setMaximumHeight(360)` cap on the top-right
  info panel. Stats (Stones/Cap/Terr) and komi line were clipped on systems with larger fonts
  or different DPI. Panel now expands freely via the splitter; default initial size bumped
  from 340 to 400px.

---

## [v50_R21-COMPLEX-SCORING] - 2026-04-29

### Added
- **Complex scoring engine (calc_scoring_markers_complex port from q5Go)**
  - Full port of q5Go's `go_board::calc_scoring_markers_complex()` into `score_engine.cpp`
  - False-eye detection: iterative loop identifies groups touching exactly one candidate
    territory point with external liberties; those points are removed from territory and
    flagged as `falseeye`
  - Dead-stone propagation: territory containing dead stones is "real"; liveness propagates
    outward from real territory through bordering units and their adjacent empty points
  - Seki exclusion: groups with `m_seki` flag set contribute a `seki_neighbours` exclusion
    mask passed to `finish_scoring_markers`; seki points are not counted for either side
  - Benson's algorithm (`benson()` + `find_eas()`) fully ported but kept behind `#if 0`
    matching q5Go upstream — can be activated when needed
  - New `sc_mark` values: `seki`, `falseeye` — populated by complex path, empty in simple path

- **False-eye upstream bug fix (q5Go improvement)**
  - q5Go's original false-eye loop only sets `changed = true` when a territory unit becomes
    completely empty, leaving trimmed-but-non-empty units' removed points stranded in
    `cand_territory`.  Those stranded points survived to `finish_scoring_markers` uncategorised
    and appeared as disputed — the 1-point error visible in q5Go's complex scoring.
  - Fix: `changed` is set whenever *any* point is removed from any territory unit.
    `cand_territory` is fully rebuilt from actual unit contents after each pass.
    Result: no points can persist as phantom candidates between iterations.

- **Disputed-point visual marker**
  - `GoBoardWidget` gains `disputed_positions` member and `setDisputedPoints()` setter
  - `drawDisputedMarkers()`: hollow grey square (cell_size/3), 2px grey outline, no fill —
    visually distinct from filled territory boxes and dead-stone markers
  - Drawn in `paintEvent` after `drawDeadStoneMarkers`, only when `scoring_mode_enabled`
  - `exitScoringMode()` clears disputed points from the widget

- **Scoring method preference (Simple vs Complex)**
  - `ScoringMethod` enum in `score_engine.h`: `Simple` (default), `Complex`
  - `ScoreEngine::estimate()` extended with `disputed_out` (QSet) and `method` parameters
  - `settings.h`: `getScoringMethod()` / `setScoringMethod()` — key `scoring_method`
  - Preferences dialog: new "Scoring" group in Application Settings tab with combo box:
    "Simple (flood-fill, fast)" / "Complex (false-eye + seki detection)"
  - Setting persists across sessions; both methods can be compared by re-scoring same position

### Technical Details
- score_engine.h: `ScoringMethod` enum; `estimate()` signature extended
- score_engine.cpp: `stone_unit` gains `m_n_vital` (short) + `m_seki` (bool); `sc_mark`
  gains `seki` + `falseeye`; `bit_array` gains `subset_of()`; `ScoreBoard` gains
  `enclosed_area`, `find_eas()`, `benson()`, `calc_scoring_markers_complex()`;
  `finish_scoring_markers()` now accepts `const bit_array *do_not_count` (nullptr for simple)
- board_window.h: `disputed_positions` member; `setDisputedPoints()` inline setter;
  `drawDisputedMarkers()` declaration
- board_window.cpp: `drawDisputedMarkers()` implementation; all three `ScoreEngine::estimate()`
  call sites updated (markStoneAsDead, onScoreClicked, receiveScoreEnd path unchanged);
  method read from settings at each call
- preferences_dialog.h/cpp: `m_scoring_method_combo`; Scoring group UI; saved in onApply/onOk

### Notes
- Default scoring method remains Simple — no behavior change for existing users
- Complex method produces identical results to Simple for positions with no seki/false-eyes
- The Benson #if 0 gate matches q5Go upstream; activating it seeds m_seki for live-group
  detection but is not needed for the false-eye + propagation path currently in use

---

## [v50_R20-patch1-KOMI-GUARD] - 2026-04-29

### Fixed
- **CRITICAL: Komi corruption via IGS game ID reuse**
  - Root cause: IGS recycles game numbers. When a finished game's board window remained open,
    a new game assigned the same ID would send a Command 7 line with different players and komi.
    The Command 7 handler matched only on game ID and blindly overwrote the stale board's komi,
    handicap, and game type with values from the new game.
  - Discovery: Pro teaching game kansai1 vs kansai1 (game 167, komi 6.5) was observed and played
    to conclusion (W resigns). After the game ended IGS reused game ID 167 for a new game
    (RCC vs krka, komi -5.5). Periodic Command 7 updates then overwrote the open kansai1 board's
    komi to -5.5. When the SGF was saved it recorded `KM[-5.500000]` instead of `KM[6.500000]`,
    flipping the game result from W+2.5 to B+9.5. Confirmed via session log at line ~2310555:
    `"7 [167]         RCC [ 1d ] vs.        krka [ 1k ] (  2   19  0 -5.5  5  I) (  0)"`.
  - Fix: Command 7 board-update loop now guards on two conditions before applying any update:
    1. `!board->isFinished()` — finished/stale boards are never updated.
    2. Player name match — if neither white nor black from Command 7 matches the board's players,
       the game ID was reused; update is skipped and a diagnostic message is logged.
  - Location: xgospel2_fixed.cpp, Command 7 handler, board-update loop (~line 4873).
  - Impact: Protects komi, handicap, and game type from silent corruption for any game where
    the board window is kept open after the game ends (common during post-game analysis).

---

## [v50_R20-SGF-EDITOR] - 2026-04-27

### Added
- **SGF Editor: Full edit window launched from observed/played games**
  - "Edit Game" button opens a dedicated analysis board (separate BoardWindow in edit mode)
  - Inherits player names, game metadata, and full game tree from source window
  - Horizontal game tree navigation strip at bottom (stone icons, scrollable)
  - Active node highlighted in red; user-edited nodes drawn with blue border
  - Click any node in strip to jump directly to that position

- **SGF Editor: Edit Position mode (free stone placement)**
  - "Edit Position" button transforms panel from 4-button view to 5-button edit layout
  - Stones placed in alternating color with full capture rule enforcement
  - Ghost cursor (semi-transparent stone) follows mouse in edit mode
  - Red circle marks last move from unedited game record (preserved through edits)
  - Blue circle marks last stone placed in current edit session
  - Both markers visible simultaneously and survive clearBoard() re-renders

- **SGF Editor: Per-stone Undo in edit position mode**
  - "Undo" button (orange) appears only in edit position mode
  - Undoes one stone at a time back to the start of the edit session
  - Board state, blue marker, red marker, and next-player color all correctly restored
  - Stack cleared on entering/leaving edit position mode

- **SGF Editor: Append and Pass**
  - "Append" commits current scratchpad board state as a new edited GameNode
  - "Pass" creates a pass node (x=-1, y=-1) marked as edited
  - Edited nodes flagged with `isEdited()` for distinct rendering in tree strip

- **SGF Editor: Cancel Edit and Update**
  - "Cancel Edit" discards scratchpad, returns to view mode at current node
  - "Update" regenerates SGF from source board, re-parses, reloads tree in edit window

- **IGS game resume detection**
  - Parser now handles `21 {Game <id>: <white> vs <black> @ Move <move_num>}` resume notification
  - If a board window for that game exists and is marked finished (adjourned), it is reactivated and re-subscribed via `observe <id>`
  - If no window exists, a fresh observation is opened automatically
  - Handles game ID reassignment that IGS performs on resume

### Fixed
- **Say/Tell protocol revert after game ends**
  - Incoming Command 24 (`say`) messages were still routing to finished game board windows
  - Root cause: routing condition checked `isPlaying()` but not `isFinished()`
  - Fix: added `&& !board->isFinished()` guard — messages now fall through to player dialog `tell` channel as soon as game result is received, even with board window still open
  - Location: xgospel2_fixed.cpp Command 24 handler

- **Red circle (last game move marker) disappearing during edit**
  - `clearBoard()` was resetting `last_move_x/y = -1`
  - Fix: after every `clearBoard()` + re-render in edit mode, `setLastMove()` is called with `current_node` coordinates to restore red marker

- **Blue circle (last edit stone marker) lost after Undo**
  - Undo stack stored only `GoBoard` snapshots, not the previous blue marker position
  - Fix: stack entries now carry `{GoBoard, edit_x, edit_y}`; Undo restores both board and blue marker

### Technical Details
- board_window.h: Added `GameTreeStrip` class, `GoBoardWidget` hover/edit extensions, `BoardWindow` edit members
- board_window.cpp: `setupEditUI()`, `switchToEditPositionMode()`, `switchToViewMode()`, `updateGameTreeStrip()`, `onBoardClicked()`, `onAppendClicked()`, `onPassClicked()`, `onCancelEditClicked()`, `onUpdateClicked()`, `onUndoEditClicked()`
- game_tree.h/cpp: Added `m_edited` flag to `GameNode` with `isEdited()`/`setEdited()` accessors
- xgospel2_fixed.cpp: Command 21 resume handler; Command 24 `isFinished()` guard

### Notes
- Version updated from v50_R19 to v50_R20
- Score estimation (Benson's algorithm + flood-fill territory) deferred to Phase 2
- Variation support in game tree strip deferred to Phase 2
- All features from v50_R19 are included in this release

---

## [v50_R19-FILTER-PREFERENCES] - 2026-04-22

### Added
- **Feature: Player filter preference persistence**
  - All player list filters now save and restore across sessions
  - "Open" filter checkbox state persisted
  - "Hide Guests" filter checkbox state persisted
  - Rank range "From" and "To" selections persisted
  - Filter states loaded on startup and automatically applied
  - Preferences saved immediately when filters change
  - Settings keys: `players_filter_open`, `players_filter_hide_guests`, `players_filter_rank_from`, `players_filter_rank_to`
  - Default values: open=false, hide_guests=false, rank_from="BC", rank_to="9p"

### Fixed
- **Player dialog toggle flags resetting after refresh**
  - Fixed local player toggle flags (Looking, Open, Quiet, Shout) resetting to OFF/FALSE after players list refresh
  - Root cause: Players list refresh doesn't update dialog stats, stat field becomes stale
  - Solution: Auto-request fresh stats for local player if their dialog is open after player list refresh
  - Added parseToggleStatesFromStat() function to extract flags from IGS stat field
  - IGS stat field flags: '!' = Looking, 'X' = Not Open, 'Q' = Quiet, 'S' = Shout
  - parseToggleStatesFromStat() called in constructor before setupUI()
  - parseToggleStatesFromStat() called in updateStatsData() before rebuilding UI
  - Auto-stats request triggered at end of players list load if local dialog open
  - Added getLocalUsername() method to FixedPlayersWindow
  - Location: xgospel2_fixed.cpp:131-147, 530, 1573-1576, 4221-4235
  - Result: Toggle button states now refresh automatically and stay correct

- **CRITICAL: Filter conflict causing lost state after refresh**
  - Fixed three filters (open, rank range, hide guests) overriding each other
  - Root cause: Sequential filter application caused later filters to unconditionally override earlier ones
  - Old behavior: `applyRankRangeFilter()` would show all players in rank range, ignoring open filter
  - Problem: Filters were fighting each other instead of cooperating
  - New behavior: Unified `applyAllFilters()` function combines all conditions in single pass
  - A player is now hidden if ANY condition is true:
    - Open filter is on AND (player is playing OR has X mark)
    - Guest filter is on AND player is a guest
    - Player's rank is outside selected range
  - Grey formatting for X-marked players now applied in same pass as visibility logic
  - Result: All filters work together dynamically without conflicts or requiring refresh

### Changed
- **Filter application strategy**
  - Replaced three separate filter functions with unified `applyAllFilters()`
  - All checkbox/combo signals now call unified filter function
  - Filter preferences saved to settings when changed (using lambda captures)
  - Preferences loaded in constructor after setupUI()
  - Eliminated race conditions between filter types

### Technical Details
- xgospel2_fixed.cpp:723-738: Load filter preferences from settings in constructor
- xgospel2_fixed.cpp:871-873: Modified open filter checkbox to save preference on toggle
- xgospel2_fixed.cpp:876-879: Modified hide guests checkbox to save preference on toggle
- xgospel2_fixed.cpp:901-910: Modified rank combos to save preferences on change
- xgospel2_fixed.cpp:1225-1310: Unified `applyAllFilters()` function (replaces three separate functions)
- settings.h:35-40: Using `writeEntry()`, `readEntry()`, `writeBoolEntry()`, `readBoolEntry()`

### Notes
- Version updated from v50_R18 to v50_R19
- All features from v50_R18 are included in this release
- Filter preferences persist in `~/.config/xgospel2/xgospel2.conf`
- Setting checkbox states during preference load triggers save (harmless redundancy)

---

## [v50_R18-GITHUB-PREP] - 2026-04-18

### Changed
- **Code organization and attribution**
  - Renamed parser files from `q5go_*` prefix to `xgospel2_*` prefix
  - Files renamed: `q5go_parser.{h,cpp}` → `xgospel2_parser.{h,cpp}`
  - Files renamed: `q5go_games_parser.{h,cpp}` → `xgospel2_games_parser.{h,cpp}`
  - Added proper attribution headers crediting q5Go project (https://github.com/bernds/q5Go)
  - Updated all `#include` statements and Makefile references
  - Organized documentation files into `docs/` subdirectory
  - Organized debug binaries into `debug/` subdirectory
  - Created `cleanup_for_github.sh` script for release preparation

### Technical Details
- xgospel2_parser.h/cpp: Added attribution header, updated header guards
- xgospel2_games_parser.h/cpp: Added attribution header, updated header guards
- xgospel2_fixed.cpp:40-41: Updated include statements
- Makefile:23-24,126,129-130: Updated source file references and dependencies
- cleanup_for_github.sh: Bash script for automated cleanup (docs, debug, artifacts)

### Notes
- Version updated from v50_R17 to v50_R18 for GitHub release preparation
- All features from v50_R17 are included in this release
- Parser files remain functionally identical, only naming and attribution changed

---

## [v50_R17-KIBITZ-FIX] - 2026-04-17

### Fixed
- **CRITICAL: Kibitz command missing game number**
  - Fixed kibitz failing with "You need to specify a game #" error
  - Root cause: sendComment() was sending "kibitz <message>" without game number
  - Old command: `kibitz hello everyone`
  - New command: `kibitz 207 hello everyone` (includes game ID)
  - IGS requires format: "kibitz <game_number> <message>"
  - Location: xgospel2_fixed.cpp:5905
  - Result: Kibitz now works correctly when observing games

### Notes
- Version updated from v50_R16 to v50_R17 to reflect kibitz fix as standalone release
- All features from v50_R16 are included in this release

---

## [v50_R16-PLAYER-STATUS-SYNC-FIX] - 2026-04-14

### Added
- **Feature: "Suppress All Output" console option**
  - Added "Suppress All Output" checkbox in Console menu
  - Controls all three suppression options at once (Games, Moves, Debug)
  - Provides quick way to silence all console output with one click
  - Located at top of Console menu with separator for clarity
  - Location: xgospel2_fixed.cpp:2950-2987

### Fixed
- **CRITICAL: Player game status sync timing issue**
  - Fixed intermittent player status updates that required multiple refresh cycles
  - Root cause: Games list and players list load asynchronously in unpredictable order
  - Old behavior: syncAllPlayerStatuses() only called after games list completed
  - Problem: If games list completed before players list, status updates were lost
  - New behavior: syncAllPlayerStatuses() called after BOTH lists complete
  - Location: xgospel2_fixed.cpp:4053, 4107
  - Result: Player game statuses now update immediately on first refresh

- **CRITICAL: Observing status being wiped out**
  - Fixed ALL players' observing status showing "--" instead of game numbers
  - Root cause: resetAllPlayerGameStatuses() was wiping observing column data
  - Old behavior: Reset both "pl" (playing) AND "ob" (observing) columns to "--"
  - Problem: WHO/userlist provides observing data, but sync was erasing it
  - New behavior: Only reset "pl" column, preserve "ob" column from WHO/userlist
  - Location: xgospel2_fixed.cpp:1182-1189
  - Result: ALL players now show correct observing status (e.g., "mituo2100 observing 204")

### Technical Details
- xgospel2_fixed.cpp:4053, 4107: Added syncAllPlayerStatuses() call after players list completes
- This ensures status sync happens regardless of which list (games or players) completes first
- Eliminates race condition where player status would show "--" despite being in a game
- Example: Player "tomyaki" in game 300 now shows "300" immediately after refresh

- xgospel2_fixed.cpp:1182-1189: Modified resetAllPlayerGameStatuses() to preserve observing data
- Only resets "pl" (playing) column - the data we're syncing from games list
- Preserves "ob" (observing) column - the data that comes from WHO/userlist command
- WHO command format includes observing: "Q -- 204 mituo2100 13s 7k" = observing game 204
- userlist (cmd42) also provides obs_str field with observing game numbers

- xgospel2_fixed.cpp:2199-2220: Simplified syncAllPlayerStatuses() function
- Removed observed_game_ids and login_username parameters (no longer needed)
- Observing data now comes entirely from WHO/userlist parsing
- No need to separately track local player observing - WHO already provides it

### Testing
- Test that player game status appears immediately after first refresh
- Verify status updates work when games list completes before players list
- Verify status updates work when players list completes before games list
- **NEW:** Test that ALL players' observing status shows correctly (not just local player)
- **NEW:** Verify observing column shows game numbers like "204", "109", etc.
- **NEW:** Check that players observing multiple games show correct data

---

## [v50_R15X-PLAYER-FILTER-FIXES] - 2026-04-11

### Added
- **Feature: "Hide Guests" filter checkbox**
  - Added checkbox to players window to hide guest accounts (guestXXXX)
  - Filter persists across refreshes
  - Checkbox appears next to "open" filter in UI
  - Significantly reduces clutter by filtering out all guest accounts
  - Guest detection: case-insensitive check for names starting with "guest"

- **Feature: Player game status tracking**
  - Players now show which game they're playing in the "pl" (playing) column
  - Status automatically updates when games start or end
  - Synced from games list after each refresh
  - Example: Player "tomyaki" playing in game 245 shows "245" in pl column
  - All player statuses reset to "--" then updated from current games list

- **Feature: Game count indicator**
  - Games window title now shows count: "Games Online (345)"
  - Updates in real-time as games are added
  - Resets to (0) when games list is cleared
  - Easy comparison with other clients to verify parsing accuracy

### Fixed
- **CRITICAL: BC/NR player filtering bug**
  - Fixed regex filter that was excluding BC/NR players from players list
  - Old regex: `line.contains(QRegExp("[0-9]+[dkp*]"))` required numbers before rank suffix
  - New regex: Also accepts `BC` and `NR` without numbers
  - This was THE root cause of intermittent BC/NR player visibility
  - Location: xgospel2_fixed.cpp:3841

- **Guest filter persistence**
  - Fixed issue where guests reappeared after refresh despite checkbox being checked
  - Added safety check to skip filter during data population (while sorting disabled)
  - Filter now properly reapplies after players list completes loading

- **Games window refresh**
  - Fixed issue where games window stopped repainting after implementing player status sync
  - syncAllPlayerStatuses() was being called at wrong point in data flow
  - Now called inside games completion block after all games are loaded

### Changed
- **Filter application strategy**
  - All filters (open, hide guests, rank range) now reapply together after player status updates
  - Prevents filters from conflicting with each other
  - Single batch update instead of multiple individual updates improves performance

- **Player status update flow**
  - Added resetAllPlayerGameStatuses() to clear all pl/ob columns to "--" before sync
  - Added syncAllPlayerStatuses() to update from current games list
  - Added updatePlayerGameStatus() to update individual player status
  - Games window maintains reference to players window for status updates

### Technical Details
- xgospel2_fixed.cpp:3841: BC/NR regex filter fix
- xgospel2_fixed.cpp:706: Added hide_guests_checkbox member
- xgospel2_fixed.cpp:874-877: Hide guests checkbox UI initialization
- xgospel2_fixed.cpp:1687-1727: applyHideGuestsFilter() function
- xgospel2_fixed.cpp:1159-1195: updatePlayerGameStatus() function
- xgospel2_fixed.cpp:1197-1220: resetAllPlayerGameStatuses() function
- xgospel2_fixed.cpp:1919: Added players_window_ref to FixedGamesWindow
- xgospel2_fixed.cpp:2193-2196: setPlayersWindowRef() function
- xgospel2_fixed.cpp:2198-2219: syncAllPlayerStatuses() function
- xgospel2_fixed.cpp:2221-2224: updateGamesCount() function
- xgospel2_fixed.cpp:2147-2150: Update player status when games added to table
- xgospel2_fixed.cpp:4101-4102: Sync player statuses and update count after games list completes
- xgospel2_fixed.cpp:5415, 5665: Link games_window to players_window on creation

### Known Issues
- **Intermittent player game status updates** ✅ FIXED in v50_R16
  - ~~Some players may intermittently not show correct game status after refresh~~
  - ~~Example: Player tomyaki in game 300 may show "--" instead of "300"~~
  - ~~Root cause: Timing issue between games list and players list updates~~
  - **RESOLUTION:** Added syncAllPlayerStatuses() call after players list completes
  - See v50_R16 changelog for details

- **Intermittent BC/NR player visibility** (from v50_R15)
  - Some BC/NR players may intermittently not appear after auto-refresh
  - Root cause: Race condition in proxy model index mapping during filter application
  - Workaround: Click "Refresh Players" button or sort by different column
  - Proper fix planned: Refactor to use QSortFilterProxyModel::filterAcceptsRow()
  - Estimated effort: 2-3 hours

### Notes
- Session included accidental source restore from backup, requiring re-application of all fixes
- Created comprehensive reapply script: reapply_all_fixes.sh
- All features successfully restored and compiled
- Backup recommended: v50_R15X_player_filtering_fixes

---

## [v50_R15-PLAYERS-BC-FILTER] - 2026-04-10

### Added
- **Feature 35: BC/NR/Guest players now visible in players list**
  - Implemented hybrid userlist + who approach
  - userlist provides detailed info for ranked players
  - WHO supplements with BC/NR/guest players (limited info)
  - Duplicate players are automatically filtered out
  - BC players now appear with rank "BC" (Beginner Class)
  - Guest players now show all guests online (not just first one)
  - thirdstone and other BC players now consistently visible

- **Feature 35a: Rank range filter (q5Go style)**
  - Added "From" and "To" rank dropdown menus
  - Filter players by rank range (e.g., BC to 9p)
  - Default range: BC to 9p (shows all players from beginners to professionals)
  - Rank ranges work with professional (9p-1p), dan (10d-1d), kyu (1k-30k), and BC ranks
  - Numeric rank conversion for accurate filtering (Pro: 9p=900, Dan: 1d=10, Kyu: 1k=9, BC=-30)
  - Filters persist across player list refreshes
  - Filters automatically reapply when user changes sort order

### Changed
- **Players list population strategy**
  - Now uses BOTH userlist AND who commands
  - userlist executes first (detailed player data)
  - WHO executes as supplement (BC/NR/guests only)
  - Player count includes all players from both sources
  - No duplicate players shown (tracked via QSet)

- **Players list sorting behavior**
  - Sorting disabled during population for better performance
  - Sorting re-enabled and applied after all data loads
  - Default sort: by rank (strongest to weakest)
  - Sort indicator appears on Rank column header after loading
  - Filters reapply automatically when user sorts by different columns

### Fixed
- **WHO format parsing improvements**
  - Fixed column misalignment where player names appeared in Idle column
  - Correctly handle WHO format with optional number field (5, 6, or 7 parts)
  - 7-part format: [number] [flag1] [flag2] [flag3] [name] [idle] [rank]
  - 6-part format: [flag1] [flag2] [flag3] [name] [idle] [rank]
  - 5-part format: [flag2] [flag3] [name] [idle] [rank]
  - Filter out userlist format entries (8-10+ parts) from WHO parser
  - Filter out game list entries (cmd 7) with bracket notation

- **WHO command termination detection**
  - Fixed WHO terminator not being detected: "27  ******** N Players M Games ********"
  - Removed overly restrictive `!line.contains("27 ")` check
  - Added explicit terminator pattern: contains "Players" + "Games" + "**"
  - Now properly triggers completion, sorting, and filter application
  - Resolved intermittent "thirdstone missing" issue caused by incomplete loading

- **Filter persistence across operations**
  - Filters now reapply when user clicks column headers to sort
  - Connected to QHeaderView::sortIndicatorChanged signal
  - Both "open" filter and rank range filter reapply on sort changes
  - Prevents rows from disappearing when changing sort order
  - Default rank filter changed from "BC to 3p" to "BC to 9p" (shows all players)

### Known Issues
- **Intermittent BC/NR player visibility**
  - Some BC/NR players may intermittently not appear in the players list after auto-refresh
  - Root cause: Race condition in proxy model index mapping during filter application
  - Players ARE parsed and added to the model, but setRowHidden() may map to wrong proxy row
  - Workaround: Click "Refresh Players" button or sort by different column to make player appear
  - Proper fix planned for future release: Refactor to use QSortFilterProxyModel::filterAcceptsRow()
  - See "Future Improvements" section below for technical details

### Technical Details
- xgospel2_fixed.cpp:2418-2419: Added waiting_for_who_supplement and userlist_player_names state variables
- xgospel2_fixed.cpp:2614-2615: Initialize hybrid tracking in constructor
- xgospel2_fixed.cpp:703-706: Added from_rank_combo and to_rank_combo UI members
- xgospel2_fixed.cpp:858-896: Implemented rank filter UI with combo boxes
- xgospel2_fixed.cpp:1117-1124: Added reapplyRankRangeFilter() wrapper function
- xgospel2_fixed.cpp:1581-1627: Added applyRankRangeFilter() function
- xgospel2_fixed.cpp:1629-1655: Added getRankNumericValue() helper function
- xgospel2_fixed.cpp:3819-3846: Modified player parsing to track names and skip duplicates
- xgospel2_fixed.cpp:3891-3898: Fixed WHO terminator detection in end-of-list handler
- xgospel2_fixed.cpp:3911-3930: Modified completion handler to trigger WHO supplement
- xgospel2_fixed.cpp:1085-1087: Disable sorting during clearPlayers() for better performance
- xgospel2_fixed.cpp:1117-1122: Re-enable sorting in sortByRank() and force proxy model sort
- xgospel2_fixed.cpp:850-858: Connect to sortIndicatorChanged signal to reapply filters
- q5go_parser.cpp:443-448: Fixed WHO format prefix detection
- q5go_parser.cpp:467-468: Added upper bound check for WHO format
- q5go_parser.cpp:475-495: Fixed WHO field index calculation for 7-part format
- xgospel2_fixed.cpp:891: Changed default rank filter from "3p" to "9p"
- xgospel2_fixed.cpp:1607-1615: Added safety check to skip filter while sorting disabled

### Future Improvements

**Planned Fix for Intermittent Player Visibility Issue:**

The current implementation uses `QTreeView::setRowHidden()` with manual proxy model index mapping, which causes race conditions during sorting/filtering. The proper Qt approach is to override `QSortFilterProxyModel::filterAcceptsRow()`.

**Refactoring Effort Estimate: 2-3 hours**

**Required Changes:**

1. **Modify FixedRankSortProxyModel class** (xgospel2_fixed.cpp:52-67)
   - Add member variables to store filter state:
     - `bool open_filter_enabled`
     - `QString rank_from`, `QString rank_to`
   - Add public setter methods:
     - `setOpenFilterEnabled(bool enabled)`
     - `setRankRange(QString from, QString to)`
   - Override `filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent)`
     - Check column 0 (Stat) for "X" flag if open_filter_enabled
     - Check column 2 (Rank) against rank_from/rank_to range
     - Return true if row passes all active filters
     - Return false to hide row
   - Move getRankNumericValue() to proxy model class as helper

2. **Simplify FixedPlayersWindow filter functions** (xgospel2_fixed.cpp:1534-1684)
   - applyOpenFilter(): Just call proxy_model->setOpenFilterEnabled() + invalidateFilter()
   - applyRankRangeFilter(): Just call proxy_model->setRankRange() + invalidateFilter()
   - Remove all manual setRowHidden() calls
   - Remove proxy index mapping code
   - Let Qt's proxy model handle visibility automatically

3. **Benefits:**
   - Eliminates race conditions completely
   - Cleaner, more maintainable code
   - Better performance (Qt optimized)
   - Filters work correctly during all operations

4. **Testing:**
   - Verify open filter hides playing/X-marked players
   - Verify rank range filter works across full range (BC to 9p)
   - Test filter persistence across refresh
   - Test sorting with filters active
   - Verify no players disappear intermittently

---
