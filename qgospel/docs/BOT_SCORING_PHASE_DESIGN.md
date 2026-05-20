# XGospel2 Bot Mode — Scoring Phase Design

**Date:** 2026-05-16  
**Status:** Design / Pre-implementation (v109 is current)  
**Depends on:** Phase 2 bot mode complete (v94); scoring phase partially working (v109)

---

## 0. Purpose of This Document

Bot mode scoring has been partially implemented through v100–v109 via iterative experimentation.
Several approaches have been attempted and either failed or proven inadequate:

- `interval 100 + stop`: streaming output not captured by GTP parser (v103, reverted v106)
- `visits 50`: not supported by local KataGo build (v108, reverted v109)
- `interval 0` at 1–4 visits: insufficient for reading complex dead groups (ongoing)

This document consolidates what is known, documents the correct design for each element of
the scoring phase, and specifies the implementation path forward. No code changes should be
made without first reaching agreement on the design described here.

---

## 0.1 Game 583 Post-Mortem (got2go vs YUGO, 2026-05-16)

This game provided definitive diagnosis of the current failure modes.
4-stone handicap, got2go=White(bot), YUGO=Black. Actual result: W+25.5.

**What KataGo reported (visits=1, scoreMean=-25.029):** KataGo correctly knew White was
winning by ~25 points. But at visits=1, the per-intersection ownership values were too
uncertain to identify the large dead Black groups.

**Sequence of events (from session log `dump5_05-16-2026`):**

1. Move 260(B)=Pass: `onBotPassReady()` fires → `kata-analyze #1` sent (prefetch)
2. Move 261(W)=Pass (engine passed): `kata-analyze #2` sent by engine's `time_left` sequence
3. Scoring trigger arrives (`check your score`): cache is empty (prefetch not returned yet)
   → `kata-analyze #3` sent from scoring handler
