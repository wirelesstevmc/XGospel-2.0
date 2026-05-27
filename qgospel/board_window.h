#ifndef BOARD_WINDOW_H
#define BOARD_WINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFrame>
#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtGui/QPaintEvent>
#include <QtCore/QDateTime>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QScrollArea>

// game_types.h provides StoneColor, GameMode, GameMove — must come before game_tree.h
#include "game_types.h"
#include "stone_renderer.h"
#include "game_selection_dock.h"
#include "game_slot.h"

// Horizontal game tree navigation strip (linear games only for now)
class GameTreeStrip : public QWidget {
    Q_OBJECT

    int m_node_size = 24;       // Pixel width/height of each node icon
    int m_active_index = 0;       // Currently highlighted move index
    QList<StoneColor> m_moves;    // Color sequence (EMPTY = root/setup node)
    QList<bool> m_edited;         // Whether each node was user-edited

public:
    GameTreeStrip(QWidget *parent = nullptr);

    void setMoves(const QList<StoneColor> &moves, int active_index,
                  const QList<bool> &edited = QList<bool>());
    void setActiveIndex(int index);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void nodeClicked(int index);
};

class GoBoardWidget : public QFrame {
    Q_OBJECT

private:
    int board_size;
    int **board_state;  // 2D array for board state
    int margin;
    int cell_size;
    int stone_size;  // Actual generated stone diameter (96% of cell_size)
    int last_move_x, last_move_y;        // Last original game move (red marker)
    int last_edit_x, last_edit_y;        // Last user-edited stone (blue marker)
    bool show_coordinates;

    // Scoring mode visualization
    bool scoring_mode_enabled;
    QMap<QPair<int, int>, StoneColor> territory_map;  // Empty points and their territory owner
    QSet<QPair<int, int>> dead_stone_positions;
    QSet<QPair<int, int>> disputed_positions;          // Seki / false-eye points (complex scoring)

    // Board texture (xgospel 1.X style)
    QPixmap board_texture;

    // q5Go-style stone rendering
    StoneRenderer *stone_renderer;

    // Edit mode support
    GameMode game_mode;
    int mouse_down_x, mouse_down_y;  // Anti-clicko support
    StoneColor next_player_color;  // Track which color to place next in edit mode
    int hover_x, hover_y;          // Current hover position (-1 when outside board)

public:
    GoBoardWidget(QWidget *parent = nullptr);
    ~GoBoardWidget();
    
    void setBoardSize(int size);
    void placeMoveAt(int x, int y, StoneColor color);
    StoneColor getStoneAt(int x, int y) const;
    void clearBoard();
    void clearHover();
    void setLastMove(int x, int y);
    void setLastEditMove(int x, int y);  // Blue marker for user-edited stones
    void clearEditMarker() { last_edit_x = last_edit_y = -1; update(); }
    int getLastEditX() const { return last_edit_x; }
    int getLastEditY() const { return last_edit_y; }
    void setShowCoordinates(bool show) { show_coordinates = show; update(); }
    StoneColor getBoardState(int x, int y) const;
    
    // Scoring mode visualization
    void setScoringMode(bool enabled) { scoring_mode_enabled = enabled; update(); }
    bool getScoringModeEnabled() const { return scoring_mode_enabled; }
    void setTerritoryMap(const QMap<QPair<int, int>, StoneColor> &map) { territory_map = map; update(); }
    const QMap<QPair<int, int>, StoneColor> &getTerritoryMap() const { return territory_map; }
    void setDeadStones(const QSet<QPair<int, int>> &dead_stones);
    void setDisputedPoints(const QSet<QPair<int, int>> &disputed) { disputed_positions = disputed; update(); }

    // Accessors for snapshotToSlot
    int getBoardSize()    const { return board_size; }
    int getLastMoveX()    const { return last_move_x; }
    int getLastMoveY()    const { return last_move_y; }
    const QSet<QPair<int,int>>& getDeadStonePositions() const { return dead_stone_positions; }
    const QSet<QPair<int,int>>& getDisputedPositions()  const { return disputed_positions; }

    // Edit mode
    void setGameMode(GameMode mode) { game_mode = mode; hover_x = hover_y = -1; if (mode != MODE_EDIT) { last_edit_x = last_edit_y = -1; } update(); }
    GameMode getGameMode() const { return game_mode; }
    void removeStoneAt(int x, int y);  // Remove stone during edit
    void setNextPlayerColor(StoneColor color) { next_player_color = color; }
    StoneColor getNextPlayerColor() const { return next_player_color; }

