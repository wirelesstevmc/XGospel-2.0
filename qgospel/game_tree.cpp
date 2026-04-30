#include "game_tree.h"
#include <QDebug>

// ============================================================================
// GoBoard Implementation
// ============================================================================

GoBoard::GoBoard() {
    clear();
}

void GoBoard::clear() {
    for (int x = 0; x < 19; x++) {
        for (int y = 0; y < 19; y++) {
            board[x][y] = EMPTY_STONE;
        }
    }
}

void GoBoard::placeStone(int x, int y, StoneColor color) {
    if (x >= 0 && x < 19 && y >= 0 && y < 19) {
        board[x][y] = color;
    }
}

void GoBoard::removeStone(int x, int y) {
    if (x >= 0 && x < 19 && y >= 0 && y < 19) {
        board[x][y] = EMPTY_STONE;
    }
}

StoneColor GoBoard::getStone(int x, int y) const {
    if (x >= 0 && x < 19 && y >= 0 && y < 19) {
        return board[x][y];
    }
    return EMPTY_STONE;
}

GoBoard GoBoard::copy() const {
    GoBoard result;
    for (int x = 0; x < 19; x++) {
        for (int y = 0; y < 19; y++) {
            result.board[x][y] = board[x][y];
        }
    }
    return result;
}

bool GoBoard::isEmpty() const {
    for (int x = 0; x < 19; x++) {
        for (int y = 0; y < 19; y++) {
            if (board[x][y] != EMPTY_STONE) {
                return false;
            }
        }
    }
    return true;
}

// ============================================================================
// GameNode Implementation
// ============================================================================

GameNode::GameNode(GameNode* parent, int move_num)
    : m_move_number(move_num),
      m_x(-1),
      m_y(-1),
      m_color(EMPTY_STONE),
      m_edited(false),
      m_parent(parent),
      m_active_child(0)
{
}

GameNode::~GameNode() {
    // Recursively delete all children
    for (GameNode* child : m_children) {
        delete child;
    }
    m_children.clear();
}

GameNode* GameNode::nextMove() {
    if (m_children.isEmpty()) {
        return nullptr;
    }
    if (m_active_child >= 0 && m_active_child < m_children.size()) {
        return m_children[m_active_child];
    }
    return nullptr;
}

GameNode* GameNode::prevMove() {
    return m_parent;
}

GameNode* GameNode::getChild(int index) {
    if (index >= 0 && index < m_children.size()) {
        return m_children[index];
    }
    return nullptr;
}

int GameNode::childCount() const {
    return m_children.size();
}

bool GameNode::isRoot() const {
    return m_parent == nullptr;
}

void GameNode::setBoard(const GoBoard& board) {
    m_board = board.copy();
}

void GameNode::setComment(const QString& comment) {
    m_comment = comment;
}

void GameNode::setTerritoryMap(const QMap<QPair<int,int>, StoneColor>& territory) {
    m_territory = territory;
}

void GameNode::setMoveData(int x, int y, StoneColor color) {
    m_x = x;
    m_y = y;
    m_color = color;
}

GameNode* GameNode::addMove(int x, int y, StoneColor color) {
    // DIAGNOSTIC: Track every move added to game tree (commented out for cleaner output)
    // QString color_str = (color == BLACK_STONE) ? "BLACK" : (color == WHITE_STONE) ? "WHITE" : "INVALID";
    // QString coord_str;
    // if (x == -1 && y == -1) {
    //     coord_str = "PASS";
    // } else if (x == -2) {
    //     coord_str = QString("HANDICAP(y=%1)").arg(y);
    // } else {
    //     coord_str = QString("(%1,%2)").arg(x).arg(y);
    // }
    // qDebug() << "🎯 GAME_TREE::addMove() called - Move#" << (m_move_number + 1)
    //          << color_str << coord_str;

    // Create new child node with incremented move number
    GameNode* new_node = new GameNode(this, m_move_number + 1);
    new_node->setMoveData(x, y, color);

    // Add to children list
    m_children.append(new_node);

    // Make this the active variation
    m_active_child = m_children.size() - 1;

    return new_node;
}

void GameNode::makeActive() {
    // Mark this node's path as the active variation by updating all ancestors
    GameNode* p = m_parent;
    GameNode* prev = this;

    while (p != nullptr) {
        // Find which child we are and mark it as active
        int n = p->m_children.size();
        for (int i = 0; i < n; i++) {
            if (p->m_children[i] == prev) {
                p->m_active_child = i;
                break;
            }
        }
        prev = p;
        p = p->m_parent;
    }
}

int GameNode::activeVariationMax() const {
    // Follow the active variation to the end and return its move number
    const GameNode* st = this;
    while (!st->m_children.isEmpty() && st->m_active_child >= 0 && st->m_active_child < st->m_children.size()) {
        st = st->m_children[st->m_active_child];
    }
    return st->m_move_number;
}
