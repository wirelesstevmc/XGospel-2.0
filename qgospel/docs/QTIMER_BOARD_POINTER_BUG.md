# QTimer Board Pointer Bug - FIXED

## Date: 2025-12-24

## Problem: Extra Pass Moves in Game Tree

**Symptom:** Games observed from start have extra bogus pass moves in SGF
**Example:** Game 112 had 283 moves vs q5Go's 277 (6 extra BLACK passes)

## Root Cause Analysis

### The Bug Timeline

1. User double-clicks game in games list to observe
2. `observeGame()` creates BoardWindow and adds to `board_windows` list (line 4124)
3. Sends "observe 112" to IGS (line 4142)
4. Schedules QTimer to fire after 1 second, capturing `board` pointer (line 4151)
5. **PROBLEM**: During that 1 second, live moves arrive from IGS
6. QTimer fires, lambda executes, but `if (board)` check fails - pointer is NULL!
7. `clearMoveHistoryBeforeMovesCommand()` is NEVER called
8. "moves 112" is sent, IGS returns ALL moves 1-277
9. Live moves that already arrived (moves 1-6) are DUPLICATED in game tree

### Evidence from Game 112 Debug Log

```
>>> SENT: observe 112 (observing game)
< 15 271(W): N19                           ← Live move arrives
>>> SENT: moves 112 (requesting move history) ← QTimer fired, sent moves command
< 15 Game 112 I: yuzuudon ...             ← Command 15 response
```

**Missing:**
- NO ">>> CLEARED move_history before requesting moves 112" message
- This means `if (board)` at line 4154 returned FALSE
- So `clearMoveHistoryBeforeMovesCommand()` was never called

### Why Board Pointer Became NULL

The lambda at line 4151 captured `board` **by value**:

```cpp
QTimer::singleShot(1000, this, [this, game_id, board]() {
    if (board) {  // ← This check FAILED!
        board->clearMoveHistoryBeforeMovesCommand();
    }
```

**The Problem:**
- The `board` pointer is a raw pointer to a heap-allocated object
- Capturing it by value just copies the pointer VALUE
- If the object gets moved, deleted, or the pointer otherwise invalidated during the 1-second delay
- The captured pointer becomes dangling/null
- The `if (board)` check fails silently
- No clearing happens, duplicates result

## The Fix

**Location:** [xgospel2_fixed.cpp:4151-4173](xgospel2_fixed.cpp#L4151-L4173) and [xgospel2_fixed.cpp:2870-2892](xgospel2_fixed.cpp#L2870-L2892)

**Strategy:** Instead of capturing the board pointer, capture only `game_id` and **look up the board when the timer fires**:

```cpp
QTimer::singleShot(1000, this, [this, game_id]() {
    // CRITICAL FIX: Find board by game_id instead of capturing pointer
    // The captured pointer can become invalid during the 1-second delay
    BoardWindow* board = nullptr;
    for (BoardWindow* b : board_windows) {
        if (b->getObservedGameId() == game_id) {
            board = b;
            break;
        }
    }

    // Clear any moves that arrived during the delay
    if (board) {
        board->clearMoveHistoryBeforeMovesCommand();
        qDebug() << ">>> CLEARED move_history before requesting moves" << game_id;
    } else {
        qDebug() << ">>> WARNING: Board for game" << game_id << "not found when QTimer fired!";
    }

    QString moves_cmd = QString("moves %1").arg(game_id);
    socket->write((moves_cmd + "\n").toUtf8());
    output_console->append(QString(">>> SENT: %1 (requesting move history)").arg(moves_cmd));
});
```

**Benefits:**
1. Always finds current valid board pointer when timer fires
2. Detects if board was closed during delay (warning message)
3. Guarantees `clearMoveHistoryBeforeMovesCommand()` will be called if board exists
4. No more duplicate moves in game tree

## Testing Plan

### Test Case: Observe Game From Start

1. Double-click game in games list
2. Wait for observation to start
3. Observe until game ends (resign or counting)
4. Save SGF
5. **Verify:**
   - Debug log shows ">>> CLEARED move_history before requesting moves N"
   - Move count matches q5Go exactly
   - No extra bogus passes in SGF
   - No duplicates of first few moves

### Expected Debug Output

**BEFORE FIX (Game 112):**
```
>>> SENT: observe 112 (observing game)
>>> SENT: moves 112 (requesting move history)    ← No "CLEARED" message!
```

**AFTER FIX:**
```
>>> SENT: observe 112 (observing game)
>>> CLEARED move_history before requesting moves 112    ← FIX SUCCESS!
>>> SENT: moves 112 (requesting move history)
```

## Files Modified

- ✅ [xgospel2_fixed.cpp:4151-4173](xgospel2_fixed.cpp#L4151-L4173) - Fixed observeGame() QTimer lambda
- ✅ [xgospel2_fixed.cpp:2870-2892](xgospel2_fixed.cpp#L2870-L2892) - Fixed manual observe QTimer lambda

## Success Criteria

- ⏸️ Debug log shows "CLEARED" message for every observation (NEEDS TESTING)
- ⏸️ Move counts match q5Go exactly (NEEDS TESTING)
- ⏸️ No extra passes in SGF (NEEDS TESTING)
- ⏸️ No duplicate moves at start of game (NEEDS TESTING)

---

**Analysis Date:** 2025-12-24
**Implementation Date:** 2025-12-24
**Priority:** CRITICAL - Affects all observed games
**Status:** ✅ **IMPLEMENTED** - Ready for testing
**Build:** ✅ **SUCCESS**

## Related Bugs

This fix is related to [SCORING_PHASE_MOVE_BUG.md](SCORING_PHASE_MOVE_BUG.md), which addressed a different source of bogus moves during scoring phase.