    // Off-screen render for hover popup previews.
    // Accepts an external board state array so it works for non-active game slots
    // without disturbing the live widget state.
    QPixmap renderToPixmap(int size,
                           const int ext_board_state[19][19],
                           int ext_board_size,
                           int ext_last_move_x, int ext_last_move_y,
                           const QString &title_overlay = QString()) const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void calculateSizes();
    void drawBoard(QPainter &painter);
    void drawStones(QPainter &painter);
    void drawCoordinates(QPainter &painter);
    void drawLastMoveMarker(QPainter &painter);
    void drawHoverCursor(QPainter &painter);
    void drawTerritoryMarkers(QPainter &painter);
    void drawDeadStoneMarkers(QPainter &painter);
    void drawDisputedMarkers(QPainter &painter);
    QPoint boardToScreen(int x, int y);
    QPoint screenToBoard(int px, int py);

signals:
    void boardClicked(int x, int y);
};

class BoardWindow : public QMainWindow {
    Q_OBJECT

private:
    GoBoardWidget *board_widget;

    // Compact game info labels
    QLabel *teaching_title_label;     // Teaching game title (xgospel style - separate pane)
    QLabel *game_info_label;          // Game# + Next player (combined for space efficiency)
    QLabel *handicap_komi_label;      // Komi, Handicap, Rated/Free, Captures (combined)

    // Player info groups (q5Go style - name/rank, clock, and captures)
    QLabel      *white_stone_icon;        // Small white stone image (q5Go style)
    QPushButton *white_player_label;     // Name + rank (clickable → player dialog)
    QLabel      *white_clock_label;       // Clock display (large font)
    QLabel      *white_captures_label;    // Capture count
    QLabel      *black_stone_icon;        // Small black stone image (q5Go style)
    QPushButton *black_player_label;     // Name + rank (clickable → player dialog)
    QLabel      *black_clock_label;       // Clock display (large font)
    QLabel      *black_captures_label;    // Capture count
    QLabel *to_play_stone_icon;       // Dynamic stone indicator for "to play" (q5Go style)
    QPixmap icon_black_pixmap;        // Cached 20px black stone icon
    QPixmap icon_white_pixmap;        // Cached 20px white stone icon

    QPushButton *save_button;          // Save game to SGF
    QPushButton *edit_button;          // Edit/Analyze button (q5Go style)
    QPushButton *resign_button;
    QPushButton *done_button;
    QPushButton *close_button;

    // Splitters for resizable panels
    QSplitter *main_splitter;          // Horizontal: board vs info panel
    QSplitter *right_splitter;         // Vertical: info/comments/observers
    QSplitter *info_splitter;          // Horizontal: player info vs analysis pane

    // Dockable game selection pane (non-null only when use_docked_game_pane = true)
    GameSelectionDock *game_selection_dock = nullptr;

    // Move navigation controls
    QPushButton *first_move_button;
    QPushButton *prev_move_button;
    QPushButton *next_move_button;
    QPushButton *last_move_button;
    QSlider *move_slider;
    QLabel *move_number_label;
    int current_move_index;
    
    // Comment/Kibitz panel
    QTextEdit *comment_display;
    QLineEdit *comment_input;
    QPushButton *send_comment_button;
    
    // Observers panel
    QListWidget *observers_list;
    QLabel      *observers_title;
    QPushButton *observers_sort_btn;
    bool         observers_sort_by_rank = true;  // true=by strength, false=by join order
    QList<QPair<QString,QString>> observers_raw;  // {name, rank} in join order
    
    // Game state
    int observed_game_id;
    QString white_player, black_player;
    QString white_rank, black_rank;
    QString my_username;  // Logged-in user's name
    QString custom_game_title;
    int current_move;
    StoneColor current_player;
    bool is_observing;
    bool is_playing;
    bool is_scoring_mode;
    QDateTime game_start_time;
    GameMode game_mode;  // Current game mode (normal/edit/score)
    bool is_edit_window;  // True if this is an edit/analysis window (child of observe window)
    bool is_shared_window = false; // True when shared across multiple docked games — X closes active game only
    bool in_edit_position_mode;  // True when Edit Position (free placement) is active
    bool local_play_mode = false; // True when playing a local engine game (no IGS)
    bool engine_ready    = false; // True after engineReady() fires — gates board clicks

