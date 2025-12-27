# q5Go Duplicate Prevention - The Real Solution

## Date: 2025-12-24

## Discovery

After examining q5Go source code, I found **THE** solution to preventing duplicate move history. It's completely different from what we attempted!

## q5Go's Elegant Strategy

### Key Files
- **parser.cpp:1269-1362** - cmd15() handles Command 15 (moves response)
- **parser.cpp:1359** - `emit signal_set_observe(aGameInfo->nr);` - Called for EVERY move!
- **qgo_interface.cpp:807-828** - set_observe() - The magic happens here

### The Solution (Lines 807-828)

```cpp
void qGoIF::set_observe (const QString& gameno)
{
	int nr = gameno.toInt ();
	qGoBoard *b = find_game_id (nr);
	if (b != nullptr)
		return;  // ← CRITICAL: Board exists? Just return!

	// Only executed ONCE (on first history move):
	b = new qGoBoard (this, nr);
	boardlist.append (b);

	client_window->sendcommand ("games " + gameno, false);
	client_window->sendcommand ("moves " + gameno, false);
	client_window->sendcommand ("all " + gameno, false);

	b->set_gsName (gsName);
	b->set_myName (myName);
	b->set_Mode_real (modeObserve);

	n_observed++;
	client_window->update_observed_games (n_observed);
}
```

### How It Works

**The Flow:**

1. User observes game → sends "observe N"
2. Live moves may arrive → processed normally
3. **"moves N" command response starts** → Command 15 messages
4. **FIRST history move arrives:**
   - parser calls `signal_set_observe(game_nr)`
   - set_observe() looks up board by game_id
   - **Board NOT found** (this is the first history move!)
   - **Creates new board and sends "moves" command**
   - Processes the move
5. **SECOND history move arrives:**
   - parser calls `signal_set_observe(game_nr)`
   - set_observe() looks up board by game_id
   - **Board FOUND** → **RETURNS IMMEDIATELY**
   - Move still gets processed by signal_move()
6. **All subsequent history moves:**
   - Same as step 5 - early return from set_observe()

### The Genius

**set_observe() acts as a "board creation gate":**
- Called on EVERY Command 15 move
- But only creates board + sends "moves" command on the FIRST move
- All subsequent calls early-return immediately

**This prevents duplicates because:**
- The "moves" command is ONLY sent when the first history move arrives
- NOT sent when user clicks "observe" button
- NOT sent with a timer
- NOT sent based on observation state

**It's self-synchronizing:**
- Works for regular games
- Works for handicap games (board created on first real move, not handicap setup)
- Works regardless of timing
- No race conditions

## Applying to xgospel2

We need to change from:

**CURRENT (Wrong):**
```cpp
// observeGame() at line 4141:
socket->write("observe N\n");
socket->write("moves N\n");  // ← Sends moves immediately!
```

**TO:**
```cpp
// observeGame() at line 4141:
socket->write("observe N\n");
// DON'T send "moves" here!

// Then in Command 15 handler (around line 3060):
void processCommand15Move(GameMove &move) {
    BoardWindow *board = findOrCreateBoard(move.game_id);

    if (board == nullptr) {
        // Board was just created - this is first history move!
        // NOW send the "moves" command:
        QString moves_cmd = QString("moves %1").arg(move.game_id);
        socket->write((moves_cmd + "\n").toUtf8());
        qDebug() << ">>> SENT: moves" << move.game_id << "(first history move)";
    }

    board->processMove(move);
}

BoardWindow* findOrCreateBoard(int game_id) {
    // Look for existing board
    for (BoardWindow* b : board_windows) {
        if (b->getObservedGameId() == game_id) {
            return b;  // Found - return existing
        }
    }

    // Not found - create new board
    BoardWindow* board = new BoardWindow(this, username);
    board->setObservedGameId(game_id);
    board_windows.append(board);
    board->show();

    return nullptr;  // Signal that board was just created
}
```

## Benefits

1. **No timers** - No QTimer, no event loop issues
2. **No observation state tracking** - Don't need RECONSTRUCTING vs LIVE states
3. **Works for handicap games** - First REAL move triggers "moves", not handicap setup
4. **Self-synchronizing** - Doesn't matter when live moves arrive
5. **Simple logic** - Just "board exists? return : create"

## Testing

After implementation, we should see:
```
>>> SENT: observe 344 (observing game)
>>> SENT: moves 344 (first history move)  ← Only appears ONCE, when first history move arrives
```

And:
- Move count matches q5Go exactly
- No bogus passes in SGF
- Works for both regular and handicap games

---

**Discovery Date:** 2025-12-24
**Status:** ✅ **SOLUTION IDENTIFIED** - Ready to implement
**Source:** q5Go qgo_interface.cpp:807-828
