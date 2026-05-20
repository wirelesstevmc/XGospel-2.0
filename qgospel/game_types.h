#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QSet>
#include <QtCore/QPair>

// StoneColor must come from game_tree.h (defined there with include guard)
#include "game_tree.h"

// Game modes (inspired by q5Go)
enum GameMode {
    MODE_NORMAL = 0,
    MODE_EDIT   = 1,
    MODE_SCORE  = 2
};

// Shared group-tracking struct used by both BoardWindow and GameSlot
struct StoneGroup {
    QSet<QPair<int, int>> stones;
    int  liberties;
    bool alive;
    StoneGroup(const QSet<QPair<int, int>> &s, int lib)
        : stones(s), liberties(lib), alive(true) {}
};

struct GameMove {
    int        game_id;
    int        move_number;
    StoneColor color;
    int        x, y;
    QString    time_info;
    QString    captured;
    bool       is_live_move;
    QDateTime  received_time;
};

#endif // GAME_TYPES_H