    // Engine console panel (local play mode only)
    QFrame    *engine_console_panel;
    QLabel    *engine_status_label;
    QPushButton *engine_go_btn;    // "Engine: Go" — triggers genmove
    QPushButton *engine_clear_btn; // "Clear Board" — sends clear_board
    QTextEdit *engine_log;
    QLineEdit *engine_cmd_input;
    QPushButton *engine_send_btn;

    // Game tree strip (edit window only)
    GameTreeStrip *game_tree_strip;
    QScrollArea  *game_tree_scroll;

    // Edit window button panel (only used in edit windows)
    QPushButton *update_button;
    QPushButton *pass_button;
    QPushButton *score_button;
    QPushButton *undo_button;   // local play mode undo (sends GTP undo x2 via signal)
    QPushButton *edit_position_button;
    QPushButton *cancel_edit_button;
    QPushButton *append_button;
    QPushButton *undo_edit_button;
    struct EditSnapshot { GoBoard board; int edit_x, edit_y; };
    QList<EditSnapshot> edit_undo_stack;  // Per-stone undo history within one edit-position session

    // The parent BoardWindow this edit window was launched from (for Update)
    BoardWindow *source_board_window;

    // Observation state for SGF accuracy
    // Consecutive pass tracking for counting phase detection
    int consecutive_passes;

    // Server-provided move count (official count from IGS)
    int server_move_count;

    // q5Go-style move counter for preventing race conditions
    // -1 = waiting for move history, >= 0 = processing moves
    int mv_counter;

    enum ObservationState {
        NOT_OBSERVING = 0,
        JOINING_GAME = 1,      // Initial connection
        RECONSTRUCTING = 2,    // Getting board state/history
        LIVE_OBSERVATION = 3   // Receiving real-time moves
    };
    ObservationState observation_state;
    QDateTime observation_start_time;
    int moves_received_during_live;

    // Game setup info
    int handicap;
    double komi;
    QString time_control;
    QString game_type;    // "Free" or "Rated"
    bool game_type_locked;  // Prevent overriding corrected game type
    int byoyomi_time;     // Byoyomi seconds
    
    // Capture tracking
    int white_captures;
    int black_captures;
    
    // Byoyomi tracking
    int white_byo_moves;
    int black_byo_moves;
    
    // Clock timing (for potential server lag compensation)
    QTimer *clock_timer;
    QDateTime last_time_update;
    int white_time_seconds;
    int black_time_seconds;
    
    // Game result
    QString game_result;
    bool game_finished;
    
    // Counting phase data
    QSet<QPair<int, int>> dead_stones;
    int white_territory;
    int black_territory;
    int white_prisoners;
    int black_prisoners;
    double final_score;
    
    // Server-provided scoring (from IGS)
    double server_white_score;
    double server_black_score;
    bool has_server_score;
    
    // IGS territory data (following q5Go protocol)
    bool receiving_territory_data;
    int territory_data_row;
    QMap<QPair<int, int>, int> territory_ownership; // 4=white territory, 5=black territory
    
    // Move history for SGF saving
    QList<GameMove> move_history;

    // Game tree structure (q5Go-style implementation)
    GameNode* game_root;           // Root of game tree
    GameNode* current_node;        // Currently displayed node
    bool slider_update_in_progress; // Prevent recursive slider signals
    bool auto_follow_mode;         // Auto-follow new moves (disabled when user navigates backward)

protected:
    void closeEvent(QCloseEvent *event) override;

public:
    BoardWindow(QWidget *parent = nullptr, const QString &username = "", bool edit_window = false);
    ~BoardWindow();

    GameSelectionDock *getGameSelectionDock() const { return game_selection_dock; }

    // Render an external board state to a pixmap (for dock button thumbnails).
    QPixmap renderSlotToPixmap(int size,
                               const int board_state[19][19],
                               int board_size,
                               int last_move_x, int last_move_y,
                               const QString &title_overlay = QString()) const;

    // Docked-pane mode: load a GameSlot's state into this window's UI widgets.
    void loadSlot(GameSlot *slot);
    // Docked-pane mode: snapshot current UI / widget state back into the slot
    // before switching away.
    void snapshotToSlot(GameSlot *slot);
    // Lightweight snapshot of just the board position (for hover pixmap refresh
    // after a live move — does NOT disconnect the clock timer).
    void snapshotBoardStateToSlot(GameSlot *slot);

