#include "board_window.h"
#include "igs_move_parser.h"
#include <QtWidgets/QMenuBar>
#include <QtGui/QFont>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QStack>
#include <QtCore/QTime>
#include <cmath>
#include <climits>

// GoBoardWidget Implementation
GoBoardWidget::GoBoardWidget(QWidget *parent)
    : QFrame(parent), board_size(19), board_state(nullptr), margin(30), 
      cell_size(25), last_move_x(-1), last_move_y(-1), show_coordinates(true),
      scoring_mode_enabled(false)
{
    setFrameStyle(QFrame::Sunken | QFrame::Panel);
    setLineWidth(2);
    setMinimumSize(500, 500);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // Allocate board state
    setBoardSize(19);
    
    setStyleSheet(
        "GoBoardWidget {"
        "    background-color: #DEB887;"  // Burlywood - classic Go board color
        "    border: 2px inset #8B7355;"
        "}"
    );
}

GoBoardWidget::~GoBoardWidget() {
    if (board_state) {
        for (int i = 0; i < board_size; i++) {
            delete[] board_state[i];
        }
        delete[] board_state;
    }
}

void GoBoardWidget::setBoardSize(int size) {
    if (board_state) {
        for (int i = 0; i < board_size; i++) {
            delete[] board_state[i];
        }
        delete[] board_state;
    }
    
    board_size = size;
    board_state = new int*[board_size];
    for (int i = 0; i < board_size; i++) {
        board_state[i] = new int[board_size];
        for (int j = 0; j < board_size; j++) {
            board_state[i][j] = EMPTY;
        }
    }
    
    calculateSizes();
    update();
}

void GoBoardWidget::placeMoveAt(int x, int y, StoneColor color) {
    qDebug() << "GoBoardWidget::placeMoveAt called with x=" << x << "y=" << y << "color=" << color << "board_size=" << board_size;
    
    if (x >= 0 && x < board_size && y >= 0 && y < board_size) {
        board_state[x][y] = color;
        setLastMove(x, y);
        qDebug() << "Stone placed successfully at (" << x << "," << y << ")";
        update();
    } else {
        qDebug() << "ERROR: Invalid coordinates - stone NOT placed!";
    }
}

void GoBoardWidget::clearBoard() {
    for (int i = 0; i < board_size; i++) {
        for (int j = 0; j < board_size; j++) {
            board_state[i][j] = EMPTY;
        }
    }
    last_move_x = last_move_y = -1;
    update();
}

void GoBoardWidget::setLastMove(int x, int y) {
    last_move_x = x;
    last_move_y = y;
    update();
}

StoneColor GoBoardWidget::getBoardState(int x, int y) const {
    if (x >= 0 && x < board_size && y >= 0 && y < board_size) {
        return static_cast<StoneColor>(board_state[x][y]);
    }
    return EMPTY;
}

StoneColor GoBoardWidget::getStoneAt(int x, int y) const {
    return getBoardState(x, y);
}

void GoBoardWidget::calculateSizes() {
    int available_width = width() - 2 * margin;
    int available_height = height() - 2 * margin;
    int min_dimension = std::min(available_width, available_height);
    
    if (board_size > 1) {
        cell_size = min_dimension / (board_size - 1);
        cell_size = std::max(cell_size, 15);  // Minimum cell size
    }
}

void GoBoardWidget::paintEvent(QPaintEvent *event) {
    QFrame::paintEvent(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    calculateSizes();
    drawBoard(painter);
    
    // Draw territory markings in scoring mode (before stones)
    if (scoring_mode_enabled) {
        drawTerritoryMarkers(painter);
    }
    
    drawStones(painter);
    if (show_coordinates) {
        drawCoordinates(painter);
    }
    drawLastMoveMarker(painter);
    
    // Draw dead stone markers on top of stones
    if (scoring_mode_enabled) {
        drawDeadStoneMarkers(painter);
    }
}

void GoBoardWidget::drawBoard(QPainter &painter) {
    painter.setPen(QPen(Qt::black, 1));
    
    // Draw grid lines
    for (int i = 0; i < board_size; i++) {
        QPoint start = boardToScreen(i, 0);
        QPoint end = boardToScreen(i, board_size - 1);
        painter.drawLine(start, end);
        
        start = boardToScreen(0, i);
        end = boardToScreen(board_size - 1, i);
        painter.drawLine(start, end);
    }
    
    // Draw star points (hoshi) for standard board sizes
    if (board_size == 19) {
        int hoshi_points[][2] = {{3,3}, {9,3}, {15,3}, {3,9}, {9,9}, {15,9}, {3,15}, {9,15}, {15,15}};
        painter.setBrush(Qt::black);
        for (int i = 0; i < 9; i++) {
            QPoint center = boardToScreen(hoshi_points[i][0], hoshi_points[i][1]);
            painter.drawEllipse(center, 3, 3);
        }
    } else if (board_size == 13) {
        int hoshi_points[][2] = {{3,3}, {6,6}, {9,3}, {3,9}, {9,9}};
        painter.setBrush(Qt::black);
        for (int i = 0; i < 5; i++) {
            QPoint center = boardToScreen(hoshi_points[i][0], hoshi_points[i][1]);
            painter.drawEllipse(center, 3, 3);
        }
    }
}

void GoBoardWidget::drawStones(QPainter &painter) {
    int stone_radius = cell_size / 2 - 2;
    
    for (int i = 0; i < board_size; i++) {
        for (int j = 0; j < board_size; j++) {
            if (board_state[i][j] != EMPTY) {
                QPoint center = boardToScreen(i, j);
                
                if (board_state[i][j] == BLACK_STONE) {
                    painter.setBrush(Qt::black);
                    painter.setPen(QPen(Qt::darkGray, 1));
                } else {
                    painter.setBrush(Qt::white);
                    painter.setPen(QPen(Qt::black, 1));
                }
                
                painter.drawEllipse(center, stone_radius, stone_radius);
            }
        }
    }
}

void GoBoardWidget::drawCoordinates(QPainter &painter) {
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 10));
    
    // Draw letters (A-T skipping I) for columns
    QString letters = "ABCDEFGHJKLMNOPQRST";
    for (int i = 0; i < board_size && i < letters.length(); i++) {
        QPoint pos = boardToScreen(i, 0);
        painter.drawText(pos.x() - 5, margin - 5, QString(letters[i]));
        painter.drawText(pos.x() - 5, height() - 5, QString(letters[i]));
    }
    
    // Draw numbers for rows
    for (int i = 0; i < board_size; i++) {
        QPoint pos = boardToScreen(0, i);
        QString num = QString::number(board_size - i);
        painter.drawText(5, pos.y() + 5, num);
        painter.drawText(width() - 20, pos.y() + 5, num);
    }
}

void GoBoardWidget::drawLastMoveMarker(QPainter &painter) {
    if (last_move_x >= 0 && last_move_y >= 0) {
        QPoint center = boardToScreen(last_move_x, last_move_y);
        painter.setPen(QPen(Qt::red, 3));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(center, cell_size/3, cell_size/3);
    }
}

void GoBoardWidget::drawTerritoryMarkers(QPainter &painter) {
    for (auto it = territory_map.begin(); it != territory_map.end(); ++it) {
        QPair<int, int> pos = it.key();
        StoneColor owner = it.value();
        
        QPoint center = boardToScreen(pos.first, pos.second);
        
        // Draw territory marking exactly like q5Go - X marks
        if (owner == WHITE_STONE) {
            painter.setPen(QPen(Qt::white, 3)); // White X for white territory
        } else if (owner == BLACK_STONE) {
            painter.setPen(QPen(Qt::black, 3)); // Black X for black territory
        } else {
            continue; // Skip neutral territory
        }
        
        // Draw X mark for territory (45-degree rotated cross like q5Go)
        int cross_size = cell_size / 3;
        painter.drawLine(center.x() - cross_size, center.y() - cross_size,
                        center.x() + cross_size, center.y() + cross_size);
        painter.drawLine(center.x() + cross_size, center.y() - cross_size,
                        center.x() - cross_size, center.y() + cross_size);
    }
}

void GoBoardWidget::drawDeadStoneMarkers(QPainter &painter) {
    painter.setPen(QPen(Qt::red, 3));
    
    for (const auto& pos : dead_stone_positions) {
        QPoint center = boardToScreen(pos.first, pos.second);
        
        // Draw X mark over dead stones like q5Go
        int cross_size = cell_size / 3;
        painter.drawLine(center.x() - cross_size, center.y() - cross_size,
                        center.x() + cross_size, center.y() + cross_size);
        painter.drawLine(center.x() + cross_size, center.y() - cross_size,
                        center.x() - cross_size, center.y() + cross_size);
    }
}

QPoint GoBoardWidget::boardToScreen(int x, int y) {
    return QPoint(margin + x * cell_size, margin + y * cell_size);
}

QPoint GoBoardWidget::screenToBoard(int px, int py) {
    int x = (px - margin + cell_size/2) / cell_size;
    int y = (py - margin + cell_size/2) / cell_size;
    return QPoint(x, y);
}

void GoBoardWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QPoint board_pos = screenToBoard(event->x(), event->y());
        if (board_pos.x() >= 0 && board_pos.x() < board_size &&
            board_pos.y() >= 0 && board_pos.y() < board_size) {
            emit boardClicked(board_pos.x(), board_pos.y());
        }
    }
}

void GoBoardWidget::resizeEvent(QResizeEvent *event) {
    QFrame::resizeEvent(event);
    calculateSizes();
}

// BoardWindow Implementation
BoardWindow::BoardWindow(QWidget *parent)
    : QMainWindow(parent), observed_game_id(-1), current_move(0), 
      current_player(BLACK_STONE), is_observing(false), is_scoring_mode(false), handicap(0), 
      komi(0.5), game_type("Free"), byoyomi_time(0), 
      white_captures(0), black_captures(0), white_byo_moves(0), black_byo_moves(0),
      white_time_seconds(0), black_time_seconds(0), game_finished(false),
      white_territory(0), black_territory(0), white_prisoners(0), black_prisoners(0), final_score(0.0),
      server_white_score(0.0), server_black_score(0.0), has_server_score(false),
      consecutive_passes(0), server_move_count(0), observation_state(NOT_OBSERVING), moves_received_during_live(0)
{
    qDebug() << "DEBUG: BoardWindow constructor started";
    qDebug() << "DEBUG: Window title set";
    setMinimumSize(800, 700);
    qDebug() << "DEBUG: Minimum size set";
    setupUI();
    qDebug() << "DEBUG: UI setup completed";
    
    // Initialize clock timer for server lag compensation
    clock_timer = new QTimer(this);
    qDebug() << "DEBUG: Timer created";
    connect(clock_timer, &QTimer::timeout, this, &BoardWindow::updateClockDisplay);
    clock_timer->start(1000); // Update every second to compensate for server lag
    qDebug() << "DEBUG: Timer connected and started";
    
    // Set initial window title
    updateWindowTitle();
    
    qDebug() << "DEBUG: BoardWindow constructor completed";
}

BoardWindow::~BoardWindow() {
}

void BoardWindow::setupUI() {
    QWidget *central = new QWidget;
    setCentralWidget(central);
    
    central->setStyleSheet(
        "QWidget {"
        "    background-color: #f5f5f5;"
        "    color: black;"
        "}"
    );
    
    QHBoxLayout *main_layout = new QHBoxLayout(central);
    main_layout->setSpacing(10);
    main_layout->setMargin(10);
    
    // Left side - Board
    QFrame *board_frame = new QFrame;
    board_frame->setFrameStyle(QFrame::Raised | QFrame::Panel);
    board_frame->setLineWidth(3);
    board_frame->setStyleSheet(
        "QFrame {"
        "    border: 3px outset #888;"
        "    background-color: #e0e0e0;"
        "}"
    );
    
    QVBoxLayout *board_layout = new QVBoxLayout(board_frame);
    board_layout->setMargin(8);
    
    board_widget = new GoBoardWidget;
    board_layout->addWidget(board_widget);
    
    // Connect board clicks to dead stone marking during scoring mode
    connect(board_widget, &GoBoardWidget::boardClicked, this, &BoardWindow::markStoneAsDead);
    
    // Move navigation controls
    QFrame *nav_frame = new QFrame;
    nav_frame->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    nav_frame->setStyleSheet(
        "QFrame {"
        "    border: 1px inset #666;"
        "    background-color: #f0f0f0;"
        "    padding: 5px;"
        "}"
    );
    
    QHBoxLayout *nav_layout = new QHBoxLayout(nav_frame);
    nav_layout->setSpacing(5);
    nav_layout->setMargin(5);
    
    // Navigation buttons
    first_move_button = new QPushButton("⏮");
    first_move_button->setFixedSize(30, 25);
    first_move_button->setToolTip("Go to first move");
    
    prev_move_button = new QPushButton("⏪");
    prev_move_button->setFixedSize(30, 25);
    prev_move_button->setToolTip("Previous move");
    
    next_move_button = new QPushButton("⏩");
    next_move_button->setFixedSize(30, 25);
    next_move_button->setToolTip("Next move");
    
    last_move_button = new QPushButton("⏭");
    last_move_button->setFixedSize(30, 25);
    last_move_button->setToolTip("Go to last move");
    
    // Move slider and number display
    move_slider = new QSlider(Qt::Horizontal);
    move_slider->setMinimum(0);
    move_slider->setMaximum(0);
    move_slider->setValue(0);
    move_slider->setTickPosition(QSlider::TicksBelow);
    move_slider->setTickInterval(10);
    
    move_number_label = new QLabel("Move: 0/0");
    move_number_label->setMinimumWidth(80);
    move_number_label->setAlignment(Qt::AlignCenter);
    
    // Add components to layout
    nav_layout->addWidget(first_move_button);
    nav_layout->addWidget(prev_move_button);
    nav_layout->addWidget(move_slider, 1);
    nav_layout->addWidget(next_move_button);
    nav_layout->addWidget(last_move_button);
    nav_layout->addWidget(move_number_label);
    
    board_layout->addWidget(nav_frame);
    
    // Initialize move navigation
    current_move_index = 0;
    
    // Connect navigation signals
    connect(first_move_button, &QPushButton::clicked, this, &BoardWindow::onFirstMoveClicked);
    connect(prev_move_button, &QPushButton::clicked, this, &BoardWindow::onPreviousMoveClicked);
    connect(next_move_button, &QPushButton::clicked, this, &BoardWindow::onNextMoveClicked);
    connect(last_move_button, &QPushButton::clicked, this, &BoardWindow::onLastMoveClicked);
    connect(move_slider, &QSlider::valueChanged, this, &BoardWindow::onMoveSliderChanged);
    
    main_layout->addWidget(board_frame, 3);
    
    // Right side - Game info and controls with comment panel
    QSplitter *right_splitter = new QSplitter(Qt::Vertical);
    right_splitter->setMaximumWidth(300);
    
    // Top: Game info panel
    QFrame *info_frame = new QFrame;
    info_frame->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    info_frame->setLineWidth(2);
    info_frame->setStyleSheet(
        "QFrame {"
        "    border: 2px inset #888;"
        "    background-color: white;"
        "    padding: 5px;"
        "}"
    );
    
    QVBoxLayout *info_layout = new QVBoxLayout(info_frame);
    info_layout->setSpacing(8);
    info_layout->setMargin(10);
    
    // Game title
    QLabel *title = new QLabel("Game Observer");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(
        "QLabel {"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "    color: #333;"
        "    background-color: #f0f0f0;"
        "    border: 1px solid #ccc;"
        "    padding: 5px;"
        "}"
    );
    info_layout->addWidget(title);
    
    // Game info labels
    game_info_label = new QLabel("No game observed");
    game_info_label->setWordWrap(true);
    game_info_label->setStyleSheet("font-weight: bold; color: #444;");
    info_layout->addWidget(game_info_label);
    
    players_label = new QLabel("");
    players_label->setWordWrap(true);
    players_label->setStyleSheet("color: #666;");
    info_layout->addWidget(players_label);
    
    move_info_label = new QLabel("");
    move_info_label->setStyleSheet("color: #444;");
    info_layout->addWidget(move_info_label);
    
    time_label = new QLabel("");
    time_label->setStyleSheet("color: #444;");
    info_layout->addWidget(time_label);
    
    handicap_komi_label = new QLabel("");
    handicap_komi_label->setStyleSheet("color: #444; font-size: 11px;");
    info_layout->addWidget(handicap_komi_label);
    
    result_label = new QLabel("");
    result_label->setStyleSheet("color: #d00; font-weight: bold; font-size: 12px;");
    result_label->setWordWrap(true);
    info_layout->addWidget(result_label);
    
    info_layout->addStretch();
    
    // Control buttons
    save_button = new QPushButton("Save Game");
    save_button->setStyleSheet(
        "QPushButton {"
        "    background-color: #4CAF50;"
        "    color: white;"
        "    border: none;"
        "    padding: 8px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #45a049;"
        "}"
    );
    connect(save_button, &QPushButton::clicked, this, &BoardWindow::saveGame);
    info_layout->addWidget(save_button);
    
    close_button = new QPushButton("Close Board");
    close_button->setStyleSheet(
        "QPushButton {"
        "    background-color: #f44336;"
        "    color: white;"
        "    border: none;"
        "    padding: 8px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #da190b;"
        "}"
    );
    connect(close_button, &QPushButton::clicked, this, &BoardWindow::closeBoard);
    info_layout->addWidget(close_button);
    
    right_splitter->addWidget(info_frame);
    
    // Bottom: Comment/Kibitz panel
    QFrame *comment_frame = new QFrame;
    comment_frame->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    comment_frame->setLineWidth(2);
    comment_frame->setStyleSheet(
        "QFrame {"
        "    border: 2px inset #888;"
        "    background-color: #f9f9f9;"
        "    padding: 5px;"
        "}"
    );
    
    QVBoxLayout *comment_layout = new QVBoxLayout(comment_frame);
    comment_layout->setSpacing(5);
    comment_layout->setMargin(8);
    
    // Comment title
    QLabel *comment_title = new QLabel("Comments & Kibitz");
    comment_title->setAlignment(Qt::AlignCenter);
    comment_title->setStyleSheet(
        "QLabel {"
        "    font-weight: bold;"
        "    font-size: 12px;"
        "    color: #333;"
        "    background-color: #e0e0e0;"
        "    border: 1px solid #ccc;"
        "    padding: 3px;"
        "}"
    );
    comment_layout->addWidget(comment_title);
    
    // Comment display area
    comment_display = new QTextEdit;
    comment_display->setReadOnly(true);
    comment_display->setMaximumHeight(150);
    comment_display->setStyleSheet(
        "QTextEdit {"
        "    background-color: white;"
        "    border: 1px solid #ccc;"
        "    font-family: monospace;"
        "    font-size: 10px;"
        "}"
    );
    comment_display->setPlaceholderText("Comments and kibitz will appear here...");
    comment_layout->addWidget(comment_display);
    
    // Comment input area
    comment_input = new QLineEdit;
    comment_input->setPlaceholderText("Type comment or kibitz...");
    comment_input->setStyleSheet(
        "QLineEdit {"
        "    border: 1px solid #ccc;"
        "    padding: 4px;"
        "    font-size: 10px;"
        "}"
    );
    connect(comment_input, &QLineEdit::returnPressed, this, &BoardWindow::onCommentInputReturn);
    comment_layout->addWidget(comment_input);
    
    // Comment buttons
    QHBoxLayout *comment_buttons = new QHBoxLayout;
    comment_buttons->setSpacing(3);
    
    send_comment_button = new QPushButton("Kibitz (Enter)");
    send_comment_button->setStyleSheet(
        "QPushButton {"
        "    background-color: #2196F3;"
        "    color: white;"
        "    border: none;"
        "    padding: 4px 8px;"
        "    font-size: 10px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #1976D2;"
        "}"
    );
    connect(send_comment_button, &QPushButton::clicked, [this]() {
        qDebug() << "Kibitz button clicked!";
        sendComment();
    });
    comment_buttons->addWidget(send_comment_button);
    
    comment_layout->addLayout(comment_buttons);
    
    right_splitter->addWidget(comment_frame);
    
    // Bottom: Observers panel
    QFrame *observers_frame = new QFrame;
    observers_frame->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    observers_frame->setLineWidth(2);
    observers_frame->setStyleSheet(
        "QFrame {"
        "    border: 2px inset #888;"
        "    background-color: #f9f9f9;"
        "    padding: 5px;"
        "}"
    );
    
    QVBoxLayout *observers_layout = new QVBoxLayout(observers_frame);
    observers_layout->setSpacing(5);
    observers_layout->setMargin(8);
    
    // Observers title
    QLabel *observers_title = new QLabel("Observers");
    observers_title->setAlignment(Qt::AlignCenter);
    observers_title->setStyleSheet(
        "QLabel {"
        "    font-weight: bold;"
        "    font-size: 12px;"
        "    color: #333;"
        "    background-color: #e0e0e0;"
        "    border: 1px solid #ccc;"
        "    padding: 3px;"
        "}"
    );
    observers_layout->addWidget(observers_title);
    
    // Observers list
    observers_list = new QListWidget;
    observers_list->setMaximumHeight(120);
    observers_list->setStyleSheet(
        "QListWidget {"
        "    background-color: white;"
        "    border: 1px solid #ccc;"
        "    font-family: monospace;"
        "    font-size: 10px;"
        "}"
    );
    observers_layout->addWidget(observers_list);
    
    // Observers refresh button
    QPushButton *refresh_observers_button = new QPushButton("Refresh Observers");
    refresh_observers_button->setStyleSheet(
        "QPushButton {"
        "    background-color: #FF9800;"
        "    color: white;"
        "    border: none;"
        "    padding: 4px 8px;"
        "    font-size: 10px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #F57C00;"
        "}"
    );
    connect(refresh_observers_button, &QPushButton::clicked, this, &BoardWindow::requestObservers);
    observers_layout->addWidget(refresh_observers_button);
    
    right_splitter->addWidget(observers_frame);
    right_splitter->setSizes({150, 120, 100}); // Set initial sizes for all three panels
    
    main_layout->addWidget(right_splitter, 1);
}

