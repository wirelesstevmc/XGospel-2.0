#ifndef GAME_SLOT_H
#define GAME_SLOT_H

#include <QtCore/QTimer>
#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtCore/QObject>

#include "game_types.h"     // GameMode, GameMove, StoneColor (no board_window.h dependency)
#include "game_tree.h"
#include "igs_move_parser.h"

// ---------------------------------------------------------------------------
// GameSlot — owns all per-game state for one observed (or played) game.
//
// In docked-pane mode there is one shared BoardWindow UI shell and one
// GameSlot per game.  The slot is the single source of truth for that game's
// state whether or not it is currently displayed.  In non-docked mode GameSlot
// is never instantiated — all state lives in BoardWindow as before.
// ---------------------------------------------------------------------------

class GameSlot : public QObject {
    Q_OBJECT

public:
    // -----------------------------------------------------------------------
    // Nested types (mirror BoardWindow's nested types so callers need only
    // include game_slot.h when working with slot data)
    // -----------------------------------------------------------------------

    enum ObservationState {
        NOT_OBSERVING  = 0,
        JOINING_GAME   = 1,
        RECONSTRUCTING = 2,
        LIVE_OBSERVATION = 3
    };

    struct CommentEntry {
        QString user;
        QString text;
        bool    is_kibitz;
    };

    struct ObserverEntry {
        QString name;
        QString rank;
    };

    struct MoveAudit {
        int     move_number;
        QString server_input;
        QString parsed_coords;
        int     final_x, final_y;
        StoneColor color;
        QString captures;
        QString timestamp;
        bool    placement_success;
    };

    static const int MAX_AUDIT_ENTRIES = 50;

    // -----------------------------------------------------------------------
    // Lifecycle
    // -----------------------------------------------------------------------

    explicit GameSlot(QObject *parent = nullptr);
    ~GameSlot();

    // Reset all fields to initial state — call before re-observing the same
    // game ID (rare but possible if IGS reuses a number).
    void reset();

    // -----------------------------------------------------------------------
    // Identity & observation state
    // -----------------------------------------------------------------------

    int              game_id          = 0;
    QString          white_player;
    QString          black_player;
    QString          white_rank;
    QString          black_rank;
    QString          my_username;
    QString          custom_game_title;   // non-empty for teaching games

    bool             is_observing        = false;
    bool             is_playing          = false;
    bool             is_scoring_mode     = false;
    bool             is_edit_window      = false;   // always false for observed slots
    bool             in_edit_position_mode = false;

    QDateTime        game_start_time;
    int              game_mode           = 0;   // GameMode enum value (MODE_NORMAL = 0)

    ObservationState observation_state   = NOT_OBSERVING;
    QDateTime        observation_start_time;
    int              moves_received_during_live = 0;

    // -----------------------------------------------------------------------
    // Board state snapshot
    // Flat copy of GoBoardWidget internals — not a pointer to widget state.
    // Updated by FixedXGospelWindow after every move, and by snapshotToSlot()
    // when switching away from this game.
    // -----------------------------------------------------------------------

    int              board_size          = 19;
    int              board_state[19][19];           // EMPTY/BLACK_STONE/WHITE_STONE
    int              last_move_x         = -1;
    int              last_move_y         = -1;
    int              current_player      = 1;       // StoneColor: BLACK_STONE = 1

    // GoBoardWidget scoring overlays (restored into the widget on loadSlot)
    QMap<QPair<int,int>, int>         territory_map;        // StoneColor value per point
    QSet<QPair<int,int>>              dead_stone_positions; // widget-level dead markers
    QSet<QPair<int,int>>              disputed_positions;   // seki / false-eye points

    // -----------------------------------------------------------------------
    // Move history & game tree
    // -----------------------------------------------------------------------

