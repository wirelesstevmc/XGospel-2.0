# Scoring Phase Bogus Move Bug

## Date: 2025-12-24

## Problem: Extra Pass Moves in Counting Results

**Symptom:** Games that end by counting (not resign) have bogus white pass moves inserted into the SGF.

**Evidence from Game 1** (k44441224 vs AlterIgo):
- **q5Go**: 286 moves (CORRECT)
- **xgospel2**: 304 moves (+18 bogus passes)

### SGF Comparison (Final Moves):

**q5Go (CORRECT):**
```sgf
;W[ag]WL[320]OW[13]
;B[]     ← BLACK PASS
;W[]     ← WHITE PASS
;B[]     ← BLACK PASS (game ends)
```

**xgospel2 (WRONG):**
```sgf
;W[ag]
;W[]     ← BOGUS
;B[]     ← BLACK PASS
;W[]     ← WHITE PASS
;W[]     ← BOGUS
;W[]     ← BOGUS
;B[]     ← BLACK PASS
```

**Pattern:** Bogus `W[]` pass inserted after EVERY white move during endgame/scoring phase!

## Root Cause: Scoring Messages Parsed as Moves

### The Bug Timeline

1. Game reaches endgame, both players pass
2. IGS enters **scoring mode** - players mark dead stones
3. IGS sends messages about dead stone marking:
   ```
   15 287(W): D7 B4 B5 C4 C5     ← Dead stones being marked
   15 288(B): Pass               ← Agreement/confirmation
   ```