void BoardWindow::startObserving(int game_id, const QString &white, const QString &black,
                                const QString &w_rank, const QString &b_rank) {
    observed_game_id = game_id;
    white_player = white;
    black_player = black;
    white_rank = w_rank;
    black_rank = b_rank;
    current_move = 0;
    current_player = BLACK_STONE;
    is_observing = true;
    game_start_time = QDateTime::currentDateTime();
    
    // Initialize observation state for SGF accuracy
    observation_state = JOINING_GAME;
    observation_start_time = QDateTime::currentDateTime();
    moves_received_during_live = 0;
    qDebug() << "🎯 OBSERVATION STATE: Changed to JOINING_GAME for game" << game_id;
    
    // Reset game setup and result info
    handicap = 0;
    komi = 0.5; // Will be updated by updateGameSetup() when Command 7 is received
    time_control = "";
    game_type = "Free";
    byoyomi_time = 0;
    white_captures = 0;
    black_captures = 0;
    game_result = "";
    game_finished = false;
    move_history.clear();
    clearObservers();
    
    board_widget->clearBoard();
    
    // Initialize group tracking
    white_groups.clear();
    black_groups.clear();
    
    updateLabels();
    updateWindowTitle();
}

void BoardWindow::stopObserving() {
    is_observing = false;
    observed_game_id = -1;
    observation_state = NOT_OBSERVING;
    moves_received_during_live = 0;
    qDebug() << "🎯 OBSERVATION STATE: Changed to NOT_OBSERVING";
    updateLabels();
    updateWindowTitle();
}

void BoardWindow::updateObservationState(GameMove &move) {
    // Algorithm to detect transition from board reconstruction to live moves
    switch (observation_state) {
        case NOT_OBSERVING:
            // Should not happen, but be safe
            move.is_live_move = false;
            break;
            
        case JOINING_GAME:
            // First few moves are likely board reconstruction
            move.is_live_move = false;
            observation_state = RECONSTRUCTING;
            qDebug() << "🎯 OBSERVATION STATE: Changed to RECONSTRUCTING";
            break;
            
        case RECONSTRUCTING:
            // Detect pattern that suggests we've reached live moves
            // Key insight: In live observation, moves come in real-time with delays
            // In reconstruction, moves come rapidly in sequence
            
            // If we haven't received any moves for a few seconds, next move is likely live
            if (!observation_start_time.isNull()) {
                qint64 seconds_since_start = observation_start_time.secsTo(QDateTime::currentDateTime());
                
                // Strategy: If we've been receiving moves for >2 seconds and this move
                // has a reasonable time gap, consider it live
                if (seconds_since_start > 2) {
                    observation_state = LIVE_OBSERVATION;
                    moves_received_during_live = 0;
                    move.is_live_move = true;
                    qDebug() << "🎯 OBSERVATION STATE: Changed to LIVE_OBSERVATION - move" << move.move_number << "marked as live";
                } else {
                    move.is_live_move = false;
                }
            } else {
                move.is_live_move = false;
            }
            break;
            
        case LIVE_OBSERVATION:
            // All moves during live observation are game moves
            move.is_live_move = true;
            moves_received_during_live++;
            qDebug() << "🎯 LIVE MOVE: #" << moves_received_during_live << "move" << move.move_number;
            break;
    }
    
    qDebug() << "🎯 MOVE TRACKING: State=" << observation_state << "Move" << move.move_number << "Live=" << move.is_live_move;
}

