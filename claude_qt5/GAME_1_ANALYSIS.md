# Game 1 Move Count Discrepancy Analysis

## Date: 2025-12-24

## Summary

**Game:** k44441224 (White, 5d+) vs AlterIgo (Black, 4d)
**Result:** B+2.5
**Handicap:** 2 stones

### Move Count Comparison

| Source | Move Count | Extra Moves |
|--------|-----------|-------------|
| q5Go (CORRECT) | 286 moves | baseline |
| xgospel2 | 304 moves | **+18 moves** |

### Bogus Pass Analysis

Examining the xgospel2 SGF file reveals:
- **19 empty white passes** `W[]`
- **2 empty black passes** `B[]`
- **Total: 21 bogus passes**

The discrepancy math: 304 - 286 = 18, which closely matches the 21 bogus passes.

## Evidence: Duplicate Node Pattern

### xgospel2 SGF (END SECTION):
```sgf
W[si];W[];B[os];W[];W[qs];W[];B[de];W[];W[ha];W[];B[gb];W[];W[ga];W[];
B[ik];W[];W[il];W[];B[sh];W[];W[sj];W[];B[ak];W[];W[af];W[];B[ah];W[];
W[ag];W[];B[];W[];W[];W[];B[]
```

**Pattern:** Every white move followed by bogus `W[]` empty pass!

### q5Go SGF (SAME SECTION):
```sgf
;W[si]WL[452]OW[20];B[os]BL[544]OB[20];W[qs]WL[447]OW[19];B[de]BL[533]OB[19];
W[ha]WL[378]OW[18];B[gb]BL[526]OB[18];W[ga]WL[376]OW[17];B[ik]BL[517]OB[17];
W[il]WL[363]OW[16];B[sh]BL[502]OB[16];W[sj]WL[336]OW[15];B[ak]BL[493]OB[15];
W[af]WL[322]OW[14];B[ah]BL[490]OB[14];W[ag]WL[320]OW[13];B[];W[];B[]
```

**Pattern:** Clean alternation with no duplicate nodes until final passes.

## Root Cause: SAME BUG AS GAME 97

This is the EXACT SAME duplicate node bug documented in [DUPLICATE_NODES_FIX.md](DUPLICATE_NODES_FIX.md).

### Bug Mechanism:
1. User starts observing game
2. Live move arrives during 1-second observation delay
3. `processMove()` adds it to BOTH `move_history` AND `game_root` tree
4. `clearMoveHistoryBeforeMovesCommand()` is called
5. If game tree NOT cleared → tree keeps live move
6. `moves <game_id>` command sent to IGS
7. IGS responds with ALL moves including the live move again
8. `processMove()` adds the move to tree a SECOND time → duplicate node
9. `generateSGF()` traverses tree and outputs duplicate nodes

## Fix Status: APPLIED BUT NOT USED?

### Timeline Evidence:

```bash
-rw-r--r-- game_97_pione29_vs_Colinwong_20251224_085238.sgf  (08:52)
-rwxr-xr-x xgospel2*                                         (09:03) <- FIXED BINARY
-rw-r--r-- game_1_k44441224_vs_AlterIgo_20251224_091943.sgf (09:19)
```

**The Problem:** Game 1 was observed at 09:19, which is **16 minutes AFTER** the fixed binary was compiled at 09:03.

### Possible Explanations:

**Hypothesis 1: Old Binary Still Running**
The user may have compiled the fixed version at 09:03 but the OLD unfixed xgospel2 binary was still running from before. The running process would not automatically reload the new binary.

**Hypothesis 2: Fix Not Actually Applied**
Although the source code shows the fix at [board_window.cpp:790-793](board_window.cpp#L790-L793), it's possible the fix was added to the file AFTER game 1 was observed, and the MD file timestamp is misleading.

**Hypothesis 3: clearMoveHistoryBeforeMovesCommand() Not Called**
The function exists but may not be getting called in the observe path. Need to verify both call sites are active:
- xgospel2_fixed.cpp:2870 (manual observe)
- xgospel2_fixed.cpp:4137 (menu observe)

## Verification Required

To confirm the fix is working correctly, the user should:

1. **Rebuild from clean state:**
   ```bash
   make clean
   make
   ```

2. **Kill ALL running xgospel2 instances:**
   ```bash
   killall xgospel2
   ```

3. **Start fresh instance:**
   ```bash
   ./xgospel2
   ```

4. **Observe a NEW game from start to finish**

5. **Check debug output for:**
   ```
   🧹 CLEARED N moves from history AND reset game tree before 'moves' command for game <id>
   ```

6. **Save SGF and compare with q5Go**

7. **Verify no bogus passes:**
   ```bash
   grep -o "W\[\]" game_file.sgf | wc -l  # Should be only actual passes
   ```

## Expected Behavior (After Fix)

When `clearMoveHistoryBeforeMovesCommand()` is called:
- Both `move_history` AND `game_root` are completely cleared
- All moves 0-N from IGS are added to fresh, empty structures
- No duplicates possible
- Move count matches q5Go exactly

## Recommendation

**The user should observe a NEW game with the current codebase and verify the fix is working.**

If duplicate nodes still appear after rebuilding and restarting:
1. Check that `clearMoveHistoryBeforeMovesCommand()` is actually being called
2. Verify the game tree is being properly cleared (add more debug output)
3. Check if there are other code paths that bypass this function

---

**Analysis Date:** 2025-12-24
**Status:** ⚠️ **NEEDS VERIFICATION** - Game 1 SGF shows unfixed behavior despite fix being in source code
**Priority:** HIGH - Affects SGF accuracy for all observed games
