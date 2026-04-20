# Move History Slider Fix - Complete Analysis and Implementation Plan

## Problem Statement

The move history slider in xgospel2 is currently broken because it **clears the board and loses game history** when navigating backward. This is fundamentally different from q5Go's robust implementation.

## Current xgospel2 Implementation (BROKEN)

### What Happens Now (board_window.cpp:2186-2225)

```cpp
void BoardWindow::goToMove(int move_index) {
    // PROBLEM 1: Clears the board completely
    board_widget->clearBoard();
    white_groups.clear();
    black_groups.clear();

    // PROBLEM 2: Replays moves from scratch
    for (int i = 0; i < move_index; i++) {
        // Replays each move...
    }
}
```

**Why This Fails:**
1. **Clears the board** - Loses all state
2. **Replays from move 0** - Inefficient and error-prone
3. **No permanent game tree** - move_history is just a linear list
4. **Loses future moves** - When you go back, future moves disappear

## q5Go Implementation (CORRECT)

### Core Architecture

q5Go uses an **immutable game tree** structure where:
- Each `game_state` node contains complete board position
- Nodes are never modified, only traversed
- Parent/child pointers preserve the entire game
- Navigation just changes which node is "displayed"

### Key Data Structure (from q5Go gogame.h)

```cpp
class game_state {
private:
    int m_move_number;                      // This node's move number
    std::vector<game_state *> m_children;   // Child variations
    size_t m_active = 0;                    // Which child is active
    game_state *m_parent;                   // Parent node
    go_board m_board;                       // Complete board state at this node
    stone_color m_to_move;                  // Color to move next
    // ... comments, analysis, figures, etc.
};
```

### Navigation Methods

| Method | Purpose |
|--------|---------|
| `next_move()` | Returns m_children[m_active] |
| `prev_move()` | Returns m_parent |
| `move_number()` | Returns this node's number |
| `root_node_p()` | Checks if this is the root |

### Slider Change Handler (mainwindow.cpp:904-916)

```cpp
void MainWindow::nav_goto_nth_move_in_var(int n) {
    game_state *st = ui->gfx_board->displayed();

    while (st->move_number() != n) {
        game_state *next = st->move_number() < n ?
                           st->next_move() : st->prev_move();
        if (next == nullptr) break;
        st = next;
    }
    set_displayed(st);  // Only changes what's displayed!
}
```

**Key Insight:** The tree is NEVER modified. Only the "displayed" pointer changes.

## Implementation Plan for xgospel2

### Phase 1: Create Game Tree Data Structure

Create new file: `game_tree.h` and `game_tree.cpp`

```cpp
// game_tree.h
#ifndef GAME_TREE_H
#define GAME_TREE_H

#include <QList>
#include <QString>
#include "game_structs.h"

// Simplified 19x19 board representation
class GoBoard {
public:
    GoBoard();
    void clear();
    void placeStone(int x, int y, StoneColor color);
    StoneColor getStone(int x, int y) const;
    GoBoard copy() const;

private:
    StoneColor board[19][19];
};

// Game tree node (similar to q5Go's game_state)
class GameNode {
public:
    GameNode(GameNode* parent = nullptr);
    ~GameNode();

    // Navigation
    GameNode* nextMove();               // Get active child
    GameNode* prevMove();               // Get parent
    GameNode* getChild(int index);      // Get specific child
    int childCount() const;

    // Properties
    int moveNumber() const { return m_move_number; }
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    StoneColor getColor() const { return m_color; }
    const GoBoard& getBoard() const { return m_board; }

    // Tree building
    GameNode* addMove(int x, int y, StoneColor color);
    void setBoard(const GoBoard& board);

private:
    int m_move_number;
    int m_x, m_y;                       // Move coordinates (-1,-1 for pass)
    StoneColor m_color;
    GoBoard m_board;                    // Complete board state at this node

    GameNode* m_parent;
    QList<GameNode*> m_children;
    int m_active_child;                 // Which variation is active

    QString m_comment;                  // Optional comment
};

#endif // GAME_TREE_H
```

### Phase 2: Modify BoardWindow to Use Game Tree

**Changes to board_window.h:**

```cpp
class BoardWindow : public QMainWindow {
    Q_OBJECT

private:
    // OLD (remove these):
    // QList<GameMove> move_history;
    // int current_move_index;

    // NEW (add these):
    GameNode* game_root;           // Root of game tree
    GameNode* current_node;        // Currently displayed node
    bool slider_update_in_progress; // Prevent recursive signals
};
```

