#include "game_slot.h"

GameSlot::GameSlot(QObject *parent)
    : QObject(parent)
{
    game_root    = new GameNode();
    current_node = game_root;

    clock_timer = new QTimer(this);
    clock_timer->setInterval(1000);
    connect(clock_timer, &QTimer::timeout, this, &GameSlot::clockTick);
    clock_timer->start();

    initBoardState();
}

GameSlot::~GameSlot()
{
    clock_timer->stop();
    delete game_root;   // recursively deletes entire tree
    game_root    = nullptr;
    current_node = nullptr;
}

void GameSlot::reset()
{
    // Identity
    game_id    = 0;
    white_player.clear();
    black_player.clear();
    white_rank.clear();
    black_rank.clear();
    my_username.clear();
    custom_game_title.clear();

    is_observing          = false;
    is_playing            = false;
    is_scoring_mode       = false;
    is_edit_window        = false;
    in_edit_position_mode = false;

    game_start_time       = QDateTime();
    game_mode             = 0;   // MODE_NORMAL
    observation_state     = NOT_OBSERVING;
    observation_start_time = QDateTime();
    moves_received_during_live = 0;

    // Board state
    board_size   = 19;
    last_move_x  = -1;
    last_move_y  = -1;
    current_player = 1;   // BLACK_STONE
    initBoardState();
    territory_map.clear();
    dead_stone_positions.clear();
    disputed_positions.clear();

    // Move history & game tree
    replay_state        = WAITING_FOR_MOVES0;
    catchup_high        = -1;
    current_move        = 0;
    current_move_index  = 0;
    server_move_count   = 0;
    consecutive_passes  = 0;
    auto_follow_mode    = true;
    slider_update_in_progress = false;

    move_history.clear();
    delete game_root;
    game_root    = new GameNode();
    current_node = game_root;

    // Game setup
    handicap         = 0;
    komi             = 6.5;
    time_control.clear();
    game_type.clear();
    game_type_locked = false;
    byoyomi_time     = 0;

    // Clocks & captures
    white_time_seconds = 0;
    black_time_seconds = 0;
    white_byo_moves    = -1;
    black_byo_moves    = -1;
    last_time_update   = QDateTime();
    white_captures     = 0;
    black_captures     = 0;

    // Scoring
    dead_stones.clear();
    white_territory    = 0;
    black_territory    = 0;
    white_prisoners    = 0;
    black_prisoners    = 0;
    final_score        = 0.0;
    server_white_score = 0.0;
    server_black_score = 0.0;
    has_server_score   = false;
    receiving_territory_data = false;
    territory_data_row = 0;
    territory_ownership.clear();

    // Result
    game_result.clear();
    game_finished = false;

    // Comments & observers
    comments.clear();
    system_messages.clear();
    observers.clear();

    // Group tracking
    white_groups.clear();
    black_groups.clear();

    // Audit trail
    move_audit_trail.clear();
}

void GameSlot::initBoardState()
{
    for (int x = 0; x < 19; ++x)
        for (int y = 0; y < 19; ++y)
            board_state[x][y] = 0;   // EMPTY
}

#include "game_slot.moc"
