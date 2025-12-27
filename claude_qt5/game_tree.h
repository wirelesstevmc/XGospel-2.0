#ifndef GAME_TREE_H
#define GAME_TREE_H

#include <QList>
#include <QString>

// StoneColor enum (also defined in board_window.h - must match!)
#ifndef STONE_COLOR_DEFINED
#define STONE_COLOR_DEFINED
enum StoneColor {
    EMPTY = 0,
    BLACK_STONE = 1,
    WHITE_STONE = 2,
    EMPTY_STONE = 0
};
#endif

// Simple 19x19 board representation that stores complete board state
class GoBoard {
public:
    GoBoard();
    void clear();
    void placeStone(int x, int y, StoneColor color);
    void removeStone(int x, int y);
    StoneColor getStone(int x, int y) const;
    GoBoard copy() const;
    bool isEmpty() const;

private:
    StoneColor board[19][19];
};

// Game tree node - represents a single position in the game tree
// Similar to q5Go's game_state class
class GameNode {
public:
    GameNode(GameNode* parent = nullptr, int move_num = 0);
    ~GameNode();

    // Navigation (following q5Go's pattern)
    GameNode* nextMove();               // Get active child (main variation)
    GameNode* prevMove();               // Get parent
    GameNode* getChild(int index);      // Get specific child variation
    int childCount() const;
    bool isRoot() const;

    // Properties
    int moveNumber() const { return m_move_number; }
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    StoneColor getColor() const { return m_color; }
    const GoBoard& getBoard() const { return m_board; }
    QString getComment() const { return m_comment; }

    // Setters
    void setBoard(const GoBoard& board);
    void setComment(const QString& comment);
    void setMoveData(int x, int y, StoneColor color);

    // Tree building
    GameNode* addMove(int x, int y, StoneColor color);
    void makeActive();  // Mark this node's path as the active variation

    // Find maximum move number in active variation
    int activeVariationMax() const;

private:
    int m_move_number;
    int m_x, m_y;                       // Move coordinates (-1,-1 for pass, -2,N for handicap)
    StoneColor m_color;
    GoBoard m_board;                    // Complete board state at this node

    GameNode* m_parent;
    QList<GameNode*> m_children;
    int m_active_child;                 // Which child is the active variation

    QString m_comment;                  // Optional comment for this move
};

#endif // GAME_TREE_H
