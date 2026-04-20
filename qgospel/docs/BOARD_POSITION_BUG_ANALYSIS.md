# Critical Bug: Invalid Move 0 Causing Board Position Mismatch

## Problem Summary
xgospel2 board positions do NOT match q5Go for the same observed game. Analysis of SGF files reveals xgospel2 is inserting an invalid move at position 0.

## Evidence from SGF Files

### xgospel2 SGF (WRONG)
File: `game_167_minoyan_vs_morikawa_20251223_073757.sgf`

Line 11 move sequence:
```sgf
;B[bs];B[pd];W[dp];B[pp];W[dc];B[dj];W[ch];B[de]...
```

**Problem**: Starts with `B[bs]` which should NOT be move 0!

### q5Go SGF (CORRECT)
File: `2025-12-23-minoyan-morikawa_game_167_q5Go.sgf`

Line 12 move sequence:
```sgf
;B[pd];W[dp];B[pp];W[dc];B[dj];W[ch];B[de]...
```

**Correct**: Starts with `B[pd]` as first move.

## Impact

1. **All moves shifted by 1** - Every stone after move 0 is in the wrong position
2. **Board position completely wrong** - Looks like a different game
3. **Move count off by 1** - xgospel2 shows 362, q5Go shows 363
4. **Impossible to score correctly** - Territory calculation will be wrong

## Where is `B[bs]` Coming From?

### Theory 1: Handicap Stone Processing
- xgospel2 may be treating something as a handicap stone
- But this is NOT a handicap game (no handicap in SGF header)
- `bs` coordinate (b1 in 1-indexed, or a0 in 0-indexed) is unlikely for handicap

### Theory 2: Previous Game Bleed
- If previous observed game had move at `bs`, it might not be cleared
- Board state not properly reset between game observations

### Theory 3: IGS "moves" Command Parsing Error
- IGS sends move history via "moves <game_id>" command
- Response format: `15 Game <id> I: <player1> vs <player2> (<move_data>)`
- Each move line: `15 <game_id> <color> <coord>`
- xgospel2 may be parsing move 0 incorrectly or including a pass/setup move

### Theory 4: Ko Fight Confusion
Looking at the SGF, `bs` appears legitimately multiple times later:
- Move ~230: `B[bs];W[as];B[dd];W[ea];B[bs];W[hk];B[hj];W[as]`

This is a ko fight at B1/A1. Perhaps:
- A late-game ko move is being cached
- Board state from END of game is polluting the START
- Move history buffer not properly cleared

## Code Locations to Investigate

### 1. board_window.cpp - Move Processing
Location where IGS move responses (Command 15) are parsed and applied to board.

**Check**:
- How is move 0 handled?
- Is there initialization of board state before moves applied?
- Are moves cleared between observations?

### 2. igs_move_parser.cpp - Move Parsing
Parses IGS move format (e.g., "15 167 B pd")

**Check**:
- Does it skip move 0?
- Does it validate coordinates before adding moves?
- Are there debug logs showing what moves are parsed?

### 3. xgospel2_fixed.cpp - Game Observation Logic
Handles "observe <game_id>" command response and "moves <game_id>" response.

**Check lines ~1044-1047** (from INCOMPLETE_MOVES_INVESTIGATION.md):
```cpp
QByteArray data = socket->readAll();
QString text = QString::fromUtf8(data);
for (const QString& line : text.split('\n')) {
```

**Specific Issues**:
- Is board cleared before applying move history?
- Are old moves from previous game still in memory?
- Is there proper initialization?

## Debugging Strategy

### Step 1: Add Debug Logging
Add logging to track EVERY move being added to the SGF:

```cpp
// In board_window.cpp or wherever moves are added to SGF
qDebug() << "[MOVE-DEBUG] Adding move #" << move_number
         << " Color:" << (color == BLACK ? "B" : "W")
         << " Coord:" << coord_string;
```

### Step 2: Verify Board Initialization
Ensure board is COMPLETELY cleared when starting observation:

```cpp
void BoardWindow::startObservation(int game_id) {
    qDebug() << "[OBS-DEBUG] Starting observation of game" << game_id;

    // CRITICAL: Clear all previous moves
    move_history.clear();
    sgf_moves.clear();
    board_state.clear();
    move_count = 0;

    // Then request moves
    sendMovesCommand(game_id);
}
```

### Step 3: Check Move 0 Handling
Look for any code that handles move 0 specially:

```bash
grep -n "move.*0\|move_count.*0\|move_number.*0" board_window.cpp
```