void BoardWindow::processMove(const GameMove &move) {
    qDebug() << "DEBUG: BoardWindow::processMove called - Game:" << move.game_id 
             << "Observing:" << is_observing << "Expected game:" << observed_game_id;
    
    if (!is_observing || move.game_id != observed_game_id) {
        qDebug() << "DEBUG: Move rejected - not observing or wrong game";
        return;
    }
    
    
    // Create a mutable copy to track observation state
    GameMove tracked_move = move;
    tracked_move.received_time = QDateTime::currentDateTime();
    
    // Update observation state based on move patterns
    updateObservationState(tracked_move);
    
    // Check if this is a handicap stone placement
    if (move.x == -2) {
        int handicap_count = move.y;
        qDebug() << "DEBUG: Placing" << handicap_count << "handicap stones";
        
        // Set handicap info for display
        handicap = handicap_count;
        
        // Get handicap positions and place stones
        QList<QPair<int, int>> positions = IGSMoveParser::getHandicapPositions(handicap_count);
        for (const auto& pos : positions) {
            board_widget->placeMoveAt(pos.first, pos.second, BLACK_STONE);
            qDebug() << "DEBUG: Placed handicap stone at (" << pos.first << "," << pos.second << ")";
        }
        
        current_move = move.move_number;
        server_move_count = move.move_number;  // Track official server count
        
        // Rebuild groups after handicap placement
        rebuildAllGroups();
        
        updateLabels();
        updateMoveNavigation();
        return;
    }
    
    // Handle Pass moves (counting phase)
    if (tracked_move.x == -1 && tracked_move.y == -1) {
        qDebug() << "DEBUG: Pass move" << tracked_move.move_number << "by" << (tracked_move.color == BLACK_STONE ? "BLACK" : "WHITE");
        
        // Track consecutive passes for counting phase detection
        consecutive_passes++;
        qDebug() << "*** CONSECUTIVE PASSES COUNT:" << consecutive_passes;
        
        // Store pass move in history  
        qDebug() << "🗂️  MOVE HISTORY: Adding PASS move" << tracked_move.move_number << "total moves:" << move_history.size() << "Live=" << tracked_move.is_live_move;
        move_history.append(tracked_move);
        current_move = move.move_number;
        server_move_count = move.move_number;  // Track official server count
        
        qDebug() << "*** PASS MOVE PROCESSED: client history size=" << move_history.size() << "server count=" << server_move_count;
        current_player = (move.color == BLACK_STONE) ? WHITE_STONE : BLACK_STONE;
        
        // Check for counting phase trigger (3 consecutive passes) - AUTO-ENTER SCORING MODE
        if (consecutive_passes >= 3 && !is_scoring_mode) {
            qDebug() << "*** 3 consecutive passes detected - AUTO-ENTERING COUNTING PHASE";
            qDebug() << "*** Current move count:" << move_history.size() << "Last move:" << move.move_number;
            qDebug() << "*** Pattern detected - following q5Go behavior: automatic scoring mode entry";
            
            // Automatically enter scoring mode (like q5Go does)
            enterScoringMode();
            
            qDebug() << "*** SCORING MODE ACTIVATED - ready for territory marking and dead stone selection";
        }
        
        // Reset lag compensation timer when player switches
        last_time_update = QDateTime::currentDateTime();
        
        updateLabels();
        updateMoveNavigation();
        return;
    }
    
    qDebug() << "DEBUG: Placing move at (" << tracked_move.x << "," << tracked_move.y << ") color:" << tracked_move.color;
    
    // Reset consecutive pass counter for regular moves
    consecutive_passes = 0;
    qDebug() << "DEBUG: Regular move - reset consecutive passes counter";
    
    // Store move in history for SGF saving
    qDebug() << "🗂️  MOVE HISTORY: Adding move" << tracked_move.move_number << "total moves:" << move_history.size() << "Live=" << tracked_move.is_live_move;
    move_history.append(tracked_move);
    
    // Place the move on the board and calculate captures client-side
    board_widget->placeMoveAt(tracked_move.x, tracked_move.y, tracked_move.color);
    
    // 🔍 AUDIT: Log this move placement
    MoveAudit audit;
    audit.move_number = move.move_number;
    audit.server_input = QString("Game %1: Move %2").arg(observed_game_id).arg(move.move_number);
    audit.parsed_coords = QString("%1%2").arg(QChar('A' + move.x + (move.x >= 8 ? 1 : 0))).arg(19 - move.y);
    audit.final_x = move.x;
    audit.final_y = move.y;
    audit.color = move.color;
    audit.timestamp = QTime::currentTime().toString("hh:mm:ss.zzz");
    audit.placement_success = (move.x >= 0 && move.y >= 0 && move.x < 19 && move.y < 19);
    
    // Calculate and execute captures using client-side logic
    int captured_count = addStoneWithCaptures(move.x, move.y, move.color);
    
    // Complete audit entry with capture information
    if (captured_count > 0) {
        audit.captures = QString("client-captures:%1").arg(captured_count);
        qDebug() << "DEBUG: Live move captured" << captured_count << "stones";
    }
    
    // Also handle server-provided capture data (for counting phase stone removal)
    if (!move.captured.isEmpty()) {
        QStringList all_captures = move.captured.split(";");
        audit.captures += QString(" server-captures:%1").arg(move.captured);
        
        for (const QString& capture : all_captures) {
            QStringList cap_coords = capture.split(",");
            if (cap_coords.size() == 2) {
                int cap_x = cap_coords[0].toInt();
                int cap_y = cap_coords[1].toInt();
                
                // Remove the captured stone from the board
                board_widget->placeMoveAt(cap_x, cap_y, EMPTY);
                qDebug() << "DEBUG: Server-specified capture - removed stone at (" << cap_x << "," << cap_y << ")";
            }
        }
        qDebug() << "DEBUG: Processed" << all_captures.size() << "server-specified captured stones";
    }
    
    // 🔍 AUDIT: Log the completed move audit entry
    logMoveAudit(audit);
    
    qDebug() << "DEBUG: Placed move" << move.move_number << "at (" << move.x << "," << move.y << ") color:" << (move.color == BLACK_STONE ? "BLACK" : "WHITE");
    
    current_move = move.move_number;
    current_player = (move.color == BLACK_STONE) ? WHITE_STONE : BLACK_STONE;
    
    // Reset lag compensation timer when player switches
    last_time_update = QDateTime::currentDateTime();
    
    updateLabels();
    
    // Update move navigation to current move (auto-follow) - use server count
    current_move_index = server_move_count > 0 ? server_move_count : move_history.size();
    updateMoveNavigation();
}

void BoardWindow::updateGameInfo(const QString &info) {
    // Handle any additional game information updates
    updateLabels();
}

void BoardWindow::updateTimeInfo(const QString &white_time, const QString &black_time) {
    time_label->setText(QString("Time: W:%1 B:%2").arg(white_time, black_time));
}

void BoardWindow::updateByoyomi(int white_time, int black_time, int white_moves, int black_moves) {
    // Store the time and move data for lag compensation
    white_time_seconds = white_time;
    black_time_seconds = black_time; 
    white_byo_moves = white_moves;
    black_byo_moves = black_moves;
    
    // Record timestamp for lag compensation
    last_time_update = QDateTime::currentDateTime();
    
    // Update display immediately with server data
    updateClockDisplay();
    
    qDebug() << "DEBUG: IGS time update - W:" << white_time << "s (" << white_moves << "moves) B:" << black_time << "s (" << black_moves << "moves)";
}

void BoardWindow::updateClockDisplay() {
    if (!is_observing || last_time_update.isNull()) {
        return;
    }
    
    // Calculate elapsed time since last IGS update
    qint64 elapsed_ms = last_time_update.msecsTo(QDateTime::currentDateTime());
    int elapsed_seconds = elapsed_ms / 1000;
    
    // Apply lag compensation - only countdown current player's time
    int current_white_time = white_time_seconds;
    int current_black_time = black_time_seconds;
    
    if (current_player == WHITE_STONE) {
        current_white_time = qMax(0, white_time_seconds - elapsed_seconds);
    } else if (current_player == BLACK_STONE) {
        current_black_time = qMax(0, black_time_seconds - elapsed_seconds);
    }
    
    // Convert time from seconds to MM:SS format like q5Go
    auto formatTime = [](int seconds) -> QString {
        int minutes = seconds / 60;
        int secs = seconds % 60;
        return QString("%1:%2").arg(minutes).arg(secs, 2, 10, QChar('0'));
    };
    
    // Format: "7:37 / 15" like in q5Go, or just "5:26" for main time
    QString white_display, black_display;
    
    if (white_byo_moves >= 0) {
        // In byoyomi period - show time / moves format
        white_display = QString("%1 / %2").arg(formatTime(current_white_time)).arg(white_byo_moves);
    } else {
        // In main time - show just time
        white_display = formatTime(current_white_time);
    }
    
    if (black_byo_moves >= 0) {
        // In byoyomi period - show time / moves format
        black_display = QString("%1 / %2").arg(formatTime(current_black_time)).arg(black_byo_moves);
    } else {
        // In main time - show just time
        black_display = formatTime(current_black_time);
    }
    
    time_label->setText(QString("W: %1  B: %2").arg(white_display, black_display));
}

void BoardWindow::saveGame() {
    if (!is_observing) {
        return;
    }
    
    // Create filename with game info
    QString filename = QString("game_%1_%2_vs_%3_%4.sgf")
                      .arg(observed_game_id)
                      .arg(white_player)
                      .arg(black_player)
                      .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    
    QString sgf_content = generateSGF();
    
    // Save to file
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << sgf_content;
        file.close();
        
        if (comment_display) {
            comment_display->append(QString("✓ Game saved to: %1").arg(filename));
        }
    } else {
        if (comment_display) {
            comment_display->append(QString("✗ Error saving game to: %1").arg(filename));
        }
    }
}

void BoardWindow::closeBoard() {
    if (is_observing) {
        emit boardClosed(observed_game_id);
    }
    close();
}

void BoardWindow::updateLabels() {
    game_info_label->setText(formatGameInfo());
    players_label->setText(formatPlayersInfo());
    move_info_label->setText(formatMoveInfo());
    handicap_komi_label->setText(formatHandicapKomiInfo());
    if (!game_result.isEmpty()) {
        result_label->setText(game_result);
    }
}

QString BoardWindow::formatGameInfo() {
    if (is_observing) {
        return QString("Game #%1").arg(observed_game_id);
    }
    return "No game observed";
}

QString BoardWindow::formatPlayersInfo() {
    if (is_observing) {
        return QString("White: %1 [%2]\nBlack: %3 [%4]")
               .arg(white_player, white_rank, black_player, black_rank);
    }
    return "";
}

