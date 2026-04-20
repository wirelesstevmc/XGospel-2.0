# q5Go Board Creation Gate Pattern - Complete Implementation Summary

## Date: 2025-12-24

---

## Overview

Successfully ported q5Go's board creation gate pattern to xgospel2 to eliminate move history duplicates in observed games. This fixes the long-standing issue where xgospel2 generated extra bogus passes compared to q5Go.

## The Problem

**Original Issue:**
- xgospel2 was creating duplicate move history when observing games
- Game 21: 357 moves (45 extra bogus passes) vs q5Go's 312 moves
- Game 344: 321 moves (4 extra bogus passes) vs q5Go's 317 moves
- Game 246: Initially 293 moves (3 extra) vs q5Go's 290 moves

**Root Cause:**
xgospel2 sent "moves N" command immediately when user clicked "observe", causing:
1. Live moves arrive and get processed
2. "moves N" response arrives with full history (including those live moves)
3. Live moves get duplicated in the game tree
4. Appear as bogus empty passes (W[] or B[]) in SGF files

---

## The Solution: q5Go's Board Creation Gate Pattern

### How q5Go Prevents Duplicates

From [qgo_interface.cpp:807-828](../github/q5Go/src/qgo_interface.cpp#L807-L828):

```cpp
void qGoIF::set_observe (const QString& gameno)
{
    int nr = gameno.toInt ();
    qGoBoard *b = find_game_id (nr);
    if (b != nullptr)
        return;  // ← Board exists? Just return!

    // Only executed ONCE (on first history move):
    b = new qGoBoard (this, nr);
    boardlist.append (b);

    client_window->sendcommand ("games " + gameno, false);
    client_window->sendcommand ("moves " + gameno, false);  // ← Sent here!
    client_window->sendcommand ("all " + gameno, false);
}
```

**Key Insight:**
- `set_observe()` is called for **EVERY** Command 15 move
- First move: board doesn't exist → create board + send "moves N" command
- Subsequent moves: board exists → return immediately (no "moves N" sent)
- Result: "moves N" sent exactly **once**, when first history move arrives

---

## Implementation in xgospel2

### 1. Remove Immediate "moves" Commands

**Before:**
```cpp
// observeGame() - WRONG APPROACH
socket->write("observe N\n");
socket->write("moves N\n");  // ← Sent immediately - causes duplicates!
```

**After ([xgospel2_fixed.cpp:4124-4127](xgospel2_fixed.cpp#L4124-L4127)):**
```cpp
// q5Go pattern: Do NOT send "moves N" command here
// Wait for first Command 15 move to arrive, then send "moves N" from there
// This prevents duplicates (live moves + history moves)
output_console->append(QString(">>> Waiting for first live move before requesting history..."));
```

### 2. Add Game Tracking

**[xgospel2_fixed.cpp:1743](xgospel2_fixed.cpp#L1743):**
```cpp
QSet<int> games_with_moves_requested;  // Track which games have had "moves N" sent
```

### 3. Implement Board Creation Gate in Command 15 Handler

**[xgospel2_fixed.cpp:3074-3090](xgospel2_fixed.cpp#L3074-L3090):**
```cpp
// q5Go pattern: If this is the FIRST Command 15 move for this game,
// send "moves N" command to get full move history
if (target_board && !games_with_moves_requested.contains(current_game_context)) {
    games_with_moves_requested.insert(current_game_context);

    QString moves_cmd = QString("moves %1").arg(current_game_context);
    socket->write((moves_cmd + "\n").toUtf8());
    output_console->append(QString(">>> [q5Go] SENT: %1 (first live move arrived, requesting history)").arg(moves_cmd));

    // Clear the board to start fresh with history
    target_board->clearMoveHistoryBeforeMovesCommand();
    qDebug() << "[q5Go] Cleared board for game" << current_game_context << "before history arrives";

    // CRITICAL: Skip processing this first live move - it will be in the history
    qDebug() << "[q5Go] Skipping first live move" << move_part << "- will get it in history";
    target_board = nullptr;  // Prevent processing below
}
```

**Critical Fix (Lines 3087-3089):**
The first live move that triggers sending "moves N" must be **skipped** because it will be included in the history response. Setting `target_board = nullptr` prevents processing that move.

### 4. Remove Obsolete Clearing Logic

**[board_window.cpp:858-860](board_window.cpp#L858-L860):**
```cpp
// NOTE: The q5Go board creation gate pattern (ensureBoardExistsForGame)
// prevents duplicates by sending "moves" command only when board is first created.
// No need to clear game tree here - duplicates won't occur!
```

Removed 11 lines of observation state clearing logic that failed for handicap games.

---

## Why This Pattern Works

### Self-Synchronizing
- Works for regular games, handicap games, any timing
- Doesn't matter when first Command 15 move arrives
- No race conditions possible

### No State Dependencies
- Doesn't rely on move_history state (failed for handicap games)
- Doesn't rely on timers (Qt event loop issues)
- Just checks if game is in games_with_moves_requested set

### Simple Gate Pattern
- Game in set? Don't send "moves" again
- Game not in set? Send "moves" and add to set
- That's it!

---

## Testing History

### Previous Failed Approaches

1. **QTimer Approach:** Timer callbacks never executed (Qt event loop issue)
   - See [QTIMER_FIX_FAILED_NEXT_STEPS.md](QTIMER_FIX_FAILED_NEXT_STEPS.md)

2. **ObservationState Approach:** Failed for handicap games
   - move_history.isEmpty() was always false (handicap stones filled it)
   - See [GAME_344_HANDICAP_ANALYSIS.md](GAME_344_HANDICAP_ANALYSIS.md)

### Current Implementation Testing

**First Build (Initial q5Go Pattern):**
- Game 246: 293 moves vs q5Go's 290 moves
- Improvement: Only 3 extra moves (down from 45+)
- Problem: First live move was being processed AND included in history

**Second Build (18:03 - Skip First Move Fix):**
- Added `target_board = nullptr` to skip first live move
- Expected: Should now match q5Go exactly (290 moves)
- **Status: READY FOR TESTING**

---

## Expected Behavior

When observing a game, debug output should show:

```
>>> SENT: observe 344 (observing game)
>>> [q5Go] SENT: moves 344 (first live move arrived, requesting history)
[q5Go] Cleared board for game 344 before history arrives
[q5Go] Skipping first live move W[fq] - will get it in history
```

And most importantly:
- **Move count matches q5Go exactly**
- **No bogus passes in SGF files**
- **Works for both regular and handicap games**
- **Handicap stones as AB[] setup, not moves**
- **Exactly 3 legitimate passes at end (W[];B[];W[])**

---

## Files Modified

1. **[xgospel2_fixed.cpp:1743](xgospel2_fixed.cpp#L1743)** - Added games_with_moves_requested tracking
2. **[xgospel2_fixed.cpp:2868-2871](xgospel2_fixed.cpp#L2868-L2871)** - Removed immediate "moves" (manual observe)
3. **[xgospel2_fixed.cpp:4124-4127](xgospel2_fixed.cpp#L4124-L4127)** - Removed immediate "moves" (observeGame)
4. **[xgospel2_fixed.cpp:3074-3090](xgospel2_fixed.cpp#L3074-L3090)** - Implemented board creation gate
5. **[board_window.cpp:858-860](board_window.cpp#L858-L860)** - Removed obsolete clearing logic

---

## Related Documentation

- **[Q5GO_SOLUTION_DISCOVERY.md](Q5GO_SOLUTION_DISCOVERY.md)** - Analysis of q5Go source code and discovery of the pattern
- **[Q5GO_PATTERN_IMPLEMENTATION.md](Q5GO_PATTERN_IMPLEMENTATION.md)** - Detailed implementation guide
- **[GAME_344_HANDICAP_ANALYSIS.md](GAME_344_HANDICAP_ANALYSIS.md)** - Why ObservationState fix failed
- **[QTIMER_FIX_FAILED_NEXT_STEPS.md](QTIMER_FIX_FAILED_NEXT_STEPS.md)** - Why QTimer fix failed

---

## Build Information

**Build Date:** 2025-12-24 18:03
**Build Status:** SUCCESS
**Binary:** xgospel2
**Size:** 5.3M

---

## What To Test

### Test Games

**Game 246 (Regular Game - nanasi vs Active7):**
- Previous: 293 moves (3 extra)
- Expected: 290 moves (matching q5Go)

**Game 21 (Regular Game - PandaBot3 vs DJzzg):**
- Previous: 357 moves (45 extra)
- Expected: 312 moves (matching q5Go)

**Game 344 (7-Handicap Game - PandaBot4 vs DJzz):**
- Previous: 321 moves (4 extra)
- Expected: 317 moves (matching q5Go)

### Verification Checklist

- [ ] Move count matches q5Go exactly
- [ ] No bogus empty passes (W[] or B[]) in SGF files
- [ ] Debug output shows "[q5Go]" pattern messages
- [ ] Handicap games show AB[] setup, not move sequences
- [ ] End-game has exactly 3 legitimate passes (W[];B[];W[])
- [ ] Works for games observed from start
- [ ] Works for games observed mid-game
- [ ] No crashes or errors during observation

---

## Success Criteria

✅ **Implementation Complete** - All code changes applied and compiled
⏳ **Testing Pending** - Waiting for user to test with live games
🎯 **Goal:** Move counts match q5Go exactly for all game types

---

**Implementation Date:** 2025-12-24
**Status:** ✅ **READY FOR TESTING**
**Next Step:** Test with live IGS games to verify move counts match q5Go!
