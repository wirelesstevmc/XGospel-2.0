# q5Go Scoring and Territory Marking - Code Analysis

## Overview
This document analyzes q5Go's implementation of IGS scoring, territory marking, and dead stone removal for both playing and observing modes.

---

## 1. Data Structures (goboard.h)

### Mark Types
```cpp
enum class mark {
    none = 0, move, triangle, circle, square, plus, cross,
    text, num, letter,
    dead,      // Mark stone as dead/captured
    seki,      // Mark as seki (mutual life)
    terr,      // Territory marker
    falseeye   // False eye marker
};
```

### Score Structure
```cpp
struct go_score {
    int score_b = 0, score_w = 0;  // Territory + captures
    int caps_b = 0, caps_w = 0;    // Captured stones
};
```

### go_board Class Members
```cpp
// Score tracking
int m_score_b = 0;
int m_score_w = 0;

// Dead stones count (during scoring phase)
// m_dead_b = number of BLACK stones marked as dead
// m_dead_w = number of WHITE stones marked as dead
int m_dead_b = 0;
int m_dead_w = 0;

// Territory calculation
go_score get_scores() const;
void territory_from_markers();
void find_territory_units(const bit_array &w_stones, const bit_array &b_stones);
```

**Key Insight**: Territory markers are NOT copied during board state copies, but scores ARE preserved.

---

## 2. IGS Protocol Commands

### For Playing Games
```cpp
void qGoBoard::doDone() {
    if (id > 0)
        client_window->sendcommand("done", false);
}
```

**IGS "done" command**:
- Sent when player clicks "Done" button after marking dead stones
- Signals agreement with territory/dead stone assessment
- Server responds with final score

### Dead Stone Marking (qgo_interface.cpp:1288-1298)
```cpp
// Parse coordinate (e.g., "D4" -> column 3, row 3)
int i = pt[0].toLatin1() - 'A' + 1;
if (pt[0].toLatin1() > 'I')  // Skip 'I' in coordinates
    i--;

int j;
if (pt.length() > 2 && pt[2].toLatin1() >= '0' && pt[2].toLatin1() <= '9')
    j = qb->get_boardsize() + 1 - pt.mid(1,2).toInt();
else
    j = qb->get_boardsize() + 1 - pt[1].digitValue();

// Mark the stone as dead
qb->mark_dead_stone(i - 1, j - 1);

// Send kibitz notification
qb->send_kibitz("removing @ " + pt + "\n");
```

**Usage**:
- Click on stone group during scoring phase
- q5Go calculates group members
- Sends "removing @ <coord>" to server
- Updates local board visualization

---

## 3. Observing Mode

### IGS Command 15 Response Format
When observing a finished game, IGS sends Command 15 with:
```
15 <game_id> <white_score> <black_score> <result_string>
```

### Game Status Handling (qgo_interface.cpp:1301-1307)
```cpp
void qGoIF::slot_result(const QString &txt, const QString &line,
                        bool isplayer, const QString &komi)
{
    static qGoBoard *qb;
    static int column;

    if (isplayer)
        // ... handle playing mode
```

**Key Points**:
- Observers receive final score via Command 15
- Observers see territory markings if transmitted
- Cannot modify dead stone assessment (read-only view)

---

## 4. Territory Calculation Methods

### find_territory_units()
**Purpose**: Identify connected regions of empty intersections

**Algorithm**:
1. Create bit arrays for white stones and black stones
2. Find all empty regions (flood fill)
3. Determine which color controls each region
4. Count territory for scoring

### territory_from_markers()
**Purpose**: Convert visual territory markers to score calculation

**Process**:
- Scans board for `mark::terr` markers
- Accumulates territory count per color
- Updates `m_score_b` and `m_score_w`

### get_scores()
**Purpose**: Return current score state

**Returns**:
```cpp
go_score {
    .score_b = territory_black + captures_white - dead_black,
    .score_w = territory_white + captures_black - dead_white,
    .caps_b = captures_black,
    .caps_w = captures_white
};
```

---

## 5. Implementation Requirements for xgospel2

### For PLAYING Mode

#### UI Elements Needed
1. **"Done" Button**
   - Location: Board window toolbar
   - Action: Send "done" command to IGS
   - Enable: Only during scoring phase

2. **Dead Stone Toggle**
   - Click handler on board intersections
   - Toggle mark between normal and `dead`
   - Send "removing @ <coord>" to server

3. **Score Display**
   - Show current territory count
   - Display dead stone count
   - Update dynamically as stones marked

#### State Machine
```cpp
enum ScoringPhase {
    PLAYING,           // Normal game moves
    SCORING_MARKING,   // Marking dead stones
    SCORING_AGREED,    // Both players sent "done"
    FINISHED           // Final score determined
};
```

### For OBSERVING Mode

#### Requirements
1. **Read-only Display**
   - Show territory markers if present
   - Display final score from Command 15
   - No interaction during scoring

2. **Territory Visualization**
   - Semi-transparent overlay on empty regions
   - Different colors for B/W territory
   - Visual distinction from played stones

---

## 6. IGS Scoring Phase Protocol

### Sequence for Playing

```
1. Game ends (both players pass)
2. IGS: "Remove dead stones"
3. Player clicks stones → send "removing @ D4"
4. IGS updates board state
5. Player clicks "Done" → send "done"
6. IGS waits for opponent "done"
7. If disagreement: repeat from step 2
8. If agreement: IGS sends final score (Command 15)
```

