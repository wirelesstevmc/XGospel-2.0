# IGS CMD22 Territory Encoding Reference

**DO NOT MODIFY the encoding or storage convention described here without updating this document and bumping the version. This encoding was established at v266, confirmed correct at v272, v275, and v286 — each via hard-won revert after a failed attempt to "fix" it.**

---

## CMD22 Wire Format

The server sends 19 lines of 19 characters, one digit per intersection:

```
22 {
22  1111111111111111111   ← row 0 (sent first) = IGS board row 1 (BOTTOM of board)
22  ...
22  1111111111111111111   ← row 18 (sent last) = IGS board row 19 (TOP of board)
22 }
```

### Digit Values

| Digit | Meaning |
|-------|---------|
| 0 | Dame / neutral (empty, uncounted) |
| 1 | White stone (alive, on board) |
| 2 | Black stone (alive, on board) |
| 3 | Dame / neutral (empty, uncounted — same as 0) |
| 4 | **White territory** (empty intersection counted for white) |
| 5 | **Black territory** (empty intersection counted for black) |

**Key facts confirmed empirically (game 417, 2026-08-09):**
- Digit 4 = white territory. Cross-check: 58 cells with digit==4 + 1 white capture − 5.5 komi = 53.5 = CMD20 white score ✓
- Digit 5 = black territory. Follows from the above.
- Digits 0 and 3 are both dame/neutral — treat identically.
- A stone sitting inside opponent territory (digit==4 with a black stone, or digit==5 with a white stone) is a **dead stone**.

---

## Storage Convention — QPair(row, col)

`receiveScoreLine(int row, const QString &line)` stores each intersection as:

```cpp
QPair<int, int> pos(row, col);   // row FIRST, col SECOND — always
territory_ownership[pos] = digit;
```

Where:
- `row` = CMD22 row index, 0–18. Row 0 = bottom of board (IGS row 1). Row 18 = top of board (IGS row 19).
- `col` = character position in the line, 0–18. Col 0 = column A. Col 18 = column T.

**This is the only convention used throughout the codebase. Do not change it.**

---

## How the Convention Is Used — All Four Call Sites

Every site that reads `territory_ownership` must be consistent with `QPair(row, col)`.

### 1. `drawTerritoryMarkers` (board_window.cpp)

```cpp
QPoint center = boardToScreen(pos.first, pos.second);
// pos.first = row, pos.second = col
// boardToScreen(row, col) → correct screen position
```

### 2. `drawDeadStoneMarkers` (board_window.cpp)

```cpp
QPoint center = boardToScreen(pos.first, pos.second);
StoneColor c = getStoneAt(pos.first, pos.second);
// Same order — row first, col second
```

### 3. `drawDisputedMarkers` (board_window.cpp)

```cpp
QPoint center = boardToScreen(pos.first, pos.second);
```

### 4. `receiveScoreEnd` dead stone detection (board_window.cpp)

```cpp
StoneColor actual_stone = final_board->getStone(pos.first, pos.second);
// Black stone in white territory (digit==4):
if (digit == 4 && actual_stone == BLACK_STONE) { dead stone }
// White stone in black territory (digit==5):
if (digit == 5 && actual_stone == WHITE_STONE) { dead stone }
```

### 5. SGF TW/TB generation (board_window.cpp)

```cpp
char col = 'a' + pos.second;  // col = pos.SECOND
char row = 'a' + pos.first;   // row = pos.FIRST
// ownership == 4 → TW; ownership == 5 → TB
```

### 6. Inactive-slot CMD22 buffer (xgospel2_fixed.cpp)

```cpp
territory_slot->territory_ownership[QPair<int,int>(row_number, col)] = digit;
// Same QPair(row, col) convention — row_number first, col second
```

### 7. Inactive-slot territory decode and prisoner counting (xgospel2_fixed.cpp)

```cpp
if (digit == 4) { WHITE_STONE territory; dead if board_state has BLACK_STONE }
else if (digit == 5) { BLACK_STONE territory; dead if board_state has WHITE_STONE }
else if (digit == 2 || digit == 3) { dame → EMPTY }

// Prisoner counting:
if (digit == 4) slot->white_prisoners++;   // dead black stone in white territory
else if (digit == 5) slot->black_prisoners++; // dead white stone in black territory
```

---

## `boardToScreen(row, col)` Signature

```cpp
QPoint GoBoardWidget::boardToScreen(int row, int col)
```

- First argument: board row (matches `pos.first`)
- Second argument: board column (matches `pos.second`)

This matches the QPair convention exactly. **Do not swap the arguments.**

---

## History of Failed "Fix" Attempts

This encoding has been changed incorrectly three times. Each time it caused cascading test failures and required a deliberate revert.

| Version | Change attempted | Result |
|---------|-----------------|--------|
| v270 | Changed storage to `QPair(col, row_from_top)` | Incorrect overlays on all scored games |
| v271 | Debug patches on top of v270 | Still wrong |
| v272 | **Reverted to QPair(row, col)** | Restored correct behavior |
| v274 | Swapped `boardToScreen` arguments instead of storage | Broke editor scoring overlay |
| v275 | **Reverted to pos.first/pos.second** | Restored correct behavior |
| v285 | Changed to `QPair(col, 18-row)` + digit 3=black territory | Identical to v270 mistake |
| v286 | **Reverted to QPair(row, col)** | Restored correct behavior |

**The pattern:** The encoding looks "wrong" because row 0 is the bottom (not top) and pos.first is row (not col). It is self-consistent internally. Changing any subset of the four call sites without changing all of them breaks the self-consistency.

**The test:** After any scoring change, verify that white territory cells in the CMD22 output (digit==4) match the CMD20 white score: `count(digit==4) + white_captures - komi = CMD20_white_score`. If this arithmetic holds, the encoding is correct.

---

## Summary Cheat Sheet

```
CMD22 digit → meaning
  0, 3  →  dame (neutral, uncounted)
  1     →  white stone on board
  2     →  black stone on board
  4     →  WHITE territory (empty, counted for white)
  5     →  BLACK territory (empty, counted for black)
  4 + black stone present  →  dead black stone
  5 + white stone present  →  dead white stone

Storage:  QPair(row, col)  — row FIRST, col SECOND
          row 0 = bottom of board = IGS row 1
          row 18 = top of board   = IGS row 19
          col 0 = A-column
          col 18 = T-column

boardToScreen(pos.first, pos.second) — always this order
SGF:  col='a'+pos.second   row='a'+pos.first
```