4. **BUG**: xgospel2's `parseMoveLine()` matches these as regular moves!
5. Lines 76-94 in [igs_move_parser.cpp](igs_move_parser.cpp#L76-L94) process "D7 B4 B5..." as multi-capture
6. These get added to `move_history` AND `game_tree` via `processMove()`
7. SGF generation includes these bogus "moves"

### Why This Doesn't Affect Resign Results

**Resign games:** Game ends immediately when resign command received. No scoring phase.
**Counting games:** Game enters scoring phase with stone removal protocol → bogus moves added.

## Code Analysis

### [igs_move_parser.cpp:76-94](igs_move_parser.cpp#L76-L94) - The Culprit

```cpp
// Check for stone removal notation during counting phase (e.g., "R8 R7" or "D7 B4 B5 C4 C5 E6 C6 D5 D6")
QStringList coords = coordinates.split(QRegExp("\\s+"), QString::SkipEmptyParts);
if (coords.size() >= 2) {
    // First coordinate is the move, rest are captured stones
    parseCoordinates(coords[0], move.x, move.y);

    // Process all captured stone coordinates
    QStringList captured_coords;
    for (int i = 1; i < coords.size(); i++) {
        int cap_x, cap_y;
        parseCoordinates(coords[i], cap_x, cap_y);
        captured_coords << QString("%1,%2").arg(cap_x).arg(cap_y);
    }
    move.captured = captured_coords.join(";");

    qDebug() << "DEBUG: Multi-capture move detected...";
    return true;  // ← Signals this is a valid MOVE (WRONG!)
}
```

**The Problem:**
- This code was designed to handle dead stone removal visualization
- But it returns `true`, indicating a valid **game move**
- These "moves" get added to move history and game tree
- They should be handled separately for **scoring visualization only**

### [xgospel2_fixed.cpp:3069-3088](xgospel2_fixed.cpp#L3069-L3088) - Where Moves Are Routed

```cpp
if (move_parser->parseMoveLine(move_part, move)) {
    BoardWindow* target_board = nullptr;

    if (current_game_context != -1) {
        for (BoardWindow* board : board_windows) {
            if (board->getObservedGameId() == current_game_context) {
                target_board = board;
                break;
            }
        }
    }

    if (target_board) {
        move.game_id = current_game_context;
        target_board->processMove(move);  // ← Scoring messages processed as moves!
    }
}
```

**The Problem:**
- No check for `target_board->isScoringMode()`
- Scoring-phase messages routed to `processMove()` just like regular game moves
- They get added to game tree and exported to SGF

## The Fix

### Option 1: Filter in xgospel2_fixed.cpp (RECOMMENDED)

**Location:** [xgospel2_fixed.cpp:3069-3088](xgospel2_fixed.cpp#L3069-L3088)

**Change:**
```cpp
if (move_parser->parseMoveLine(move_part, move)) {
    BoardWindow* target_board = nullptr;

    if (current_game_context != -1) {
        for (BoardWindow* board : board_windows) {
            if (board->getObservedGameId() == current_game_context) {
                target_board = board;
                break;
            }
        }
    }

    if (target_board) {
        // CRITICAL FIX: Do NOT add scoring-phase messages to game tree!
        if (target_board->isScoringMode()) {
            // Scoring phase: Handle dead stone visualization only, don't add to move history
            qDebug() << "[SCORING] Ignoring scoring-phase message (not a game move):" << move_part;

            // TODO (future): Update visual dead stone markers on board
            // target_board->updateDeadStoneMarkers(move);

            // DO NOT call processMove() - these aren't real game moves!
        } else {
            // Normal game move: Add to history and game tree
            move.game_id = current_game_context;
            target_board->processMove(move);
            qDebug() << "[MENU] MOVE ROUTED: Game" << current_game_context << "move" << move_part;
        }
    }
}
```

### Option 2: Return Different Code from Parser

**Location:** [igs_move_parser.cpp:76-94](igs_move_parser.cpp#L76-L94)

**Change:** Add a `move_type` field to `GameMove` struct:
```cpp
enum MoveType {
    REGULAR_MOVE,
    PASS_MOVE,
    HANDICAP_PLACEMENT,
    SCORING_ACTION  // ← New type
};
```

Then in parser:
```cpp
if (coords.size() >= 2) {
    // Mark as scoring action, not regular move
    move.move_type = SCORING_ACTION;
    parseCoordinates(coords[0], move.x, move.y);
    // ... rest of code ...
    return true;
}
```

And in xgospel2_fixed.cpp:
```cpp
if (move_parser->parseMoveLine(move_part, move)) {
    if (target_board && move.move_type != SCORING_ACTION) {
        target_board->processMove(move);
    }
}
```

## Recommended Solution

**Use Option 1** - it's simpler and doesn't require changing data structures.

### Implementation Steps:

1. ✅ **Verify `isScoringMode()` exists** in BoardWindow (already exists at line 1182)

2. **Add scoring mode check** in [xgospel2_fixed.cpp:3085](xgospel2_fixed.cpp#L3085)

3. **Test with counting game:**
   - Observe game from start
   - Let it go to counting (both pass)
   - Save SGF
   - Compare with q5Go - move counts should match!

4. **Verify no regression on resign games** (should still work)

## Testing Plan

### Test Case 1: Counting Result
1. Observe new game from start to counting phase
2. Let both players pass and enter scoring
3. Save SGF after final score determined
4. Check move count matches q5Go
5. Verify no bogus `W[]` or `B[]` between real moves

### Test Case 2: Resign Result
1. Observe game that ends in resignation
2. Save SGF
3. Verify no regression (should still work correctly)

### Test Case 3: Mid-Game Observation
1. Join game already in progress
2. Observe to counting finish
3. Verify move counts correct

## Expected Debug Output

**BEFORE FIX:**
```
[MENU] MOVE ROUTED: Game 1 move 287(W): D7 B4 B5    ← BAD
*** PASS MOVE PROCESSED: client history size=288
```

**AFTER FIX:**
```
[SCORING] Ignoring scoring-phase message (not a game move): 287(W): D7 B4 B5    ← GOOD
```

## Files Modified

- ✅ [xgospel2_fixed.cpp:3085-3106](xgospel2_fixed.cpp#L3085-L3106) - Added scoring mode check
- ✅ [board_window.h:223](board_window.h#L223) - Added `isScoringMode()` getter

## Implementation Complete

### Changes Made:

**1. Added scoring mode check in xgospel2_fixed.cpp:**
```cpp
if (target_board->isScoringMode()) {
    // Scoring phase: Ignore these messages - they're not game moves
    qDebug() << "[SCORING] Ignoring scoring-phase message (not a game move):" << move_part;
    // DO NOT call processMove() - these aren't real game moves!
} else {
    // Normal game move: Add to history and game tree
    move.game_id = current_game_context;
    target_board->processMove(move);
}
```

**2. Added getter method in board_window.h:**
```cpp
bool isScoringMode() const { return is_scoring_mode; }
```

## Success Criteria

- ⏸️ Counting games: Move count matches q5Go exactly (NEEDS TESTING)
- ⏸️ No bogus `W[]` or `B[]` in SGF between real moves (NEEDS TESTING)
- ⏸️ Resign games: Still work correctly - no regression (NEEDS TESTING)
- ⏸️ Debug output shows "[SCORING] Ignoring..." messages during scoring phase (NEEDS TESTING)

## Testing Plan

**Next Steps:**
1. Run xgospel2 with the fix
2. Observe a game that goes to counting
3. Save SGF after game completes
4. Compare move count with q5Go
5. Check for "[SCORING] Ignoring..." messages in debug output
6. Verify no bogus passes between real moves

---

**Analysis Date:** 2025-12-24
**Implementation Date:** 2025-12-24
**Priority:** CRITICAL - Affects all counting result games
**Status:** ✅ **IMPLEMENTED** - Ready for testing
**Build:** ✅ **SUCCESS**