### Step 4: Capture Raw IGS Response
Add logging to see EXACT moves from IGS:

```cpp
// In onDataReceived() when processing "moves" response
if (line.startsWith("15") && line.contains("B ") || line.contains("W ")) {
    qDebug() << "[IGS-RAW-MOVE]" << line;
}
```

## Expected Fix Location

Based on the symptoms, the bug is likely in **board_window.cpp** in one of these scenarios:

### Scenario A: Move History Not Cleared
```cpp
// WRONG - old moves persist
void BoardWindow::observeGame(int game_id) {
    // Missing: move_history.clear();
    requestMoves(game_id);
}

// FIXED
void BoardWindow::observeGame(int game_id) {
    move_history.clear();  // ADD THIS
    sgf_moves.clear();      // ADD THIS
    move_count = 0;         // ADD THIS
    requestMoves(game_id);
}
```

### Scenario B: Off-by-One in Move Indexing
```cpp
// WRONG - starts at 0 instead of 1
for (int i = 0; i < moves.size(); i++) {
    addMoveToSGF(moves[i]);  // Adds move 0!
}

// FIXED
for (int i = 1; i < moves.size(); i++) {  // Start at 1
    addMoveToSGF(moves[i]);
}
```

### Scenario C: Handicap/Setup Confusion
```cpp
// WRONG - treating non-handicap move as setup
if (move_number == 0) {
    addSetupStone(color, coord);  // Wrong for non-handicap!
}

// FIXED
if (move_number == 0 && is_handicap_game) {
    addSetupStone(color, coord);
} else if (move_number > 0) {
    addMove(color, coord);
}
```

## Testing Plan

### Test 1: Fresh Observation
1. Start xgospel2 fresh (no previous games)
2. Observe game 167
3. Check if `B[bs]` still appears as move 0
4. **Expected**: If bug is "previous game bleed", this should be clean

### Test 2: Sequential Observations
1. Observe game A (any game)
2. Close observation
3. Observe game B (game 167)
4. Check if game A's last move appears as game B's move 0
5. **Expected**: If bug is "not clearing state", this will reproduce it

### Test 3: SGF Move Logging
1. Add debug logging before each SGF move write
2. Observe game 167
3. Check logs to see where `B[bs]` is being added
4. **Expected**: Log will show exact code path adding bogus move 0

## Priority

**CRITICAL** - This bug makes xgospel2 fundamentally broken for game observation. Board positions are completely wrong, making the client unusable for:
- Following games
- Learning from games
- Territory marking (future feature)
- Game analysis

This MUST be fixed before any other features can be implemented.

## Investigation Progress

### Code Analysis Completed (2025-12-23)

**Flow Traced**:
1. IGS sends moves via Command 15 → `xgospel2_fixed.cpp:3060`
2. `IGSMoveParser::parseMoveLine()` parses format `"144(B): B12"` → `igs_move_parser.cpp:14`
3. `BoardWindow::processMove()` called → `board_window.cpp:811`
4. Move added to `move_history` → `board_window.cpp:867` (pass) or `903` (regular)
5. SGF generated from `move_history` → `board_window.cpp:1476`

**Key Finding**:
- `move_history.clear()` IS called in `startObserving()` at line 738
- Moves are added to `move_history` in order they're received
- SGF generates moves in same order as `move_history`

**Debugging Added**:
- Enhanced logging now shows SGF coordinates for each move (e.g., `B[pd]`, `W[dp]`, `B[bs]`)
- Will clearly identify when and where bogus `B[bs]` is added to history

### Critical Observation from SGF Comparison

**xgospel2 SGF line 11**: `RE[W+1.5];B[bs];B[pd];W[dp]...`
**q5Go SGF line 11**: `RE[W+1.5];B[pd];W[dp];B[pp]...`

The bogus `B[bs]` appears RIGHT AFTER the game result (`RE[W+1.5]`) tag, suggesting it might be:
- A ko fight move from late in the game (BS appears legitimately in ko fights near move 230)
- Being inserted at position 0 of move_history
- Possibly from incomplete TCP packet reception during moves history request

## SOLUTION IMPLEMENTED (2025-12-23)

### Root Cause Identified
The bug was caused by live moves arriving during the 1-second delay between starting observation and sending the `moves <game_id>` command. These live moves were added to `move_history`, then when the `moves` response arrived with the complete game history (including those same moves), they were added AGAIN, causing duplicates.

### Fix Applied
Modified TWO code paths in [xgospel2_fixed.cpp](xgospel2_fixed.cpp):