    void startObserving(int game_id, const QString &white, const QString &black,
                       const QString &w_rank, const QString &b_rank);
    void loadSGF(GameNode* root, const QString &white, const QString &black,
                 const QString &w_rank, const QString &b_rank,
                 double komi_value, int handicap_value,
                 const QString &result, const QString &filename,
                 const QString &game_name = QString());
    void stopObserving();
    void clearMoveHistoryBeforeMovesCommand();  // Clear move_history before requesting moves to prevent duplicates
    GameNode* getGameRoot() const { return game_root; } // For slot sync after tree reset
    void setPlayingMode(bool playing);
    bool isObserving() const { return is_observing; }
    bool isPlaying() const { return is_playing; }
    bool isFinished() const { return game_finished; }
    int getObservedGameId() const { return observed_game_id; }
    QString getWhitePlayer() const { return white_player; }
    QString getBlackPlayer() const { return black_player; }
    int getHandicap() const { return handicap; }
    double getKomi() const { return komi; }
    
    void processMove(const GameMove &move);
    void updateObservationState(GameMove &move);
    void processComment(const QString &user, const QString &message, bool is_kibitz = false);
    void processUndo(int move_number);
    void updateGameInfo(const QString &info);
    void updateTimeInfo(const QString &white_time, const QString &black_time);
    void updateByoyomi(int white_time, int black_time, int white_moves, int black_moves);
    void updateGameSetup(int handicap_stones, double komi_points, const QString &time_ctrl = "");
    void updateGameResult(const QString &result);
    void updateGameDetails(const QString &game_type, int byoyomi_seconds);
    void lockGameType();  // Lock game type to prevent IGS 15 format from overriding
    void setLocalPlayMode(bool enabled);
    void setEngineReady(bool ready);   // Called when KataGo finishes init
    void stepBackOneMove();            // Local play undo — steps back one node in game tree
    void enableUndoButton(bool on);    // Called by xgospel2_fixed after each move pair
    int  getCurrentMove()        const { return current_move; }
    int  getConsecutivePasses()  const { return consecutive_passes; }

    // Engine console (local play mode)
    void appendEngineLog(const QString &text, bool is_sent);  // is_sent=true → cyan, false → white
    void setEngineStatus(const QString &name, bool ready);    // Updates status dot + label
    void updateCaptures(int white_caps, int black_caps);
    void updatePlayerNames(const QString &white, const QString &black);
    void setWhiteRank(const QString &rank);
    void setBlackRank(const QString &rank);
    const QMap<QPair<int,int>,int> &getTerritoryOwnership() const { return territory_ownership; }
    StoneColor getStoneAt(int x, int y) const { return board_widget ? board_widget->getBoardState(x, y) : EMPTY; }
    QSet<QPair<int,int>> getDeadStones() const { return board_widget ? board_widget->getDeadStonePositions() : QSet<QPair<int,int>>(); }
    void setCustomGameTitle(const QString &title);
    void setSharedWindow(bool shared) { is_shared_window = shared; }
    void clearObservers();
    void addObserver(const QString &name, const QString &rank);
    
    // Counting phase functions
    void enterScoringMode();
    void enterScoringModeForResult(); // Like enterScoringMode but always runs calculateScore
    void exitScoringMode();
    void markStoneAsDead(int x, int y);
    void calculateScore();
    void setScoringModeWithTerritory(const QMap<QPair<int,int>, StoneColor> &tmap);

    // Server scoring functions
    void setServerScore(double white_score, double black_score);
    
    // IGS territory marking functions (following q5Go protocol)
    void receiveScoreBegin();
    void receiveScoreLine(int row, const QString &line);
    void receiveScoreEnd();
    bool hasStoneAt(int x, int y) const;
    void clearDeadStones();
    void setBoardPosition(int x, int y, StoneColor color);
    void markTerritory(int x, int y, StoneColor owner);
    
    // Audit and logging system
    struct MoveAudit {
        int move_number;
        QString server_input;
        QString parsed_coords;
        int final_x, final_y;
        StoneColor color;
        QString captures;
        QString timestamp;
        bool placement_success;
    };
    
    void logMoveAudit(const MoveAudit& audit);
    void dumpMoveAuditTrail();
    void dumpBoardState(); // Dump current board position for verification
    QList<MoveAudit> move_audit_trail;
    static const int MAX_AUDIT_ENTRIES = 50; // Circular buffer