4. Ownership intercepted: `= info move pass visits 1 ... scoreMean -25.029` with 361 values
5. Only **2 dead stone candidates** identified at threshold 0.50:
   - M15: dead Black stone (correct — opponent stone)
   - R8: White stone (bot's own stone!) — polarity bug or threshold hitting low-confidence cells
6. Bot sent: `remove M15`, `remove R8`, `done`
7. **Server replied `< 5 Sorry.` twice** — the `remove` commands were REJECTED
8. Server scored game without any removes: `CMD20: W:42.5 B:48.0 → B+5.5`
9. User manually clicked R8 and M15 on the board post-game → local score showed W+25.5
10. SGF was saved with RE[W+25.5] from the local calculation — correct but not matching server

**Why `remove` was rejected (`< 5 Sorry.`):**
The two consecutive passes triggered the scoring mode, but the bot sent `remove` commands
before the server fully entered scoring mode for the bot account. This is a race condition:
the scoring trigger message and the actual server-side state transition happen at slightly
different times. The server rejected the removes because it wasn't ready to process them,
or because the bot account had already been placed in a state where `done` was expected
without prior removes.

**Three confirmed bugs from this game:**

| # | Bug | Evidence |
|---|---|---|
| 1 | Double/triple `kata-analyze` sent simultaneously | Lines 239492, 239510 in session log |
| 2 | `remove` commands rejected by server (`< 5 Sorry.`) | Console dump lines 7065-7066 |
| 3 | Only 2 stones found at visits=1 — large dead groups missed entirely | `Dead stone candidates: 2` |

**Potential 4th bug:** R8 is a White stone (bot's own) but was sent as a `remove` target.
This indicates the ownership threshold at visits=1 was producing values above 0.50 for
White stones in uncertain positions — the filter should prevent this but may have been
overwhelmed by low-confidence neural net prior values.

---

## 1. Server Scoring Interaction (IGS Protocol)

### 1.1 How Scoring Is Triggered

IGS triggers scoring after **two consecutive passes** from both players.  
The third pass is NOT a scoring trigger — it is the human response *during* scoring
(some clients send a pass to acknowledge the score prompt, others do not require it).

The trigger message received by the active player (not observers) is:

```
9 {Game N: players...} has ended by mutual agreement.  
Please check your score with the score command, type 'done' when finished.
```

**Key detail:** Observers receive CMD22 (territory data) at this point; active players do NOT.
We cannot use CMD22 for bot scoring decisions.

### 1.2 Stone Removal Protocol

After the trigger, both players may send `remove <coord>` to mark dead stones.
Each `remove` command:

1. The sending client marks the stone visually.
2. The server echoes to both clients:  
   `15 <game_id> <player> is removing @ <coord>`  
   This is already parsed and `markStoneAsDead()` is called on the board (line 6047–6112).
3. A second player can dispute by sending `remove <coord>` back to un-remove a stone.
4. Opponent agrees silently by not disputing and eventually sending `done`.

**Timing requirement (confirmed from Game 583):** The `remove` commands must NOT be sent
immediately when the "check your score" line is received. The server needs time to process
the two-pass sequence and fully enter scoring mode. Sending `remove` too soon results in
`< 5 Sorry.` (command rejected). A small delay (e.g., 500ms) must be inserted before the
first `remove` command is sent. The `done` must come AFTER all `remove` commands, not
concurrently.

**Important:** `remove` marks the **connected group** at that coordinate as dead,
not just the single stone. The IGS `remove` command takes one coordinate as the seed
of the group.

### 1.3 The `done` Handshake

Both players must type `done` to conclude scoring. The sequence:

1. Bot sends `remove <coord>` for each dead group seed, then `done`.
2. Server broadcasts: `"X has typed done."` to both players.
   This message appears **twice** in the receive buffer (IGS echoes to all game participants).
   The two appearances are the same single `done` — not two separate players.
3. Human opponent reviews removed stones, then types `done`.
4. Server assigns the score and sends CMD20.

**Implication:** The bot should not interpret two occurrences of `"has typed done"` as
both players having accepted. We cannot track the done state by counting these messages.
The authoritative end-of-scoring signal is CMD20.

### 1.4 IGS Messages During Scoring Phase

| Message | Direction | Meaning |
|---|---|---|
| `9 {...} has ended... check your score` | Server → active player | Scoring trigger |
| CMD22 (territory row data) | Server → observers only | Territory — NOT sent to active player |
| `remove <coord>` | Client → server | Mark dead group |
| `15 <game> <player> is removing @ <coord>` | Server → all | Dead stone removal echo |
| `9 Removing @ <coord1> <coord2> ...` | Server → ? | Bulk removal (uncommon) |
| `<player> has typed done.` | Server → all | One player accepted score |
| `20 <wplayer> (W:O): <wscore> to <bplayer> (B:#): <bscore>` | Server → all | Authoritative final score |
| `9 {Game N: ... : W 95.5 B 116.0}` | Server → all | Counting result summary |

### 1.5 Current Implementation Status

- Scoring trigger detected at line 5693: **working**
- `enterScoringMode()` called on board: **working**
- CMD15 `is removing @` parsed → `markStoneAsDead()`: **working**
- CMD20 parsed → `setServerScore()` + `updateGameResult()`: **working** (v105)
- `done` sent after removes: **working**
- CMD9 counting result `{Game N: ... W score B score}` parsed: **working** (line 5739)
- Visual marking of removed stones on bot-mode board: **not implemented** (TODO at line 6043)

---

## 2. Engine-to-Client Handoff for Scoring

### 2.1 What the Engine Must Communicate

When the scoring trigger arrives, the bot needs to know:

1. **Which opponent stones are dead?** — derived from KataGo ownership analysis
2. **What coordinate does each dead group seed map to?** — internal (x,y) → IGS letter+row
3. **When to send `done`?** — immediately after all `remove` commands

The engine does not know about IGS; the client (`xgospel2_fixed.cpp`) drives the interaction.
The engine's role is to provide ownership data via `kata-analyze`.

### 2.2 Current Handoff: `kata-analyze interval 0`

**Signal path:**

```
[scoring trigger received]
    → requestOwnership()
    → KataGoEngine enqueues: "kata-analyze interval 0 ownership true maxmoves 1"
    → KataGo responds asynchronously: "= info move PASS ... ownership f1 f2 ... f361"
    → onStdoutReady intercept catches response (before normal GTP routing)
    → emit ownershipReady(QVector<float>)
    → onBotOwnershipReady() processes ownership, sends removes + done
```

**Why the intercept is in `onStdoutReady` (not `handleResponse`):**
`kata-analyze` is a streaming/async GTP command. Its response arrives as an unsolicited
`= info move ...` block while `m_pending_cmd` may hold a different command (e.g., `play black pass`).
Normal GTP dispatch would route it to the wrong handler or discard it.

**`m_awaiting_ownership` state machine:**

| State | Meaning |
|---|---|
| `false` | No ownership request in flight |
| `true` | `kata-analyze` has been sent; waiting for `= info move ... ownership ...` |

Cleared to `false` by:
- Successful intercept (ownership vector emitted)
- `?` error with `failCmd.startsWith("kata-analyze")` (v109 fix)

**Critical bug fixed in v109:** When `visits 50` (unsupported) caused a `?` error, the flag
was never cleared. All subsequent calls to `requestOwnership()` silently returned 0 values
(intercept triggered but no "ownership" keyword in `?` line).

### 2.3 Proactive Prefetch

During normal play, the engine prefetches ownership after every pass:

```
onBotPassReady() → requestOwnership()  [if !bot_scoring_pending]
    → bot_cached_ownership = ownership  [stored in onBotOwnershipReady when !bot_scoring_pending]
```

When the scoring trigger arrives:

```
if (bot_cached_ownership.size() == 361)  →  use cache directly (fast path)
else                                     →  request fresh kata-analyze
```

The cache is **invalidated** if the engine subsequently plays a real move
(`onBotMoveReady` clears `bot_cached_ownership`).

**Race condition handled:** If the scoring trigger arrives before the async `ownershipReady`
signal fires (opponent typed `done` first), `bot_scoring_pending` is set to `true` before the
signal arrives. `onBotOwnershipReady` checks `bot_scoring_pending` to determine whether the
incoming data is a prefetch or an active scoring result.

### 2.4 Known Limitations of `interval 0`

`interval 0` instructs KataGo to return immediately after the initial neural net evaluation,
without any MCTS tree search. In practice this produces 1–4 visits.

**Effect on ownership quality:**

| Position type | 1–4 visit ownership quality |
|---|---|
| Clear alive stones in settled positions | Accurate (≥0.9 confidence) |
| Simple dead groups (no eye, fully surrounded) | Usually accurate (0.6–0.8) |
| Complex dead groups (semeai, ko, under-the-stones) | Unreliable (0.4–0.6) |
| Large dead groups in handicap games | Frequently missed (0.3–0.5) |

The YUGO game (3-stone hc, W+16.5 actual): bot found only 3 dead stones, sent B+28.5.
This confirms 1–4 visit ownership is insufficient for serious handicap games.

### 2.5 Failed Alternatives (Do Not Retry)

**`kata-analyze interval 100 + stop` (v103, reverted v106):**
- KataGo sends streaming output during the 100ms window, not a proper GTP response block.
- The `stop` command ACK is `= \n\n`. The analysis lines arrive as unsolicited stdout
  after the ACK, not as part of any GTP response.
- The GTP parser discards them (they have no matching `m_pending_cmd`).
- This approach requires a fundamentally different parsing strategy.

**`kata-analyze interval 0 visits 50` (v108, reverted v109):**
- The local KataGo build does not support the `visits` parameter in `kata-analyze`.
- Returns: `? Could not parse analyze arguments or arguments out of range`
- The `visits` parameter may exist in newer KataGo versions but cannot be assumed.

### 2.6 Alternative Approaches to Explore

The following alternatives should be evaluated before choosing an implementation path:

#### Option A: `kata-analyze` with `pondering` budget (async, longer search)
Instead of `interval 0`, use a moderate interval (e.g., `interval 500`) and capture the
**last** ownership line emitted before a `stop` is sent. This requires:
- Tracking all `= info move ...` lines emitted while `m_awaiting_ownership = true`
- Overwriting the cached ownership each time a new line arrives
- Sending `stop` after a fixed elapsed time (e.g., 500ms via QTimer)
- The `stop` response (`= \n\n`) signals no more analysis lines will arrive
- After `stop` ACK, emit `ownershipReady` with the last-seen values

**Risk:** The analysis lines continue to arrive after `stop` is sent (before KataGo stops).
Must keep buffering until the `stop` ACK actually arrives.

**Advantage:** Works with the existing local KataGo build. More visits = better ownership.

#### Option B: `genmove_analyze` (single response, not streaming)
`genmove_analyze interval 0 ownership true` makes KataGo choose a move AND return ownership
in a single non-streaming response (`= <coord>\n\n`). The ownership is embedded in the
extended info line. This would be the most reliable approach but:
- KataGo will play a move during scoring phase, which we do not want.
- Would need to `undo` immediately, which is ugly.

**Not recommended.**

#### Option C: `analyze` (not `kata-analyze`) with ownership
Standard `analyze` does not return ownership. Ownership is a kata-specific extension.
**Not applicable.**

#### Option D: Board-based dead stone inference (no KataGo)
Use classical Go theory: a group is dead if it cannot form two eyes.
Steps:
1. Flood-fill all opponent groups.
2. For each group, check if it has ≥2 internal liberties surrounded entirely by bot territory.
3. If not, mark as dead.

**Advantage:** No KataGo involvement; fast; deterministic.  
**Disadvantage:** Requires implementing Go endgame reading (complex, error-prone).
Does not handle semeai, ko threats, or complex positions correctly.

#### Option E: Increase `maxVisits` in KataGo config (not `kata-analyze` visits param)
The KataGo config file supports `maxVisits` which caps tree search during normal play.
Setting `maxVisits` high (e.g., 200) and using `kata-analyze interval 0` would give KataGo
more time to search before the neural net evaluation is complete.

**Important:** This is NOT the same as the `visits` parameter in the GTP command.
`interval 0` means "return immediately after the first batch"; `maxVisits` in config
determines how many visits are in that batch before interval 0 fires.

**Investigation needed:** Confirm whether `numAnalysisThreads` and `maxVisits` in
`default_gtp.cfg` affect the number of visits returned by `interval 0`.

#### Option F: Use `kata-analyze` with moderate interval, parse incrementally
Same as Option A but specifically: send `kata-analyze interval 500 ownership true maxmoves 1`,
then after 1–2 seconds send `stop`. The intercept in `onStdoutReady` updates the cached
ownership vector each time it sees `= info move ... ownership`. After `stop` ACK, emit
the most recently cached vector.

**This is the recommended approach.** See Section 4 for implementation design.

---

## 3. Dead Stone Identification and Marking Algorithm

### 3.1 Coordinate Systems

Three coordinate systems are in use:

| System | Format | Origin | Notes |
|---|---|---|---|
| Internal (x, y) | integer pair | (0,0) = top-left | Used by board_state, getStoneAt |
| KataGo ownership | array index | row 0 = top (y=0), col 0 = left (x=0) | index = y*19 + x |
| IGS coord | Letter + number | A1 = bottom-left | Skip letter I; row = 19 - y |

**Conversion internal → IGS:**
```cpp
char letter = (x < 8) ? ('A' + x) : ('A' + x + 1); // skip I
int igs_row = 19 - y;
QString coord = QString("%1%2").arg(letter).arg(igs_row);
```

**Conversion IGS → internal:**
```cpp
int x = (col >= 'I') ? (col - 'A' - 1) : (col - 'A'); // skip I
int y = 19 - igs_row;
```

### 3.2 Ownership Semantics

KataGo ownership values (per-intersection float):

- `+1.0` = Black owns this intersection (Black stone or Black territory)
- `-1.0` = White owns this intersection (White stone or White territory)
- Values near 0 = contested / unclear

**Dead stone identification:**
- Bot is Black: dead White stone at (x,y) if `ownership[y*19+x] > +THRESHOLD`
- Bot is White: dead Black stone at (x,y) if `ownership[y*19+x] < -THRESHOLD`

Current threshold: **0.50** (lowered from 0.75 in v104).

### 3.3 Current Algorithm: Majority-Vote BFS

**Steps:**
1. Scan all intersections for opponent stones where ownership exceeds threshold.
   Call these `dead_candidates`.
2. For each unvisited seed in `dead_candidates`:
   - BFS-expand to find the full connected group of opponent stones (not just candidates).
   - Count how many stones in the full group are also in `dead_candidates`.
   - If ≥60% of the full group is above threshold → mark entire group as dead.
3. For each confirmed dead group:
   - Pick one stone from the group as the seed coordinate.
   - Send `remove <IGS_coord>` (IGS removes the connected group from that seed).
4. Send `done`.

**Why BFS over full group, not just candidates:**
Large dead groups in complex positions have edge stones near contested areas that score
slightly below the threshold (e.g., 0.40–0.49). If we only BFS over candidates, the group
is split into disconnected fragments and each small piece is classified separately. The
majority-vote approach handles this by expanding to the full group first, then asking
what fraction is above threshold.

**Current threshold tuning:**
- 0.50 is aggressive (may catch some alive stones near disputed areas)
- 0.60 is conservative (misses large dead groups at 1–4 visits)
- At ≥50 visits, 0.75 would be appropriate

### 3.4 Known Algorithm Failures

**Game 583 — got2go vs YUGO (4-stone hc, v109, 2026-05-16):**
- bot=White, 4-stone handicap, actual result W+25.5
- KataGo at visits=1, scoreMean=-25 (correctly predicted W+25)
- Only 2 dead stone candidates found at threshold 0.50
- M15 (dead Black stone): correctly identified
- R8 (White stone — bot's OWN stone): incorrectly identified as dead candidate
- Both `remove` commands rejected by server with `< 5 Sorry.` — removes sent too soon
- Game scored without removes: CMD20 → B+5.5
- User manually clicked board post-game → local score W+25.5 (matches actual result)
- SGF RE[W+25.5] saved from local calculation; server recorded B+5.5

**Earlier YUGO game (3-stone hc, v109):**
- Board: W has large dead group in lower right (actual W+16.5)
- KataGo at 1–4 visits: ownership of dead White stones ranged 0.3–0.5
- Threshold 0.50: only 3 stones exceeded threshold (bottom of group)
- BFS found group of 3 (not connected to larger group because seeds were isolated)
- Majority vote: 3/3 = 100% → sent `remove` for 3 stones only
- Remaining 7+ stones not removed → server counted them as alive → B+28.5 recorded

**Root cause (both games):** At visits=1, KataGo ownership is based on neural net prior alone
before any MCTS search has propagated group life/death up the tree. KataGo "knows" the correct
result (scoreMean matches) but per-intersection ownership values at visits=1 are too uncertain
to reliably identify which specific groups are dead.

### 3.5 Improved Algorithm Design

The algorithm improvement depends on which ownership-quality solution is chosen (Section 2.6).
With more visits (Option A or F), threshold can be raised and the algorithm becomes reliable.
With 1–4 visits, a secondary heuristic is needed.

**Proposed improvement for 1–4 visits (fallback path):**

After BFS majority-vote fails to confirm a group, apply a secondary liberty-count heuristic:
1. Find all opponent groups NOT confirmed dead by ownership analysis.
2. For each such group, flood-fill to count distinct eye-spaces (connected empty intersections
   entirely enclosed by that group's stones + board edge).
3. If the group has < 2 eye-spaces, flag for human review rather than auto-removing.
   (Do not auto-remove without ownership confirmation.)

**This heuristic prevents false positives** (removing alive stones) while potentially
providing guidance. The safest behavior when uncertain is to send `done` without removing
and let the human opponent handle the correction.

**Proposed improvement with ≥50 visits:**

At ≥50 visits, ownership is stable and reliable. Use:
- Threshold: 0.65 (tighter than current 0.50)
- BFS majority-vote: ≥70% of group above threshold
- This prevents false positives on alive groups near disputed territory

### 3.6 `remove` Command Semantics

The bot sends **one** `remove <coord>` per dead group, using any stone in the group as the
seed. IGS expands the group automatically from that seed. Do NOT send one `remove` per stone.

Current implementation sends one `remove` per confirmed dead stone (line 8404–8414).
This is **incorrect** if multiple stones in the same group are in `confirmed_dead` — it sends
redundant `remove` commands for the same group. While IGS likely handles this gracefully
(un-remove then re-remove), it is wasteful and potentially fragile.

**Fix needed:** Track which stones have already been "covered" by a sent `remove` command
and skip redundant sends. One approach: after sending `remove` for seed (x,y), immediately
flood-fill the group and mark all its stones as "done" so BFS seeds from that group are skipped.

### 3.7 Visual Dead Stone Marking on Bot Board

When the bot sends `remove <coord>`, the server echoes:
`15 <game_id> <player> is removing @ <coord>`

This CMD15 message is already parsed (line 6047) and calls `markStoneAsDead(dead_col, dead_row)`.
So the bot board WILL visually mark stones as dead when CMD15 arrives, assuming:
- The board window is the active slot's board (docked mode).
- `markStoneAsDead` is connected to the correct board.

**The cosmetic issue (stones not visually marked)** observed in testing may be caused by:
1. CMD15 arriving before the board is in scoring mode (race condition).
2. `enterScoringMode()` not called on `engine_board` specifically (only `shared_board_window`).
3. CMD15 being routed to the wrong board in docked mode.

This is low-priority but should be investigated separately.

---

## 4. Recommended Implementation Plan

### Phase A0: Fix `remove` Timing (Critical — confirmed from Game 583)

**File:** `xgospel2_fixed.cpp`, `onBotOwnershipReady()`

The `remove` commands must be delayed after the scoring trigger arrives. The server needs
time to transition into scoring mode before it will accept `remove` commands. Sending
immediately results in `< 5 Sorry.` rejections.

**Fix:** After computing `confirmed_dead`, do NOT send `remove` commands immediately.
Instead use `QTimer::singleShot(500, ...)` to send the first `remove` command 500ms after
ownership processing completes. Chain subsequent removes with small delays (50–100ms each)
via a recursive QTimer or a QList queue, then send `done` after the last remove + another
200ms delay.

Alternatively: Use a single 500ms delay before sending ALL removes in one burst, then
`done` immediately after the burst. The server likely processes them sequentially.

**Also fix:** Double `kata-analyze` send. When `onBotPassReady` fires and prefetch is
already in flight (`m_awaiting_ownership == true`), skip the prefetch request. Similarly,
when the scoring trigger fires and `m_awaiting_ownership` is already true from the prefetch,
set `bot_scoring_pending = true` and let the in-flight response be consumed as the scoring
result (not a prefetch). This removes the race entirely.

### Phase A: Fix `remove` Redundancy (Quick fix, no new GTP commands)

**File:** `xgospel2_fixed.cpp`, `onBotOwnershipReady()`

After BFS produces `confirmed_dead`, restructure the send loop:
- Use a `QSet<QPair<int,int>> seeds_sent` to track which groups have had their seed sent.
- For each stone in `confirmed_dead`, check if any stone in its BFS group is already in
  `seeds_sent`. If so, skip. Otherwise, send `remove` for this stone and add all stones
  in its group to `seeds_sent`.

This is a correctness fix regardless of which quality improvement is chosen.

### Phase B: `kata-analyze` with Moderate Interval + Stop (Option F)

**Goal:** Get ≥50 visits of ownership data using the existing local KataGo build.

**New flow:**
1. `requestOwnership()` sends: `kata-analyze interval 500 ownership true maxmoves 1`
2. `onStdoutReady` intercept: for every `= info move ... ownership ...` line while
   `m_awaiting_ownership`, update `m_latest_ownership` (QVector<float>) in place.
3. After 1000ms (QTimer), send `stop` to KataGo.
4. `stop` ACK (`= \n\n`) recognized in normal GTP dispatch.
   Add a check: if `m_awaiting_ownership && last_cmd_was_stop`, emit `ownershipReady(m_latest_ownership)`.

**Changes to `katago_engine.cpp`:**
- Add member `QVector<float> m_latest_ownership;`
- Add member `QTimer *m_ownership_stop_timer;`
- `requestOwnership()`: start timer, send `kata-analyze interval 500 ownership true maxmoves 1`
- `onStdoutReady` intercept: update `m_latest_ownership` on each ownership line; do NOT emit yet.
- Stop timer slot: send `stop` command.
- `stop` ACK handler: emit `ownershipReady(m_latest_ownership)`, clear `m_awaiting_ownership`.

**Risk and mitigation:**
- KataGo may send more ownership lines after `stop` arrives at the engine.
  Once `stop` ACK is received, ignore any further `= info move ...` lines that arrive.
- If `stop` is sent and KataGo never responds with `= \n\n`, add a fallback timeout
  (2000ms after stop) that emits whatever is in `m_latest_ownership`.

### Phase C: Threshold Tuning with More Visits

With ≥50 visits:
- Raise `DEAD_STONE_THRESHOLD` to 0.65
- Raise majority-vote requirement to 70%
- Test with YUGO-style handicap games

### Phase D: CMD15 Routing Verification

Verify that dead stone removal echo (CMD15) reaches the bot-mode board correctly.
Add debug logging to confirm `markStoneAsDead` is called with correct coordinates
during a live bot-mode scoring phase.

---

## 5. State Machine Summary

### Bot scoring phase state

```
[Normal play]
    onBotPassReady() → requestOwnership() [prefetch] → bot_cached_ownership set

[Scoring trigger arrives: "check your score"]
    bot_scoring_pending = true
    if bot_cached_ownership.size()==361:
        QTimer::singleShot(0) → onBotOwnershipReady(cached)  [fast path]
    else:
        requestOwnership() → emit ownershipReady → onBotOwnershipReady(fresh)

[onBotOwnershipReady]
    bot_scoring_pending = false
    if ownership.size() != 361: send "done" → DONE
    scan board for dead_candidates (opponent stones above threshold)
    BFS expand each seed to full group
    majority-vote: ≥60% above threshold → confirmed_dead
    for each confirmed dead group: send "remove <seed_coord>"
    send "done"

[CMD15 arrives: "player is removing @ coord"]
    markStoneAsDead(coord) on board

[CMD20 arrives: authoritative score]
    setServerScore() + updateGameResult() on board
    botEndGame()
```

### `m_awaiting_ownership` state machine

```
false → [requestOwnership() called] → true
true  → [= info move ... ownership ... received] → false, emit ownershipReady(vec)
true  → [? error for kata-analyze] → false, emit ownershipReady(empty)  [v109 fix]
```

---

## 6. Test Plan

### Test 1: Mock game with equal-skill opponent, no dead stones
- Both sides pass, scoring triggered
- Verify: `done` sent without any `remove` commands
- Verify: CMD20 received and score displayed on board

### Test 2: Mock game with obvious dead group (small eyeless chain)
- Set up position where engine has clearly dead 3-stone chain in opponent territory
- Verify: correct group identified, one `remove <coord>` sent, `done` sent
- Verify: CMD15 arrives and marks stones on board

### Test 3: Mock game with large dead group (YUGO-style)
- Handicap game, large White dead group in lower-right corner
- Verify: after Phase B implementation, all stones in group identified
- Compare: old (interval 0, 1-4 visits) vs new (interval 500 + stop, ≥50 visits)

### Test 4: Opponent disputes a removal
- Opponent sends `remove` for a stone the bot marked as dead
- Verify: bot handles dispute correctly (currently no dispute-handling code)
- Note: this is a deferred requirement; current bot does not dispute or re-remove

### Test 5: Ownership prefetch race condition
- Engine passes, prefetch queued
- Scoring trigger arrives before prefetch response
- Verify: `bot_scoring_pending` set before `ownershipReady` fires → ownership used for scoring

---

## 7. Deferred / Out of Scope

- **Dispute handling:** If opponent un-removes a stone the bot identified as dead, the bot
  currently does nothing. Correct behavior would be to either accept the dispute (play `done`
  anyway) or escalate to human intervention. Deferred.

- **9x9 / 13x13 board support:** All coordinate math assumes 19×19. Deferred.

- **SGF recording of dead stones:** Removed stones should be recorded in the SGF file.
  Currently only the score result is stored. Deferred.

- **Phase 3 Analysis/Hints:** Uses similar KataGo infrastructure but is a separate feature.

---

*End of design document.*