#### Fix 1: Line 2870 - Manual Observe Path
```cpp
QTimer::singleShot(1000, this, [this, game_id, board]() {
    // Use the captured 'board' pointer directly (it's already in scope)
    if (board) {
        board->clearMoveHistoryBeforeMovesCommand();
        qDebug() << ">>> CLEARED move_history before requesting moves" << game_id;
    }

    QString moves_cmd = QString("moves %1").arg(game_id);
    socket->write((moves_cmd + "\n").toUtf8());
    output_console->append(QString(">>> SENT: %1 (requesting move history)").arg(moves_cmd));
});
```

#### Fix 2: Line 4137 - Menu Observe Path
```cpp
QTimer::singleShot(1000, this, [this, game_id, board]() {
    // Use the captured 'board' pointer directly (it's already in scope)
    if (board) {
        board->clearMoveHistoryBeforeMovesCommand();
        qDebug() << ">>> CLEARED move_history before requesting moves" << game_id;
    }

    QString moves_cmd = QString("moves %1").arg(game_id);
    socket->write((moves_cmd + "\n").toUtf8());
    output_console->append(QString(">>> SENT: %1 (requesting move history)").arg(moves_cmd));
});
```

#### Supporting Method: board_window.cpp:761-783
```cpp
void BoardWindow::clearMoveHistoryBeforeMovesCommand() {
    // CRITICAL FIX: Clear move_history before the "moves <game_id>" response arrives
    // to prevent duplicates of any live moves that arrived during the 1-second delay.

    int old_size = move_history.size();
    move_history.clear();
    board_widget->clearBoard();  // Also clear visual board to match

    qDebug() << "🧹 CLEARED" << old_size << "moves from history before 'moves' command"
             << "for game" << observed_game_id;
}
```

### Why Game 181 Failed with Initial Fix
- Initial fix only applied to line 2870 (manual observe path)
- Game 181 was observed via menu (line 4137 code path)
- Lambda at line 4137 didn't capture `board` pointer, so couldn't call the clear method
- Fixed by adding `board` to lambda capture list and using it directly

### Testing Plan
1. Observe new games using BOTH observation methods (manual `observe <id>` and menu)
2. Verify "🧹 CLEARED" message appears in debug output
3. Compare SGF files with q5Go for byte-for-byte match
4. Test with multiple games to ensure fix is robust

## Next Steps

1. ✅ Add comprehensive debug logging to move processing (COMPLETED - lines 863-867 and 896-903 in board_window.cpp)
2. ✅ Identify root cause: Live moves during 1-second delay (COMPLETED)
3. ✅ Implement fix in BOTH observation code paths (COMPLETED - lines 2870 and 4137)
4. ⏭️ Test with new game observations to verify fix works
5. ⏭️ Compare SGF output byte-for-byte with q5Go

## Debug Output Analysis (2025-12-23)

### game_386_debug_output.txt Analysis

**File Contents**:
- Lines 1-19: Game 386 info, time, and properties received when observation started
- Lines 20-791: Live moves 258-291 as they occurred in real-time
- Line 793: Resignation result `W+R`

**CRITICAL MISSING DATA**:
The debug output does NOT contain:
1. The initial `moves 386` command response that would have loaded moves 1-257
2. ANY `🗂️ MOVE HISTORY` logging entries
3. Evidence of where/when the bogus `W[bh]` move was added to move_history

**WHY THIS MATTERS**:
From the SGF file comparison, we know:
- **xgospel2 SGF**: `RE[W+R];B[];W[bh];B[];B[pd];W[qp]...`
  - Has bogus empty passes and `W[bh]` at position 0
- **q5Go SGF**: `RE[W+R];B[pd];W[qp];B[dq]...`
  - Correctly starts with `B[pd]`

The `W[bh]` move appears LATE in the game (around move 290 based on live moves), but somehow ends up at position 0 in the SGF. The bug must be occurring during the initial move history load (moves 1-257) which is NOT captured in this debug output.

**CONCLUSION**:
1. The running binary did NOT have the enhanced logging code compiled in
2. We need a fresh debug capture starting from observation start with the NEW binary
3. The bug is likely in how the initial `moves <game_id>` response is processed

---

## Additional Notes

The `bs` coordinate (B1 in standard notation) is particularly suspicious because:
- It appears in legitimate ko fights late in the game
- It's being incorrectly prepended to move sequence
- Suggests buffer/memory not properly managed
- Could be last move from previous observation session
