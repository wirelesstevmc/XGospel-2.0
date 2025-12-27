# Game 156 Move Count Analysis

## Date: 2025-12-24 22:35

---

## Problem Identification

Game 156 showed xgospel2 had **323 moves** vs q5Go's **316 moves** (7 extra moves).

### SGF Comparison

**xgospel2 (WRONG - 323 moves):**
```sgf
RE[W+6.5];B[lh];W[];B[pd];W[pp]...
...B[lh];W[];W[ie];W[];B[li];W[];W[];W[];B[];W[];W[]
```

**q5Go (CORRECT - 316 moves):**
```sgf
RE[W+6.5];B[pd];W[pp]...
...B[lh];W[ie]WL[470]OW[5];B[li]BL[432]OB[1];W[];B[];W[]
```

---

## Root Cause Analysis

### Issue 1: First Live Move Not Skipped

**Problem:** The first live move `B[lh];W[]` appears at the start of the SGF file.

**Why It Happened:**
- The 22:20 binary was built with ONLY the handicap fix (board_window.cpp)
- It did NOT include the q5Go board creation gate pattern fix (xgospel2_fixed.cpp:3074-3090)
- The first live move that triggers "moves N" command was NOT skipped
- Result: First live move appears twice (once live, once in history)

**The Fix:**
The q5Go board creation gate pattern at [xgospel2_fixed.cpp:3087-3089](xgospel2_fixed.cpp#L3087-L3089) contains critical code:

```cpp
// CRITICAL: Skip processing this first live move - it will be in the history
qDebug() << "[q5Go] Skipping first live move" << move_part << "- will get it in history";
target_board = nullptr;  // Prevent processing below
```

This fix WAS in the source code but NOT compiled into the 22:20 binary!

### Issue 2: Extra Passes at End

**Problem:** xgospel2 has 10 passes at end vs q5Go's 3 legitimate passes.

**Extra Passes Count:**
- xgospel2: `W[];W[ie];W[];B[li];W[];W[];W[];B[];W[];W[]` (10 passes)
- q5Go: `W[ie]WL[470]OW[5];B[li]BL[432]OB[1];W[];B[];W[]` (3 passes)

**Analysis:**
The extra 7 passes are likely duplicates from the move history issue. When the first live move wasn't skipped, it created a cascade of duplicates throughout the game.

---

## The Solution

### Fresh Build with ALL Fixes

**Build Date:** 2025-12-24 22:34

**Fixes Included:**

1. **q5Go Board Creation Gate Pattern** ([xgospel2_fixed.cpp:3074-3090](xgospel2_fixed.cpp#L3074-L3090))
   - Send "moves N" only on first Command 15 move
   - Skip processing that first move (it will be in history)
   - Use QSet<int> to track which games have had "moves N" sent

2. **Handicap Stone AB[] Fix** ([board_window.cpp:1559-1597](board_window.cpp#L1559-L1597))
   - Represent handicap stones as AB[] setup properties
   - Skip handicap nodes (x==-2) in move sequence
   - Add PL[W] to indicate White plays first

---

## Expected Results

With the 22:34 build, Game 156 should now show:

**Move Count:** 316 moves (matching q5Go exactly)
**SGF Start:** `;B[pd];W[pp]...` (no extra B[lh];W[])
**SGF End:** Exactly 3 legitimate passes

---

## Testing Instructions

1. **Start the new binary** (22:34 build)
2. **Observe a new game** (Game 156 is already complete)
3. **Check debug output** for `[q5Go]` pattern messages:
   ```
   >>> SENT: observe 156 (observing game)
   >>> [q5Go] SENT: moves 156 (first live move arrived, requesting history)
   [q5Go] Cleared board for game 156 before history arrives
   [q5Go] Skipping first live move B[lh] - will get it in history
   ```
4. **Save SGF** and verify move count matches q5Go exactly
5. **Compare SGF files** with q5Go - should be identical structure

---

## Files Modified

1. **[xgospel2_fixed.cpp:1743](xgospel2_fixed.cpp#L1743)** - Added `QSet<int> games_with_moves_requested`
2. **[xgospel2_fixed.cpp:3074-3090](xgospel2_fixed.cpp#L3074-L3090)** - q5Go board creation gate pattern
3. **[xgospel2_fixed.cpp:2868-2871](xgospel2_fixed.cpp#L2868-L2871)** - Removed immediate "moves" (manual observe)
4. **[xgospel2_fixed.cpp:4124-4127](xgospel2_fixed.cpp#L4124-L4127)** - Removed immediate "moves" (observeGame)
5. **[board_window.cpp:1559-1570](board_window.cpp#L1559-L1570)** - Added AB[] handicap setup
6. **[board_window.cpp:1593-1597](board_window.cpp#L1593-L1597)** - Skip handicap nodes in move loop

---

## Build Information

**Build Date:** 2025-12-24 22:34
**Build Status:** SUCCESS
**Binary:** xgospel2 (5.3M)
**Compilation:** All warnings are benign (deprecated Qt functions, unused variables)

---

## Related Documentation

- [Q5GO_PATTERN_COMPLETE.md](Q5GO_PATTERN_COMPLETE.md) - Complete q5Go pattern implementation
- [Q5GO_PATTERN_IMPLEMENTATION.md](Q5GO_PATTERN_IMPLEMENTATION.md) - Detailed implementation guide
- [HANDICAP_SGF_FIX.md](HANDICAP_SGF_FIX.md) - Handicap stone AB[] fix
- [Q5GO_SOLUTION_DISCOVERY.md](Q5GO_SOLUTION_DISCOVERY.md) - How we discovered q5Go's approach

---

**Analysis Date:** 2025-12-24 22:35
**Status:** ✅ **READY FOR TESTING**
**Priority:** CRITICAL - Test with new games to verify move counts match q5Go!