QString BoardWindow::formatMoveInfo() {
    if (is_observing) {
        if (is_scoring_mode) {
            // Show scoring information
            return QString("SCORING MODE\nWhite: %.1f\nBlack: %.1f\nScore: %+.1f")
                   .arg(white_territory + white_prisoners + komi)
                   .arg(black_territory + black_prisoners)
                   .arg(final_score);
        } else {
            QString next_player = (current_player == BLACK_STONE) ? "Black" : "White";
            return QString("Move: %1\nNext: %2").arg(current_move).arg(next_player);
        }
    }
    return "";
}

QString BoardWindow::formatHandicapKomiInfo() {
    if (is_observing) {
        // Format: "Captures: W/B • H: X • Komi: X.X • Byoyomi: Xs • Type"
        QStringList details;
        
        // Always show captures
        details << QString("Captures: %1/%2").arg(white_captures).arg(black_captures);
        
        // Show handicap if > 0
        if (handicap > 0) {
            details << QString("H: %1").arg(handicap);
        }
        
        // Always show komi
        details << QString("Komi: %1").arg(komi, 0, 'f', 1);
        
        // Show byoyomi if available
        if (byoyomi_time > 0) {
            details << QString("Byoyomi: %1s").arg(byoyomi_time);
        }
        
        // Show game type
        if (!game_type.isEmpty()) {
            details << game_type;
        }
        
        return details.join(" • ");
    }
    return "";
}

void BoardWindow::processComment(const QString &user, const QString &message, bool is_kibitz) {
    if (!comment_display) return;
    
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString prefix = is_kibitz ? "KIBITZ" : "COMMENT";
    QString formatted_message = QString("[%1] %2 %3: %4")
                               .arg(timestamp)
                               .arg(prefix)
                               .arg(user)
                               .arg(message);
    
    comment_display->append(formatted_message);
    
    // Auto-scroll to bottom
    QTextCursor cursor = comment_display->textCursor();
    cursor.movePosition(QTextCursor::End);
    comment_display->setTextCursor(cursor);
}

void BoardWindow::processUndo(int move_number) {
    if (!is_observing) return;
    
    // Undo moves back to the specified move number
    // This would require rebuilding the board state from move history
    // For now, just display a message
    QString message = QString("Game %1: Undo to move %2").arg(observed_game_id).arg(move_number);
    processComment("SYSTEM", message, false);
    
    // TODO: Implement actual undo logic by replaying moves from history
    current_move = move_number;
    updateLabels();
}

void BoardWindow::sendComment() {
    qDebug() << "BoardWindow::sendComment() called";
    
    if (!comment_input) {
        qDebug() << "ERROR: comment_input is null";
        // Show error in comment display if possible
        if (comment_display) {
            comment_display->append("ERROR: Comment input widget not initialized");
        }
        return;
    }
    
    QString message = comment_input->text().trimmed();
    qDebug() << "Raw input text:" << comment_input->text();
    qDebug() << "Trimmed message:" << message;
    qDebug() << "Message length:" << message.length();
    qDebug() << "Is observing:" << is_observing;
    qDebug() << "Game ID:" << observed_game_id;
    
    if (message.isEmpty()) {
        qDebug() << "ERROR: comment input is empty";
        if (comment_display) {
            comment_display->append("ERROR: Please enter a message before clicking Comment");
        }
        return;
    }
    
    // Show the message locally first
    if (comment_display) {
        comment_display->append(QString("LOCAL: %1").arg(message));
    }
    
    comment_input->clear();
    
    if (is_observing) {
        qDebug() << "Emitting commentRequested signal with game_id:" << observed_game_id;
        emit commentRequested(observed_game_id, message);
    } else {
        qDebug() << "ERROR: Not observing a game";
        if (comment_display) {
            comment_display->append("ERROR: Not currently observing a game");
        }
    }
}


void BoardWindow::onCommentInputReturn() {
    // Default to sending as comment when Enter is pressed
    sendComment();
}

void BoardWindow::requestObservers() {
    qDebug() << "*** BUTTON CLICKED - requestObservers() called! ***";
    qDebug() << "*** is_observing:" << is_observing << "observed_game_id:" << observed_game_id;
    
    // Force add test observer immediately to confirm button works
    clearObservers();
    addObserver("BUTTON_TEST", "1k");
    addObserver("CLICK_WORKS", "2d");
    
    if (comment_display) {
        comment_display->append("*** BUTTON CLICKED - Test observers added! ***");
    }
    
    if (is_observing) {
        qDebug() << "Emitting observersRequested signal for game:" << observed_game_id;
        emit observersRequested(observed_game_id);
        
        if (comment_display) {
            comment_display->append(QString(">>> Requesting observers for game %1...").arg(observed_game_id));
        }
    } else {
        qDebug() << "Not observing a game - cannot request observers";
        if (comment_display) {
            comment_display->append("ERROR: Not observing a game");
        }
    }
}

void BoardWindow::updateGameSetup(int handicap_stones, double komi_points, const QString &time_ctrl) {
    qDebug() << "*** DEBUG KOMI: updateGameSetup called with komi=" << komi_points << "handicap=" << handicap_stones;
    handicap = handicap_stones;
    komi = komi_points;
    time_control = time_ctrl;
    qDebug() << "*** DEBUG KOMI: After assignment, this->komi=" << this->komi;
    updateLabels();
}

void BoardWindow::updateGameDetails(const QString &type, int byoyomi_seconds) {
    // Don't overwrite Teaching type if it was set by custom title
    if (game_type != "Teaching") {
        game_type = type;
    }
    byoyomi_time = byoyomi_seconds;
    updateLabels();
    
    qDebug() << "updateGameDetails called with type:" << type << "- Current game_type:" << game_type;
}

void BoardWindow::updateCaptures(int white_caps, int black_caps) {
    white_captures = white_caps;
    black_captures = black_caps;
    updateLabels();
}

void BoardWindow::updateGameResult(const QString &result) {
    game_result = result;
    game_finished = true;
    
    // Display result in comment area (q5Go style)
    if (comment_display) {
        QString standard_result = convertIGSResultToStandard(result);
        QString result_message = QString("Game finished: %1 (%2)").arg(result).arg(standard_result);
        comment_display->append(result_message);
    }
    
    updateLabels();
    
    // Update window title to show game finished
    updateWindowTitle();
}

void BoardWindow::updatePlayerNames(const QString &white, const QString &black) {
    white_player = white;
    black_player = black;
    updateLabels();
    updateWindowTitle();
    
    qDebug() << "Updated player names: White:" << white_player << "Black:" << black_player;
}

void BoardWindow::setCustomGameTitle(const QString &title) {
    custom_game_title = title;
    
    // Teaching games are the only ones with custom titles, so update game type
    game_type = "Teaching";
    
    updateLabels(); // Update labels to show Teaching type
    updateWindowTitle();
    qDebug() << "Set custom game title:" << custom_game_title << "- Game type changed to Teaching";
}

void BoardWindow::updateWindowTitle() {
    if (!is_observing) {
        setWindowTitle("XGospel 2.0 - Board Window");
        return;
    }
    
    if (game_finished) {
        setWindowTitle(QString("XGospel 2.0 - Game %1 [FINISHED]").arg(observed_game_id));
        return;
    }
    
    // Use custom title if available (for teaching games), otherwise format like q5Go
    QString title;
    if (!custom_game_title.isEmpty()) {
        title = QString("Observe: %1 XGospel 2.0").arg(custom_game_title);
    } else if (!white_player.isEmpty() && !black_player.isEmpty()) {
        QString white_info = white_player;
        if (!white_rank.isEmpty() && white_rank != "?") {
            white_info += QString(" [%1]").arg(white_rank);
        }
        
        QString black_info = black_player;
        if (!black_rank.isEmpty() && black_rank != "?") {
            black_info += QString(" [%1]").arg(black_rank);
        }
        
        title = QString("Observe: %1 vs. %2 XGospel 2.0").arg(white_info, black_info);
    } else {
        title = QString("XGospel 2.0 - Observing Game %1").arg(observed_game_id);
    }
    
    setWindowTitle(title);
}

void BoardWindow::clearObservers() {
    if (observers_list) {
        observers_list->clear();
    }
}

void BoardWindow::addObserver(const QString &name, const QString &rank) {
    if (!observers_list) return;
    
    QString observer_text = QString("%1 %2").arg(name, rank);
    observers_list->addItem(observer_text);
}

