#ifndef SCORE_ENGINE_H
#define SCORE_ENGINE_H

#include <QtCore/QMap>
#include <QtCore/QSet>
#include <QtCore/QPair>

#include "game_tree.h"   // GoBoard, StoneColor

enum class ScoringMethod {
    Simple,   // Flood-fill only — fast, ignores seki/false-eyes
    Complex   // False-eye detection + dead-stone propagation + seki exclusion
};

class ScoreEngine {
public:
    /* Estimate territory from a board position.
       territory_out  : empty intersections mapped to owner (BLACK_STONE or WHITE_STONE)
       dead_stones_out: positions of dead stones (user-marked; passed in so engine can exclude them)
       disputed_out   : intersections the engine cannot assign (seki, false-eyes); draw neutral marker
       black_score    : black territory intersection count (not including prisoners or komi)
       white_score    : white territory intersection count
       method         : Simple (default) or Complex */
    static void estimate(const GoBoard &board,
                         QMap<QPair<int,int>, StoneColor> &territory_out,
                         QSet<QPair<int,int>>             &dead_stones_out,
                         QSet<QPair<int,int>>             &disputed_out,
                         int &black_score,
                         int &white_score,
                         ScoringMethod method = ScoringMethod::Simple);
};

#endif // SCORE_ENGINE_H
