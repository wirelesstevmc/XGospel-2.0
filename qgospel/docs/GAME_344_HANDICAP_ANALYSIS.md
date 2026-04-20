# Game 344 Handicap Game Analysis

## Date: 2025-12-24

## Test Results

**Game:** PandaBot4 [1p] vs DJzz [2d] - 7 Handicap - B+39.5

- **q5Go:** 317 moves (CORRECT)
- **xgospel2:** 321 moves (4 EXTRA bogus passes)
- **Binary:** Dec 24 17:12 (contains ObservationState fix only)

## Move Count Breakdown

```
xgospel2: 321 total moves
q5Go:     317 total moves
Difference: 4 extra moves
```

**Empty Pass Count:**
```
xgospel2: 7 empty passes (B[] or W[])
q5Go:     3 empty passes (legitimate end-game passes)
Extra:    4 bogus passes
```

## Location of Bogus Passes

### START of Game (3 bogus passes)

**xgospel2:**
```sgf
HA[7]PW[PandaBot4]
PB[DJzz]
...
RE[B+39.5];W[];B[];W[];W[fq];B[hq];W[nc]
              ^^^^^^^^^^^^^
              3 BOGUS PASSES before first real move!
```

**q5Go (CORRECT):**
```sgf
HA[7]
RE[B+39.5]PL[W]AB[dd][dj][dp][jj][pd][pj][pp];W[fq];B[hq];W[nc]
                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
                Handicap stones as SETUP, not moves!
```

###END of Game (1 net extra - scoring phase)

**xgospel2:**
```sgf
;B[oh];W[ab];B[ba];W[];B[];W[];W[]
                     ^^^^^^^^^^^^^^^
                     4 bogus passes at end
```

**q5Go (CORRECT):**
```sgf
;B[oh];W[ab];B[ba];W[];B[];W[];TW[aj][ar]...
                     ^^^^^^^^^^^
                     3 legitimate passes at end
```

## Why ObservationState Fix Failed

**The Fix (board_window.cpp:858-868):**
```cpp
if (observation_state == RECONSTRUCTING && move_history.isEmpty()) {
    qDebug() << "🧹 FIRST HISTORY MOVE: Clearing game tree to prevent duplicates";
    delete game_root;
    game_root = new GameNode();
    current_node = game_root;
    move_history.clear();
    board_widget->clearBoard();
}
```

**Why It Didn't Fire:**
- NO "🧹 FIRST HISTORY MOVE" message in debug output
- Condition `move_history.isEmpty()` returned **FALSE**
- **Handicap stones filled move_history BEFORE first real game move arrived**

## The Handicap Game Problem

In handicap games, the sequence is:

1. User observes game → `observe 344` sent
2. IGS sends handicap info → xgospel2 processes handicap (-2, 7) notation
3. **Handicap placement fills move_history with 7 entries**
4. `moves 344` response arrives with ALL moves
5. First history move arrives, but `move_history.isEmpty()` is **FALSE** (has 7 handicaps!)
6. Clearing logic NEVER executes
7. Live moves that arrived get DUPLICATED in game tree

## Files and Evidence

- **xgospel2 SGF:** [game_344_PandaBot4_vs_DJzz_20251224_172106.sgf](file:///home/cahill/Claude_Projects/64-bit/game_files/game_344_PandaBot4_vs_DJzz_20251224_172106.sgf)
- **q5Go SGF:** [2025-12-24-PandaBot4-DJzz_game344_q5Go.sgf](file:///home/cahill/Claude_Projects/64-bit/game_files/2025-12-24-PandaBot4-DJzz_game344_q5Go.sgf)
- **Debug output:** [game_344_debug_out.txt](file:///home/cahill/Claude_Projects/game_344_debug_out.txt)

## Additional Issue: Scoring Phase Fix Missing

The binary tested (Dec 24 17:12) does NOT contain the scoring phase fix from SCORING_PHASE_MOVE_BUG.md:
- NO "[SCORING] Ignoring..." messages in debug output
- The isScoringMode() check exists in source but wasn't in this binary
- Need to rebuild with BOTH fixes

## The Real Solution

The ObservationState approach won't work for handicap games. We need a different detection strategy:

### Option 1: Detect "moves" Command Response
When Command 15 response with move history starts arriving, clear immediately

### Option 2: Clear Based on Move Number
If we receive move N and current tree has move N, we're getting duplicates - clear and start over

### Option 3: Track "moves" Command State
Add flag `waiting_for_move_history` that's set when "moves N" is sent, cleared when complete

---

**Analysis Date:** 2025-12-24 17:30
**Status:** ⚠️ **HANDICAP GAMES BREAK OBSERVATIONSTATE FIX**
**Priority:** CRITICAL - Need new approach for handicap games