QString BoardWindow::generateSGF() {
    QString sgf;
    
    // SGF header - following q5Go format exactly
    sgf += "(;FF[4]GM[1]CA[UTF-8]AP[q5go:2.0]\n";
    sgf += "SZ[19]\n";
    sgf += QString("KM[%1]\n").arg(komi, 0, 'f', 6); // Use q5Go's 6 decimal precision
    
    if (handicap > 0) {
        sgf += QString("HA[%1]").arg(handicap);
    }
    
    sgf += QString("PW[%1]\n").arg(white_player);
    sgf += QString("PB[%1]\n").arg(black_player);
    sgf += QString("WR[%1]\n").arg(white_rank);
    sgf += QString("BR[%1]\n").arg(black_rank);
    sgf += QString("DT[%1]\n").arg(game_start_time.toString("yyyy-MM-dd"));
    sgf += "PC[IGS]\n";
    
    // Add time control information if available (q5Go style)
    if (!time_control.isEmpty()) {
        sgf += QString("TM[%1]\n").arg(time_control);
    }
    if (byoyomi_time > 0) {
        sgf += QString("OT[25/600 Canadian]\n"); // Standard IGS overtime
    }
    
    sgf += QString("RE[%1]").arg(game_result.isEmpty() ? "?" : game_result);
    
    // Add moves - include ALL moves (both reconstruction and live) like q5Go
    QList<GameMove> filtered_moves;
    
    // q5Go-style: Include ALL valid moves regardless of live/reconstruction status
    for (const GameMove &move : move_history) {
        // Ensure this is a valid board coordinate or pass move
        if ((move.x >= 0 && move.x < 19 && move.y >= 0 && move.y < 19) || (move.x == -1 && move.y == -1)) {
            filtered_moves.append(move);
        }
    }
    
    qDebug() << "📊 SGF: Original moves:" << move_history.size() << "Live moves:" << filtered_moves.size() 
             << "Observation state:" << observation_state << "Live count:" << moves_received_during_live;
    
    // Convert filtered moves to SGF format
    for (const GameMove &move : filtered_moves) {
        QString move_color = (move.color == BLACK_STONE) ? "B" : "W";
        
        if (move.x >= 0 && move.x < 19 && move.y >= 0 && move.y < 19) {
            // Regular move
            char col = 'a' + move.x;
            char row = 'a' + move.y;
            sgf += QString(";%1[%2%3]").arg(move_color).arg(col).arg(row);
        } else if (move.x == -1 && move.y == -1) {
            // Pass move
            sgf += QString(";%1[]").arg(move_color);
        }
    }
    
    // Add territory information if in scoring mode (q5Go style)
    if (is_scoring_mode && game_finished) {
        // Add white territory (TW)
        if (white_territory > 0) {
            sgf += "TW";
            // For now, add basic territory marker - this would need proper territory calculation
            sgf += "[aa]"; // Placeholder - should be calculated territory
        }
        
        // Add black territory (TB)
        if (black_territory > 0) {
            sgf += "TB";
            // For now, add basic territory marker - this would need proper territory calculation
            sgf += "[tt]"; // Placeholder - should be calculated territory
        }
    }
    
    // Add game result comment if available (q5Go style)
    if (!game_result.isEmpty() && game_finished) {
        // Format like q5Go: C[(move_number) Result text.\n(move_number) Formatted_result\n]
        QString result_comment = formatGameResultComment();
        if (!result_comment.isEmpty()) {
            sgf += result_comment;
        }
    }
    
    sgf += ")\n";
    return sgf;
}

QString BoardWindow::formatGameResultComment() {
    if (game_result.isEmpty()) {
        return QString();
    }
    
    // Get current move number for the comment (total moves played)
    int final_move_number = 0;
    for (const GameMove &move : move_history) {
        if (move.is_live_move && move.move_number > final_move_number) {
            final_move_number = move.move_number;
        }
    }
    
    // Convert IGS result format to standard format
    QString standard_result = convertIGSResultToStandard(game_result);
    
    // Format like q5Go: C[(199) White resigns.\n(199) B+R\n]
    QString comment = QString("\nC[(%1) %2\\n(%3) %4\\n]")
                        .arg(final_move_number)
                        .arg(game_result)
                        .arg(final_move_number)  
                        .arg(standard_result);
    
    return comment;
}

QString BoardWindow::convertIGSResultToStandard(const QString &igs_result) {
    // Convert IGS result format to standard SGF result format
    QString result = igs_result.trimmed();
    
    // Handle resign cases
    if (result.contains("resign", Qt::CaseInsensitive)) {
        if (result.contains("Black", Qt::CaseInsensitive) || result.contains("black", Qt::CaseInsensitive)) {
            return "W+R";  // White wins by resignation
        } else if (result.contains("White", Qt::CaseInsensitive) || result.contains("white", Qt::CaseInsensitive)) {
            return "B+R";  // Black wins by resignation  
        }
    }
    
    // Handle time loss cases
    if (result.contains("time", Qt::CaseInsensitive)) {
        if (result.contains("Black", Qt::CaseInsensitive) || result.contains("black", Qt::CaseInsensitive)) {
            return "W+T";  // White wins by time
        } else if (result.contains("White", Qt::CaseInsensitive) || result.contains("white", Qt::CaseInsensitive)) {
            return "B+T";  // Black wins by time
        }
    }
    
    // Handle score results (e.g., "Black won by 5.5 points")
    QRegExp score_re("(Black|White).*?(\\d+\\.?\\d*)\\s*points?", Qt::CaseInsensitive);
    if (score_re.indexIn(result) != -1) {
        QString winner = score_re.cap(1).toLower();
        QString score = score_re.cap(2);
        if (winner == "black") {
            return QString("B+%1").arg(score);
        } else {
            return QString("W+%1").arg(score);
        }
    }
    
    // Default: return the original result
    return result;
}

// q5Go-style group tracking and capture detection
int BoardWindow::addStoneWithCaptures(int x, int y, StoneColor color) {
    QPair<int, int> new_pos(x, y);
    int total_captured = 0;
    
    // Get reference to groups
    QList<StoneGroup>& opponent_groups = (color == BLACK_STONE) ? white_groups : black_groups;
    QList<StoneGroup>& player_groups = (color == BLACK_STONE) ? black_groups : white_groups;
    
    // Step 1: Check for captures of opponent groups (q5Go logic)
    QSet<QPair<int, int>> adjacent_to_new_stone = getAdjacentPositions(QSet<QPair<int, int>>() << new_pos);
    
    qDebug() << "DEBUG: Move" << x << "," << y << "by" << (color == BLACK_STONE ? "BLACK" : "WHITE");
    qDebug() << "DEBUG: Checking" << opponent_groups.size() << "opponent groups";
    
    for (auto& group : opponent_groups) {
        // Check if this group is adjacent to the new stone
        if (group.stones.intersects(adjacent_to_new_stone)) {
            qDebug() << "DEBUG: Group adjacent - has" << group.liberties << "liberties";
            
            // q5Go logic: if group currently has 1 liberty, it will be captured
            if (group.liberties == 1) {
                qDebug() << "DEBUG: *** CAPTURING opponent group of" << group.stones.size() << "stones (had 1 liberty) ***";
                
                // Remove stones from board
                for (const auto& pos : group.stones) {
                    qDebug() << "DEBUG: Removing stone at (" << pos.first << "," << pos.second << ")";
                    board_widget->placeMoveAt(pos.first, pos.second, EMPTY);
                }
                
                total_captured += group.stones.size();
                group.alive = false; // Mark for removal
                
                qDebug() << "DEBUG: Total captured this move:" << total_captured;
            } else {
                // Reduce liberty count (q5Go: it.m_n_liberties--)
                group.liberties--;
                qDebug() << "DEBUG: Opponent group of size" << group.stones.size() << "reduced to" << group.liberties << "liberties";
            }
        } else {
            qDebug() << "DEBUG: Group not adjacent - no liberty change";
        }
    }
    
    // Remove captured groups
    removeDeadGroups(opponent_groups);
    
    // Step 2: Merge new stone with adjacent friendly groups
    QSet<QPair<int, int>> new_group_stones;
    new_group_stones.insert(new_pos);
    
    QList<int> groups_to_merge;
    for (int i = 0; i < player_groups.size(); i++) {
        if (player_groups[i].stones.intersects(adjacent_to_new_stone)) {
            groups_to_merge.append(i);
            new_group_stones.unite(player_groups[i].stones);
        }
    }
    
    // Remove merged groups (in reverse order to maintain indices)
    for (int i = groups_to_merge.size() - 1; i >= 0; i--) {
        player_groups.removeAt(groups_to_merge[i]);
    }
    
    // Add new merged group
    int new_liberties = countGroupLiberties(new_group_stones);
    player_groups.append(StoneGroup(new_group_stones, new_liberties));
    
    qDebug() << "DEBUG: Created/merged group of size" << new_group_stones.size() << "with" << new_liberties << "liberties";
    
    // DEBUG: Rebuild all groups after every move to ensure accuracy
    rebuildAllGroups();
    
    // Step 3: Check for suicide (if new group has 0 liberties)
    if (new_liberties == 0 && total_captured == 0) {
        qDebug() << "DEBUG: Suicide move detected - removing own group";
        
        // Remove own stones
        for (const auto& pos : new_group_stones) {
            board_widget->placeMoveAt(pos.first, pos.second, EMPTY);
        }
        
        // Count as captures for opponent
        if (color == BLACK_STONE) {
            white_captures += new_group_stones.size();
        } else {
            black_captures += new_group_stones.size();
        }
        
        // Remove the suicided group
        player_groups.removeLast();
    }
    
    return total_captured;
}

int BoardWindow::countGroupLiberties(const QSet<QPair<int, int>>& group) {
    QSet<QPair<int, int>> liberties;
    
    for (const auto& pos : group) {
        int dx[] = {0, 0, -1, 1};
        int dy[] = {-1, 1, 0, 0};
        
        for (int i = 0; i < 4; i++) {
            int adj_x = pos.first + dx[i];
            int adj_y = pos.second + dy[i];
            
            if (adj_x >= 0 && adj_x < 19 && adj_y >= 0 && adj_y < 19) {
                if (board_widget->getBoardState(adj_x, adj_y) == EMPTY) {
                    liberties.insert(QPair<int, int>(adj_x, adj_y));
                }
            }
        }
    }
    
    return liberties.size();
}