    // Per-slot replay state machine (replaces global history_replay_game_id pin)
    enum ReplayState {
        WAITING_FOR_MOVES0, // catch-up flood phase: buffer moves, track high-water mark
        REPLAYING,          // moves N history replay from move 0: apply all moves
        LIVE                // replay done: apply live moves, drop re-sends <= catchup_high
    };
    ReplayState        replay_state            = WAITING_FOR_MOVES0;
    int                catchup_high            = -1;   // highest move_number seen in catch-up flood
    QList<GameMove>    pending_catchup_moves;           // catch-up moves buffered before history arrives

    int              current_move        = 0;
    int              current_move_index  = 0;   // move slider position
    int              server_move_count   = 0;
    int              consecutive_passes  = 0;
    bool             auto_follow_mode    = true;
    bool             slider_update_in_progress = false;

    QList<GameMove>  move_history;
    GameNode        *game_root    = nullptr;    // owns the entire game tree
    GameNode        *current_node = nullptr;    // currently displayed node

    // -----------------------------------------------------------------------
    // Game setup
    // -----------------------------------------------------------------------

    int              handicap            = 0;
    double           komi                = 6.5;
    QString          time_control;
    QString          game_type;          // "Free" or "Rated"
    bool             game_type_locked    = false;
    int              byoyomi_time        = 0;

    // -----------------------------------------------------------------------
    // Clocks & captures
    // -----------------------------------------------------------------------

    int              white_time_seconds  = 0;
    int              black_time_seconds  = 0;
    int              white_byo_moves     = -1;
    int              black_byo_moves     = -1;
    QDateTime        last_time_update;
    int              white_captures      = 0;
    int              black_captures      = 0;

    // -----------------------------------------------------------------------
    // Scoring / counting phase
    // -----------------------------------------------------------------------

    QSet<QPair<int,int>>              dead_stones;          // engine-level dead set
    int              white_territory     = 0;
    int              black_territory     = 0;
    int              white_prisoners     = 0;
    int              black_prisoners     = 0;
    double           final_score         = 0.0;
    double           server_white_score  = 0.0;
    double           server_black_score  = 0.0;
    bool             has_server_score    = false;
    bool             receiving_territory_data = false;
    int              territory_data_row  = 0;
    QMap<QPair<int,int>, int>         territory_ownership;  // 4=white, 5=black (IGS)

    // -----------------------------------------------------------------------
    // Game result
    // -----------------------------------------------------------------------

    QString          game_result;
    bool             game_finished       = false;
    QString          adjourned_player;   // set on CMD48; used to compute resign result if not resumed

    // -----------------------------------------------------------------------
    // Comments — full accumulated history for this game
    // -----------------------------------------------------------------------

    QList<CommentEntry> comments;
    QString             comment_html;   // Full rendered HTML snapshot of comment_display

    // -----------------------------------------------------------------------
    // Observers — current list for this game
    // -----------------------------------------------------------------------

    QList<ObserverEntry> observers;

    // -----------------------------------------------------------------------
    // Group tracking (capture calculation, mirrors BoardWindow)
    // -----------------------------------------------------------------------

    QList<StoneGroup> white_groups;
    QList<StoneGroup> black_groups;

    // -----------------------------------------------------------------------
    // Audit trail (circular buffer, MAX_AUDIT_ENTRIES)
    // -----------------------------------------------------------------------

    QList<MoveAudit> move_audit_trail;

    // -----------------------------------------------------------------------
    // Clock timer — always ticking regardless of which game is displayed.
    // Timeout signal: connected to a lambda in FixedXGospelWindow that
    // decrements this slot's time fields.  When this slot is the active one,
    // the lambda also calls BoardWindow::updateClockDisplay().
    // -----------------------------------------------------------------------

    QTimer *clock_timer = nullptr;

signals:
    // Emitted every second by clock_timer so FixedXGospelWindow can update
    // this slot's time fields and, if active, the board window display.
    void clockTick();

private:
    void initBoardState();  // zero-fill board_state array
};

#endif // GAME_SLOT_H