    // Move navigation functions
    void goToMove(int move_index);
    void goToFirstMove();
    void goToPreviousMove();
    void goToNextMove();
    void goToLastMove();
    void updateMoveNavigation();
    
    // q5Go-style group tracking (StoneGroup defined in game_types.h)
    QList<StoneGroup> white_groups;
    QList<StoneGroup> black_groups;
    
    int addStoneWithCaptures(int x, int y, StoneColor color);
    int countGroupLiberties(const QSet<QPair<int, int>>& group);
    QSet<QPair<int, int>> getAdjacentPositions(const QSet<QPair<int, int>>& group);
    QSet<QPair<int, int>> floodFillGroup(int x, int y, StoneColor color);
    void rebuildAllGroups();
    void removeDeadGroups(QList<StoneGroup>& groups);

    // Territory marking methods
    void calculateTerritoryMarkers();  // Public wrapper that calls calculateTerritory()
    bool isScoringMode() const { return is_scoring_mode; }
    void detectDeadStones();  // Algorithmically detect dead stone groups
    bool isLibertySurroundedByOpponent(int x, int y, StoneColor opponent_color);  // Helper for dead stone detection

private slots:
    void saveGame();
    void editGame();      // Edit/Analyze game (opens SGF in separate window)
    void resignGame();
    void closeBoard();
    void sendComment();
    void onCommentInputReturn();
    void requestObservers();
    void toggleObserverSort();
    void updateClockDisplay();
    void onBoardClicked(int x, int y);
    void makeMove(int x, int y);

    // Move navigation slots
    void onFirstMoveClicked();
    void onPreviousMoveClicked();
    void onNextMoveClicked();
    void onLastMoveClicked();
    void onMoveSliderChanged(int value);

    // Edit window button slots
    void onUpdateClicked();
    void onPassClicked();
    void onScoreClicked();
    void onEditPositionClicked();
    void onCancelEditClicked();
    void onAppendClicked();
    void onUndoEditClicked();
    void onEngineCmdSend();  // Engine console manual command send

private:
    void setupUI();
    void setupEditUI();   // SGF editor window layout (called instead of setupUI for edit windows)
    void switchToEditPositionMode();   // Transform button panel: 4 buttons -> 5
    void switchToViewMode();           // Restore button panel: 5 buttons -> 4
    void updateGameTreeStrip();        // Rebuild game tree strip from current game_root
    void updateLabels();
    void updatePlayerInfoGroups();  // Update White/Black player info groups (q5Go style)
    void updateWindowTitle();
    void updateCommentButtonText(); // Update button text based on playing/observing mode
    void calculateTerritory();
    StoneColor getTerritoryOwner(int x, int y, QSet<QPair<int, int>>& visited);
    QString formatGameInfo();
    QString formatMoveInfo();
    QString formatHandicapKomiInfo();
    QString generateSGF();
    QString formatGameResultComment();
    QString convertIGSResultToStandard(const QString &igs_result);

    // Game tree navigation helpers
    void displayNode(GameNode* node);  // Display a specific node's position
    int getTotalMoves() const;          // Get total moves in active variation
    void rebuildGameTreeFromMoveHistory(); // Reconstruct game_root tree from move_history (for docked inactive slots)

    // Capture calculation helpers for live move processing
    int countLiberties(const GoBoard& board, int x, int y);
    void removeGroup(GoBoard& board, int x, int y);
    void floodFill(const GoBoard& board, int x, int y, StoneColor color,
                   bool visited[19][19], QList<QPair<int,int>>& group);

signals:
    void boardClosed(int game_id);
    void saveRequested(int game_id);
    void resignRequested(int game_id);
    void commentRequested(int game_id, const QString &message);
    void sayRequested(int game_id, const QString &message);
    void tellRequested(const QString &player, const QString &message);
    void observersRequested(int game_id);
    void observerClicked(const QString &name);
    void whitePlayerClicked(const QString &name);
    void blackPlayerClicked(const QString &name);
    void moveRequested(int game_id, int x, int y);
    void passRequested(int game_id);
    void engineCmdRequested(const QString &cmd);  // Manual GTP command from console
    void engineGoRequested();                     // User pressed "Engine: Go"
    void engineClearRequested();                  // User pressed "Clear Board"
    void undoRequested();                         // User pressed "Undo" in local play mode
    void doneRequested(int game_id);              // User pressed "Done" in scoring phase
};

#endif // BOARD_WINDOW_H