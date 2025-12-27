# Handicap Stone SGF Fix - q5Go Pattern Implementation

## Date: 2025-12-24

## Problem Identified

Game 246 (2-handicap game) showed a discrepancy:
- **xgospel2:** 293 moves with bogus B[] passes at start: `RE[B+3.5];B[];B[];B[];W[cd]...`
- **q5Go:** 290 moves with proper AB[] setup: `RE[B+3.5]PL[W]AB[dp][pd];W[cd]...`

The issue was NOT related to ko fights or move history duplicates - it was about **handicap stone representation in SGF files**.

## Root Cause Analysis

### How xgospel2 Was Handling Handicap Stones (WRONG)

1. **Game Tree Node Creation** ([board_window.cpp:882](board_window.cpp#L882)):
   ```cpp
   GameNode* new_node = current_node->addMove(-2, handicap_count, BLACK_STONE);
   ```
   Handicap stones were added as game tree nodes with x=-2

2. **SGF Generation** ([board_window.cpp:1580-1590](board_window.cpp#L1580-L1590)):
   ```cpp
   if (x >= 0 && x < 19 && y >= 0 && y < 19) {
       // Regular move
   } else if (x == -1 && y == -1) {
       // Pass move
       sgf += QString(";%1[]").arg(move_color);
   }
   // Note: Handicap stones (x == -2) are already encoded in HA[] tag, skip them
   ```

**The Bug:** When x==-2 (handicap node), it didn't match either condition, so it **fell through** to the pass move handling, creating bogus B[] passes!

### How q5Go Handles Handicap Stones (CORRECT)

From q5Go source code analysis:

**File:** `/home/cahill/Claude_Projects/github/q5Go/src/sgf2board.cc`

1. **Lines 712-721:** Handicap stones are added as AB[] properties:
   ```cpp
   for (auto &n: s.nodes->props)
       if (n.ident == "AB") {
           n.handled = true;
           put_stones (n, size_x, size_y, [&] (int x, int y) {
               initpos.set_stone_nounits (x, y, black);
           });
       }
   ```

2. **Lines 925-956:** Clear distinction between setup and moves:
   ```cpp
   if (gs->m_parent == nullptr || gs->was_edit_p ()) {
       // Root node or edit: write AB/AW/AE properties
       maybe_add_property (s, m_board, "AB", added_b, &linecount);
   } else if (gs->was_move_p () || gs->was_pass_p ()) {
       // Actual move: write B[] or W[]
       if (col == white)
           s += "W[";
       else
           s += "B[";
   }
   ```

**Key Distinction:** Handicap stones are **setup properties** (AB[]), NOT moves (B[])!

## The Fix

### Changes Made

**1. Add Handicap as AB[] Setup Properties** ([board_window.cpp:1559-1570](board_window.cpp#L1559-L1570))

```cpp
// q5Go pattern: Add handicap stones as AB[] setup properties (not moves!)
if (handicap >= 2) {
    QList<QPair<int, int>> positions = IGSMoveParser::getHandicapPositions(handicap);
    if (!positions.isEmpty()) {
        sgf += "PL[W]AB";  // Player to move is White, Add Black setup stones
        for (const auto& pos : positions) {
            char col = 'a' + pos.second;  // x coordinate
            char row = 'a' + pos.first;   // y coordinate
            sgf += QString("[%1%2]").arg(col).arg(row);
        }
    }
}
```

**2. Skip Handicap Nodes in Move Sequence** ([board_window.cpp:1593-1597](board_window.cpp#L1593-L1597))

```cpp
// q5Go pattern: Skip handicap nodes - they're already in AB[] setup
if (x == -2) {
    qDebug() << "[SGF] Skipping handicap node (x=-2) - already in AB[] setup";
    continue;
}
```

### SGF Output Comparison

**Before Fix (xgospel2 - WRONG):**
```sgf
(;FF[4]GM[1]...HA[2]...RE[B+3.5];B[];B[];B[];W[cd];B[pp]...
```
- 3 bogus B[] passes (handicap nodes falling through)
- Handicap stones not explicitly placed

**After Fix (xgospel2 - CORRECT, matches q5Go):**
```sgf
(;FF[4]GM[1]...HA[2]...RE[B+3.5]PL[W]AB[dp][pd];W[cd];B[pp]...
```
- PL[W] = Player to move is White
- AB[dp][pd] = Add Black stones at D4 and Q16 (standard 2-stone handicap)
- First move is W[cd] (White's first actual move)

## Expected Results

For Game 246 (2-handicap game):
- **Before:** 293 moves (3 extra bogus B[] passes)
- **After:** 290 moves (matching q5Go exactly)

For Game 344 (7-handicap game):
- **Before:** 321 moves (4 extra from 3 bogus handicap passes + scoring issue)
- **After:** 317 moves (matching q5Go exactly)

## Build Information

**Build Date:** 2025-12-24
**Build Status:** SUCCESS
**Binary:** xgospel2

## Files Modified

1. **[board_window.cpp:1559-1570](board_window.cpp#L1559-L1570)** - Added AB[] handicap setup properties
2. **[board_window.cpp:1593-1597](board_window.cpp#L1593-L1597)** - Added explicit skip for handicap nodes (x==-2)

## Testing Checklist

- [ ] Game 246 (2-handicap): Should now show 290 moves (not 293)
- [ ] Game 344 (7-handicap): Should now show 317 moves (not 321)
- [ ] SGF files should have `AB[...]` properties for handicap stones
- [ ] SGF files should have `PL[W]` (White to play first in handicap games)
- [ ] No bogus B[] passes at start of handicap games
- [ ] First move after handicap should be W[..] (White's move)

## Related Documentation

- [Q5GO_PATTERN_IMPLEMENTATION.md](Q5GO_PATTERN_IMPLEMENTATION.md) - Board creation gate pattern
- [Q5GO_SOLUTION_DISCOVERY.md](Q5GO_SOLUTION_DISCOVERY.md) - How we discovered q5Go's approach
- [GAME_344_HANDICAP_ANALYSIS.md](GAME_344_HANDICAP_ANALYSIS.md) - Why ObservationState fix failed

---

**Implementation Date:** 2025-12-24
**Status:** ✅ **READY FOR TESTING**
**Priority:** CRITICAL - Affects all handicap games
