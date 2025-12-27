# Duplicate Game Tree Nodes Fix

## Date: 2025-12-24

## Problem: Bogus Pass Moves in SGF

Comparing game 97 (pione29 vs Colinwong):
- **q5Go**: 326 moves (CORRECT)
- **xgospel2**: 370 moves (WRONG - 44 extra bogus white passes)

### Evidence

**xgospel2 SGF Pattern:**
```sgf
;W[rc];W[];B[cr];W[];W[cs];W[];B[sc];W[];W[bn];W[];
```
Every white move followed by bogus `W[]` pass!

**Count:**
```bash
$ grep -o "W\[\]" game_97_pione29_vs_Colinwong_20251224_085238.sgf | wc -l
45
```

## Root Cause

The `clearMoveHistoryBeforeMovesCommand()` function was clearing `move_history` and the visual board, but **NOT clearing the game tree**.

### Timeline of Bug:

1. User starts observing game
2. **Live move arrives** during 1-second delay (e.g., move 300: `W[rc]`)
3. `processMove()` adds it to BOTH `move_history` AND `game_root` tree
4. After 1 second, `clearMoveHistoryBeforeMovesCommand()` is called
5. ❌ **BUG**: Only `move_history` is cleared, tree keeps move 300
6. `moves <game_id>` command sent to IGS
7. IGS responds with ALL moves 0-300
8. `processMove()` called for each move, including move 300 AGAIN
9. Move 300 added to tree a SECOND time as duplicate node
10. Result: Game tree has move 300 twice

### Why So Many Duplicates?

During a ko fight, moves arrive rapidly:
- White recaptures at `rc` (arrives live, added to tree)
- Tree not cleared
- Moves command response includes `rc` again
- Duplicate `W[rc]` node created
- SGF generation traverses tree and outputs both nodes

## Solution

Modified `clearMoveHistoryBeforeMovesCommand()` in [board_window.cpp:769-797](board_window.cpp#L769) to ALSO reset the game tree:

### Before (WRONG):
```cpp
void BoardWindow::clearMoveHistoryBeforeMovesCommand() {
    int old_size = move_history.size();
    move_history.clear();
    board_widget->clearBoard();  // Only cleared visual board

    qDebug() << "🧹 CLEARED" << old_size << "moves from history before 'moves' command"
             << "for game" << observed_game_id;
}
```

### After (FIXED):
```cpp
void BoardWindow::clearMoveHistoryBeforeMovesCommand() {
    int old_size = move_history.size();
    move_history.clear();
    board_widget->clearBoard();

    // ALSO CLEAR THE GAME TREE to prevent duplicate nodes
    delete game_root;
    game_root = new GameNode();
    current_node = game_root;

    qDebug() << "🧹 CLEARED" << old_size << "moves from history AND reset game tree before 'moves' command"
             << "for game" << observed_game_id;
}
```

## Why This Fix Works

Now when the "moves" command response arrives:
1. Both `move_history` AND `game_root` are completely fresh
2. All moves 0-N are added to empty structures
3. No duplicates possible
4. Move count matches q5Go exactly

## Affected Code Paths

The `clearMoveHistoryBeforeMovesCommand()` function is called from two places in [xgospel2_fixed.cpp](xgospel2_fixed.cpp):

1. **Line 2870**: Manual observe path (`observe <game_id>` command)
2. **Line 4137**: Menu observe path (Games→Observe menu)

Both paths now properly clear the game tree.

## Testing

To verify the fix:
1. Observe a game from start to finish
2. Save SGF when done
3. Compare move count with q5Go
4. Check debug output for "reset game tree" message
5. Verify no `W[];W[]` or `B[];B[]` sequences in SGF

**Expected Debug Output:**
```
🧹 CLEARED 1 moves from history AND reset game tree before 'moves' command for game 97
```

## Files Modified

- ✅ [board_window.cpp:769-797](board_window.cpp#L769) - Added game tree reset to `clearMoveHistoryBeforeMovesCommand()`

## Related Fixes

This completes the game tree implementation:
1. ✅ Game tree navigation (slider fix)
2. ✅ SGF generation from tree (move count fix)
3. ✅ Duplicate node prevention (this fix)

## Status

✅ **COMPLETED** - Game tree is now properly cleared before moves command
✅ **BUILD**: SUCCESS
⏸️ **TESTING**: Pending verification with new game observation

---

**Implementation Date:** 2025-12-24
**Priority:** CRITICAL - Prevents corrupted SGF files