**Changes to board_window.cpp:**

```cpp
// Constructor
BoardWindow::BoardWindow(QWidget *parent, const QString &username)
    : QMainWindow(parent),
      game_root(new GameNode()),
      current_node(game_root),
      slider_update_in_progress(false)
{
    // ... existing code ...
}

// Destructor
BoardWindow::~BoardWindow() {
    delete game_root;  // This will recursively delete all child nodes
}

// When a new move arrives from IGS
void BoardWindow::processMove(const GameMove& move) {
    // Create new node as child of current position
    GameNode* new_node = current_node->addMove(move.x, move.y, move.color);

    // Calculate board state for new node
    GoBoard new_board = current_node->getBoard().copy();
    new_board.placeStone(move.x, move.y, move.color);
    // ... calculate captures ...
    new_node->setBoard(new_board);

    // Move to new node
    current_node = new_node;

    // Update slider WITHOUT triggering valueChanged signal
    slider_update_in_progress = true;
    move_slider->setMaximum(getTotalMoves());
    move_slider->setValue(current_node->moveNumber());
    slider_update_in_progress = false;

    // Update display
    displayNode(current_node);
}

// Navigate to specific move
void BoardWindow::goToMove(int move_number) {
    if (slider_update_in_progress) return;  // Prevent recursion

    GameNode* target = current_node;

    // Navigate backward if needed
    while (target->moveNumber() > move_number && target->prevMove()) {
        target = target->prevMove();
    }

    // Navigate forward if needed
    while (target->moveNumber() < move_number && target->nextMove()) {
        target = target->nextMove();
    }

    // Update current position
    current_node = target;

    // Display this node's board state
    displayNode(current_node);
}

// Display a node's position
void BoardWindow::displayNode(GameNode* node) {
    // Simply set the board to match this node's stored state
    const GoBoard& board = node->getBoard();

    board_widget->clearBoard();
    for (int x = 0; x < 19; x++) {
        for (int y = 0; y < 19; y++) {
            StoneColor color = board.getStone(x, y);
            if (color != EMPTY_STONE) {
                board_widget->placeMoveAt(x, y, color);
            }
        }
    }

    // Update move number label
    move_number_label->setText(QString("Move: %1/%2")
        .arg(node->moveNumber())
        .arg(getTotalMoves()));
}

// Helper to get total moves in active variation
int BoardWindow::getTotalMoves() const {
    GameNode* node = game_root;
    while (node->nextMove()) {
        node = node->nextMove();
    }
    return node->moveNumber();
}
```

### Phase 3: Update Slider Handler

```cpp
void BoardWindow::onMoveSliderChanged(int value) {
    if (slider_update_in_progress) return;  // Prevent infinite loop
    goToMove(value);
}
```

## Benefits of This Approach

1. **✅ Game history is permanent** - Tree structure is never destroyed
2. **✅ Fast navigation** - Just traverse pointers, no replay needed
3. **✅ Future moves preserved** - Going back doesn't lose anything
4. **✅ Supports variations** - Can add branches later for game analysis
5. **✅ Efficient** - Each node stores complete board state (no replay)
6. **✅ Matches q5Go** - Same architecture as proven implementation

## Testing Plan

1. Observe a game from move 1
2. Let it progress to move 50
3. Move slider back to move 25
4. Verify board shows correct position
5. Move slider forward to move 30
6. Verify board still correct
7. Continue observing live moves
8. Verify new moves append correctly
9. Verify slider maximum updates
10. Verify navigation still works after new moves arrive

## Migration Strategy

**Step 1:** Implement GameNode and GoBoard classes
**Step 2:** Add game_root and current_node to BoardWindow
**Step 3:** Modify processMove() to build tree instead of list
**Step 4:** Rewrite goToMove() to navigate tree
**Step 5:** Update displayNode() to render from stored board
**Step 6:** Test thoroughly with live observation
**Step 7:** Remove old move_history list code

## Compatibility Notes

- Keep existing processMove() signature for IGS parsing
- SGF save can still iterate through tree (traverse from root)
- Capture calculation only needed when creating new nodes
- Existing board_widget methods still work (just used differently)

## Next Steps After This Fix

1. Territory marking (future feature)
2. Configuration tool for servers/accounts
3. Multiple variation support (already built into tree structure!)
4. Game analysis features (comments, variations)

---

**Status:** Ready for implementation
**Priority:** High - This is a core feature for game observation
**Estimated Complexity:** Medium - Clean architecture, well-defined scope