### Sequence for Observing

```
1. Observer joins finished game
2. IGS sends move history (Command 15 "moves")
3. IGS sends final score (Command 15)
4. Board shows final position with territory marks
```

---

## 7. Visual Rendering Considerations

### Territory Markers (mark::terr)
```cpp
// Suggested rendering in GoBoardWidget::drawStones()
if (marker == mark::terr) {
    // Draw semi-transparent square
    QPainter painter;
    painter.setOpacity(0.3);

    QColor territoryColor;
    if (territory_owner == BLACK)
        territoryColor = Qt::black;
    else
        territoryColor = Qt::white;

    painter.fillRect(intersection_rect, territoryColor);
}
```

### Dead Stone Markers (mark::dead)
```cpp
// Draw X over dead stones
if (marker == mark::dead) {
    QPainter painter;
    painter.setPen(QPen(Qt::red, 2));

    // Draw diagonal cross
    painter.drawLine(x1, y1, x2, y2);
    painter.drawLine(x1, y2, x2, y1);
}
```

---

## 8. Key Code Locations in q5Go

### Files to Study
- `src/goboard.h` - Data structures (lines 33, 36, 105-112)
- `src/qgo_interface.cpp` - IGS protocol handling:
  - Line 1288-1298: Dead stone marking
  - Line 1301-1307: Result handling
  - Line 2227-2231: Done command
- `src/board.cpp` - Board rendering and interaction

### Critical Functions
1. `mark_dead_stone(int x, int y)` - Toggle dead marker
2. `get_scores()` - Calculate current score
3. `territory_from_markers()` - Convert markers to score
4. `find_territory_units()` - Identify territory regions
5. `doDone()` - Send done command

---

## 9. Testing Requirements

### Test Case 1: Playing Game Scoring
1. Play game until both pass
2. Mark dead stones by clicking groups
3. Verify "removing @ <coord>" sent to IGS
4. Click "Done" button
5. Verify "done" command sent
6. Wait for opponent done
7. Verify final score displayed correctly

### Test Case 2: Observing Finished Game
1. Observe completed game
2. Verify final score shown from Command 15
3. Verify territory markers displayed (if present)
4. Verify cannot interact with board

### Test Case 3: Disagreement Resolution
1. Mark different stones than opponent
2. Both click "Done"
3. IGS rejects (disagreement)
4. Re-mark stones
5. Click "Done" again
6. Repeat until agreement

---

## 10. Implementation Phases

### Phase 1: Basic Score Display (Observing)
- Parse Command 15 score data
- Display final score in board window
- Show basic territory visualization

### Phase 2: Dead Stone Marking (Playing)
- Add click handler for marking dead stones
- Implement mark toggle logic
- Send "removing @ <coord>" to IGS
- Visual feedback (red X on dead stones)

### Phase 3: Done Command (Playing)
- Add "Done" button to UI
- Enable only during scoring phase
- Send "done" command to IGS
- Handle response and final score display

### Phase 4: Territory Calculation (Both)
- Implement `find_territory_units()` algorithm
- Calculate score dynamically
- Display live score during marking phase
- Match q5Go territory visualization

---

## 11. Known Challenges

### Challenge 1: Detecting Scoring Phase
**Problem**: How to know when scoring phase begins?

**Solution**: IGS sends specific message after both players pass:
```
15 <game_id> {Game 123 I: weikkyu (0 0 0) vs opponent (0 0 0)}
```

Look for pattern indicating scoring has started.

### Challenge 2: Group Detection
**Problem**: Clicking one stone should mark entire connected group

**Solution**: Use flood-fill algorithm to find connected stones:
```cpp
void markDeadGroup(int x, int y) {
    stone_color color = board_state[x][y];
    std::queue<std::pair<int,int>> to_visit;
    std::set<std::pair<int,int>> visited;

    to_visit.push({x, y});

    while (!to_visit.empty()) {
        auto [cx, cy] = to_visit.front();
        to_visit.pop();

        if (visited.count({cx, cy})) continue;
        visited.insert({cx, cy});

        // Mark as dead
        board_state[cx][cy] |= DEAD_MARKER;

        // Check 4 neighbors
        for (auto [dx, dy] : {{-1,0}, {1,0}, {0,-1}, {0,1}}) {
            int nx = cx + dx, ny = cy + dy;
            if (nx >= 0 && nx < 19 && ny >= 0 && ny < 19 &&
                board_state[nx][ny] == color) {
                to_visit.push({nx, ny});
            }
        }
    }
}
```

### Challenge 3: Territory Auto-Detection
**Problem**: Should xgospel2 calculate territory automatically or wait for IGS?

**Recommendation**:
- **Playing mode**: Calculate locally for preview, but respect IGS final determination
- **Observing mode**: Use IGS-provided territory data only

---

## 12. Next Steps

Tomorrow's implementation should focus on:

1. **Read Command 15 response format** from IGS protocol docs
2. **Add scoring state machine** to BoardWindow class
3. **Implement basic dead stone marking** (click + group detection)
4. **Add "Done" button** to board window toolbar
5. **Parse and display final score** from Command 15
6. **Test with real IGS connection** (both playing and observing)

---

## References
- q5Go source: `/home/cahill/Claude_Projects/github/q5Go/src/`
- IGS Protocol: Need to capture actual Command 15 responses
- xgospel2 board: `/home/cahill/Claude_Projects/64-bit/xgospel2/board_window.cpp`
