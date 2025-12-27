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

// Simple Go board representation (shared with game_tree.h)
// Define enum BEFORE including game_tree.h
#ifndef STONE_COLOR_DEFINED
#define STONE_COLOR_DEFINED
enum StoneColor {
    EMPTY = 0,          // Empty point
    BLACK_STONE = 1,
    WHITE_STONE = 2,
    EMPTY_STONE = 0     // Alias for EMPTY (used by game_tree)
};
#endif

#include "game_tree.h"

struct GameMove {
    int game_id;
    int move_number;
    StoneColor color;
    int x, y;  // Board coordinates
    QString time_info;
    QString captured;
    bool is_live_move;  // true for live moves, false for board reconstruction
    QDateTime received_time;
};

class GoBoardWidget : public QFrame {
    Q_OBJECT

private:
    int board_size;
    int **board_state;  // 2D array for board state
    int margin;
    int cell_size;
    int last_move_x, last_move_y;  // Highlight last move
    bool show_coordinates;
    
    // Scoring mode visualization
    bool scoring_mode_enabled;
    QMap<QPair<int, int>, StoneColor> territory_map;  // Empty points and their territory owner
    QSet<QPair<int, int>> dead_stone_positions;
    
public:
    GoBoardWidget(QWidget *parent = nullptr);
    ~GoBoardWidget();
    
    void setBoardSize(int size);
    void placeMoveAt(int x, int y, StoneColor color);
    StoneColor getStoneAt(int x, int y) const;
    void clearBoard();
    void setLastMove(int x, int y);
    void setShowCoordinates(bool show) { show_coordinates = show; update(); }
    StoneColor getBoardState(int x, int y) const;
    
    // Scoring mode visualization
    void setScoringMode(bool enabled) { scoring_mode_enabled = enabled; update(); }
    void setTerritoryMap(const QMap<QPair<int, int>, StoneColor> &map) { territory_map = map; update(); }
    void setDeadStones(const QSet<QPair<int, int>> &dead_stones) { dead_stone_positions = dead_stones; update(); }
    
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    
private:
    void calculateSizes();
    void drawBoard(QPainter &painter);
    void drawStones(QPainter &painter);
    void drawCoordinates(QPainter &painter);
    void drawLastMoveMarker(QPainter &painter);
    void drawTerritoryMarkers(QPainter &painter);
    void drawDeadStoneMarkers(QPainter &painter);
    QPoint boardToScreen(int x, int y);
    QPoint screenToBoard(int px, int py);

signals:
    void boardClicked(int x, int y);
};

class BoardWindow : public QMainWindow {
    Q_OBJECT

private:
    GoBoardWidget *board_widget;
    QLabel *game_info_label;
    QLabel *move_info_label;
    QLabel *players_label;
    QLabel *time_label;
    QLabel *handicap_komi_label;
    QLabel *result_label;
    QPushButton *save_button;
    QPushButton *resign_button;
    QPushButton *close_button;

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

public:
    BoardWindow(QWidget *parent = nullptr, const QString &username = "");
    ~BoardWindow();
    
    void startObserving(int game_id, const QString &white, const QString &black,
                       const QString &w_rank, const QString &b_rank);
    void stopObserving();
    void clearMoveHistoryBeforeMovesCommand();  // Clear move_history before requesting moves to prevent duplicates
    void setPlayingMode(bool playing);
    bool isObserving() const { return is_observing; }
    bool isPlaying() const { return is_playing; }
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
    void updateCaptures(int white_caps, int black_caps);
    void updatePlayerNames(const QString &white, const QString &black);
    void setCustomGameTitle(const QString &title);
    void clearObservers();
    void addObserver(const QString &name, const QString &rank);
    
    // Counting phase functions
    void enterScoringMode();
    void exitScoringMode();
    void markStoneAsDead(int x, int y);
    void calculateScore();
    
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
    
    // q5Go-style group tracking
    struct StoneGroup {
        QSet<QPair<int, int>> stones;
        int liberties;
        bool alive;
        
        StoneGroup(const QSet<QPair<int, int>>& s, int lib) : stones(s), liberties(lib), alive(true) {}
    };
    
    QList<StoneGroup> white_groups;
    QList<StoneGroup> black_groups;
    
    int addStoneWithCaptures(int x, int y, StoneColor color);
    int countGroupLiberties(const QSet<QPair<int, int>>& group);
    QSet<QPair<int, int>> getAdjacentPositions(const QSet<QPair<int, int>>& group);
    QSet<QPair<int, int>> floodFillGroup(int x, int y, StoneColor color);
    void rebuildAllGroups();
    void removeDeadGroups(QList<StoneGroup>& groups);
    
private slots:
    void saveGame();
    void resignGame();
    void closeBoard();
    void sendComment();
    void onCommentInputReturn();
    void requestObservers();
    void updateClockDisplay();
    void onBoardClicked(int x, int y);
    void makeMove(int x, int y);
    
    // Move navigation slots
    void onFirstMoveClicked();
    void onPreviousMoveClicked();
    void onNextMoveClicked();
    void onLastMoveClicked();
    void onMoveSliderChanged(int value);

private:
    void setupUI();
    void updateLabels();
    void updateWindowTitle();
    void updateCommentButtonText(); // Update button text based on playing/observing mode
    void calculateTerritory();
    StoneColor getTerritoryOwner(int x, int y, QSet<QPair<int, int>>& visited);
    QString formatGameInfo();
    QString formatPlayersInfo();
    QString formatMoveInfo();
    QString formatHandicapKomiInfo();
    QString generateSGF();
    QString formatGameResultComment();
    QString convertIGSResultToStandard(const QString &igs_result);

    // Game tree navigation helpers
    void displayNode(GameNode* node);  // Display a specific node's position
    int getTotalMoves() const;          // Get total moves in active variation

signals:
    void boardClosed(int game_id);
    void saveRequested(int game_id);
    void resignRequested(int game_id);
    void commentRequested(int game_id, const QString &message);
    void sayRequested(int game_id, const QString &message); // For private player communication
    void observersRequested(int game_id);
    void moveRequested(int game_id, int x, int y);
};

#endif // BOARD_WINDOW_H