QSet<QPair<int, int>> BoardWindow::getAdjacentPositions(const QSet<QPair<int, int>>& group) {
    QSet<QPair<int, int>> adjacent;
    
    int dx[] = {0, 0, -1, 1};
    int dy[] = {-1, 1, 0, 0};
    
    for (const auto& pos : group) {
        for (int i = 0; i < 4; i++) {
            int adj_x = pos.first + dx[i];
            int adj_y = pos.second + dy[i];
            
            if (adj_x >= 0 && adj_x < 19 && adj_y >= 0 && adj_y < 19) {
                adjacent.insert(QPair<int, int>(adj_x, adj_y));
            }
        }
    }
    
    return adjacent;
}

QSet<QPair<int, int>> BoardWindow::floodFillGroup(int x, int y, StoneColor color) {
    QSet<QPair<int, int>> group;
    QList<QPair<int, int>> stack;
    stack.append(QPair<int, int>(x, y));
    
    while (!stack.isEmpty()) {
        QPair<int, int> current = stack.takeLast();
        
        if (group.contains(current)) continue;
        
        int cx = current.first;
        int cy = current.second;
        
        if (cx < 0 || cx >= 19 || cy < 0 || cy >= 19) continue;
        if (board_widget->getBoardState(cx, cy) != color) continue;
        
        group.insert(current);
        
        int dx[] = {0, 0, -1, 1};
        int dy[] = {-1, 1, 0, 0};
        for (int i = 0; i < 4; i++) {
            stack.append(QPair<int, int>(cx + dx[i], cy + dy[i]));
        }
    }
    
    return group;
}

void BoardWindow::rebuildAllGroups() {
    white_groups.clear();
    black_groups.clear();
    
    QSet<QPair<int, int>> visited;
    
    // Find all groups on the board
    for (int x = 0; x < 19; x++) {
        for (int y = 0; y < 19; y++) {
            QPair<int, int> pos(x, y);
            if (visited.contains(pos)) continue;
            
            StoneColor color = board_widget->getBoardState(x, y);
            if (color == EMPTY) continue;
            
            QSet<QPair<int, int>> group = floodFillGroup(x, y, color);
            for (const auto& group_pos : group) {
                visited.insert(group_pos);
            }
            
            int liberties = countGroupLiberties(group);
            
            if (color == WHITE_STONE) {
                white_groups.append(StoneGroup(group, liberties));
            } else {
                black_groups.append(StoneGroup(group, liberties));
            }
        }
    }
    
    qDebug() << "DEBUG: Rebuilt groups - White:" << white_groups.size() << "Black:" << black_groups.size();
}

void BoardWindow::removeDeadGroups(QList<StoneGroup>& groups) {
    // First, remove stones from the board display
    for (auto it = groups.begin(); it != groups.end(); ) {
        if (!it->alive) {
            // Remove all stones in this group from the board
            for (const QPair<int, int>& pos : it->stones) {
                int x = pos.first;
                int y = pos.second;
                if (x >= 0 && x < 19 && y >= 0 && y < 19) {
                    board_widget->placeMoveAt(x, y, EMPTY); // Clear the stone
                    qDebug() << "DEBUG: Removed captured stone at (" << x << "," << y << ")";
                }
            }
            it = groups.erase(it);
        } else {
            ++it;
        }
    }
}

// Counting phase implementation
void BoardWindow::enterScoringMode() {
    if (is_scoring_mode) return;
    
    is_scoring_mode = true;
    dead_stones.clear();
    white_territory = 0;
    black_territory = 0;
    white_prisoners = white_captures; // Start with captured stones
    black_prisoners = black_captures;
    
    qDebug() << "Entered scoring mode for game" << observed_game_id;
    
    // Enable visual scoring mode on the board
    if (board_widget) {
        board_widget->setScoringMode(true);
    }
    
    // Update UI to show scoring mode
    updateLabels();
    
    // Calculate initial score
    calculateScore();
}

void BoardWindow::exitScoringMode() {
    if (!is_scoring_mode) return;
    
    is_scoring_mode = false;
    dead_stones.clear();
    
    // Disable visual scoring mode on the board
    if (board_widget) {
        board_widget->setScoringMode(false);
    }
    
    qDebug() << "Exited scoring mode for game" << observed_game_id;
    updateLabels();
}

void BoardWindow::markStoneAsDead(int x, int y) {
    if (!is_scoring_mode) return;
    if (x < 0 || x >= 19 || y < 0 || y >= 19) return;
    
    // Check if there's a stone at this position
    StoneColor stone_color = board_widget->getStoneAt(x, y);
    if (stone_color == EMPTY) return;
    
    // Find the entire connected group using flood fill
    QSet<QPair<int, int>> connected_group = floodFillGroup(x, y, stone_color);
    
    QPair<int, int> clicked_pos(x, y);
    bool currently_marked = dead_stones.contains(clicked_pos);
    
    if (currently_marked) {
        // Unmark the entire connected group as dead
        for (const auto& pos : connected_group) {
            dead_stones.remove(pos);
        }
        qDebug() << "Unmarked connected group of" << connected_group.size() << "stones as dead";
    } else {
        // Mark the entire connected group as dead
        for (const auto& pos : connected_group) {
            dead_stones.insert(pos);
        }
        qDebug() << "Marked connected group of" << connected_group.size() << "stones as dead";
    }
    
    // Update board visualization
    board_widget->setDeadStones(dead_stones);
    
    // Recalculate score
    calculateScore();
}

bool BoardWindow::hasStoneAt(int x, int y) const {
    if (x < 0 || x >= 19 || y < 0 || y >= 19) return false;
    StoneColor color = board_widget->getStoneAt(x, y);
    return (color != EMPTY);
}

void BoardWindow::clearDeadStones() {
    dead_stones.clear();
    board_widget->setDeadStones(dead_stones);
}

void BoardWindow::setBoardPosition(int x, int y, StoneColor color) {
    if (x < 0 || x >= 19 || y < 0 || y >= 19) return;
    board_widget->placeMoveAt(x, y, color);
}

void BoardWindow::markTerritory(int x, int y, StoneColor owner) {
    // For now, just log territory marking - we can enhance this later
    qDebug() << "Territory marked at (" << x << "," << y << ") for" << 
                (owner == WHITE_STONE ? "WHITE" : owner == BLACK_STONE ? "BLACK" : "NEUTRAL");
}

void BoardWindow::logMoveAudit(const MoveAudit& audit) {
    // Circular buffer - remove oldest if at capacity
    if (move_audit_trail.size() >= MAX_AUDIT_ENTRIES) {
        move_audit_trail.removeFirst();
    }
    
    move_audit_trail.append(audit);
    
    // Log critical info immediately
    qDebug() << QString("🔍 AUDIT[%1]: %2 → (%3,%4) %5 SUCCESS=%6")
                .arg(audit.move_number)
                .arg(audit.parsed_coords)
                .arg(audit.final_x)
                .arg(audit.final_y)
                .arg(audit.color == BLACK_STONE ? "BLACK" : "WHITE")
                .arg(audit.placement_success ? "YES" : "NO");
}

void BoardWindow::dumpMoveAuditTrail() {
    qDebug() << "📋 MOVE AUDIT TRAIL DUMP (" << move_audit_trail.size() << " entries):";
    for (const auto& audit : move_audit_trail) {
        qDebug() << QString("  [%1] %2 %3→(%4,%5) %6 %7 SUCCESS=%8")
                    .arg(audit.move_number)
                    .arg(audit.timestamp)
                    .arg(audit.parsed_coords)
                    .arg(audit.final_x)
                    .arg(audit.final_y)
                    .arg(audit.color == BLACK_STONE ? "B" : "W")
                    .arg(audit.captures.isEmpty() ? "no-captures" : audit.captures)
                    .arg(audit.placement_success ? "YES" : "NO");
    }
}

void BoardWindow::dumpBoardState() {
    qDebug() << "🎯 BOARD STATE DUMP for game" << observed_game_id << ":";
    qDebug() << "   Move count:" << move_history.size() << "Current move:" << current_move;
    qDebug() << "   Scoring mode:" << (is_scoring_mode ? "YES" : "NO");
    qDebug() << "   Dead stones:" << dead_stones.size();
    
    // Sample a few key positions to verify board state
    QStringList sample_positions = {"Q16", "D4", "Q4", "D16", "P11", "K10", "J17"};
    for (const QString& pos : sample_positions) {
        if (pos.length() >= 2) {
            QChar letter = pos[0];
            int number = pos.mid(1).toInt();
            int x = letter.unicode() - 'A' - (letter > 'I' ? 1 : 0);
            int y = 19 - number;
            if (x >= 0 && x < 19 && y >= 0 && y < 19) {
                StoneColor color = board_widget->getStoneAt(x, y);
                QString color_str = (color == BLACK_STONE ? "BLACK" : 
                                   color == WHITE_STONE ? "WHITE" : "EMPTY");
                qDebug() << QString("   %1 (%2,%3): %4").arg(pos).arg(x).arg(y).arg(color_str);
            }
        }
    }
}

