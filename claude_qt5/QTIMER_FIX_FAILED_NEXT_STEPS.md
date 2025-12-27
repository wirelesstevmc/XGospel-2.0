# QTimer Fix Failed - Next Steps

## Date: 2025-12-24

## Problem Summary

**Game 21 Test Results:**
- q5Go: 312 moves (CORRECT)
- xgospel2: 357 moves (45 EXTRA bogus white passes)
- Binary: Dec 24 13:13 (contains QTimer fix)
- Game observed: Dec 24 13:30

**Root Cause:** The QTimer::singleShot callback at lines 4162-4184 NEVER FIRES. Qt event loop doesn't process it.

**Evidence:**
- Code IS in binary (`strings` confirms it)
- NO "CLEARED move_history" message in debug output
- NO "WARNING: Board for game" message in debug output
- QTimer callback silently fails

## Why QTimer Doesn't Work

Qt timers require the event loop to process them. Something prevents the timer from firing:
- Event loop may be blocked
- Timer destroyed before firing
- Unreliable in this context

**Conclusion:** Abandon QTimer approach entirely - it's fundamentally flawed for this use case.

## The Reliable Solution

Use existing `ObservationState` infrastructure (already in BoardWindow.h lines 144-150):

```cpp
enum ObservationState {
    NOT_OBSERVING = 0,
    JOINING_GAME = 1,
    RECONSTRUCTING = 2,    // Getting board state/history
    LIVE_OBSERVATION = 3   // Receiving real-time moves
};
```

### Implementation Strategy

**Step 1: Remove QTimer code** (lines 4162-4184 in xgospel2_fixed.cpp)

Replace with immediate "moves" command:
```cpp
// Request move history immediately - no timer!
QString moves_cmd = QString("moves %1").arg(game_id);
socket->write((moves_cmd + "\n").toUtf8());
output_console->append(QString(">>> SENT: %1 (requesting move history)").arg(moves_cmd));
```

**Step 2: Modify Board Window::processMove()** (around line 860 in board_window.cpp)

Add check at START of processMove():
```cpp
void BoardWindow::processMove(const GameMove &move) {
    // CRITICAL FIX: Clear game tree when FIRST history move arrives
    if (observation_state == RECONSTRUCTING && move_history.isEmpty()) {
        // This is the first move from history dump - clear any live moves that arrived first
        qDebug() << "🧹 FIRST HISTORY MOVE: Clearing game tree to prevent duplicates";

        delete game_root;
        game_root = new GameNode();
        current_node = game_root;
        move_history.clear();
        board_widget->clearBoard();
    }

    // ... rest of existing processMove() code ...
}
```

**Step 3: Also fix manual observe path** (line 2870 in xgospel2_fixed.cpp)

Same change - remove QTimer, send "moves" immediately.

### Why This Works

1. **No timers** - relies on move arrival, which is guaranteed
2. **Detects first history move** - when `observation_state == RECONSTRUCTING` and `move_history.isEmpty()`
3. **Clears exactly once** - after first move, `move_history` is no longer empty
4. **Self-synchronizing** - doesn't matter when "moves" command is sent
5. **Already has infrastructure** - `ObservationState` already tracks RECONSTRUCTING vs LIVE

### Testing

After fix, debug output should show:
```
>>> SENT: observe 21 (observing game)
>>> SENT: moves 21 (requesting move history)
🧹 FIRST HISTORY MOVE: Clearing game tree to prevent duplicates
🎯 OBSERVATION STATE: Changed to RECONSTRUCTING
```

And most importantly:
- **Move count matches q5Go exactly**
- **No bogus passes in SGF**

## Files To Modify

1. **xgospel2_fixed.cpp:4162-4184** - Remove QTimer, send moves immediately
2. **xgospel2_fixed.cpp:2870-2892** - Remove QTimer, send moves immediately
3. **board_window.cpp:860** - Add first-history-move detection and clearing logic

## Priority

**CRITICAL** - Currently ALL observed games have incorrect move counts.

---

**Analysis Date:** 2025-12-24
**Implementation Date:** 2025-12-24 17:12
**Status:** ✅ **IMPLEMENTED - READY FOR TESTING**

## Implementation Summary

All three fixes have been successfully implemented:

1. ✅ [xgospel2_fixed.cpp:4159-4163](xgospel2_fixed.cpp#L4159-L4163) - Removed QTimer, send moves immediately
2. ✅ [xgospel2_fixed.cpp:2867-2871](xgospel2_fixed.cpp#L2867-L2871) - Removed QTimer, send moves immediately
3. ✅ [board_window.cpp:858-868](board_window.cpp#L858-L868) - Added first-history-move detection and clearing logic

**Binary:** xgospel2 (Dec 24 17:12)

## Expected Debug Output

When observing a game, you should now see:
```
>>> SENT: observe N (observing game)
>>> SENT: moves N (requesting move history)
🧹 FIRST HISTORY MOVE: Clearing game tree to prevent duplicates
🎯 OBSERVATION STATE: Changed to RECONSTRUCTING
```

And most importantly:
- **Move count should match q5Go exactly**
- **No bogus passes in SGF**