void BoardWindow::calculateScore() {
    if (!is_scoring_mode) return;
    
    white_territory = 0;
    black_territory = 0;
    white_prisoners = white_captures;
    black_prisoners = black_captures;
    
    // Count dead stones as prisoners
    for (const auto& pos : dead_stones) {
        StoneColor stone_color = board_widget->getStoneAt(pos.first, pos.second);
        if (stone_color == WHITE_STONE) {
            black_prisoners++;
        } else if (stone_color == BLACK_STONE) {
            white_prisoners++;
        }
    }
    
    // Calculate territory using flood fill
    calculateTerritory();
    
    // Calculate final score (Japanese rules: territory + prisoners + komi)
    final_score = (white_territory + white_prisoners + komi) - (black_territory + black_prisoners);
    
    qDebug() << "Score calculated - White territory:" << white_territory << "prisoners:" << white_prisoners << "total:" << (white_territory + white_prisoners + komi)
             << "Black territory:" << black_territory << "prisoners:" << black_prisoners << "total:" << (black_territory + black_prisoners)
             << "Final score:" << final_score;
    
    // Format score result like q5Go (e.g., "B+5.5" or "W+12.0")
    if (final_score > 0) {
        game_result = QString("W+%1").arg(final_score, 0, 'f', 1);
    } else if (final_score < 0) {
        game_result = QString("B+%1").arg(-final_score, 0, 'f', 1);
    } else {
        game_result = "Draw";
    }
    
    updateLabels();
}

void BoardWindow::calculateTerritory() {
    if (!board_widget) return;
    
    int board_size = 19; // Assuming 19x19 board
    QSet<QPair<int, int>> visited;
    QMap<QPair<int, int>, StoneColor> territory_map;
    
    white_territory = 0;
    black_territory = 0;
    
    // Iterate through all empty points on the board
    for (int x = 0; x < board_size; x++) {
        for (int y = 0; y < board_size; y++) {
            QPair<int, int> pos(x, y);
            
            // Skip if already visited or if there's a stone
            if (visited.contains(pos)) continue;
            
            StoneColor stone = board_widget->getStoneAt(x, y);
            if (stone != EMPTY) continue;
            
            // Skip if this empty point has a dead stone
            if (dead_stones.contains(pos)) continue;
            
            // Use flood fill to find connected empty territory
            QSet<QPair<int, int>> territory_visited;
            StoneColor owner = getTerritoryOwner(x, y, territory_visited);
            
            // Add all points in this territory to global visited set
            visited.unite(territory_visited);
            
            // Add territory points to visualization map
            for (const auto& territory_point : territory_visited) {
                territory_map[territory_point] = owner;
            }
            
            // Count territory points for the owner
            if (owner == WHITE_STONE) {
                white_territory += territory_visited.size();
            } else if (owner == BLACK_STONE) {
                black_territory += territory_visited.size();
            }
            // If owner is EMPTY, it's neutral territory (not counted for either side)
        }
    }
    
    // Update visual display with territory and dead stones
    if (board_widget) {
        board_widget->setTerritoryMap(territory_map);
        board_widget->setDeadStones(dead_stones);
    }
}

StoneColor BoardWindow::getTerritoryOwner(int x, int y, QSet<QPair<int, int>>& visited) {
    if (!board_widget) return EMPTY;
    
    int board_size = 19; // Assuming 19x19 board
    QStack<QPair<int, int>> stack;
    QSet<StoneColor> adjacent_colors;
    
    stack.push(QPair<int, int>(x, y));
    
    while (!stack.isEmpty()) {
        QPair<int, int> current = stack.pop();
        int cx = current.first;
        int cy = current.second;
        
        // Skip if out of bounds or already visited
        if (cx < 0 || cx >= board_size || cy < 0 || cy >= board_size || visited.contains(current)) {
            continue;
        }
        
        StoneColor stone = board_widget->getStoneAt(cx, cy);
        
        // If there's a live stone, record its color and don't expand further
        if (stone != EMPTY && !dead_stones.contains(current)) {
            adjacent_colors.insert(stone);
            continue;
        }
        
        // If it's empty or a dead stone, add to territory
        visited.insert(current);
        
        // Add adjacent points to stack for flood fill
        stack.push(QPair<int, int>(cx - 1, cy));
        stack.push(QPair<int, int>(cx + 1, cy));
        stack.push(QPair<int, int>(cx, cy - 1));
        stack.push(QPair<int, int>(cx, cy + 1));
    }
    
    // Determine territory owner based on surrounding stones
    if (adjacent_colors.size() == 1) {
        // Territory is surrounded by only one color
        return *adjacent_colors.begin();
    } else {
        // Territory is contested or surrounded by multiple colors (neutral)
        return EMPTY;
    }
}

// Move navigation implementation
void BoardWindow::onFirstMoveClicked() {
    goToFirstMove();
}

void BoardWindow::onPreviousMoveClicked() {
    goToPreviousMove();
}

void BoardWindow::onNextMoveClicked() {
    goToNextMove();
}

void BoardWindow::onLastMoveClicked() {
    goToLastMove();
}

void BoardWindow::onMoveSliderChanged(int value) {
    goToMove(value);
}

void BoardWindow::goToMove(int move_index) {
    if (move_index < 0 || move_index > move_history.size()) {
        return;
    }
    
    current_move_index = move_index;
    
    // Clear the board and reset groups for accurate replay
    board_widget->clearBoard();
    white_groups.clear();
    black_groups.clear();
    
    // Replay moves up to the specified index with proper capture calculation
    for (int i = 0; i < move_index && i < move_history.size(); i++) {
        const GameMove& move = move_history[i];
        
        // Handle handicap stones
        if (move.x == -2 && move.y > 0) {
            // Handicap placement - place stones without capture calculation
            QList<QPair<int, int>> handicap_positions = IGSMoveParser::getHandicapPositions(move.y);
            for (const auto& pos : handicap_positions) {
                board_widget->placeMoveAt(pos.first, pos.second, BLACK_STONE);
            }
            continue;
        }
        
        // Handle pass moves
        if (move.x == -1 && move.y == -1) {
            // Pass move - no board changes
            continue;
        }
        
        // Handle regular moves with client-side capture calculation
        if (move.x >= 0 && move.y >= 0 && move.x < 19 && move.y < 19) {
            // First place the stone
            board_widget->placeMoveAt(move.x, move.y, move.color);
            
            // Calculate and execute captures using our group logic
            int captured_count = addStoneWithCaptures(move.x, move.y, move.color);
            
            if (captured_count > 0) {
                qDebug() << "DEBUG: Move replay - move" << i << "captured" << captured_count << "stones";
            }
        }
    }
    
    updateMoveNavigation();
    board_widget->update();
}

void BoardWindow::goToFirstMove() {
    goToMove(0);
}

void BoardWindow::goToPreviousMove() {
    if (current_move_index > 0) {
        goToMove(current_move_index - 1);
    }
}

void BoardWindow::goToNextMove() {
    if (current_move_index < move_history.size()) {
        goToMove(current_move_index + 1);
    }
}

void BoardWindow::goToLastMove() {
    goToMove(move_history.size());
}

void BoardWindow::updateMoveNavigation() {
    // Use server-provided move count instead of move_history.size() for accuracy
    int total_moves = server_move_count > 0 ? server_move_count : move_history.size();
    
    // Temporarily disconnect slider to prevent triggering goToMove during live game updates
    disconnect(move_slider, &QSlider::valueChanged, this, &BoardWindow::onMoveSliderChanged);
    
    // Update slider
    move_slider->setMaximum(total_moves);
    move_slider->setValue(current_move_index);
    
    // Reconnect slider
    connect(move_slider, &QSlider::valueChanged, this, &BoardWindow::onMoveSliderChanged);
    
    // Update label using server's official move count (matches q5Go)
    int displayed_current = current_move_index;
    int displayed_total = total_moves;
    move_number_label->setText(QString("Move: %1/%2").arg(displayed_current).arg(displayed_total));
    
    qDebug() << "*** MOVE COUNT DISPLAY: history_size=" << move_history.size() << "server_count=" << server_move_count << "displayed=" << displayed_current << "/" << displayed_total;
    
    // Update button states
    first_move_button->setEnabled(current_move_index > 0);
    prev_move_button->setEnabled(current_move_index > 0);
    next_move_button->setEnabled(current_move_index < total_moves);
    last_move_button->setEnabled(current_move_index < total_moves);
}

// Server scoring implementation
void BoardWindow::setServerScore(double white_score, double black_score) {
    server_white_score = white_score;
    server_black_score = black_score;
    has_server_score = true;
    
    // Calculate final result like q5Go
    double score_difference = white_score - black_score;
    if (score_difference > 0) {
        game_result = QString("W+%1").arg(score_difference, 0, 'f', 1);
    } else if (score_difference < 0) {
        game_result = QString("B+%1").arg(-score_difference, 0, 'f', 1);
    } else {
        game_result = "Jigo";
    }
    
    qDebug() << "*** SERVER SCORE SET: White" << white_score << "Black" << black_score << "Result:" << game_result;
    
    // Update the display
    updateLabels();
}

#include "board_window.moc"