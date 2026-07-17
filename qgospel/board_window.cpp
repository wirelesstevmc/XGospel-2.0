#include "board_window.h"
#include "igs_move_parser.h"
#include "sgf_parser.h"
#include "settings.h"
#include "score_engine.h"
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QScrollBar>
#include <QtGui/QFont>
#include <QtGui/QCloseEvent>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QStack>
#include <QtCore/QSet>
#include <QtCore/QTime>
#include <cmath>
#include <climits>

// Debug macros - controlled by settings flags (all default to false)
#define DEBUG_OBSERVATION_STATE if (settings->getDebugObservationState()) qDebug()
#define DEBUG_MOVE_PROCESSING if (settings->getDebugMoveProcessing()) qDebug()
#define DEBUG_EDIT_MODE if (settings->getDebugEditMode()) qDebug()
#define DEBUG_SCORING if (settings->getDebugScoring()) qDebug()
#define DEBUG_PROTOCOL if (settings->getDebugProtocol()) qDebug()
#define DEBUG_MATCH if (settings->getDebugMatch()) qDebug()

// GoBoardWidget Implementation
GoBoardWidget::GoBoardWidget(QWidget *parent)
 : QFrame(parent), board_size(19), board_state(nullptr), margin(30),
 cell_size(25), stone_size(24), last_move_x(-1), last_move_y(-1), last_edit_x(-1), last_edit_y(-1), show_coordinates(true),
 scoring_mode_enabled(false), game_mode(MODE_NORMAL), mouse_down_x(-1), mouse_down_y(-1),
 next_player_color(BLACK_STONE), hover_x(-1), hover_y(-1)
{
 setMouseTracking(true);
 setFrameStyle(QFrame::Sunken | QFrame::Panel);
 setLineWidth(2);
 setMinimumSize(500, 500);
 setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

 // Initialize stone renderer BEFORE setting board size (which calls calculateSizes)
 stone_renderer = new StoneRenderer();

 // Allocate board state
 setBoardSize(19);

 // Load board texture from xgospel 1.X
 QString texture_path = QDir::homePath() + "/board.xpm";
 if (QFile::exists(texture_path)) {
 board_texture.load(texture_path);
 qDebug() << "Board texture loaded successfully from:" << texture_path;
 } else {
 qDebug() << "Board texture not found at:" << texture_path << "- using solid color";
 }

 setStyleSheet(
 "GoBoardWidget {" " background-color: #F4C542;" // Bright golden yellow fallback
 " border: 2px inset #8B7355;" "}"
 );
}

GoBoardWidget::~GoBoardWidget() {
 if (board_state) {
 for (int i = 0; i < board_size; i++) {
 delete[] board_state[i];
 }
 delete[] board_state;
 }
 delete stone_renderer;
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
 DEBUG_MOVE_PROCESSING << "GoBoardWidget::placeMoveAt called with x=" << x << "y=" << y << "color=" << color << "board_size=" << board_size;

 if (x >= 0 && x < board_size && y >= 0 && y < board_size) {
 board_state[x][y] = color;
 // NOTE: Do NOT call setLastMove() here - it's set explicitly in displayNode()
 // to mark only the actual move for the current node, not every stone on the board
 DEBUG_MOVE_PROCESSING << "Stone placed successfully at (" << x << "," << y << ")";
 update();
 } else {
 qDebug() << "ERROR: Invalid coordinates - stone NOT placed!";
 }
}

void GoBoardWidget::removeStoneAt(int x, int y) {
 if (x >= 0 && x < board_size && y >= 0 && y < board_size) {
 board_state[x][y] = EMPTY;
 update();
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

void GoBoardWidget::setLastEditMove(int x, int y) {
 last_edit_x = x;
 last_edit_y = y;
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
 // We need to calculate margin dynamically to give proper clearance for coordinates
 // Margin must accommodate: stone_radius + (0.75 * stone_diameter) clearance + text space
 //
 // The edge stones sit at grid intersections which are at distance 'margin' from widget edge
 // Stone extends 'stone_radius' beyond the intersection toward the edge
 // We want 0.75 * stone_diameter clearance beyond the stone edge
 // Plus room for coordinate text

 int base_margin = 50; // Starting estimate
 int available_width = width() - 2 * base_margin;
 int available_height = height() - 2 * base_margin;
 int min_dimension = std::min(available_width, available_height);

 if (board_size > 1) {
 // First pass: estimate cell_size
 cell_size = min_dimension / (board_size - 1);
 cell_size = std::max(cell_size, 15); // Minimum cell size

 // Calculate stone diameter - account for render() pic_radius=0.97 shrinkage
 // To get 96% visible stone: pixmap_size = (cell_size * 0.96) / 0.97 ≈ 0.99 * cell_size
 int stone_diameter = static_cast<int>(cell_size * 0.99 + 0.5);
 int stone_radius = stone_diameter / 2;

 // Required margin = stone_radius + 0.75*stone_diameter + text_space
 // Text space: ~15px for text height/width + small buffer
 int clearance = static_cast<int>(0.75 * stone_diameter);
 int required_margin = stone_radius + clearance + 18;
 margin = std::max(required_margin, 40); // Minimum 40px margin

 // Second pass: recalculate cell_size with new margin
 available_width = width() - 2 * margin;
 available_height = height() - 2 * margin;
 min_dimension = std::min(available_width, available_height);
 cell_size = min_dimension / (board_size - 1);
 cell_size = std::max(cell_size, 15); // Minimum cell size

 // Generate q5Go-style stone pixmaps only when size changes
 int new_stone_size = static_cast<int>(cell_size * 0.99 + 0.5);
 if (new_stone_size != stone_size) {
     stone_size = new_stone_size;
     stone_renderer->generateStones(stone_size);
     qDebug() << "=== STONE SIZE DEBUG === cell_size:" << cell_size << "stone_size:" << stone_size << "ratio:" << (double)stone_size/cell_size;
 }
 }
}

void GoBoardWidget::paintEvent(QPaintEvent *event) {
 QFrame::paintEvent(event);

 QPainter painter(this);
 painter.setRenderHint(QPainter::Antialiasing);

 calculateSizes();

 // Draw tiled board texture if loaded, otherwise use solid background
 if (!board_texture.isNull()) {
 // Tile the texture across the widget
 for (int x = 0; x < width(); x += board_texture.width()) {
 for (int y = 0; y < height(); y += board_texture.height()) {
 painter.drawPixmap(x, y, board_texture);
 }
 }
 }

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
 drawHoverCursor(painter);

 // Draw dead stone markers on top of stones
 if (scoring_mode_enabled) {
 drawDeadStoneMarkers(painter);
 drawDisputedMarkers(painter);
 }
}

void GoBoardWidget::drawBoard(QPainter &painter) {
 // Disable antialiasing for crisp black lines (antialiasing was causing gray appearance)
 painter.setRenderHint(QPainter::Antialiasing, false);

 // Use cosmetic black grid lines (thinnest possible - 1 device pixel, pure black)
 QPen grid_pen(Qt::black);
 grid_pen.setWidth(0); // Cosmetic pen - exactly 1 device pixel
 grid_pen.setCosmetic(true); // Explicitly set cosmetic mode
 painter.setPen(grid_pen);

 static bool debug_logged = false;
 if (!debug_logged) {
 qDebug() << "=== GRID DEBUG === pen color:" << grid_pen.color() << "width:" << grid_pen.width() << "cosmetic:" << grid_pen.isCosmetic();
 debug_logged = true;
 }

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
 // Use the actual stone size that was generated (stored in member variable)
 int stone_radius = stone_size / 2;

 static bool debug_logged = false;
 if (!debug_logged) {
 qDebug() << "=== STONE POSITIONING DEBUG === stone_size:" << stone_size << "stone_radius:" << stone_radius
 << "left_offset:" << stone_radius << "right_offset:" << (stone_size - stone_radius);
 debug_logged = true;
 }

 for (int i = 0; i < board_size; i++) {
 for (int j = 0; j < board_size; j++) {
 if (board_state[i][j] != EMPTY) {
 QPoint center = boardToScreen(i, j);

 // Draw shadow first (offset down and left for upper-right light source)
 // Increased offset to 4 pixels to match q5Go's prominent shadow depth
 int shadow_offset = 4;
 QPoint shadow_pos(center.x() - stone_radius - shadow_offset, center.y() - stone_radius + shadow_offset);
 painter.drawPixmap(shadow_pos, stone_renderer->getShadow());

 // Draw the stone
 QPoint stone_pos(center.x() - stone_radius, center.y() - stone_radius);

 if (board_state[i][j] == BLACK_STONE) {
 painter.drawPixmap(stone_pos, stone_renderer->getBlackStone());
 } else {
 // Use variation based on board position for natural white stone appearance
 int variation = (i * 19 + j) % 10; // Pseudo-random variation
 painter.drawPixmap(stone_pos, stone_renderer->getWhiteStone(variation));
 }
 }
 }
 }
}

void GoBoardWidget::drawCoordinates(QPainter &painter) {
 painter.setPen(Qt::black);
 QFont font("Arial", 10);
 painter.setFont(font);
 QFontMetrics fm(font);

 // Calculate stone radius for proper spacing
 int stone_radius = cell_size / 2 - 2;
 int stone_diameter = stone_radius * 2;
 int clearance = static_cast<int>(0.75 * stone_diameter);

 // Draw letters (A-T skipping I) for columns
 QString letters = "ABCDEFGHJKLMNOPQRST";

 // Get the top and bottom row positions
 QPoint top_edge = boardToScreen(0, 0);
 QPoint bottom_edge = boardToScreen(0, board_size - 1);

 for (int i = 0; i < board_size && i < letters.length(); i++) {
 QPoint pos = boardToScreen(i, 0);
 QString letter(letters[i]);
 int text_width = fm.horizontalAdvance(letter);

 // Top coordinates - positioned above the board with proper clearance
 // Distance from grid line: stone_radius + (clearance / 2)
 int top_y = top_edge.y() - stone_radius - clearance / 2;
 painter.drawText(pos.x() - text_width / 2, top_y, letter);

 // Bottom coordinates - positioned below the board with proper clearance
 // Distance from grid line: stone_radius + (clearance / 2) + text_height
 int bottom_y = bottom_edge.y() + stone_radius + clearance / 2 + fm.height();
 painter.drawText(pos.x() - text_width / 2, bottom_y, letter);
 }

 // Draw numbers for rows
 for (int i = 0; i < board_size; i++) {
 QPoint pos = boardToScreen(0, i);
 QString num = QString::number(board_size - i);
 int text_width = fm.horizontalAdvance(num);
 int text_height = fm.height();

 // Left coordinates - positioned to the left of the board, vertically centered
 QPoint left_edge = boardToScreen(0, i);
 painter.drawText(left_edge.x() - margin / 2 - text_width / 2, pos.y() + text_height / 3, num);

 // Right coordinates - positioned to the right of the board, vertically centered
 QPoint right_edge = boardToScreen(board_size - 1, i);
 painter.drawText(right_edge.x() + margin / 2 - text_width / 2, pos.y() + text_height / 3, num);
 }
}

void GoBoardWidget::drawLastMoveMarker(QPainter &painter) {
 painter.setBrush(Qt::NoBrush);
 // Red circle — last move from the original game record
 if (last_move_x >= 0 && last_move_y >= 0) {
     painter.setPen(QPen(Qt::red, 3));
     painter.drawEllipse(boardToScreen(last_move_x, last_move_y), cell_size/3, cell_size/3);
 }
 // Blue circle — last stone placed during edit
 if (last_edit_x >= 0 && last_edit_y >= 0) {
     painter.setPen(QPen(QColor(0, 120, 255), 3));
     painter.drawEllipse(boardToScreen(last_edit_x, last_edit_y), cell_size/3, cell_size/3);
 }
}

void GoBoardWidget::drawHoverCursor(QPainter &painter) {
 if (game_mode != MODE_EDIT) return;
 if (hover_x < 0 || hover_y < 0 || hover_x >= board_size || hover_y >= board_size) return;
 if (board_state[hover_x][hover_y] != EMPTY) return;

 QPoint center = boardToScreen(hover_x, hover_y);
 int radius = stone_size / 2;

 painter.setRenderHint(QPainter::Antialiasing, true);
 if (next_player_color == BLACK_STONE) {
     painter.setBrush(QColor(0, 0, 0, 130));
     painter.setPen(QPen(QColor(0, 0, 0, 180), 1));
 } else {
     painter.setBrush(QColor(255, 255, 255, 160));
     painter.setPen(QPen(QColor(100, 100, 100, 180), 1));
 }
 painter.drawEllipse(center, radius, radius);
}

void GoBoardWidget::mouseMoveEvent(QMouseEvent *event) {
 if (game_mode == MODE_EDIT) {
     QPoint bp = screenToBoard(event->x(), event->y());
     int bx = bp.x();
     int by = bp.y();
     // Clamp to valid board range — screenToBoard can return out-of-bounds values
     // when the mouse is near or outside the board edge
     if (bx < 0 || bx >= board_size || by < 0 || by >= board_size)
         bx = by = -1;
     if (bx != hover_x || by != hover_y) {
         hover_x = bx;
         hover_y = by;
         update();
     }
 }
}

void GoBoardWidget::leaveEvent(QEvent *) {
 if (hover_x != -1 || hover_y != -1) {
     hover_x = hover_y = -1;
     update();
 }
}

void GoBoardWidget::clearHover() {
 hover_x = hover_y = -1;
 update();
}

void GoBoardWidget::drawTerritoryMarkers(QPainter &painter) {
 for (auto it = territory_map.begin(); it != territory_map.end(); ++it) {
 QPair<int, int> pos = it.key();
 StoneColor owner = it.value();

 QPoint center = boardToScreen(pos.first, pos.second);

 // Draw territory marking like xgospel 1.X - rectangular boxes
 QColor fill_color;
 if (owner == WHITE_STONE) {
 fill_color = QColor(255, 255, 255, 160); // Semi-transparent white
 } else if (owner == BLACK_STONE) {
 fill_color = QColor(0, 0, 0, 160); // Semi-transparent black
 } else {
 // Dame (neutral/uncounted territory) - draw green rectangle like xgospel 1.X
 fill_color = QColor(0, 200, 100, 140); // Semi-transparent green
 }

 // Draw filled rectangle with border
 int box_size = cell_size / 2; // Half the cell size for nice proportions
 QRect territory_rect(center.x() - box_size/2, center.y() - box_size/2,
 box_size, box_size);

 painter.setPen(QPen(fill_color.darker(150), 1)); // Darker border
 painter.setBrush(fill_color);
 painter.drawRect(territory_rect);
 }
}

void GoBoardWidget::setDeadStones(const QSet<QPair<int, int>> &dead_stones) {
 qDebug() << "[DEAD-MARKER-DEBUG] setDeadStones called with" << dead_stones.size() << "dead stones";
 dead_stone_positions = dead_stones;
 update();
}

void GoBoardWidget::drawDeadStoneMarkers(QPainter &painter) {
 qDebug() << "[DEAD-MARKER-DEBUG] drawDeadStoneMarkers called, dead_stone_positions.size() =" << dead_stone_positions.size();

 for (const auto& pos : dead_stone_positions) {
 QPoint center = boardToScreen(pos.first, pos.second);

 // Get the stone color to determine rectangle color (inverted)
 StoneColor stone_color = getStoneAt(pos.first, pos.second);

 // Draw inverted rectangle on dead stones (same style as territory markers)
 // Black stones get white rectangles, white stones get black rectangles
 QColor fill_color;
 if (stone_color == BLACK_STONE) {
 fill_color = QColor(255, 255, 255, 160); // Semi-transparent white
 } else if (stone_color == WHITE_STONE) {
 fill_color = QColor(0, 0, 0, 160); // Semi-transparent black
 } else {
 continue; // Skip if no stone at this position
 }

 // Draw filled rectangle with border (same size as territory markers)
 int box_size = cell_size / 2; // Same size as territory markers
 QRect dead_rect(center.x() - box_size/2, center.y() - box_size/2,
 box_size, box_size);

 painter.setPen(QPen(fill_color.darker(150), 1)); // Darker border
 painter.setBrush(fill_color);
 painter.drawRect(dead_rect);
 }
}

void GoBoardWidget::drawDisputedMarkers(QPainter &painter) {
 // Draw a small hollow grey square for seki / false-eye points (complex scoring only).
 // Distinct from territory boxes (filled black/white) and dead stone boxes.
 for (const auto &pos : disputed_positions) {
     QPoint center = boardToScreen(pos.first, pos.second);
     int box_size = cell_size / 3;  // Slightly smaller than territory markers
     QRect disp_rect(center.x() - box_size/2, center.y() - box_size/2,
                     box_size, box_size);
     painter.setPen(QPen(QColor(220, 40, 40, 240), 2));  // Red outline
     painter.setBrush(Qt::NoBrush);                       // Hollow
     painter.drawRect(disp_rect);
 }
}

QPoint GoBoardWidget::boardToScreen(int x, int y) {
 return QPoint(margin + x * cell_size, margin + y * cell_size);
}

QPoint GoBoardWidget::screenToBoard(int px, int py) {
 int x = (px - margin + cell_size/2) / cell_size;
 int y = (py - margin + cell_size/2) / cell_size;
 
 qDebug() << "screenToBoard: pixel(" << px << "," << py << ") -> board(" << x << "," << y << ") [margin=" << margin << ", cell_size=" << cell_size << "]";
 return QPoint(x, y);
}

void GoBoardWidget::mousePressEvent(QMouseEvent *event) {
 // Record mouse down position for anti-clicko
 QPoint board_pos = screenToBoard(event->x(), event->y());
 mouse_down_x = board_pos.x();
 mouse_down_y = board_pos.y();
}

void GoBoardWidget::mouseReleaseEvent(QMouseEvent *event) {
 // Anti-clicko: only process if release position matches press position
 QPoint board_pos = screenToBoard(event->x(), event->y());
 int x = board_pos.x();
 int y = board_pos.y();

 if (mouse_down_x == -1 || x != mouse_down_x || y != mouse_down_y) {
 return; // Click was dragged, ignore
 }

 // Reset mouse down position
 mouse_down_x = -1;
 mouse_down_y = -1;

 // Check valid board position
 if (x < 0 || x >= board_size || y < 0 || y >= board_size) {
 return;
 }

 // Edit mode: emit boardClicked so BoardWindow can handle stone placement
 // and update the game tree / last-move marker correctly.
 // Right-click removal is handled here since it doesn't need game tree tracking.
 if (game_mode == MODE_EDIT) {
 if (event->button() == Qt::LeftButton) {
     emit boardClicked(x, y);
 } else if (event->button() == Qt::RightButton) {
     StoneColor existing_stone = (StoneColor)board_state[x][y];
     if (existing_stone != EMPTY) {
         removeStoneAt(x, y);
     }
 }
 return;
 }

 // Normal mode: emit boardClicked for move handling
 if (game_mode == MODE_NORMAL && event->button() == Qt::LeftButton) {
 emit boardClicked(x, y);
 }
}

QPixmap GoBoardWidget::renderToPixmap(int size,
                                       const int ext_board_state[19][19],
                                       int ext_board_size,
                                       int ext_last_move_x, int ext_last_move_y,
                                       const QString &title_overlay) const
{
    // --- size geometry (mirrors calculateSizes() but against 'size' not widget dims) ---
    const int bs = (ext_board_size > 1 && ext_board_size <= 19) ? ext_board_size : 19;

    // First pass: estimate cell_size
    int base_margin = 8;
    int available   = size - 2 * base_margin;
    int cs          = available / (bs - 1);
    cs = std::max(cs, 4);

    int stone_diam  = static_cast<int>(cs * 0.99 + 0.5);
    int stone_rad   = stone_diam / 2;
    int clearance   = static_cast<int>(0.75 * stone_diam);
    int req_margin  = stone_rad + clearance + 4;
    int mg          = std::max(req_margin, base_margin);

    // Second pass: recalculate with real margin
    available = size - 2 * mg;
    cs        = available / (bs - 1);
    cs = std::max(cs, 4);

    int ss = static_cast<int>(cs * 0.99 + 0.5);   // stone pixmap size for this render

    // Helper: board coords -> pixel centre within the pixmap
    auto b2s = [&](int bx, int by) -> QPoint {
        return QPoint(mg + bx * cs, mg + by * cs);
    };

    // --- create off-screen pixmap and painter ---
    QPixmap px(size, size);
    px.fill(QColor(0xdd, 0xa0, 0x4c));  // plain wood colour fallback

    // Tile board texture if the live widget has one loaded
    if (!board_texture.isNull()) {
        QPainter tp(&px);
        for (int tx = 0; tx < size; tx += board_texture.width())
            for (int ty = 0; ty < size; ty += board_texture.height())
                tp.drawPixmap(tx, ty, board_texture);
    }

    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing, false);

    // --- grid ---
    QPen grid_pen(Qt::black);
    grid_pen.setWidth(0);
    grid_pen.setCosmetic(true);
    p.setPen(grid_pen);

    for (int i = 0; i < bs; ++i) {
        p.drawLine(b2s(i, 0),      b2s(i, bs - 1));
        p.drawLine(b2s(0, i),      b2s(bs - 1, i));
    }

    // --- star points ---
    p.setBrush(Qt::black);
    p.setPen(Qt::NoPen);
    auto drawHoshi = [&](int x, int y) {
        p.drawEllipse(b2s(x, y), 2, 2);
    };
    if (bs == 19) {
        for (int hx : {3, 9, 15})
            for (int hy : {3, 9, 15})
                drawHoshi(hx, hy);
    } else if (bs == 13) {
        for (auto &pt : std::initializer_list<std::pair<int,int>>{{3,3},{6,6},{9,3},{3,9},{9,9}})
            drawHoshi(pt.first, pt.second);
    } else if (bs == 9) {
        for (auto &pt : std::initializer_list<std::pair<int,int>>{{2,2},{6,2},{4,4},{2,6},{6,6}})
            drawHoshi(pt.first, pt.second);
    }

    // --- stones ---
    // Generate stone pixmaps at the required size using a temporary StoneRenderer.
    // We reuse the live widget's renderer if the size matches; otherwise create a
    // temporary one.  For small previews the renderer is cheap to create.
    StoneRenderer *sr = stone_renderer;
    StoneRenderer  tmp_renderer;
    if (ss != stone_size) {
        tmp_renderer.generateStones(ss);
        sr = &tmp_renderer;
    }

    p.setRenderHint(QPainter::Antialiasing, true);
    for (int i = 0; i < bs; ++i) {
        for (int j = 0; j < bs; ++j) {
            if (ext_board_state[i][j] == EMPTY) continue;
            QPoint centre = b2s(i, j);
            int sr_rad = ss / 2;

            // Shadow
            QPoint shadow_pos(centre.x() - sr_rad - 2, centre.y() - sr_rad + 2);
            p.drawPixmap(shadow_pos, sr->getShadow());

            QPoint stone_pos(centre.x() - sr_rad, centre.y() - sr_rad);
            if (ext_board_state[i][j] == BLACK_STONE) {
                p.drawPixmap(stone_pos, sr->getBlackStone());
            } else {
                int variation = (i * 19 + j) % 10;
                p.drawPixmap(stone_pos, sr->getWhiteStone(variation));
            }
        }
    }

    // --- last move marker ---
    if (ext_last_move_x >= 0 && ext_last_move_y >= 0) {
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(Qt::red, std::max(1, cs / 8)));
        p.drawEllipse(b2s(ext_last_move_x, ext_last_move_y), cs / 3, cs / 3);
    }

    // --- teaching game title overlay (top edge) ---
    if (!title_overlay.isEmpty()) {
        const int bar_h = std::max(14, size / 14);
        QRect bar(0, 0, size, bar_h);
        p.fillRect(bar, QColor(0, 0, 0, 160));
        p.setPen(Qt::white);
        QFont f = p.font();
        f.setPixelSize(std::max(9, bar_h - 3));
        p.setFont(f);
        p.drawText(bar.adjusted(3, 0, -3, 0), Qt::AlignVCenter | Qt::AlignLeft,
                   title_overlay);
    }

    return px;
}

void GoBoardWidget::resizeEvent(QResizeEvent *event) {
 QFrame::resizeEvent(event);
 calculateSizes();
}

// GameTreeStrip Implementation

GameTreeStrip::GameTreeStrip(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(m_node_size + 4);
    setMaximumHeight(m_node_size + 4);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMouseTracking(false);
}

QSize GameTreeStrip::sizeHint() const
{
    return QSize(m_moves.size() * m_node_size, m_node_size + 4);
}

void GameTreeStrip::setMoves(const QList<StoneColor> &moves, int active_index,
                              const QList<bool> &edited)
{
    m_moves = moves;
    m_active_index = active_index;
    m_edited = edited;
    setMinimumWidth(m_moves.size() * m_node_size);
    update();
}

void GameTreeStrip::setActiveIndex(int index)
{
    m_active_index = index;
    update();
}

void GameTreeStrip::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int sz = m_node_size;
    int radius = sz / 2 - 2;

    for (int i = 0; i < m_moves.size(); i++) {
        int x = i * sz + sz / 2;
        int y = height() / 2;
        QPoint center(x, y);

        // Highlight active node with red background
        if (i == m_active_index) {
            p.setBrush(QColor(200, 0, 0, 160));
            p.setPen(Qt::NoPen);
            p.drawRect(i * sz, 0, sz, height());
        }

        bool edited = (i < m_edited.size()) && m_edited[i];
        QColor border_color = edited ? QColor(0, 120, 255) : Qt::black;
        int border_width = edited ? 2 : 1;

        StoneColor col = m_moves[i];
        if (col == BLACK_STONE) {
            QRadialGradient grad(center - QPoint(radius/3, radius/3), radius * 1.2);
            grad.setColorAt(0, QColor(80, 80, 80));
            grad.setColorAt(1, QColor(0, 0, 0));
            p.setBrush(grad);
            p.setPen(QPen(border_color, border_width));
            p.drawEllipse(center, radius, radius);
        } else if (col == WHITE_STONE) {
            QRadialGradient grad(center - QPoint(radius/3, radius/3), radius * 1.2);
            grad.setColorAt(0, QColor(255, 255, 255));
            grad.setColorAt(1, QColor(180, 180, 180));
            p.setBrush(grad);
            p.setPen(QPen(edited ? border_color : Qt::darkGray, border_width));
            p.drawEllipse(center, radius, radius);
        } else {
            // Root or setup node — draw a small diamond
            p.setBrush(QColor(150, 150, 150));
            p.setPen(QPen(Qt::darkGray, 1));
            QPolygon diamond;
            diamond << QPoint(x, y - radius)
                    << QPoint(x + radius, y)
                    << QPoint(x, y + radius)
                    << QPoint(x - radius, y);
            p.drawPolygon(diamond);
        }

        // Draw connecting line between nodes
        if (i > 0) {
            p.setPen(QPen(Qt::darkGray, 1));
            p.drawLine((i - 1) * sz + sz / 2 + radius, y,
                       i * sz + sz / 2 - radius, y);
        }
    }
}

void GameTreeStrip::mousePressEvent(QMouseEvent *event)
{
    int index = event->x() / m_node_size;
    if (index >= 0 && index < m_moves.size())
        emit nodeClicked(index);
}

// BoardWindow Implementation
BoardWindow::BoardWindow(QWidget *parent, const QString &username, bool edit_window)
 : QMainWindow(parent), observed_game_id(-1), my_username(username), current_move(0),
 current_player(BLACK_STONE), is_observing(false), is_playing(false), is_scoring_mode(false), game_mode(MODE_NORMAL),
 is_edit_window(edit_window), in_edit_position_mode(false),
 engine_console_panel(nullptr), engine_status_label(nullptr), engine_go_btn(nullptr),
 engine_clear_btn(nullptr),
 engine_log(nullptr), engine_cmd_input(nullptr), engine_send_btn(nullptr),
 undo_button(nullptr), done_button(nullptr),
 game_tree_strip(nullptr), game_tree_scroll(nullptr),
 update_button(nullptr), pass_button(nullptr), score_button(nullptr),
 edit_position_button(nullptr), cancel_edit_button(nullptr), append_button(nullptr),
 undo_edit_button(nullptr),
 source_board_window(nullptr),
 handicap(0), komi(0.5), game_type("Free"), byoyomi_time(0),
 white_captures(0), black_captures(0), white_byo_moves(0), black_byo_moves(0),
 white_time_seconds(0), black_time_seconds(0), game_finished(false),
 white_territory(0), black_territory(0), white_prisoners(0), black_prisoners(0), final_score(0.0),
 server_white_score(0.0), server_black_score(0.0), has_server_score(false),
 receiving_territory_data(false), territory_data_row(0),
 consecutive_passes(0), server_move_count(0), mv_counter(-1), observation_state(NOT_OBSERVING), moves_received_during_live(0),
 game_root(new GameNode()), current_node(game_root), slider_update_in_progress(false), auto_follow_mode(true)
{
 setMinimumSize(800, 700);
 if (is_edit_window)
     setupEditUI();
 else
     setupUI();

 // Initialize clock timer for server lag compensation
 own_clock_timer = new QTimer(this);
 connect(own_clock_timer, &QTimer::timeout, this, &BoardWindow::updateClockDisplay);
 own_clock_timer->start(1000); // Update every second to compensate for server lag
 clock_timer = own_clock_timer; // clock_timer may be swapped to a slot's timer by loadSlot
 
 // Set initial window title
 updateWindowTitle();

 // Restore window geometry from settings (xgospel1 .Xdefaults style)
 QRect savedGeometry = settings->loadWindowGeometry("board", QRect(150, 150, 900, 800));
 setGeometry(savedGeometry);

 // Restore dock/toolbar layout (dock position, float state)
 QByteArray windowState = settings->loadByteArray("board_window_state");
 if (!windowState.isEmpty())
     restoreState(windowState);

}

BoardWindow::~BoardWindow() {
 delete game_root; // Recursively deletes entire game tree
}

void BoardWindow::closeEvent(QCloseEvent *event) {
 // In docked (shared) mode the window is reused across games.
 // Pressing X should close the active slot only — not destroy the window.
 // observed_game_id may be negative (synthetic ID for a remapped finished slot).
 // closeBoardWindow() will hide the window if no slots remain.
 if (is_shared_window) {
     event->ignore();
     if (observed_game_id != 0)
         emit boardClosed(observed_game_id);
     return;
 }

 // Emit boardClosed signal to trigger unobserve and unhighlight (xgospel1 pattern)
 // ALWAYS emit if we have a valid game ID, regardless of is_observing flag state
 if (observed_game_id > 0) {
     qDebug() << "[BoardWindow::closeEvent] Emitting boardClosed for game" << observed_game_id << ", is_observing=" << is_observing;
     emit boardClosed(observed_game_id);
 }

 // Save board window geometry before closing (xgospel1 style)
 settings->saveWindowGeometry("board", geometry());

 // Save QMainWindow dock/toolbar layout (captures dock position and float state)
 settings->saveByteArray("board_window_state", saveState());

 // Save splitter sizes for panel positions
 if (main_splitter) {
     settings->saveSplitterSizes("board_main_splitter", main_splitter->sizes());
 }
 if (right_splitter) {
     settings->saveSplitterSizes("board_right_splitter", right_splitter->sizes());
 }
 if (info_splitter) {
     settings->saveSplitterSizes("board_info_splitter", info_splitter->sizes());
 }

 settings->save();
 QMainWindow::closeEvent(event);
}

void BoardWindow::setupUI() {
 QWidget *central = new QWidget;
 setCentralWidget(central);
 
 central->setStyleSheet(
 "QWidget {" " background-" " " "}"
 );
 
 QHBoxLayout *main_layout = new QHBoxLayout(central);
 main_layout->setSpacing(10);
 main_layout->setMargin(10);
 
 // Left side - Board
 QFrame *board_frame = new QFrame;
 board_frame->setFrameStyle(QFrame::Raised | QFrame::Panel);
 board_frame->setLineWidth(3);
 board_frame->setStyleSheet(
 "QFrame {" " border: 3px outset #888;" " background-" "}"
 );
 
 QVBoxLayout *board_layout = new QVBoxLayout(board_frame);
 board_layout->setMargin(8);

 // Teaching game title label (xgospel style - at top of board frame)
 // Hidden by default, shown only for teaching games with custom titles
 teaching_title_label = new QLabel();
 teaching_title_label->setStyleSheet(
 "font-size: 10px; font-weight: bold; color: #000; "
 "background-color: #edd20d; border: 1px solid #ccc; padding: 4px;"
 );
 teaching_title_label->setAlignment(Qt::AlignCenter);
 teaching_title_label->setWordWrap(true);
 teaching_title_label->setMaximumHeight(60); // Limit height to avoid taking too much space
 teaching_title_label->hide(); // Hidden by default
 board_layout->addWidget(teaching_title_label);

 board_widget = new GoBoardWidget;
 board_layout->addWidget(board_widget);
 
 // Connect board clicks to appropriate handler based on game mode
 connect(board_widget, &GoBoardWidget::boardClicked, this, &BoardWindow::onBoardClicked);
 
 // Move navigation controls
 QFrame *nav_frame = new QFrame;
 nav_frame->setFrameStyle(QFrame::Sunken | QFrame::Panel);
 nav_frame->setStyleSheet(
 "QFrame {" " border: 1px inset #666;" " background-" " padding: 5px;" "}"
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

 // Create horizontal splitter between board and right panel (using member variable)
 main_splitter = new QSplitter(Qt::Horizontal);
 main_splitter->addWidget(board_frame);

 // Right side - Game info and controls with comment panel (using member variable)
 right_splitter = new QSplitter(Qt::Vertical);
 
 // Top: Game info panel (borderless for maximum space efficiency)
 QFrame *info_frame = new QFrame;
 info_frame->setFrameStyle(QFrame::NoFrame);
 info_frame->setStyleSheet(
 "QFrame {" " border: 1px solid white;" // Invisible white-on-white border
 " background-" " padding: 2px;" // Minimal padding
 "}"
 );

 // Horizontal splitter for player info (left) vs analysis pane (right)
 info_splitter = new QSplitter(Qt::Horizontal, info_frame);
 QHBoxLayout *info_frame_layout = new QHBoxLayout(info_frame);
 info_frame_layout->setContentsMargins(0, 0, 0, 0);
 info_frame_layout->addWidget(info_splitter);

 // Create stone renderer for player icons and "to play" indicator (20x20 pixels)
 StoneRenderer icon_renderer;
 icon_renderer.generateStones(20);
 icon_black_pixmap = icon_renderer.getBlackStone();
 icon_white_pixmap = icon_renderer.getWhiteStone(0);

 // LEFT SIDE: Player info panel
 QFrame *player_info_panel = new QFrame();
 player_info_panel->setFrameStyle(QFrame::NoFrame);
 QVBoxLayout *info_layout = new QVBoxLayout(player_info_panel);
 info_layout->setSpacing(3); // Reduced spacing for compactness
 info_layout->setContentsMargins(3, 3, 3, 3); // Minimal margins for space efficiency

 // Game info with dynamic "to play" stone indicator (q5Go style - horizontal layout)
 QHBoxLayout *game_info_layout = new QHBoxLayout();
 game_info_layout->setSpacing(6);
 game_info_layout->setContentsMargins(0, 0, 0, 0);
 game_info_layout->addStretch();

 game_info_label = new QLabel("Game Info");
 game_info_label->setStyleSheet("font-size: 13px; font-weight: bold;");
 game_info_label->setAlignment(Qt::AlignCenter);
 game_info_label->setWordWrap(false);
 game_info_label->setFrameStyle(QFrame::NoFrame);
 game_info_label->setAttribute(Qt::WA_TranslucentBackground);
 game_info_layout->addWidget(game_info_label);

 // Dynamic "to play" stone icon (starts as black, updated by updateLabels)
 to_play_stone_icon = new QLabel();
 to_play_stone_icon->setPixmap(icon_renderer.getBlackStone());
 to_play_stone_icon->setFixedSize(24, 24);  // Increased from 20x20 to prevent stone clipping
 to_play_stone_icon->setVisible(false);  // Hidden until game starts
 to_play_stone_icon->setFrameStyle(QFrame::NoFrame);
 to_play_stone_icon->setAttribute(Qt::WA_TranslucentBackground);
 game_info_layout->addWidget(to_play_stone_icon);

 game_info_layout->addStretch();
 info_layout->addLayout(game_info_layout);

 // Consolidated players group (both white and black in single borderless frame)
 // Invisible white borders for maximum space efficiency
 QFrame *players_group = new QFrame();
 players_group->setFrameStyle(QFrame::NoFrame);
 players_group->setStyleSheet("QFrame { border: none; background- }");
 players_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
 QVBoxLayout *players_layout = new QVBoxLayout(players_group);
 players_layout->setSpacing(3); // Minimal spacing between white and black
 players_layout->setContentsMargins(2, 2, 2, 2); // Minimal margins

 // White player info with stone icon (q5Go style - horizontal layout)
 QHBoxLayout *white_name_layout = new QHBoxLayout();
 white_name_layout->setSpacing(4);
 white_name_layout->setContentsMargins(0, 0, 0, 0);

 // White stone icon (uses icon_renderer created earlier)
 white_stone_icon = new QLabel();
 white_stone_icon->setPixmap(icon_renderer.getWhiteStone(0));
 white_stone_icon->setFixedSize(20, 20);
 white_name_layout->addWidget(white_stone_icon);

 white_player_label = new QPushButton("White");
 white_player_label->setStyleSheet(
     "QPushButton { font-weight: bold; font-size: 14px; padding: 2px;"
     " background: transparent; border: none; text-align: left; }"
     "QPushButton:hover { text-decoration: underline; }" );
 white_player_label->setCursor(Qt::PointingHandCursor);
 connect(white_player_label, &QPushButton::clicked, this, [this]() {
     if (!white_player.isEmpty()) emit whitePlayerClicked(white_player);
 });
 white_name_layout->addWidget(white_player_label);
 white_name_layout->addStretch();

 players_layout->addLayout(white_name_layout);

 white_clock_label = new QLabel("--:--");
 white_clock_label->setStyleSheet(
     "font-size: 18px; "
     "font-weight: bold; "
     "font-family: monospace; "
     "padding: 4px; "
     "background-color: #000000; "  // Black background (xgospel1 style)
     "color: #00FF00; "              // Bright green text
     "border: 2px solid #808080; "  // Gray rectangular border
 );
 white_clock_label->setAlignment(Qt::AlignCenter);
 white_clock_label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
 players_layout->addWidget(white_clock_label);

 // White captures (borderless, minimal padding, bold)
 white_captures_label = new QLabel("Captures: 0");
 white_captures_label->setStyleSheet("font-size: 12px; font-weight: bold; padding: 1px;");
 white_captures_label->setAlignment(Qt::AlignCenter);
 players_layout->addWidget(white_captures_label);

 // Thin separator line between players
 QFrame *separator = new QFrame();
 separator->setFrameShape(QFrame::HLine);
 separator->setStyleSheet("QFrame { margin: 2px 0px; }");
 players_layout->addWidget(separator);

 // Black player info with stone icon (q5Go style - horizontal layout)
 QHBoxLayout *black_name_layout = new QHBoxLayout();
 black_name_layout->setSpacing(4);
 black_name_layout->setContentsMargins(0, 0, 0, 0);

 // Generate small black stone icon (20x20 pixels, reuse renderer from white stone)
 black_stone_icon = new QLabel();
 black_stone_icon->setPixmap(icon_renderer.getBlackStone());
 black_stone_icon->setFixedSize(20, 20);
 black_name_layout->addWidget(black_stone_icon);

 black_player_label = new QPushButton("Black");
 black_player_label->setStyleSheet(
     "QPushButton { font-weight: bold; font-size: 14px; padding: 2px;"
     " background: transparent; border: none; text-align: left; }"
     "QPushButton:hover { text-decoration: underline; }" );
 black_player_label->setCursor(Qt::PointingHandCursor);
 connect(black_player_label, &QPushButton::clicked, this, [this]() {
     if (!black_player.isEmpty()) emit blackPlayerClicked(black_player);
 });
 black_name_layout->addWidget(black_player_label);
 black_name_layout->addStretch();

 players_layout->addLayout(black_name_layout);

 black_clock_label = new QLabel("--:--");
 black_clock_label->setStyleSheet(
     "font-size: 18px; "
     "font-weight: bold; "
     "font-family: monospace; "
     "padding: 4px; "
     "background-color: #000000; "  // Black background (xgospel1 style)
     "color: #00FF00; "              // Bright green text
     "border: 2px solid #808080; "  // Gray rectangular border
 );
 black_clock_label->setAlignment(Qt::AlignCenter);
 black_clock_label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
 players_layout->addWidget(black_clock_label);

 // Black captures (borderless, minimal padding, bold)
 black_captures_label = new QLabel("Captures: 0");
 black_captures_label->setStyleSheet("font-size: 12px; font-weight: bold; padding: 1px;");
 black_captures_label->setAlignment(Qt::AlignCenter);
 players_layout->addWidget(black_captures_label);

 info_layout->addWidget(players_group);

 // Handicap/Komi/Game-type label (borderless, larger font, bold, no redundant captures)
 handicap_komi_label = new QLabel("Komi: 6.5 Handicap: 0 Free");
 handicap_komi_label->setStyleSheet("font-size: 12px; font-weight: bold; padding: 2px; border: none; background-");
 handicap_komi_label->setAlignment(Qt::AlignCenter);
 handicap_komi_label->setWordWrap(false);
 info_layout->addWidget(handicap_komi_label);

 info_layout->addStretch();

 // Save Game button (q5Go style - green)
 save_button = new QPushButton("Save Game");
 save_button->setStyleSheet(
 "QPushButton {" " background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #27ae60, stop:1 #229954);" " " " border: 2px outset #52be80;" " border-radius: 4px;" " padding: 6px;" " font-weight: bold;" "}" "QPushButton:hover {" " background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2ecc71, stop:1 #27ae60);" "}" "QPushButton:pressed {" " background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #229954, stop:1 #27ae60);" " border: 2px inset #52be80;" "}"
 );
 connect(save_button, &QPushButton::clicked, this, &BoardWindow::saveGame);
 info_layout->addWidget(save_button);

 // Edit/Analyze button (q5Go style 3D - opens SGF in separate board window)
 edit_button = new QPushButton("Edit Game");
 edit_button->setStyleSheet(
 "QPushButton {" " background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #5dade2, stop:1 #2980b9);" " " " border: 2px outset #85c1e9;" " border-radius: 4px;" " padding: 6px;" " font-weight: bold;" "}" "QPushButton:hover {" " background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #73c2ec, stop:1 #3498db);" "}" "QPushButton:pressed {" " background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2980b9, stop:1 #5dade2);" " border: 2px inset #5dade2;" "}"
 );
 connect(edit_button, &QPushButton::clicked, this, &BoardWindow::editGame);
 info_layout->addWidget(edit_button);

 // Resign button (shown when playing, replaces Close button)
 resign_button = new QPushButton("Resign");
 resign_button->setStyleSheet(
 "QPushButton {" " background-" " " " border: none;" " padding: 8px;" " font-weight: bold;" "}" "QPushButton:pressed {" " background-" "}"
 );
 connect(resign_button, &QPushButton::clicked, this, &BoardWindow::resignGame);
 resign_button->setVisible(false); // Hidden by default, shown when playing
 info_layout->addWidget(resign_button);

 // Done button — shown during scoring phase in bot mode and normal IGS play
 done_button = new QPushButton("Done");
 done_button->setToolTip("Accept the current score and end the game");
 done_button->setStyleSheet(
     "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #e67e22,stop:1 #ca6f1e);"
     " border: 2px outset #f0a85a; border-radius: 4px; padding: 6px; font-weight: bold; color: white; }"
     "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #f39c12,stop:1 #e67e22); }"
     "QPushButton:pressed { border: 2px inset #f0a85a; }"
 );
 connect(done_button, &QPushButton::clicked, this, [this]() {
     emit doneRequested(observed_game_id);
 });
 done_button->setVisible(false); // Hidden until scoring phase
 info_layout->addWidget(done_button);

 // Pass button (local play mode only — hidden until setLocalPlayMode(true))
 pass_button = new QPushButton("Pass");
 pass_button->setStyleSheet(
     "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #27ae60,stop:1 #229954);"
     " border: 2px outset #52be80; border-radius: 4px; padding: 6px; font-weight: bold; }"
     "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #2ecc71,stop:1 #27ae60); }"
     "QPushButton:pressed { border: 2px inset #52be80; }"
     "QPushButton:disabled { background: #555; color: #999; border: 2px outset #666; }"
 );
 connect(pass_button, &QPushButton::clicked, this, &BoardWindow::onPassClicked);
 pass_button->setVisible(false);
 info_layout->addWidget(pass_button);

 // Undo button (local play mode only)
 undo_button = new QPushButton("Undo");
 undo_button->setToolTip("Take back your last move and the engine's response");
 undo_button->setStyleSheet(
     "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #2980b9,stop:1 #1f6fa8);"
     " border: 2px outset #5dade2; border-radius: 4px; padding: 6px; font-weight: bold; color: white; }"
     "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #3498db,stop:1 #2980b9); }"
     "QPushButton:pressed { border: 2px inset #2980b9; }"
     "QPushButton:disabled { background: #555; color: #999; border: 2px outset #666; }"
 );
 undo_button->setEnabled(false);
 undo_button->setVisible(false);
 connect(undo_button, &QPushButton::clicked, this, &BoardWindow::undoRequested);
 info_layout->addWidget(undo_button);

 // Score button (local play mode only)
 score_button = new QPushButton("Score");
 score_button->setToolTip("Score the current position using flood-fill territory counting");
 score_button->setStyleSheet(
     "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #7f8c8d,stop:1 #6c7a7d);"
     " border: 2px outset #aab7b8; border-radius: 4px; padding: 6px; font-weight: bold; color: white; }"
     "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #95a5a6,stop:1 #7f8c8d); }"
     "QPushButton:pressed { border: 2px inset #7f8c8d; }"
 );
 score_button->setVisible(false);
 connect(score_button, &QPushButton::clicked, this, &BoardWindow::onScoreClicked);
 info_layout->addWidget(score_button);

 // Close button (shown when observing, hidden when playing since window has title bar close)
 // q5Go style 3D button with gradient
 close_button = new QPushButton("Close Board");
 close_button->setStyleSheet(
 "QPushButton {" " background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ef5350, stop:1 #c62828);" " " " border: 2px outset #e57373;" " border-radius: 4px;" " padding: 6px;" " font-weight: bold;" "}" "QPushButton:hover {" " background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f44336, stop:1 #d32f2f);" "}" "QPushButton:pressed {" " background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #c62828, stop:1 #ef5350);" " border: 2px inset #ef5350;" "}"
 );
 connect(close_button, &QPushButton::clicked, this, &BoardWindow::closeBoard);
 info_layout->addWidget(close_button);

 // Add player info panel to left side of info splitter
 info_splitter->addWidget(player_info_panel);

 // RIGHT SIDE: Engine console panel (shown in local play mode; placeholder otherwise)
 engine_console_panel = new QFrame();
 engine_console_panel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
 engine_console_panel->setStyleSheet(
     "QFrame { border: 2px inset #444; background-color: #1a1a1a; padding: 4px; }"
 );

 QVBoxLayout *console_layout = new QVBoxLayout(engine_console_panel);
 console_layout->setSpacing(4);
 console_layout->setContentsMargins(6, 6, 6, 6);

 // Engine name + status indicator row
 QHBoxLayout *status_row = new QHBoxLayout();
 status_row->setSpacing(6);
 engine_status_label = new QLabel("No engine");
 engine_status_label->setStyleSheet(
     "QLabel { font-weight: bold; font-size: 11px; color: #aaa; }"
 );
 status_row->addWidget(engine_status_label);
 status_row->addStretch();
 console_layout->addLayout(status_row);

 // "Engine: Go" button — asks engine to generate its move
 engine_go_btn = new QPushButton("Engine: Go");
 engine_go_btn->setToolTip("Ask the engine to play its move now");
 engine_go_btn->setStyleSheet(
     "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #e67e22,stop:1 #ca6f1e);"
     " border: 2px outset #f0a45a; border-radius: 4px; padding: 5px; font-weight: bold; color: white; }"
     "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #f39c12,stop:1 #e67e22); }"
     "QPushButton:pressed { border: 2px inset #e67e22; }"
     "QPushButton:disabled { background: #555; color: #888; border: 2px outset #666; }"
 );
 engine_go_btn->setEnabled(false);  // Enabled only after engineReady
 connect(engine_go_btn, &QPushButton::clicked, this, &BoardWindow::engineGoRequested);
 console_layout->addWidget(engine_go_btn);

 // "Clear Board" button — sends clear_board to reset the engine position
 engine_clear_btn = new QPushButton("Clear Board");
 engine_clear_btn->setToolTip("Send clear_board to the engine (resets position without restarting)");
 engine_clear_btn->setStyleSheet(
     "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #6c3483,stop:1 #5b2c6f);"
     " border: 2px outset #9b59b6; border-radius: 4px; padding: 5px; font-weight: bold; color: white; }"
     "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #7d3c98,stop:1 #6c3483); }"
     "QPushButton:pressed { border: 2px inset #6c3483; }"
     "QPushButton:disabled { background: #555; color: #888; border: 2px outset #666; }"
 );
 engine_clear_btn->setEnabled(false);  // Enabled only after engineReady
 connect(engine_clear_btn, &QPushButton::clicked, this, &BoardWindow::engineClearRequested);
 console_layout->addWidget(engine_clear_btn);

 // GTP I/O log
 engine_log = new QTextEdit();
 engine_log->setReadOnly(true);
 engine_log->setFont(QFont("Monospace", 9));
 engine_log->setStyleSheet(
     "QTextEdit { background-color: #111; color: #ddd; border: 1px solid #444; }"
 );
 engine_log->setMinimumHeight(80);
 console_layout->addWidget(engine_log, 1);

 // Manual command input row
 QHBoxLayout *cmd_row = new QHBoxLayout();
 cmd_row->setSpacing(4);
 engine_cmd_input = new QLineEdit();
 engine_cmd_input->setPlaceholderText("GTP command...");
 engine_cmd_input->setFont(QFont("Monospace", 9));
 engine_cmd_input->setStyleSheet(
     "QLineEdit { background-color: #222; color: #ddd; border: 1px solid #555; padding: 3px; }"
 );
 engine_send_btn = new QPushButton("Send");
 engine_send_btn->setFixedWidth(48);
 engine_send_btn->setStyleSheet(
     "QPushButton { background: #2e6da4; color: white; border: 1px solid #1a4d7a;"
     " border-radius: 3px; padding: 3px 6px; font-size: 9px; }"
     "QPushButton:hover { background: #3a80be; }"
     "QPushButton:pressed { background: #1a4d7a; }"
 );
 connect(engine_cmd_input, &QLineEdit::returnPressed, this, &BoardWindow::onEngineCmdSend);
 connect(engine_send_btn, &QPushButton::clicked,      this, &BoardWindow::onEngineCmdSend);
 cmd_row->addWidget(engine_cmd_input);
 cmd_row->addWidget(engine_send_btn);
 console_layout->addLayout(cmd_row);

 // Always visible — shows "No engine" when idle, full controls in local/analysis mode
 engine_console_panel->setVisible(true);
 engine_console_panel->setMinimumWidth(25);

 // Add engine console panel to right side of info splitter
 info_splitter->addWidget(engine_console_panel);

 // Default: player info gets most space, analysis pane gets a 25px sliver so users
 // can see it exists and drag it open; they can collapse it to zero if desired.
 info_splitter->setSizes({475, 25});

 right_splitter->addWidget(info_frame);
 
 // Bottom: Comment/Kibitz panel
 QFrame *comment_frame = new QFrame;
 comment_frame->setFrameStyle(QFrame::Sunken | QFrame::Panel);
 comment_frame->setLineWidth(2);
 comment_frame->setStyleSheet(
 "QFrame {" " border: 2px inset #888;" " background-" " padding: 5px;" "}"
 );
 
 QVBoxLayout *comment_layout = new QVBoxLayout(comment_frame);
 comment_layout->setSpacing(5);
 comment_layout->setMargin(8);
 
 // Comment title
 QLabel *comment_title = new QLabel("Comments & Kibitz");
 comment_title->setAlignment(Qt::AlignCenter);
 comment_title->setStyleSheet(
 "QLabel {" " font-weight: bold;" " font-size: 12px;" " " " background-" " border: 1px solid #ccc;" " padding: 3px;" "}"
 );
 comment_layout->addWidget(comment_title);
 
 // Comment display area (resizable via splitter - no maximum height)
 comment_display = new QTextEdit;
 comment_display->setReadOnly(true);
 comment_display->setStyleSheet(
 "QTextEdit {" " background-" " border: 1px solid #ccc;" " font-family: monospace;" " font-size: 12px;" "}"
 );
 comment_display->setPlaceholderText("Comments and kibitz will appear here...");
 comment_layout->addWidget(comment_display);
 
 // Comment input area
 comment_input = new QLineEdit;
 comment_input->setPlaceholderText("Type comment or kibitz...");
 comment_input->setStyleSheet(
 "QLineEdit {" " border: 1px solid #ccc;" " padding: 4px;" " font-size: 12px;" "}"
 );
 connect(comment_input, &QLineEdit::returnPressed, this, &BoardWindow::onCommentInputReturn);
 comment_layout->addWidget(comment_input);
 
 // Comment buttons
 QHBoxLayout *comment_buttons = new QHBoxLayout;
 comment_buttons->setSpacing(3);
 
 send_comment_button = new QPushButton();
 updateCommentButtonText(); // Set initial text based on mode
 send_comment_button->setStyleSheet(
 "QPushButton {" " background-" " " " border: none;" " padding: 4px 8px;" " font-size: 12px;" " font-weight: bold;" "}" "QPushButton:pressed {" " background-" "}"
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
 "QFrame {" " border: 2px inset #888;" " background-" " padding: 5px;" "}"
 );
 
 QVBoxLayout *observers_layout = new QVBoxLayout(observers_frame);
 observers_layout->setSpacing(5);
 observers_layout->setMargin(8);
 
 // Observers title row: "Observers (N)" label + sort toggle button
 QHBoxLayout *observers_title_row = new QHBoxLayout;
 observers_title_row->setSpacing(4);
 observers_title_row->setContentsMargins(0, 0, 0, 0);

 observers_title = new QLabel("Observers");
 observers_title->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
 observers_title->setStyleSheet(
 "QLabel {" " font-weight: bold;" " font-size: 12px;" " background-color: transparent;" " border: none;" " padding: 3px;" "}"
 );

 observers_sort_btn = new QPushButton("Rank");
 observers_sort_btn->setCheckable(false);
 observers_sort_btn->setToolTip("Toggle sort: by rank / by join order");
 observers_sort_btn->setStyleSheet(
 "QPushButton { font-size: 10px; font-weight: bold; padding: 2px 6px;"
 " background-color: #4a90d9; color: white; border: none; border-radius: 3px; }"
 "QPushButton:pressed { background-color: #357abd; }"
 );
 connect(observers_sort_btn, &QPushButton::clicked, this, &BoardWindow::toggleObserverSort);

 observers_title_row->addWidget(observers_title, 1);
 observers_title_row->addWidget(observers_sort_btn, 0);
 observers_layout->addLayout(observers_title_row);
 
 // Observers list (resizable via splitter - no maximum height)
 observers_list = new QListWidget;
 observers_list->setStyleSheet(
 "QListWidget {" " background-color: #1a1a2e;" " border: 1px solid #ccc;" " font-family: monospace;" " font-size: 12px;" "}"
 "QListWidget::item { color: #4fc3f7; padding: 1px 4px; }"
 "QListWidget::item:hover { background-color: #2a2a4e; cursor: pointer; }"
 "QListWidget::item:selected { background-color: #2a4a7f; }"
 );
 observers_list->setCursor(Qt::PointingHandCursor);
 connect(observers_list, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
     if (!item) return;
     // Item text is "name rank" — extract just the name (first token)
     QString text = item->text().trimmed();
     QString name = text.section(' ', 0, 0);
     if (!name.isEmpty())
         emit observerClicked(name);
 });
 observers_layout->addWidget(observers_list);
 
 // Observers refresh button
 QPushButton *refresh_observers_button = new QPushButton("Refresh Observers");
 refresh_observers_button->setStyleSheet(
 "QPushButton {" " background-" " " " border: none;" " padding: 4px 8px;" " font-size: 10px;" " font-weight: bold;" "}" "QPushButton:pressed {" " background-" "}"
 );
 connect(refresh_observers_button, &QPushButton::clicked, this, &BoardWindow::requestObservers);
 observers_layout->addWidget(refresh_observers_button);
 
 right_splitter->addWidget(observers_frame);

 // Set minimum heights to prevent collapsing (user can resize via splitters)
 info_frame->setMinimumHeight(320); // game info + 2 player groups (larger) + komi/captures + buttons
 comment_frame->setMinimumHeight(60);  // Reduced minimum - user resizable
 observers_frame->setMinimumHeight(40); // Reduced minimum - user resizable

 // Add right splitter to main horizontal splitter
 main_splitter->addWidget(right_splitter);

 // Add the main splitter to the layout
 main_layout->addWidget(main_splitter, 1);

 // Set default splitter sizes (smaller comments/observers for more board space)
 right_splitter->setSizes({400, 100, 60}); // Info panel larger, comments/observers smaller (user resizable)
 main_splitter->setSizes({750, 250});      // Board gets 75%, right panel gets 25%

 // Restore saved splitter sizes (if any) - must be after addWidget
 QList<int> savedMainSizes = settings->loadSplitterSizes("board_main_splitter");
 if (!savedMainSizes.isEmpty()) {
     main_splitter->setSizes(savedMainSizes);
 }

 QList<int> savedRightSizes = settings->loadSplitterSizes("board_right_splitter");
 if (!savedRightSizes.isEmpty()) {
     right_splitter->setSizes(savedRightSizes);
 }

 QList<int> savedInfoSizes = settings->loadSplitterSizes("board_info_splitter");
 if (!savedInfoSizes.isEmpty()) {
     info_splitter->setSizes(savedInfoSizes);
 }

 // Game selection dock — created here, hidden until docked-pane mode is active.
 // FixedXGospelWindow calls getGameSelectionDock() and shows it when needed.
 if (!is_edit_window) {
     game_selection_dock = new GameSelectionDock(this);
     game_selection_dock->setObjectName("GameSelectionDock");
     addDockWidget(Qt::LeftDockWidgetArea, game_selection_dock);
     game_selection_dock->hide();
 }
}

void BoardWindow::setupEditUI() {
    QWidget *central = new QWidget;
    setCentralWidget(central);

    QHBoxLayout *main_layout = new QHBoxLayout(central);
    main_layout->setSpacing(8);
    main_layout->setMargin(8);

    // --- Left side: board + navigation strip ---
    QFrame *board_frame = new QFrame;
    board_frame->setFrameStyle(QFrame::Raised | QFrame::Panel);
    board_frame->setLineWidth(3);
    board_frame->setStyleSheet(
        "QFrame { border: 3px outset #888; background- }"
    );

    QVBoxLayout *board_layout = new QVBoxLayout(board_frame);
    board_layout->setMargin(8);

    board_widget = new GoBoardWidget;
    board_layout->addWidget(board_widget);
    connect(board_widget, &GoBoardWidget::boardClicked, this, &BoardWindow::onBoardClicked);

    // Move navigation controls (same as main window)
    QFrame *nav_frame = new QFrame;
    nav_frame->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    nav_frame->setStyleSheet("QFrame { border: 1px inset #666; background- padding: 5px; }");

    QHBoxLayout *nav_layout = new QHBoxLayout(nav_frame);
    nav_layout->setSpacing(5);
    nav_layout->setMargin(5);

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

    move_slider = new QSlider(Qt::Horizontal);
    move_slider->setMinimum(0);
    move_slider->setMaximum(0);
    move_slider->setValue(0);
    move_slider->setTickPosition(QSlider::TicksBelow);
    move_slider->setTickInterval(10);

    move_number_label = new QLabel("Move: 0/0");
    move_number_label->setMinimumWidth(80);
    move_number_label->setAlignment(Qt::AlignCenter);

    nav_layout->addWidget(first_move_button);
    nav_layout->addWidget(prev_move_button);
    nav_layout->addWidget(move_slider, 1);
    nav_layout->addWidget(next_move_button);
    nav_layout->addWidget(last_move_button);
    nav_layout->addWidget(move_number_label);

    board_layout->addWidget(nav_frame);

    // Game tree strip (horizontal scrollable stone icon row)
    game_tree_strip = new GameTreeStrip;
    game_tree_scroll = new QScrollArea;
    game_tree_scroll->setWidget(game_tree_strip);
    game_tree_scroll->setWidgetResizable(false);
    game_tree_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    game_tree_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    game_tree_scroll->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    game_tree_scroll->setFixedHeight(game_tree_strip->minimumHeight() + 20); // strip + scrollbar
    board_layout->addWidget(game_tree_scroll);

    connect(game_tree_strip, &GameTreeStrip::nodeClicked, this, &BoardWindow::goToMove);

    current_move_index = 0;

    connect(first_move_button, &QPushButton::clicked, this, &BoardWindow::onFirstMoveClicked);
    connect(prev_move_button,  &QPushButton::clicked, this, &BoardWindow::onPreviousMoveClicked);
    connect(next_move_button,  &QPushButton::clicked, this, &BoardWindow::onNextMoveClicked);
    connect(last_move_button,  &QPushButton::clicked, this, &BoardWindow::onLastMoveClicked);
    connect(move_slider, &QSlider::valueChanged, this, &BoardWindow::onMoveSliderChanged);

    // --- Right side: player info + edit buttons + comments ---
    QFrame *right_frame = new QFrame;
    right_frame->setFrameStyle(QFrame::NoFrame);
    right_frame->setMinimumWidth(200);
    right_frame->setMaximumWidth(280);

    QVBoxLayout *right_layout = new QVBoxLayout(right_frame);
    right_layout->setSpacing(6);
    right_layout->setMargin(6);

    // Player info (white)
    white_stone_icon = new QLabel();
    white_stone_icon->setPixmap(icon_white_pixmap);
    white_stone_icon->setFixedSize(20, 20);

    white_player_label = new QPushButton("White");
    white_player_label->setStyleSheet(
        "QPushButton { font-weight: bold; font-size: 14px; padding: 2px;"
        " background: transparent; border: none; text-align: left; }"
        "QPushButton:hover { text-decoration: underline; }" );
    white_player_label->setCursor(Qt::PointingHandCursor);
    connect(white_player_label, &QPushButton::clicked, this, [this]() {
        if (!white_player.isEmpty()) emit whitePlayerClicked(white_player);
    });

    QHBoxLayout *white_row = new QHBoxLayout;
    white_row->setSpacing(4);
    white_row->addWidget(white_stone_icon);
    white_row->addWidget(white_player_label);
    white_row->addStretch();
    right_layout->addLayout(white_row);

    white_clock_label = new QLabel("--:--");
    white_clock_label->setStyleSheet(
        "font-size: 16px; font-weight: bold; font-family: monospace; padding: 3px;"
        "background-color: #000000; color: #00FF00; border: 2px solid #808080;"
    );
    white_clock_label->setAlignment(Qt::AlignCenter);
    right_layout->addWidget(white_clock_label);

    white_captures_label = new QLabel("Captures: 0");
    white_captures_label->setStyleSheet("font-size: 12px; font-weight: bold; padding: 1px;");
    white_captures_label->setAlignment(Qt::AlignCenter);
    right_layout->addWidget(white_captures_label);

    QFrame *sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("QFrame { margin: 2px 0px; }");
    right_layout->addWidget(sep);

    // Player info (black)
    black_stone_icon = new QLabel();
    black_stone_icon->setPixmap(icon_black_pixmap);
    black_stone_icon->setFixedSize(20, 20);

    black_player_label = new QPushButton("Black");
    black_player_label->setStyleSheet(
        "QPushButton { font-weight: bold; font-size: 14px; padding: 2px;"
        " background: transparent; border: none; text-align: left; }"
        "QPushButton:hover { text-decoration: underline; }" );
    black_player_label->setCursor(Qt::PointingHandCursor);
    connect(black_player_label, &QPushButton::clicked, this, [this]() {
        if (!black_player.isEmpty()) emit blackPlayerClicked(black_player);
    });

    QHBoxLayout *black_row = new QHBoxLayout;
    black_row->setSpacing(4);
    black_row->addWidget(black_stone_icon);
    black_row->addWidget(black_player_label);
    black_row->addStretch();
    right_layout->addLayout(black_row);

    black_clock_label = new QLabel("--:--");
    black_clock_label->setStyleSheet(
        "font-size: 16px; font-weight: bold; font-family: monospace; padding: 3px;"
        "background-color: #000000; color: #00FF00; border: 2px solid #808080;"
    );
    black_clock_label->setAlignment(Qt::AlignCenter);
    right_layout->addWidget(black_clock_label);

    black_captures_label = new QLabel("Captures: 0");
    black_captures_label->setStyleSheet("font-size: 12px; font-weight: bold; padding: 1px;");
    black_captures_label->setAlignment(Qt::AlignCenter);
    right_layout->addWidget(black_captures_label);

    handicap_komi_label = new QLabel("Komi: 6.5");
    handicap_komi_label->setStyleSheet("font-size: 12px; font-weight: bold; padding: 2px; border: none;");
    handicap_komi_label->setAlignment(Qt::AlignCenter);
    right_layout->addWidget(handicap_komi_label);

    right_layout->addStretch();

    // --- Edit button panel (view mode: 4 buttons) ---
    // These are laid out individually so switchToEditPositionMode() can
    // hide/show them without rebuilding the layout.

    update_button = new QPushButton("Update");
    update_button->setToolTip("Refresh board to current live game position");
    update_button->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #5dade2,stop:1 #2980b9);"
        " border: 2px outset #85c1e9; border-radius: 4px; padding: 6px; font-weight: bold; }"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #73c2ec,stop:1 #3498db); }"
        "QPushButton:pressed { border: 2px inset #5dade2; }"
        "QPushButton:disabled { background: #555; color: #999; border: 2px outset #666; }"
    );
    connect(update_button, &QPushButton::clicked, this, &BoardWindow::onUpdateClicked);
    right_layout->addWidget(update_button);

    pass_button = new QPushButton("Pass");
    pass_button->setToolTip("Insert a pass move");
    pass_button->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #27ae60,stop:1 #229954);"
        " border: 2px outset #52be80; border-radius: 4px; padding: 6px; font-weight: bold; }"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #2ecc71,stop:1 #27ae60); }"
        "QPushButton:pressed { border: 2px inset #52be80; }"
        "QPushButton:disabled { background: #555; color: #999; border: 2px outset #666; }"
    );
    connect(pass_button, &QPushButton::clicked, this, &BoardWindow::onPassClicked);
    right_layout->addWidget(pass_button);

    score_button = new QPushButton("Score");
    score_button->setToolTip("Estimate territory score (completed games)");
    score_button->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #f39c12,stop:1 #d68910);"
        " border: 2px outset #f8c471; border-radius: 4px; padding: 6px; font-weight: bold; }"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #f5b041,stop:1 #f39c12); }"
        "QPushButton:pressed { border: 2px inset #f39c12; }"
        "QPushButton:disabled { background: #555; color: #999; border: 2px outset #666; }"
    );
    score_button->setEnabled(true);
    connect(score_button, &QPushButton::clicked, this, &BoardWindow::onScoreClicked);
    right_layout->addWidget(score_button);

    edit_position_button = new QPushButton("Edit Position");
    edit_position_button->setToolTip("Enter free-placement edit mode");
    edit_position_button->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #8e44ad,stop:1 #6c3483);"
        " border: 2px outset #bb8fce; border-radius: 4px; padding: 6px; font-weight: bold; }"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #9b59b6,stop:1 #8e44ad); }"
        "QPushButton:pressed { border: 2px inset #8e44ad; }"
        "QPushButton:disabled { background: #555; color: #999; border: 2px outset #666; }"
    );
    connect(edit_position_button, &QPushButton::clicked, this, &BoardWindow::onEditPositionClicked);
    right_layout->addWidget(edit_position_button);

    // Edit position mode buttons (hidden initially)
    cancel_edit_button = new QPushButton("Cancel Edit");
    cancel_edit_button->setToolTip("Discard changes and return to view mode");
    cancel_edit_button->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #ef5350,stop:1 #c62828);"
        " border: 2px outset #e57373; border-radius: 4px; padding: 6px; font-weight: bold; }"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #f44336,stop:1 #d32f2f); }"
        "QPushButton:pressed { border: 2px inset #ef5350; }"
    );
    cancel_edit_button->setVisible(false);
    connect(cancel_edit_button, &QPushButton::clicked, this, &BoardWindow::onCancelEditClicked);
    right_layout->addWidget(cancel_edit_button);

    append_button = new QPushButton("Append");
    append_button->setToolTip("Commit edited position as new SGF node after current move");
    append_button->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #27ae60,stop:1 #229954);"
        " border: 2px outset #52be80; border-radius: 4px; padding: 6px; font-weight: bold; }"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #2ecc71,stop:1 #27ae60); }"
        "QPushButton:pressed { border: 2px inset #52be80; }"
    );
    append_button->setVisible(false);
    connect(append_button, &QPushButton::clicked, this, &BoardWindow::onAppendClicked);
    right_layout->addWidget(append_button);

    undo_edit_button = new QPushButton("Undo");
    undo_edit_button->setToolTip("Remove last placed stone and step back");
    undo_edit_button->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #f39c12,stop:1 #d68910);"
        " border: 2px outset #f8c471; border-radius: 4px; padding: 6px; font-weight: bold; }"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #f5b041,stop:1 #f39c12); }"
        "QPushButton:pressed { border: 2px inset #f39c12; }"
        "QPushButton:disabled { background: #555; color: #999; border: 2px outset #666; }"
    );
    undo_edit_button->setVisible(false);
    connect(undo_edit_button, &QPushButton::clicked, this, &BoardWindow::onUndoEditClicked);
    right_layout->addWidget(undo_edit_button);

    // Close button
    close_button = new QPushButton("Close Editor");
    close_button->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #ef5350,stop:1 #c62828);"
        " border: 2px outset #e57373; border-radius: 4px; padding: 6px; font-weight: bold; }"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #f44336,stop:1 #d32f2f); }"
        "QPushButton:pressed { border: 2px inset #ef5350; }"
    );
    connect(close_button, &QPushButton::clicked, this, &BoardWindow::closeBoard);
    right_layout->addWidget(close_button);

    // Null out widgets that only exist in the main window layout
    // (referenced by updateLabels / other shared helpers)
    teaching_title_label = nullptr;
    game_info_label = nullptr;
    to_play_stone_icon = nullptr;
    save_button = nullptr;
    edit_button = nullptr;
    resign_button = nullptr;
    main_splitter = nullptr;
    right_splitter = nullptr;
    info_splitter = nullptr;
    observers_list = nullptr;
    observers_title = nullptr;
    observers_sort_btn = nullptr;
    comment_input = nullptr;
    send_comment_button = nullptr;

    // Comments panel (read-only in edit window — shows SGF node comments)
    QFrame *comment_frame = new QFrame;
    comment_frame->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    comment_frame->setStyleSheet("QFrame { border: 2px inset #888; background- padding: 5px; }");

    QVBoxLayout *comment_layout = new QVBoxLayout(comment_frame);
    comment_layout->setMargin(6);

    QLabel *comment_title = new QLabel("Comments");
    comment_title->setAlignment(Qt::AlignCenter);
    comment_title->setStyleSheet(
        "QLabel { font-weight: bold; font-size: 11px; border: 1px solid #ccc; padding: 2px; }"
    );
    comment_layout->addWidget(comment_title);

    comment_display = new QTextEdit;
    comment_display->setReadOnly(true);
    comment_display->setStyleSheet(
        "QTextEdit { background- border: 1px solid #ccc; font-family: monospace; font-size: 12px; }"
    );
    comment_display->setPlaceholderText("SGF node comments appear here...");
    comment_layout->addWidget(comment_display);

    right_layout->addWidget(comment_frame);

    // Assemble main layout
    main_layout->addWidget(board_frame, 1);
    main_layout->addWidget(right_frame, 0);

    setWindowTitle("SGF Editor");

    // Board is immediately editable — clicks append moves to the game tree
    board_widget->setGameMode(MODE_EDIT);
    in_edit_position_mode = false;
    undo_edit_button->setVisible(true);
    undo_edit_button->setEnabled(false);  // enabled once at a non-root node
}

void BoardWindow::switchToEditPositionMode() {
    edit_undo_stack.clear();
    update_button->setEnabled(false);
    edit_position_button->setVisible(false);
    cancel_edit_button->setVisible(true);
    append_button->setVisible(true);
    undo_edit_button->setVisible(true);
    undo_edit_button->setEnabled(false);  // Nothing to undo yet

    board_widget->setGameMode(MODE_EDIT);
    in_edit_position_mode = true;
}

void BoardWindow::switchToViewMode() {
    edit_undo_stack.clear();
    update_button->setEnabled(true);
    score_button->setEnabled(true);
    edit_position_button->setVisible(true);
    cancel_edit_button->setVisible(false);
    append_button->setVisible(false);
    // Undo stays visible in default edit mode — navigates back through tree
    undo_edit_button->setVisible(true);
    undo_edit_button->setEnabled(current_node && !current_node->isRoot());

    board_widget->setGameMode(MODE_EDIT);
    in_edit_position_mode = false;
}

// ---------------------------------------------------------------------------
// Docked-pane mode: load all state from a GameSlot into the UI.
// Called by FixedXGospelWindow::switchActiveGame() when the user clicks a
// different game button.  The slot is already up-to-date; we just reflect it.
// ---------------------------------------------------------------------------

QPixmap BoardWindow::renderSlotToPixmap(int size,
                                        const int board_state[19][19],
                                        int board_size,
                                        int last_move_x, int last_move_y,
                                        const QString &title_overlay) const
{
    return board_widget->renderToPixmap(size, board_state, board_size,
                                        last_move_x, last_move_y, title_overlay);
}

void BoardWindow::loadSlot(GameSlot *slot)
{
    if (!slot) return;
    // --- scalar state ---
    observed_game_id           = slot->game_id;
    white_player               = slot->white_player;
    black_player               = slot->black_player;
    white_rank                 = slot->white_rank;
    black_rank                 = slot->black_rank;
    white_rank_at_start        = slot->white_rank_at_start;
    black_rank_at_start        = slot->black_rank_at_start;
    my_username                = slot->my_username;
    custom_game_title          = slot->custom_game_title;
    if (teaching_title_label) {
        if (!custom_game_title.isEmpty()) {
            teaching_title_label->setText(custom_game_title);
            teaching_title_label->show();
        } else {
            teaching_title_label->hide();
        }
    }
    is_observing               = slot->is_observing;
    is_playing                 = slot->is_playing;
    is_scoring_mode            = slot->is_scoring_mode;
    game_start_time            = slot->game_start_time;
    game_mode                  = static_cast<GameMode>(slot->game_mode);
    observation_state          = static_cast<ObservationState>(slot->observation_state);
    observation_start_time     = slot->observation_start_time;
    moves_received_during_live = slot->moves_received_during_live;
    current_move               = slot->current_move;
    current_move_index         = slot->current_move_index;
    server_move_count          = slot->server_move_count;
    consecutive_passes         = slot->consecutive_passes;
    auto_follow_mode           = slot->auto_follow_mode;
    handicap                   = slot->handicap;
    komi                       = slot->komi;
    time_control               = slot->time_control;
    game_type                  = slot->game_type;
    game_type_locked           = slot->game_type_locked;
    byoyomi_time               = slot->byoyomi_time;
    white_time_seconds         = slot->white_time_seconds;
    black_time_seconds         = slot->black_time_seconds;
    white_byo_moves            = slot->white_byo_moves;
    black_byo_moves            = slot->black_byo_moves;
    last_time_update           = slot->last_time_update;
    white_captures             = slot->white_captures;
    black_captures             = slot->black_captures;
    dead_stones                = slot->dead_stones;
    white_territory            = slot->white_territory;
    black_territory            = slot->black_territory;
    white_prisoners            = slot->white_prisoners;
    black_prisoners            = slot->black_prisoners;
    final_score                = slot->final_score;
    server_white_score         = slot->server_white_score;
    server_black_score         = slot->server_black_score;
    has_server_score           = slot->has_server_score;
    receiving_territory_data   = slot->receiving_territory_data;
    territory_data_row         = slot->territory_data_row;
    territory_ownership        = slot->territory_ownership;
    game_result                = slot->game_result;
    game_finished              = slot->game_finished;
    white_groups               = slot->white_groups;
    black_groups               = slot->black_groups;
    move_history               = slot->move_history;

    // --- game tree: point at the slot's tree (BoardWindow does NOT own it in docked mode) ---
    // We do NOT delete game_root here — in docked mode the slot owns the tree.
    game_root    = slot->game_root;
    current_node = slot->current_node;

    // Inactive slots route moves through applyMoveToSlotBoard() which does not build
    // the game tree.  Rebuild it now from move_history so navigation and SGF export
    // always work, regardless of which slot was active when moves arrived.
    if (getTotalMoves() == 0 && !move_history.isEmpty()) {
        rebuildGameTreeFromMoveHistory();
        slot->current_node = current_node;
    }

    // Sync slider position to the actual end of the tree when auto-follow is on.
    // slot->current_move_index may be stale if moves arrived while the slot was
    // inactive (applyMoveToSlotBoard advances the tree but not the index).
    if (auto_follow_mode) {
        int last = getTotalMoves();
        // Walk current_node forward to the end of the active variation
        while (current_node && current_node->nextMove())
            current_node = current_node->nextMove();
        current_move_index = last;
        slot->current_node  = current_node;
        slot->current_move_index = last;
    }

    // --- board widget ---
    board_widget->clearBoard();
    board_widget->clearHover();  // discard stale hover from previous slot
    board_widget->setBoardSize(slot->board_size);
    for (int x = 0; x < slot->board_size; ++x)
        for (int y = 0; y < slot->board_size; ++y)
            if (slot->board_state[x][y] != EMPTY)
                board_widget->placeMoveAt(x, y,
                    static_cast<StoneColor>(slot->board_state[x][y]));
    board_widget->setLastMove(slot->last_move_x, slot->last_move_y);
    board_widget->setDeadStones(slot->dead_stone_positions);
    qDebug() << "[LOADSLOT-DEBUG] A: setDeadStones done";
    board_widget->setDisputedPoints(slot->disputed_positions);
    qDebug() << "[LOADSLOT-DEBUG] B: setDisputedPoints done";
    board_widget->setScoringMode(slot->is_scoring_mode);
    qDebug() << "[LOADSLOT-DEBUG] C: setScoringMode done";
    {
        QMap<QPair<int,int>, StoneColor> tmap;
        if (slot->is_scoring_mode && !slot->territory_map.isEmpty()) {
            // Slot already has a computed territory map — restore it directly.
            // Also sync dead_stones (engine set) from dead_stone_positions so any
            // subsequent click-to-toggle-dead and re-score works correctly.
            if (dead_stones.isEmpty() && !slot->dead_stone_positions.isEmpty())
                dead_stones = slot->dead_stone_positions;
            for (auto it = slot->territory_map.constBegin();
                 it != slot->territory_map.constEnd(); ++it)
                tmap[it.key()] = static_cast<StoneColor>(it.value());
            board_widget->setTerritoryMap(tmap);
            if (current_node) current_node->setTerritoryMap(tmap);
        } else if (slot->is_scoring_mode) {
            // Slot is in scoring mode but territory hasn't been computed yet — run it now
            board_widget->setTerritoryMap(tmap); // clear first
            calculateScore();
        } else {
            board_widget->setTerritoryMap(tmap); // clears overlay
        }
    }
    qDebug() << "[LOADSLOT-DEBUG] D: setTerritoryMap done";
    current_player = static_cast<StoneColor>(slot->current_player);

    // --- clock: disconnect previous timer from display refresh, connect new slot's timer ---
    // Do NOT stop the previous timer — if it belongs to another slot it must keep ticking.
    qDebug() << "[LOADSLOT-DEBUG] E: about to swap clock_timer";
    if (clock_timer && clock_timer != own_clock_timer)
        disconnect(clock_timer, &QTimer::timeout, this, &BoardWindow::updateClockDisplay);
    clock_timer = (slot->clock_timer) ? slot->clock_timer : own_clock_timer;
    connect(clock_timer, &QTimer::timeout, this, &BoardWindow::updateClockDisplay);
    qDebug() << "[LOADSLOT-DEBUG] F: clock_timer swapped";

    // Inactive slots never get updateByoyomi() called, so last_time_update stays null.
    // Seed it to now so the clock countdown starts ticking immediately on switch.
    if (last_time_update.isNull() && (white_time_seconds > 0 || black_time_seconds > 0))
        last_time_update = QDateTime::currentDateTime();
    updateClockDisplay();  // paint new slot's clock immediately, don't wait for next tick
    qDebug() << "[LOADSLOT-DEBUG] G: updateClockDisplay done";

    // --- observers list ---
    // Finished games get no more server updates, so clear rather than restoring
    // a stale list. Live games restore from the slot snapshot written by snapshotToSlot().
    clearObservers();
    qDebug() << "[LOADSLOT-DEBUG] H: clearObservers done";
    if (!slot->game_finished) {
        for (const auto &obs : slot->observers)
            addObserver(obs.name, obs.rank);
    }
    qDebug() << "[LOADSLOT-DEBUG] I: addObserver loop done";

    // --- comments ---
    // Always rebuild from the authoritative CommentEntry list so that kibitzes
    // appended while this slot was in the background are not lost.
    if (comment_display) {
        comment_display->clear();
        for (const auto &c : slot->comments) {
            QString prefix = c.is_kibitz
                ? QString("<b>%1:</b> ").arg(c.user.toHtmlEscaped())
                : QString("<i>%1:</i> ").arg(c.user.toHtmlEscaped());
            comment_display->append(prefix + c.text.toHtmlEscaped());
        }
        // Append system messages (save confirmations etc.) that are not in the structured list
        for (const QString &msg : slot->system_messages)
            comment_display->append(msg);
        // Scroll to bottom
        QTextCursor cursor = comment_display->textCursor();
        cursor.movePosition(QTextCursor::End);
        comment_display->setTextCursor(cursor);
    }
    qDebug() << "[LOADSLOT-DEBUG] J: comments rebuilt";
    // --- game tree strip (edit windows only) ---
    if (game_tree_strip)
        updateGameTreeStrip();
    qDebug() << "[LOADSLOT-DEBUG] K: gameTreeStrip done";

    // --- if the game already finished while this was an inactive slot, show result ---
    // Skip if the result line is already present in the restored HTML to avoid duplicates.
    if (game_finished && !game_result.isEmpty()) {
        bool already_shown = comment_display &&
                             comment_display->toPlainText().contains("Game finished:");
        if (!already_shown)
            updateGameResult(game_result);
    }
    qDebug() << "[LOADSLOT-DEBUG] L: updateGameResult done";

    // --- all labels and title ---
    updateLabels();
    qDebug() << "[LOADSLOT-DEBUG] M: updateLabels done";
    updatePlayerInfoGroups();
    qDebug() << "[LOADSLOT-DEBUG] N: updatePlayerInfoGroups done";
    updateWindowTitle();
    qDebug() << "[LOADSLOT-DEBUG] O: updateWindowTitle done";
    updateCommentButtonText();
    qDebug() << "[LOADSLOT-DEBUG] P: updateCommentButtonText done";
    updateMoveNavigation();
    // Release the mv_counter guard so live moves are accepted after slot load.
    // When loadSlot is called after replay (inactive slot going LIVE), mv_counter
    // is still -1 from the clearMoveHistoryBeforeMovesCommand() call, causing all
    // subsequent live moves to be skipped. Set it to the replay endpoint so the
    // board window accepts moves from here onward.
    if (mv_counter == -1 && slot->replay_state == GameSlot::LIVE)
        mv_counter = slot->server_move_count;
    qDebug() << "[LOADSLOT-DEBUG] Q: loadSlot complete for game" << slot->game_id
             << "mv_counter now" << mv_counter;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Revert clock_timer to the BoardWindow's own timer.
// Must be called before deleting a GameSlot whose clock_timer is currently
// active in this BoardWindow, to prevent a dangling pointer crash in loadSlot.
// ---------------------------------------------------------------------------
void BoardWindow::detachSlotClockTimer()
{
    if (clock_timer != own_clock_timer) {
        disconnect(clock_timer, &QTimer::timeout, this, &BoardWindow::updateClockDisplay);
        clock_timer = own_clock_timer;
        connect(clock_timer, &QTimer::timeout, this, &BoardWindow::updateClockDisplay);
    }
}

// ---------------------------------------------------------------------------
// Lightweight board-state snapshot for hover pixmap refresh after a live move.
// Copies only position data — does NOT disconnect the clock timer.
// ---------------------------------------------------------------------------
void BoardWindow::snapshotBoardStateToSlot(GameSlot *slot)
{
    if (!slot) return;
    slot->board_size  = board_widget->getBoardSize();
    slot->last_move_x = board_widget->getLastMoveX();
    slot->last_move_y = board_widget->getLastMoveY();
    for (int x = 0; x < slot->board_size; ++x)
        for (int y = 0; y < slot->board_size; ++y)
            slot->board_state[x][y] = board_widget->getBoardState(x, y);
}

// ---------------------------------------------------------------------------
// Docked-pane mode: write current live UI state back to the slot before
// switching away.  Captures anything that may have changed since loadSlot().
// ---------------------------------------------------------------------------
void BoardWindow::snapshotToSlot(GameSlot *slot)
{
    if (!slot) return;

    // Scalar state that may have changed during live observation
    slot->current_move               = current_move;
    slot->current_move_index         = current_move_index;
    slot->server_move_count          = server_move_count;
    slot->consecutive_passes         = consecutive_passes;
    slot->auto_follow_mode           = auto_follow_mode;
    slot->observation_state          = static_cast<GameSlot::ObservationState>(observation_state);
    slot->moves_received_during_live = moves_received_during_live;
    slot->current_player             = current_player;
    slot->last_move_x                = board_widget->getLastMoveX();
    slot->last_move_y                = board_widget->getLastMoveY();
    slot->white_captures             = white_captures;
    slot->black_captures             = black_captures;
    slot->white_time_seconds         = white_time_seconds;
    slot->black_time_seconds         = black_time_seconds;
    slot->white_byo_moves            = white_byo_moves;
    slot->black_byo_moves            = black_byo_moves;
    slot->last_time_update           = last_time_update;
    slot->is_scoring_mode            = is_scoring_mode;
    slot->dead_stones                = dead_stones;
    slot->white_territory            = white_territory;
    slot->black_territory            = black_territory;
    slot->white_prisoners            = white_prisoners;
    slot->black_prisoners            = black_prisoners;
    slot->final_score                = final_score;
    slot->server_white_score         = server_white_score;
    slot->server_black_score         = server_black_score;
    slot->has_server_score           = has_server_score;
    slot->receiving_territory_data   = receiving_territory_data;
    slot->territory_data_row         = territory_data_row;
    slot->territory_ownership        = territory_ownership;
    slot->game_result                = game_result;
    slot->game_finished              = game_finished;
    slot->move_history               = move_history;
    slot->white_groups               = white_groups;
    slot->black_groups               = black_groups;

    // Preserve the exact rendered comment history so switching back restores it perfectly
    if (comment_display)
        slot->comment_html = comment_display->toHtml();

    // Snapshot the live observer list from the widget back into the slot so that
    // loadSlot() restores exactly what was on screen, not a stale server snapshot.
    slot->observers.clear();
    for (const auto &p : observers_raw) {
        GameSlot::ObserverEntry e; e.name = p.first; e.rank = p.second;
        slot->observers.append(e);
    }

    // Board widget overlays
    slot->dead_stone_positions = board_widget->getDeadStonePositions();
    slot->disputed_positions   = board_widget->getDisputedPositions();

    // Territory map (scoring overlay) — convert StoneColor → int for slot storage
    slot->territory_map.clear();
    const auto &tmap = board_widget->getTerritoryMap();
    for (auto it = tmap.constBegin(); it != tmap.constEnd(); ++it)
        slot->territory_map[it.key()] = static_cast<int>(it.value());

    // Snapshot board state array from widget
    slot->board_size = board_widget->getBoardSize();
    for (int x = 0; x < slot->board_size; ++x)
        for (int y = 0; y < slot->board_size; ++y)
            slot->board_state[x][y] = board_widget->getBoardState(x, y);

    // Always sync both tree pointers so slot->game_root never dangles if game_root
    // was replaced (e.g. by clearMoveHistoryBeforeMovesCommand).
    slot->game_root    = game_root;
    slot->current_node = current_node;

    // Disconnect display refresh from this slot's timer — the slot keeps ticking
    disconnect(clock_timer, &QTimer::timeout, this, &BoardWindow::updateClockDisplay);
    // Restore BoardWindow's own clock_timer pointer so it is never left dangling
    if (!clock_timer->parent()) {
        clock_timer->setParent(this);
    }
}

void BoardWindow::startObserving(int game_id, const QString &white, const QString &black,
 const QString &w_rank, const QString &b_rank) {
 observed_game_id = game_id;
 white_player = white;
 black_player = black;
 white_rank = w_rank;
 black_rank = b_rank;
 white_rank_at_start = w_rank;
 black_rank_at_start = b_rank;
 current_move = 0;
 current_player = BLACK_STONE;
 is_observing = true;
 is_playing = false; // Ensure playing is false when observing
 game_start_time = QDateTime::currentDateTime();
 
 // Initialize observation state for SGF accuracy
 observation_state = JOINING_GAME;
 observation_start_time = QDateTime::currentDateTime();
 moves_received_during_live = 0;
 
 // Update UI elements for observing mode
 updateCommentButtonText();
 DEBUG_OBSERVATION_STATE << "🎯 OBSERVATION STATE: Changed to JOINING_GAME for game" << game_id;
 
 // Reset game setup and result info
 handicap = 0;
 komi = 0.5; // Will be updated by updateGameSetup() when Command 7 is received
 time_control = "";
 game_type = "Free";
 game_type_locked = false;
 byoyomi_time = 0;
 white_captures = 0;
 black_captures = 0;
 game_result = "";
 game_finished = false;
 move_history.clear();
 clearObservers();

 board_widget->clearBoard();

 // Reset game tree to fresh root node
 delete game_root;
 game_root = new GameNode();
 current_node = game_root;
 current_move_index = 0;

 // Initialize group tracking
 white_groups.clear();
 black_groups.clear();
 
 updateLabels();
 updateWindowTitle();
}

void BoardWindow::loadSGF(GameNode* root, const QString &white, const QString &black,
 const QString &w_rank, const QString &b_rank,
 double komi_value, int handicap_value,
 const QString &result, const QString &filename,
 const QString &game_name) {
 qDebug() << ">>> loadSGF called with game_name:" << game_name;

 // Set player information
 white_player = white;
 black_player = black;
 white_rank = w_rank;
 black_rank = b_rank;

 // Set game setup
 komi = komi_value;
 handicap = handicap_value;
 game_result = result;

 // Set custom game title if provided (for teaching games)
 if (!game_name.isEmpty()) {
 qDebug() << ">>> Calling setCustomGameTitle with:" << game_name;
 setCustomGameTitle(game_name);
 } else {
 qDebug() << ">>> game_name is empty, not setting custom title";
 }

 // Set read-only mode (not observing or playing)
 is_observing = false;
 is_playing = false;
 observed_game_id = -1;

 // Exit scoring mode so the edit window starts with a clean stone view,
 // not the territory overlay from the source game.
 is_scoring_mode = false;
 board_widget->setScoringMode(false);
 board_widget->setTerritoryMap(QMap<QPair<int,int>, StoneColor>());
 board_widget->setDeadStones(QSet<QPair<int,int>>());
 territory_ownership.clear();
 dead_stones.clear();

 // Clear any existing game tree
 delete game_root;
 game_root = root;
 current_node = game_root;
 current_move_index = 0;

 // Clear the board
 board_widget->clearBoard();

 // Navigate to the first move to display the initial position
 displayNode(current_node);

 // Update navigation to show full game
 updateMoveNavigation();

 // Update window title with SGF filename
 QFileInfo fileInfo(filename);
 setWindowTitle(QString("SGF: %1 - %2 vs %3")
 .arg(fileInfo.fileName())
 .arg(white_player)
 .arg(black_player));

 // Update labels to show game info
 updateLabels();

 qDebug() << "Loaded SGF:" << filename
 << "Moves:" << getTotalMoves()
 << white_player << "vs" << black_player;
}

void BoardWindow::stopObserving() {
 is_observing = false;
 observed_game_id = -1;
 observation_state = NOT_OBSERVING;
 moves_received_during_live = 0;
 DEBUG_OBSERVATION_STATE << "🎯 OBSERVATION STATE: Changed to NOT_OBSERVING";
 updateLabels();
 updateWindowTitle();
}

void BoardWindow::clearMoveHistoryBeforeMovesCommand() {
 // CRITICAL FIX: Clear move_history AND game tree before the "moves <game_id>" response arrives
 // to prevent duplicates of any live moves that arrived during the 1-second delay.
 //
 // Background: When observation starts, there's a 1-second delay before sending
 // "moves <game_id>". During this delay, live moves may arrive and get added to
 // move_history AND the game tree. Then when "moves" response arrives, it includes
 // ALL moves (0-N), including those that already arrived live. Without this clear,
 // we get duplicates.
 //
 // Example from game 386:
 // - Observation starts
 // - Live move 257 (B12/bh) arrives and gets added to move_history and game tree
 // - 1 second later, "moves 386" is sent
 // - IGS sends moves 0-257, including move 257 again
 // - Result: move 257 appears twice in both structures -> corrupted SGF

 int old_size = move_history.size();
 move_history.clear();
 board_widget->clearBoard(); // Also clear visual board to match

 // ALSO CLEAR THE GAME TREE to prevent duplicate nodes
 delete game_root;
 game_root = new GameNode();
 current_node = game_root;

 // Reset mv_counter to -1 (q5Go pattern) to block live moves until history arrives
 mv_counter = -1;

 qDebug() << "🧹 CLEARED" << old_size << "moves from history AND reset game tree before 'moves' command"
 << "for game" << observed_game_id << "- set mv_counter = -1 to wait for history";
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
 DEBUG_OBSERVATION_STATE << "🎯 OBSERVATION STATE: Changed to RECONSTRUCTING";
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
 DEBUG_OBSERVATION_STATE << "🎯 OBSERVATION STATE: Changed to LIVE_OBSERVATION - move" << move.move_number << "marked as live";
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
 DEBUG_OBSERVATION_STATE << "🎯 LIVE MOVE: #" << moves_received_during_live << "move" << move.move_number;
 break;
 }
 
 DEBUG_OBSERVATION_STATE << "🎯 MOVE TRACKING: State=" << observation_state << "Move" << move.move_number << "Live=" << move.is_live_move;
}

void BoardWindow::processMove(const GameMove &move) {
 qDebug() << "DEBUG: BoardWindow::processMove called - Game:" << move.game_id
 << "Observing:" << is_observing << "Playing:" << is_playing << "Expected game:" << observed_game_id;

 if ((!is_observing && !is_playing) || move.game_id != observed_game_id) {
 qDebug() << "DEBUG: Move rejected - not observing/playing or wrong game";
 return;
 }

 // CRITICAL: Block ALL moves after entering scoring mode (including regular moves!)
 // After 3 passes, the game is in counting phase - all subsequent "moves" are
 // stone-marking interactions, not actual game moves
 if (is_scoring_mode && !is_playing) {
 qDebug() << "*** SCORING MODE: Ignoring move" << move.move_number
 << "- game is in counting phase, not recording stone-marking as moves";
 return;
 }

 // q5Go pattern: Skip live moves that arrive before history (mv_counter == -1)
 // This prevents race condition where live moves arrive before "moves N" response
 if (mv_counter == -1 && move.move_number > 0) {
 qDebug() << "[q5Go] SKIPPING live move" << move.move_number
 << "- waiting for history (mv_counter = -1)";
 return;
 }

 // When move 0 arrives (first history move), set mv_counter to 0
 if (mv_counter == -1 && move.move_number == 0) {
 mv_counter = 0;
 qDebug() << "[q5Go] First history move arrived (move 0) - set mv_counter = 0";
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

 // BUILD GAME TREE: Add handicap as a special node
 // CRITICAL FIX: Always add to END of active variation
 GameNode* insertion_point = game_root;
 while (insertion_point->nextMove()) {
 insertion_point = insertion_point->nextMove();
 }
 GameNode* new_node = insertion_point->addMove(-2, handicap_count, BLACK_STONE);

 // Copy board state with handicap stones to new node
 GoBoard new_board;
 new_board.clear();
 for (int x = 0; x < 19; x++) {
 for (int y = 0; y < 19; y++) {
 StoneColor stone = board_widget->getBoardState(x, y);
 if (stone != EMPTY) {
 new_board.placeStone(x, y, stone);
 }
 }
 }
 new_node->setBoard(new_board);

 // AUTO-FOLLOW MODE: Only update display if auto-follow is enabled
 if (auto_follow_mode) {
 current_node = new_node;
 current_move = move.move_number;
 server_move_count = move.move_number; // Track official server count

 // Increment mv_counter after successfully processing move
 if (mv_counter >= 0) {
 mv_counter++;
 }
 } else {
 // Not following - just update server move count
 server_move_count = move.move_number;
 qDebug() << "AUTO-FOLLOW: Handicap added to tree (not displayed - viewing earlier position)";

 // Still update the slider maximum so user can see new moves exist
 int total_moves = getTotalMoves();
 slider_update_in_progress = true;
 move_slider->setMaximum(total_moves);
 slider_update_in_progress = false;
 }

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

 // q5Go pattern: DO NOT record passes after entering scoring mode
 // Passes during counting phase are territory-marking interactions, not game moves
 if (!is_scoring_mode) {
 // CRITICAL VALIDATION: Ensure pass has valid color
 if (tracked_move.color != BLACK_STONE && tracked_move.color != WHITE_STONE) {
 qDebug() << "ERROR: Rejecting PASS with invalid color!" << tracked_move.color
 << "move number:" << tracked_move.move_number;
 return;
 }

 // Store pass move in history (only for actual game passes before scoring)
 QString move_color_str = (tracked_move.color == BLACK_STONE) ? "B" : "W";
 qDebug() << "🗂️ MOVE HISTORY: Adding PASS move" << tracked_move.move_number
 << move_color_str + "[]"
 << "total moves:" << move_history.size() << "Live=" << tracked_move.is_live_move;
 move_history.append(tracked_move);

 // BUILD GAME TREE: Add pass move as new node
 // CRITICAL FIX: Always add to END of active variation
 GameNode* insertion_point = game_root;
 while (insertion_point->nextMove()) {
 insertion_point = insertion_point->nextMove();
 }
 GameNode* new_node = insertion_point->addMove(-1, -1, tracked_move.color);
 new_node->setBoard(insertion_point->getBoard().copy()); // Pass doesn't change board

 // AUTO-FOLLOW MODE: Only update display if auto-follow is enabled
 if (auto_follow_mode) {
 // Clear last move marker on pass - matches q5Go behavior (visual update only when following)
 board_widget->setLastMove(-1, -1);

 current_node = new_node;
 current_move = move.move_number;
 server_move_count = move.move_number; // Track official server count

 // Increment mv_counter after successfully processing move
 if (mv_counter >= 0) {
 mv_counter++;
 }

 qDebug() << "*** PASS MOVE PROCESSED: client history size=" << move_history.size() << "server count=" << server_move_count;
 } else {
 // Not following - just update server move count
 server_move_count = move.move_number;
 qDebug() << "AUTO-FOLLOW: Pass move" << move.move_number << "added to tree (not displayed - viewing earlier position)";

 // Still update the slider maximum so user can see new moves exist
 int total_moves = getTotalMoves();
 slider_update_in_progress = true;
 move_slider->setMaximum(total_moves);
 slider_update_in_progress = false;
 }
 } else {
 qDebug() << "*** SCORING MODE: Ignoring counting-phase pass (stone-marking interaction, not a game move)";
 }

 current_player = (move.color == BLACK_STONE) ? WHITE_STONE : BLACK_STONE;

 // CRITICAL FIX: Enter scoring mode after 3 passes to prevent counting-phase passes
 // from being recorded as game moves in the SGF
 // We set the internal flag but don't show territory markers until Command 22 arrives
 if (consecutive_passes >= 3 && !is_scoring_mode) {
 qDebug() << "*** 3 CONSECUTIVE PASSES DETECTED - entering scoring mode";
 qDebug() << "*** Territory markers will be shown when Command 22 (score data) arrives";
 is_scoring_mode = true; // Block further pass moves from being recorded
 // Note: We don't call board_widget->setScoringMode(true) yet - that happens in receiveScoreBegin()
 }

 // Reset lag compensation timer when player switches
 last_time_update = QDateTime::currentDateTime();

 updateLabels();
 updateMoveNavigation();
 return;
 }
 
 qDebug() << "DEBUG: Placing move at (" << tracked_move.x << "," << tracked_move.y << ") color:" << tracked_move.color;

 // CRITICAL VALIDATION: Reject moves with invalid color
 if (tracked_move.color != BLACK_STONE && tracked_move.color != WHITE_STONE) {
 qDebug() << "ERROR: Rejecting move with invalid color!" << tracked_move.color
 << "at (" << tracked_move.x << "," << tracked_move.y << ")";
 return;
 }

 // Reset consecutive pass counter for regular moves
 consecutive_passes = 0;
 qDebug() << "DEBUG: Regular move - reset consecutive passes counter";

 // Store move in history for SGF saving
 // Convert coordinates to SGF format for debugging
 char sgf_col = (tracked_move.x >= 0 && tracked_move.x < 19) ? ('a' + tracked_move.x) : '?';
 char sgf_row = (tracked_move.y >= 0 && tracked_move.y < 19) ? ('a' + tracked_move.y) : '?';
 QString move_color_str = (tracked_move.color == BLACK_STONE) ? "B" : "W";
 qDebug() << "🗂️ MOVE HISTORY: Adding move" << tracked_move.move_number
 << move_color_str + "[" + QString(sgf_col) + QString(sgf_row) + "]"
 << "total moves:" << move_history.size() << "Live=" << tracked_move.is_live_move;
 move_history.append(tracked_move);

 // BUILD GAME TREE: Add regular move as new node with complete board state
 // CRITICAL FIX: Always add new moves to the END of the active variation,
 // not to current_node (which may be pointing to an earlier position if user scrolled backward)
 GameNode* insertion_point = game_root;
 while (insertion_point->nextMove()) {
 insertion_point = insertion_point->nextMove();
 }
 GameNode* new_node = insertion_point->addMove(tracked_move.x, tracked_move.y, tracked_move.color);

 // CRITICAL FIX: Build new board state from insertion_point (last move), NOT from board_widget
 // board_widget may be showing an earlier position if user navigated backward!
 GoBoard new_board = insertion_point->getBoard().copy();

 // Apply the new move to the board
 new_board.placeStone(tracked_move.x, tracked_move.y, tracked_move.color);

 // Calculate and remove captured stones (Go rules)
 StoneColor opponent_color = (tracked_move.color == BLACK_STONE) ? WHITE_STONE : BLACK_STONE;
 int dx[] = {-1, 1, 0, 0};
 int dy[] = {0, 0, -1, 1};

 // Check all 4 neighbors for captured opponent groups
 for (int dir = 0; dir < 4; dir++) {
 int nx = tracked_move.x + dx[dir];
 int ny = tracked_move.y + dy[dir];

 if (nx >= 0 && nx < 19 && ny >= 0 && ny < 19 && new_board.getStone(nx, ny) == opponent_color) {
 // Check if this opponent group has no liberties
 if (countLiberties(new_board, nx, ny) == 0) {
 removeGroup(new_board, nx, ny);
 }
 }
 }

 // Check for suicide (own group has no liberties after placement)
 if (countLiberties(new_board, tracked_move.x, tracked_move.y) == 0) {
 removeGroup(new_board, tracked_move.x, tracked_move.y);
 }

 new_node->setBoard(new_board);

 // AUTO-FOLLOW MODE: Only update display if auto-follow is enabled
 // New moves are ALWAYS added to tree, but display only updates if following
 if (auto_follow_mode) {
 // UPDATE VISUAL DISPLAY: Place the move on the board widget
 board_widget->placeMoveAt(tracked_move.x, tracked_move.y, tracked_move.color);
 board_widget->setLastMove(tracked_move.x, tracked_move.y);

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

 // Calculate and execute captures using client-side logic (visual board only)
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

 // Remove the captured stone from the visual board
 board_widget->placeMoveAt(cap_x, cap_y, EMPTY);
 qDebug() << "DEBUG: Server-specified capture - removed stone at (" << cap_x << "," << cap_y << ")";
 }
 }
 qDebug() << "DEBUG: Processed" << all_captures.size() << "server-specified captured stones";
 }

 // 🔍 AUDIT: Log the completed move audit entry
 logMoveAudit(audit);

 qDebug() << "DEBUG: Placed move" << move.move_number << "at (" << move.x << "," << move.y << ") color:" << (move.color == BLACK_STONE ? "BLACK" : "WHITE");

 current_node = new_node;
 current_move = move.move_number;
 server_move_count = move.move_number; // Track official server count for regular moves too

 // Increment mv_counter after successfully processing move
 if (mv_counter >= 0) {
 mv_counter++;
 }

 current_player = (move.color == BLACK_STONE) ? WHITE_STONE : BLACK_STONE;

 // Reset lag compensation timer when player switches
 last_time_update = QDateTime::currentDateTime();

 updateLabels();

 // Update move navigation to current move (auto-follow) - use server count
 current_move_index = server_move_count > 0 ? server_move_count : move_history.size();
 updateMoveNavigation();
 } else {
 // Not following - just update the server move count for accuracy
 server_move_count = move.move_number;
 qDebug() << "AUTO-FOLLOW: Move" << move.move_number << "added to tree (not displayed - viewing earlier position)";

 // Still update the slider maximum so user can see new moves exist
 // But don't change the current position
 int total_moves = getTotalMoves();
 slider_update_in_progress = true;
 move_slider->setMaximum(total_moves);
 slider_update_in_progress = false;
 }
}

void BoardWindow::updateGameInfo(const QString &info) {
 // Handle any additional game information updates
 updateLabels();
}

void BoardWindow::updateTimeInfo(const QString &white_time, const QString &black_time) {
 if (white_clock_label && black_clock_label) {
 white_clock_label->setText(white_time);
 black_clock_label->setText(black_time);
 }
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
 if ((!is_observing && !is_playing) || last_time_update.isNull()) {
 return;
 }

 // Each timer tick: decrement the current player's stored time by elapsed seconds,
 // then slide last_time_update forward so the next tick measures from now.
 QDateTime now = QDateTime::currentDateTime();
 int elapsed_seconds = static_cast<int>(last_time_update.secsTo(now));
 if (elapsed_seconds > 0) {
     if (current_player == WHITE_STONE)
         white_time_seconds = qMax(0, white_time_seconds - elapsed_seconds);
     else if (current_player == BLACK_STONE)
         black_time_seconds = qMax(0, black_time_seconds - elapsed_seconds);
     last_time_update = now;
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
 white_display = QString("%1 / %2").arg(formatTime(white_time_seconds)).arg(white_byo_moves);
 } else {
 white_display = formatTime(white_time_seconds);
 }

 if (black_byo_moves >= 0) {
 black_display = QString("%1 / %2").arg(formatTime(black_time_seconds)).arg(black_byo_moves);
 } else {
 black_display = formatTime(black_time_seconds);
 }

 // Update individual clock labels (q5Go style)
 if (white_clock_label && black_clock_label) {
 white_clock_label->setText(white_display);
 black_clock_label->setText(black_display);
 }
}

void BoardWindow::rebuildGameTreeFromMoveHistory()
{
    // Build from the existing root in-place so that any slot pointer to game_root
    // stays valid.  Only called when the tree is empty (getTotalMoves() == 0).

    GameNode* insertion_point = game_root;
    GoBoard current_board;

    for (const GameMove &move : move_history) {
        // Handicap setup move (x == -2)
        if (move.x == -2) {
            QList<QPair<int,int>> positions = IGSMoveParser::getHandicapPositions(move.y);
            GoBoard handicap_board;
            for (const auto &pos : positions)
                handicap_board.placeStone(pos.first, pos.second, BLACK_STONE);
            GameNode *hnode = insertion_point->addMove(-2, move.y, BLACK_STONE);
            hnode->setBoard(handicap_board);
            current_board = handicap_board;
            insertion_point = hnode;
            continue;
        }

        StoneColor color = static_cast<StoneColor>(move.color);
        if (color != BLACK_STONE && color != WHITE_STONE) continue;

        GameNode *new_node = insertion_point->addMove(move.x, move.y, color);

        if (move.x >= 0 && move.y >= 0) {
            GoBoard new_board = current_board.copy();
            new_board.placeStone(move.x, move.y, color);

            StoneColor opp = (color == BLACK_STONE) ? WHITE_STONE : BLACK_STONE;
            int dx[] = {-1, 1, 0, 0};
            int dy[] = {0, 0, -1, 1};
            for (int dir = 0; dir < 4; dir++) {
                int nx = move.x + dx[dir], ny = move.y + dy[dir];
                if (nx >= 0 && nx < 19 && ny >= 0 && ny < 19 &&
                    new_board.getStone(nx, ny) == opp &&
                    countLiberties(new_board, nx, ny) == 0)
                    removeGroup(new_board, nx, ny);
            }
            if (countLiberties(new_board, move.x, move.y) == 0)
                removeGroup(new_board, move.x, move.y);

            new_node->setBoard(new_board);
            current_board = new_board;
        } else {
            // Pass — board unchanged
            new_node->setBoard(current_board.copy());
        }

        insertion_point = new_node;
    }

    current_node = insertion_point;
    current_move_index = current_node->moveNumber();
    qDebug() << "rebuildGameTreeFromMoveHistory: rebuilt" << move_history.size()
             << "moves into game tree (" << getTotalMoves() << "total)";
}

void BoardWindow::editGame() {
 if (!is_observing && !is_playing && !game_finished) {
 QMessageBox::information(this, "Edit Game",
 "No game currently being observed.\n\n" "Edit Game is available when observing a game.");
 return;
 }

 // Docked inactive slots track moves in move_history but skip game tree building.
 // Rebuild the tree now if it is empty so generateSGF() produces a full record.
 if (getTotalMoves() == 0 && !move_history.isEmpty()) {
     rebuildGameTreeFromMoveHistory();
 }

 // Generate SGF from current game state
 QString sgf_content = generateSGF();

 // Create a temporary file to hold the SGF
 QString temp_filename = QString("/tmp/xgospel2_edit_%1_%2_vs_%3.sgf")
 .arg(observed_game_id)
 .arg(white_player)
 .arg(black_player);

 QFile file(temp_filename);
 if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
 QMessageBox::warning(this, "Edit Game Error",
 QString("Failed to create temporary SGF file:\n%1").arg(temp_filename));
 return;
 }

 QTextStream out(&file);
 out << sgf_content;
 file.close();

 // Parse the SGF back to get a fresh game tree
 SGFParser parser;
 QString error;
 GameNode* root = parser.parseFile(temp_filename, error);

 if (!root) {
 QMessageBox::warning(this, "Edit Game Error",
 QString("Failed to parse generated SGF:\n%1").arg(error));
 return;
 }

 // Create a dedicated SGF editor window
 BoardWindow* edit_board = new BoardWindow(parentWidget(), my_username, true);

 // Wire back to this window so Update can re-sync from the live game
 edit_board->source_board_window = this;

 // Null out the pointer if the source window is closed before the edit window.
 // Without this, source_board_window becomes a dangling pointer and crashes on Update.
 connect(this, &QObject::destroyed, edit_board, [edit_board]() {
     edit_board->source_board_window = nullptr;
 });

 // Load the SGF into the edit window
 edit_board->loadSGF(root, white_player, black_player,
 white_rank, black_rank,
 komi, handicap,
 game_result, temp_filename);

 // Navigate to the most recent move
 edit_board->goToLastMove();

 // Set the next player color based on whose turn it is
 edit_board->board_widget->setNextPlayerColor(edit_board->current_player);

 edit_board->show();
 edit_board->raise();
 edit_board->activateWindow();

 DEBUG_EDIT_MODE << "Opened SGF editor for game #" << observed_game_id
 << white_player << "vs" << black_player;
}

void BoardWindow::saveGame() {
    // Rebuild game tree from move history if needed (docked/local play may not have built it)
    if (getTotalMoves() == 0 && !move_history.isEmpty())
        rebuildGameTreeFromMoveHistory();

    // Get save directory from settings (or use default)
    QString saveDir = settings->getSaveGameDirectory();

    // Ensure directory exists
    QDir dir(saveDir);
    if (!dir.exists()) {
        if (!dir.mkpath(saveDir)) {
            QMessageBox::warning(this, "Save Error",
                QString("Cannot create save directory:\n%1").arg(saveDir));
            return;
        }
    }

    // Build a clean filename — for local engine games, game id is -2
    QString id_str = (observed_game_id >= 0)
                     ? QString::number(observed_game_id)
                     : "local";
    QString filename = QString("%1/%2_%3_vs_%4_%5.sgf")
        .arg(saveDir)
        .arg(id_str)
        .arg(white_player.isEmpty() ? "White" : white_player)
        .arg(black_player.isEmpty() ? "Black" : black_player)
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

    QString sgf_content = generateSGF();

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << sgf_content;
        file.close();
        if (comment_display)
            comment_display->append(QString("✓ Game saved to: %1").arg(filename));
        emit gameSaved(observed_game_id, filename);
        qDebug() << "[saveGame] Saved to:" << filename;
    } else {
        QString msg = QString("✗ Error saving game to: %1").arg(filename);
        if (comment_display)
            comment_display->append(msg);
        QMessageBox::warning(this, "Save Error", msg);
        qDebug() << "[saveGame] Failed to open for writing:" << filename;
    }
}

void BoardWindow::closeBoard() {
 if (is_observing) {
 emit boardClosed(observed_game_id);
 }
 close();
}

void BoardWindow::updateLabels() {
 // Update game info label and dynamic "to play" stone icon (q5Go style)
 if (game_info_label && to_play_stone_icon) {
 QString next_player = (current_player == WHITE_STONE) ? "White to Play" : "Black to Play";
 if (is_observing || is_playing) {
 game_info_label->setText(QString("Game #%1 | %2").arg(observed_game_id).arg(next_player));

 // Update dynamic stone icon to match current player (use cached pixmaps)
 if (current_player == WHITE_STONE) {
 to_play_stone_icon->setPixmap(icon_white_pixmap);
 } else {
 to_play_stone_icon->setPixmap(icon_black_pixmap);
 }
 to_play_stone_icon->setVisible(true);
 } else {
 game_info_label->setText("No game");
 to_play_stone_icon->setVisible(false);
 }
 }

 // Update handicap/komi/game-type label (borderless, larger font, no redundant captures)
 if (handicap_komi_label) {
 // game_type is already a QString member variable ("Free", "Rated", or "Teach")
 QString type_text = game_type.isEmpty() ? "Free" : game_type;
 handicap_komi_label->setText(QString("Komi: %1 Handicap: %2 %3")
 .arg(komi, 0, 'f', 1)
 .arg(handicap)
 .arg(type_text));
 }

 // Update individual capture count labels (borderless)
 if (white_captures_label) {
 // Check if this is a scored result (not resignation/time/forfeit/adjourn)
 bool is_scored_result = game_finished &&
                         !game_result.contains("+R") &&  // Resignation
                         !game_result.contains("+T") &&  // Time
                         !game_result.contains("+F") &&  // Forfeit
                         !game_result.contains("+A");    // Adjourn
 bool show_stats = is_scoring_mode || is_scored_result;

 if (show_stats) {
 // q5Go-style scoring statistics (multi-line)
 // Count stones on board
 int white_stones = 0;
 int dead_white_stones = 0;
 for (int y = 0; y < 19; y++) {
 for (int x = 0; x < 19; x++) {
 if (board_widget->getBoardState(x, y) == WHITE_STONE) {
 white_stones++;
 }
 }
 }

 // Count dead stones
 // Dead white stones don't count in "Stones:" (matching q5Go behavior)
 // Dead black stones count as white captures
 int dead_black_stones = 0;
 for (const auto &pos : dead_stones) {
 if (board_widget->getBoardState(pos.first, pos.second) == BLACK_STONE) {
 dead_black_stones++;
 } else if (board_widget->getBoardState(pos.first, pos.second) == WHITE_STONE) {
 dead_white_stones++;
 }
 }

 // Subtract dead white stones from count (q5Go only counts living stones)
 white_stones -= dead_white_stones;

 // When CMD22 data is present, white_captures was set directly from the server
 // header and already includes dead stones — do not add dead_black_stones again.
 // Only add dead stones when falling back to client flood-fill (no CMD22).
 int white_total_captures = territory_ownership.isEmpty()
     ? white_captures + dead_black_stones
     : white_captures;
 double white_total = white_territory + white_total_captures + komi;
 white_captures_label->setText(QString("Stones: %1\nCap: %2\nTerr: %3\nTotal: %4")
 .arg(white_stones)
 .arg(white_total_captures)
 .arg(white_territory)
 .arg(white_total, 0, 'f', 1));
 } else {
 // Normal mode - just show captures
 white_captures_label->setText(QString("Captures: %1").arg(white_captures));
 }
 }
 if (black_captures_label) {
 // Check if this is a scored result (not resignation/time/forfeit/adjourn)
 bool is_scored_result = game_finished &&
                         !game_result.contains("+R") &&  // Resignation
                         !game_result.contains("+T") &&  // Time
                         !game_result.contains("+F") &&  // Forfeit
                         !game_result.contains("+A");    // Adjourn
 bool show_stats = is_scoring_mode || is_scored_result;

 if (show_stats) {
 // q5Go-style scoring statistics (multi-line)
 // Count stones on board
 int black_stones = 0;
 int dead_black_stones = 0;
 for (int y = 0; y < 19; y++) {
 for (int x = 0; x < 19; x++) {
 if (board_widget->getBoardState(x, y) == BLACK_STONE) {
 black_stones++;
 }
 }
 }

 // Count dead stones
 // Dead black stones don't count in "Stones:" (matching q5Go behavior)
 // Dead white stones count as black captures
 int dead_white_stones = 0;
 for (const auto &pos : dead_stones) {
 if (board_widget->getBoardState(pos.first, pos.second) == WHITE_STONE) {
 dead_white_stones++;
 } else if (board_widget->getBoardState(pos.first, pos.second) == BLACK_STONE) {
 dead_black_stones++;
 }
 }

 // Subtract dead black stones from count (q5Go only counts living stones)
 black_stones -= dead_black_stones;

 // When CMD22 data is present, black_captures was set directly from the server
 // header and already includes dead stones — do not add dead_white_stones again.
 // Only add dead stones when falling back to client flood-fill (no CMD22).
 int black_total_captures = territory_ownership.isEmpty()
     ? black_captures + dead_white_stones
     : black_captures;
 double black_total = black_territory + black_total_captures;
 black_captures_label->setText(QString("Stones: %1\nCap: %2\nTerr: %3\nTotal: %4")
 .arg(black_stones)
 .arg(black_total_captures)
 .arg(black_territory)
 .arg(black_total, 0, 'f', 1));
 } else {
 // Normal mode - just show captures
 black_captures_label->setText(QString("Captures: %1").arg(black_captures));
 }
 }

 // Update player info groups (q5Go style)
 updatePlayerInfoGroups();
}

QString BoardWindow::formatGameInfo() {
 if (is_observing) {
 return QString("Game #%1").arg(observed_game_id);
 }
 return "No game observed";
}

void BoardWindow::updatePlayerInfoGroups() {
 // Safety check - widgets might not be initialized yet
 if (!white_player_label || !black_player_label) {
 return;
 }

 if (is_observing || is_playing || is_edit_window) {
 // Update White player info (name/rank only - stone icon shows color)
 white_player_label->setText(QString("%1 %2").arg(white_player, white_rank));

 // Update Black player info (name/rank only - stone icon shows color)
 black_player_label->setText(QString("%1 %2").arg(black_player, black_rank));
 } else {
 white_player_label->setText("White");
 black_player_label->setText("Black");
 }
}

QString BoardWindow::formatMoveInfo() {
 if (is_observing || is_playing) {
 if (is_scoring_mode) {
 // Show scoring information - use server scores if available
 if (has_server_score) {
 // Display server-provided scores (already includes prisoners + komi)
 double score_diff = server_black_score - server_white_score;
 return QString("SCORING MODE\nWhite: %1\nBlack: %2\nScore: %3")
 .arg(server_white_score, 0, 'f', 1)
 .arg(server_black_score, 0, 'f', 1)
 .arg(score_diff > 0 ? QString("B+%1").arg(score_diff, 0, 'f', 1) : QString("W+%1").arg(-score_diff, 0, 'f', 1));
 } else {
 // Fallback to client-side calculation if server scores not available
 return QString("SCORING MODE\nWhite: %1\nBlack: %2\nScore: %3")
 .arg(white_territory + white_prisoners + komi, 0, 'f', 1)
 .arg((double)(black_territory + black_prisoners), 0, 'f', 1)
 .arg(final_score, 0, 'f', 1);
 }
 } else {
 QString next_player = (current_player == BLACK_STONE) ? "Black" : "White";
 return QString("Move: %1\nNext: %2").arg(current_move).arg(next_player);
 }
 }
 return "";
}

QString BoardWindow::formatHandicapKomiInfo() {
 if (is_observing || is_playing) {
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

 QString formatted_message;

 if (is_kibitz) {
 // Kibitz messages show move number (matching q5Go format)
 int move_num = current_node ? current_node->moveNumber() : 0;
 formatted_message = QString("[%1] KIBITZ %2: %3")
 .arg(move_num)
 .arg(user)
 .arg(message);
 } else {
 // Say messages (private) - use timestamp and user
 QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
 formatted_message = QString("[%1] %2: %3")
 .arg(timestamp)
 .arg(user)
 .arg(message);
 }

 comment_display->append(formatted_message);

 // Auto-scroll to bottom
 QTextCursor cursor = comment_display->textCursor();
 cursor.movePosition(QTextCursor::End);
 comment_display->setTextCursor(cursor);

 // q5Go pattern: Save kibitz to game tree for SGF export
 // Attach comment to current node (the most recent move)
 if (current_node && is_kibitz) {
 // Get existing comment if any, and append new kibitz
 QString existing_comment = current_node->getComment();

 // Format like q5Go: (move_number) user: message
 int move_num = current_node->moveNumber();
 QString kibitz_text = QString("(%1) %2: %3").arg(move_num).arg(user).arg(message);

 if (existing_comment.isEmpty()) {
 current_node->setComment(kibitz_text);
 } else {
 // Append to existing comment with newline separator
 current_node->setComment(existing_comment + "\n" + kibitz_text);
 }

 qDebug() << "[KIBITZ] Saved to node:" << kibitz_text;
 }
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
 
 comment_input->clear();

 if (game_finished) {
 // Game over — 'say' is rejected by IGS; use 'tell' to reach opponent directly
 QString opponent = (my_username == white_player) ? black_player : white_player;
 if (opponent.isEmpty()) {
 if (comment_display)
 comment_display->append("ERROR: Cannot send message — opponent name unknown");
 return;
 }
 qDebug() << "Emitting tellRequested for opponent:" << opponent;
 emit tellRequested(opponent, message);
 if (comment_display) {
 QString display_name = my_username.isEmpty() ? "You" : my_username;
 QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
 comment_display->append(QString("[%1] TELL %2: %3").arg(timestamp).arg(display_name).arg(message));
 }
 } else if (is_playing) {
 // Send "say" command for private player communication
 qDebug() << "Emitting sayRequested signal with game_id:" << observed_game_id;
 emit sayRequested(observed_game_id, message);
 // Show message locally with the logged-in user's name and timestamp
 if (comment_display) {
 QString display_name = my_username.isEmpty() ? "You" : my_username;
 QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
 comment_display->append(QString("[%1] %2: %3").arg(timestamp).arg(display_name).arg(message));
 }
 } else if (is_observing) {
 // Send "kibitz" command for public observer comments
 qDebug() << "Emitting commentRequested signal with game_id:" << observed_game_id;
 emit commentRequested(observed_game_id, message);

 QString display_name = my_username.isEmpty() ? "You" : my_username;
 int move_num = current_node ? current_node->moveNumber() : 0;

 // Show kibitz message locally with move number (matching q5Go format)
 if (comment_display) {
 comment_display->append(QString("[%1] KIBITZ %2: %3").arg(move_num).arg(display_name).arg(message));
 }

 // Save outgoing kibitz to game tree for SGF export (matching q5Go behavior)
 if (current_node) {
 QString existing_comment = current_node->getComment();
 QString kibitz_text = QString("(%1) %2: %3").arg(move_num).arg(display_name).arg(message);

 if (existing_comment.isEmpty()) {
 current_node->setComment(kibitz_text);
 } else {
 current_node->setComment(existing_comment + "\n" + kibitz_text);
 }

 qDebug() << "[KIBITZ] Saved outgoing to node:" << kibitz_text;
 }
 } else {
 qDebug() << "ERROR: Not in a game";
 if (comment_display) {
 comment_display->append("ERROR: Not currently in a game");
 }
 }
}


void BoardWindow::onCommentInputReturn() {
 // Default to sending as comment when Enter is pressed
 sendComment();
}

void BoardWindow::requestObservers() {
    clearObservers();
    if (observed_game_id > 0)
        emit observersRequested(observed_game_id);
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
 // Only update game type if not locked (locked = corrected type applied) 
 if (!game_type_locked) {
 // Don't overwrite Teaching type if it was set by custom title
 if (game_type != "Teaching") {
 game_type = type;
 }
 qDebug() << "updateGameDetails called with type:" << type << "- Updated game_type:" << game_type;
 } else {
 qDebug() << "updateGameDetails called with type:" << type << "- IGNORED (locked as:" << game_type << ")";
 }
 
 byoyomi_time = byoyomi_seconds;
 updateLabels();
}

void BoardWindow::lockGameType() {
 game_type_locked = true;
 qDebug() << "🔒 GAME TYPE LOCKED:" << game_type;
}

void BoardWindow::updateCaptures(int white_caps, int black_caps) {
 white_captures = white_caps;
 black_captures = black_caps;
 updateLabels();
}

void BoardWindow::updateGameResult(const QString &result) {
 game_result = result;
 game_finished = true;
 clock_timer->stop();
 // Game is over — hide Done button
 if (done_button) done_button->setVisible(false);

 // Display result in comment area (q5Go style)
 if (comment_display) {
 QString standard_result = convertIGSResultToStandard(result);
 QString display_result = result;

 // Improve resign messages to be more readable
 if (standard_result == "B+R") {
 display_result = "White resigned.";
 } else if (standard_result == "W+R") {
 display_result = "Black resigned.";
 }

 // Check if this is a scored result (not resignation/time/forfeit/adjourn)
 bool is_scored_result = !standard_result.contains("+R") &&
                         !standard_result.contains("+T") &&
                         !standard_result.contains("+F") &&
                         !standard_result.contains("+A");

 QString result_message;
 if (is_scored_result) {
 // Use white_captures/black_captures — set from CMD22 header by updateCaptures()
 // and matching the stats panel exactly. white_prisoners is not updated after
 // enterScoringModeForResult() so it can be stale; white_captures is authoritative.
 double white_total = white_territory + white_captures + komi;
 double black_total = black_territory + black_captures;

 result_message = QString("Game finished: %1\nW %2 B %3")
 .arg(standard_result)
 .arg(white_total, 0, 'f', 1)
 .arg(black_total, 0, 'f', 1);
 } else {
 // Non-scored result - keep both readable message and standard notation
 result_message = QString("Game finished: %1 (%2)").arg(display_result).arg(standard_result);
 }
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

void BoardWindow::setWhiteRank(const QString &rank) {
 white_rank = rank;
 updateLabels();
}

void BoardWindow::setBlackRank(const QString &rank) {
 black_rank = rank;
 updateLabels();
}

void BoardWindow::setCustomGameTitle(const QString &title) {
 custom_game_title = title;
 qDebug() << ">>> setCustomGameTitle called with:" << title;

 // Teaching games are the only ones with custom titles, so update game type
 game_type = "Teaching";

 // Display teaching title in dedicated label (xgospel style)
 if (teaching_title_label) {
 if (!custom_game_title.isEmpty()) {
 qDebug() << ">>> Setting teaching title label text and showing it";
 teaching_title_label->setText(custom_game_title);
 teaching_title_label->show();
 } else {
 qDebug() << ">>> Hiding teaching title label (empty title)";
 teaching_title_label->hide();
 }
 } else {
 qDebug() << ">>> WARNING: teaching_title_label is NULL!";
 }

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

void BoardWindow::updateCommentButtonText() {
 if (!send_comment_button || !comment_input) return;
 
 if (is_playing) {
 // Playing mode - use "Say" for private player communication
 // Determine opponent's name
 QString opponent_name;
 if (!my_username.isEmpty()) {
 if (white_player.compare(my_username, Qt::CaseInsensitive) == 0) {
 opponent_name = black_player;
 } else if (black_player.compare(my_username, Qt::CaseInsensitive) == 0) {
 opponent_name = white_player;
 }
 }

 QString button_text = opponent_name.isEmpty() ? "Say (Enter)" : QString("Say to %1 (Enter)").arg(opponent_name);
 send_comment_button->setText(button_text);
 comment_input->setPlaceholderText(opponent_name.isEmpty() ? "Type message to opponent..." : QString("Type message to %1...").arg(opponent_name));
 send_comment_button->setStyleSheet(
 "QPushButton {" " background-color: #4CAF50;" // Green for say
 " "
 " border: none;" " padding: 4px 8px;" " font-size: 10px;" " font-weight: bold;" "}" "QPushButton:pressed {" " background-" "}"
 );
 } else if (is_observing) {
 // Observing mode - use "Kibitz" for public observer comments
 send_comment_button->setText("Kibitz (Enter)");
 comment_input->setPlaceholderText("Type comment or kibitz...");
 send_comment_button->setStyleSheet(
 "QPushButton {" " background-" // Blue for kibitz
 " "
 " border: none;" " padding: 4px 8px;" " font-size: 10px;" " font-weight: bold;" "}" "QPushButton:pressed {" " background-" "}"
 );
 } else {
 // Default state - disabled
 send_comment_button->setText("Comment");
 comment_input->setPlaceholderText("Join a game to chat...");
 send_comment_button->setEnabled(false);
 }
}

// Converts an IGS rank string to a numeric strength for sorting.
// Higher return value = stronger player (sorts to top).
static int rankToStrength(const QString &rank) {
    QString r = rank.trimmed().toLower();
    r.remove('*').remove('+').remove('?');
    if (r.endsWith("p")) {
        bool ok; int n = r.left(r.size()-1).toInt(&ok);
        return ok ? 10000 + n : 10000;
    }
    if (r.endsWith("d")) {
        bool ok; int n = r.left(r.size()-1).toInt(&ok);
        return ok ? 1000 + n : 1000;
    }
    if (r.endsWith("k")) {
        bool ok; int n = r.left(r.size()-1).toInt(&ok);
        return ok ? (100 - n) : 0;
    }
    return 0;
}

void BoardWindow::clearObservers() {
    observers_raw.clear();
    if (observers_list) observers_list->clear();
    if (observers_title) observers_title->setText("Observers");
}

void BoardWindow::addObserver(const QString &name, const QString &rank) {
    if (!observers_list) return;

    observers_raw.append({name, rank});
    int count = observers_raw.size();

    // Rebuild list in correct order
    observers_list->clear();
    QList<QPair<QString,QString>> display = observers_raw;
    if (observers_sort_by_rank) {
        std::stable_sort(display.begin(), display.end(),
            [](const QPair<QString,QString> &a, const QPair<QString,QString> &b) {
                return rankToStrength(a.second) > rankToStrength(b.second);
            });
    }
    for (const auto &entry : display)
        observers_list->addItem(QString("%1 %2").arg(entry.first, entry.second));

    if (observers_title)
        observers_title->setText(QString("Observers (%1)").arg(count));
}

void BoardWindow::toggleObserverSort() {
    observers_sort_by_rank = !observers_sort_by_rank;
    observers_sort_btn->setText(observers_sort_by_rank ? "Rank" : "Order");

    if (!observers_list || observers_raw.isEmpty()) return;

    QList<QPair<QString,QString>> display = observers_raw;
    if (observers_sort_by_rank) {
        std::stable_sort(display.begin(), display.end(),
            [](const QPair<QString,QString> &a, const QPair<QString,QString> &b) {
                return rankToStrength(a.second) > rankToStrength(b.second);
            });
    }
    observers_list->clear();
    for (const auto &entry : display)
        observers_list->addItem(QString("%1 %2").arg(entry.first, entry.second));
}

QString BoardWindow::generateSGF() {
 QString sgf;

 // SGF header - xgospel2 format
 sgf += "(;FF[4]GM[1]CA[UTF-8]AP[xgospel2:2.0]\n";
 sgf += "SZ[19]\n";
 sgf += QString("KM[%1]\n").arg(komi, 0, 'f', 6); // Use q5Go's 6 decimal precision

 if (handicap > 0) {
 sgf += QString("HA[%1]").arg(handicap);
 }

 sgf += QString("PW[%1]\n").arg(white_player);
 sgf += QString("PB[%1]\n").arg(black_player);
 sgf += QString("WR[%1]\n").arg(white_rank_at_start.isEmpty() ? white_rank : white_rank_at_start);
 sgf += QString("BR[%1]\n").arg(black_rank_at_start.isEmpty() ? black_rank : black_rank_at_start);
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

 // q5Go pattern: Add handicap stones as AB[] setup properties (not moves!)
 if (handicap >= 2) {
 QList<QPair<int, int>> positions = IGSMoveParser::getHandicapPositions(handicap);
 if (!positions.isEmpty()) {
 sgf += "PL[W]AB"; // Player to move is White, Add Black setup stones
 for (const auto& pos : positions) {
 char col = 'a' + pos.first;  // getHandicapPositions: first=col(x), second=row(y)
 char row = 'a' + pos.second;
 sgf += QString("[%1%2]").arg(col).arg(row);
 }
 }
 }

 // GAME TREE APPROACH: Traverse the game tree instead of using move_history
 // This eliminates bogus/duplicate pass moves and ensures accuracy
 QList<GameNode*> move_sequence;

 // Traverse from root to end of main variation
 GameNode* node = game_root->nextMove(); // Skip root node
 while (node) {
 move_sequence.append(node);
 node = node->nextMove();
 }

 qDebug() << "📊 SGF: Game tree moves:" << move_sequence.size()
 << "(Old move_history had:" << move_history.size() << "entries)";

 // Convert game tree moves to SGF format
 for (GameNode* move_node : move_sequence) {
 int x = move_node->getX();
 int y = move_node->getY();
 StoneColor color = move_node->getColor();

 // CRITICAL FIX: Skip nodes with EMPTY/invalid color (data corruption)
 // These are bogus nodes that shouldn't be in the game tree
 if (color != BLACK_STONE && color != WHITE_STONE) {
 qDebug() << "[SGF] WARNING: Skipping node with invalid color at move" << move_node->moveNumber()
 << "coords (" << x << "," << y << ") color=" << color;
 continue;
 }

 QString move_color = (color == BLACK_STONE) ? "B" : "W";

 // q5Go pattern: Skip handicap nodes - they're already in AB[] setup
 if (x == -2) {
 qDebug() << "[SGF] Skipping handicap node (x=-2) - already in AB[] setup";
 continue;
 }

 if (x >= 0 && x < 19 && y >= 0 && y < 19) {
 // Regular move
 char col = 'a' + x;
 char row = 'a' + y;
 sgf += QString(";%1[%2%3]").arg(move_color).arg(col).arg(row);
 } else if (x == -1 && y == -1) {
 // Pass move
 sgf += QString(";%1[]").arg(move_color);
 }

 // q5Go pattern: Add comment if this node has one (kibitz)
 QString comment = move_node->getComment();
 if (!comment.isEmpty()) {
 // Escape backslashes and closing brackets in comment text
 QString escaped_comment = comment;
 escaped_comment.replace("\\", "\\\\");
 escaped_comment.replace("]", "\\]");
 sgf += QString("\nC[%1]").arg(escaped_comment);
 }
 }
 
 // Add territory information from IGS server data (q5Go style)
 qDebug() << "🔍 SGF GENERATION: territory_ownership.size() =" << territory_ownership.size();

 if (!territory_ownership.isEmpty()) {
 QStringList white_territory_coords;
 QStringList black_territory_coords;

 int white_count = 0, black_count = 0;

 // Convert IGS territory data to SGF coordinate format
 for (auto it = territory_ownership.begin(); it != territory_ownership.end(); ++it) {
 QPair<int, int> pos = it.key();
 int ownership = it.value();

 // Convert to SGF coordinates (a-s)
 if (pos.first >= 0 && pos.first < 19 && pos.second >= 0 && pos.second < 19) {
 char col = 'a' + pos.second; // pos.second = col (x coordinate)
 char row = 'a' + pos.first; // pos.first = row (y coordinate)
 QString coord = QString("%1%2").arg(col).arg(row);

 if (ownership == 4) { // White territory
 white_territory_coords.append(coord);
 white_count++;
 } else if (ownership == 5) { // Black territory
 black_territory_coords.append(coord);
 black_count++;
 }
 }
 }

 qDebug() << "🔍 SGF GENERATION: Found" << white_count << "white territory," << black_count << "black territory";
 qDebug() << "🔍 SGF GENERATION: White coords:" << white_territory_coords.join(",");
 qDebug() << "🔍 SGF GENERATION: Black coords:" << black_territory_coords.join(",");
 
 // Add white territory to SGF
 if (!white_territory_coords.isEmpty()) {
 sgf += ";TW";
 for (const QString &coord : white_territory_coords) {
 sgf += QString("[%1]").arg(coord);
 }
 }
 
 // Add black territory to SGF
 if (!black_territory_coords.isEmpty()) {
 sgf += ";TB";
 for (const QString &coord : black_territory_coords) {
 sgf += QString("[%1]").arg(coord);
 }
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
 return "W+R"; // White wins by resignation
 } else if (result.contains("White", Qt::CaseInsensitive) || result.contains("white", Qt::CaseInsensitive)) {
 return "B+R"; // Black wins by resignation 
 }
 }
 
 // Handle time loss cases
 if (result.contains("time", Qt::CaseInsensitive)) {
 if (result.contains("Black", Qt::CaseInsensitive) || result.contains("black", Qt::CaseInsensitive)) {
 return "W+T"; // White wins by time
 } else if (result.contains("White", Qt::CaseInsensitive) || result.contains("white", Qt::CaseInsensitive)) {
 return "B+T"; // Black wins by time
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
 
 DEBUG_SCORING << "Entered scoring mode for game" << observed_game_id;
 
 // Enable visual scoring mode on the board
 if (board_widget) {
 board_widget->setScoringMode(true);
 }
 
 // Update UI to show scoring mode
 updateLabels();
 
 // Show Done button so the player can accept the score (hidden in observe mode)
 if (done_button && !is_observing)
     done_button->setVisible(true);

 // Calculate initial score
 calculateScore();
}

void BoardWindow::enterScoringModeForResult() {
 // Like enterScoringMode() but always runs calculateScore() regardless of whether
 // is_scoring_mode was already set by the 3-pass detector in processMove().
 is_scoring_mode = true;
 if (board_widget) board_widget->setScoringMode(true);
 white_prisoners = white_captures;
 black_prisoners = black_captures;
 calculateScore();
}

void BoardWindow::exitScoringMode() {
 if (!is_scoring_mode) return;

 is_scoring_mode = false;
 dead_stones.clear();

 // Disable visual scoring mode on the board
 if (board_widget) {
 board_widget->setScoringMode(false);
 board_widget->setDisputedPoints(QSet<QPair<int,int>>());
 }

 DEBUG_SCORING << "Exited scoring mode for game" << observed_game_id;
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

 // In edit position mode use ScoreEngine + dead stone adjustment
 if (in_edit_position_mode) {
     // Rebuild board from widget, then remove dead stones so the flood fill
     // treats those intersections as empty and fills through them correctly
     GoBoard score_board;
     for (int bx = 0; bx < 19; bx++)
         for (int by = 0; by < 19; by++) {
             StoneColor sc = board_widget->getStoneAt(bx, by);
             if (sc != EMPTY && !dead_stones.contains(qMakePair(bx, by)))
                 score_board.placeStone(bx, by, sc);
         }

     QMap<QPair<int,int>, StoneColor> territory_map;
     QSet<QPair<int,int>> dummy_dead;
     QSet<QPair<int,int>> disputed_map;
     int b_score = 0, w_score = 0;
     ScoringMethod method = (settings->getScoringMethod() == "complex")
                            ? ScoringMethod::Complex : ScoringMethod::Simple;
     ScoreEngine::estimate(score_board, territory_map, dummy_dead, disputed_map, b_score, w_score, method);

     // Dead stone intersections count as opponent prisoners (already in territory
     // via the flood fill since we removed them from the board above, but add
     // them explicitly as prisoner counts too)
     int dead_white = 0, dead_black = 0;
     for (const auto &pos : dead_stones) {
         StoneColor dc = board_widget->getStoneAt(pos.first, pos.second);
         if (dc == WHITE_STONE) dead_white++;
         else if (dc == BLACK_STONE) dead_black++;
     }
     // dead white stones = black prisoners, dead black stones = white prisoners
     b_score += dead_white;
     w_score += dead_black;

     black_territory = b_score;
     white_territory = w_score;
     double w_total = w_score + white_captures + komi;
     double b_total = b_score + black_captures;
     double diff = b_total - w_total;
     if (diff > 0)
         game_result = QString("B+%1").arg(diff, 0, 'f', 1);
     else if (diff < 0)
         game_result = QString("W+%1").arg(-diff, 0, 'f', 1);
     else
         game_result = "Jigo";

     current_node->setTerritoryMap(territory_map);
     board_widget->setTerritoryMap(territory_map);
     board_widget->setDisputedPoints(disputed_map);
     updateLabels();
 } else {
     // Recalculate score using existing IGS-based path
     calculateScore();
 }
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
 qDebug() << QString(" [%1] %2 %3→(%4,%5) %6 %7 SUCCESS=%8")
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
 DEBUG_OBSERVATION_STATE << "🎯 BOARD STATE DUMP for game" << observed_game_id << ":";
 qDebug() << " Move count:" << move_history.size() << "Current move:" << current_move;
 qDebug() << " Scoring mode:" << (is_scoring_mode ? "YES" : "NO");
 qDebug() << " Dead stones:" << dead_stones.size();
 
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
 qDebug() << QString(" %1 (%2,%3): %4").arg(pos).arg(x).arg(y).arg(color_str);
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

 // Count dead stones as prisoners (dead white = black prisoners, vice versa)
 // dead_stones is populated by markStoneAsDead() from IGS "is removing @" messages.
 for (const auto& pos : dead_stones) {
     StoneColor stone_color = board_widget->getStoneAt(pos.first, pos.second);
     if (stone_color == WHITE_STONE)      black_prisoners++;
     else if (stone_color == BLACK_STONE) white_prisoners++;
 }

 // Calculate territory using flood fill (dead_stones treated as empty)
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

// Helper function to check if a liberty (empty point) is surrounded by opponent stones
// Returns true if the empty region containing this point only touches opponent stones
bool BoardWindow::isLibertySurroundedByOpponent(int x, int y, StoneColor opponent_color) {
 QSet<QPair<int, int>> visited;
 QStack<QPair<int, int>> stack;
 stack.push(QPair<int, int>(x, y));

 QSet<StoneColor> touching_colors;
 int board_size = 19;

 while (!stack.isEmpty()) {
 QPair<int, int> pos = stack.pop();
 if (visited.contains(pos)) continue;
 visited.insert(pos);

 int px = pos.first;
 int py = pos.second;

 // Check all 4 directions
 QPair<int, int> neighbors[4] = {
 QPair<int, int>(px-1, py), QPair<int, int>(px+1, py),
 QPair<int, int>(px, py-1), QPair<int, int>(px, py+1)
 };

 for (const QPair<int, int>& neighbor : neighbors) {
 int nx = neighbor.first;
 int ny = neighbor.second;

 if (nx < 0 || nx >= board_size || ny < 0 || ny >= board_size) continue;

 StoneColor stone = board_widget->getStoneAt(nx, ny);
 if (stone == EMPTY) {
 if (!visited.contains(neighbor)) {
 stack.push(neighbor);
 }
 } else {
 touching_colors.insert(stone);
 }
 }
 }

 // Liberty is surrounded by opponent if it only touches opponent stones
 return touching_colors.size() == 1 && touching_colors.contains(opponent_color);
}

// Algorithmically detect dead stones based on groups whose liberties are in opponent territory
// Revised algorithm: Check if ALL of a group's liberties are surrounded by opponent stones
void BoardWindow::detectDeadStones() {
 if (!board_widget) return;

 qDebug() << "*** DEAD STONE DETECTION: Analyzing board position...";

 dead_stones.clear();
 int board_size = 19;

 // Rebuild all stone groups
 rebuildAllGroups();

 // Analyze white groups
 for (const StoneGroup& group : white_groups) {
 // Check if group has any liberties
 if (group.liberties == 0) {
 qDebug() << " Found dead white group (0 liberties) with" << group.stones.size() << "stones";
 dead_stones.unite(group.stones);
 continue;
 }

 // Get all liberties for this group
 QSet<QPair<int, int>> liberty_points;
 for (const QPair<int, int>& stone_pos : group.stones) {
 int x = stone_pos.first;
 int y = stone_pos.second;

 // Check all 4 directions for liberties
 QPair<int, int> neighbors[4] = {
 QPair<int, int>(x-1, y), QPair<int, int>(x+1, y),
 QPair<int, int>(x, y-1), QPair<int, int>(x, y+1)
 };

 for (const QPair<int, int>& neighbor : neighbors) {
 int nx = neighbor.first;
 int ny = neighbor.second;

 if (nx < 0 || nx >= board_size || ny < 0 || ny >= board_size) continue;

 StoneColor neighbor_stone = board_widget->getStoneAt(nx, ny);
 if (neighbor_stone == EMPTY) {
 liberty_points.insert(neighbor);
 }
 }
 }

 // Check if ALL liberties are surrounded by black stones
 if (liberty_points.size() > 0 && liberty_points.size() <= 6) {
 bool all_liberties_in_black_territory = true;
 for (const QPair<int, int>& lib : liberty_points) {
 if (!isLibertySurroundedByOpponent(lib.first, lib.second, BLACK_STONE)) {
 all_liberties_in_black_territory = false;
 break;
 }
 }

 if (all_liberties_in_black_territory) {
 qDebug() << " Found dead white group (all" << liberty_points.size() << "liberties in black territory) with" << group.stones.size() << "stones";
 dead_stones.unite(group.stones);
 }
 }
 }

 // Analyze black groups
 for (const StoneGroup& group : black_groups) {
 // Check if group has any liberties
 if (group.liberties == 0) {
 qDebug() << " Found dead black group (0 liberties) with" << group.stones.size() << "stones";
 dead_stones.unite(group.stones);
 continue;
 }

 // Get all liberties for this group
 QSet<QPair<int, int>> liberty_points;
 for (const QPair<int, int>& stone_pos : group.stones) {
 int x = stone_pos.first;
 int y = stone_pos.second;

 // Check all 4 directions for liberties
 QPair<int, int> neighbors[4] = {
 QPair<int, int>(x-1, y), QPair<int, int>(x+1, y),
 QPair<int, int>(x, y-1), QPair<int, int>(x, y+1)
 };

 for (const QPair<int, int>& neighbor : neighbors) {
 int nx = neighbor.first;
 int ny = neighbor.second;

 if (nx < 0 || nx >= board_size || ny < 0 || ny >= board_size) continue;

 StoneColor neighbor_stone = board_widget->getStoneAt(nx, ny);
 if (neighbor_stone == EMPTY) {
 liberty_points.insert(neighbor);
 }
 }
 }

 // Check if ALL liberties are surrounded by white stones
 if (liberty_points.size() > 0 && liberty_points.size() <= 6) {
 bool all_liberties_in_white_territory = true;
 for (const QPair<int, int>& lib : liberty_points) {
 if (!isLibertySurroundedByOpponent(lib.first, lib.second, WHITE_STONE)) {
 all_liberties_in_white_territory = false;
 break;
 }
 }

 if (all_liberties_in_white_territory) {
 qDebug() << " Found dead black group (all" << liberty_points.size() << "liberties in white territory) with" << group.stones.size() << "stones";
 dead_stones.unite(group.stones);
 }
 }
 }

 qDebug() << "*** DEAD STONE DETECTION: Found" << dead_stones.size() << "total dead stones";
}

// Public wrapper for client-side territory calculation
// Following q5Go's calc_scoring_markers_complex() for observed games
void BoardWindow::calculateTerritoryMarkers() {
 // CHECK: Do we have server-side territory data from Command 22?
 if (!territory_ownership.isEmpty()) {
 qDebug() << "*** SERVER-SIDE TERRITORY DATA: Command 22 data already processed by receiveScoreEnd()";
 qDebug() << ">>> Territory markers already set - Dead stones:" << dead_stones.size()
 << "White territory:" << white_territory
 << "Black territory:" << black_territory;

 // receiveScoreEnd() has already called:
 // - board_widget->setTerritoryMap(territory_map)
 // - board_widget->setDeadStones(confirmed_dead_stones)
 // - Updated dead_stones, white_territory, black_territory
 // - Called updateLabels()
 // So we don't need to do anything here!
 } else {
 // FALLBACK: Use client-side territory detection
 qDebug() << "*** CLIENT-SIDE TERRITORY CALCULATION: No Command 22 data, running detection algorithm...";

 // STEP 1: Detect dead stones algorithmically
 detectDeadStones();

 // STEP 2: Calculate territories (which now uses the populated dead_stones set)
 calculateTerritory();
 }

 // STEP 3: Refresh display
 board_widget->update();
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
 DEBUG_MOVE_PROCESSING << "🎯 SLIDER: onMoveSliderChanged called, value:" << value << "slider_update_in_progress:" << slider_update_in_progress;
 if (slider_update_in_progress) {
 qDebug() << "🚫 SLIDER: Blocked by slider_update_in_progress flag";
 return; // Prevent recursion
 }
 qDebug() << "✅ SLIDER: Calling goToMove(" << value << ")";
 goToMove(value);
}

// --- Edit window button slots ---

void BoardWindow::onUpdateClicked() {
    if (!source_board_window) return;
    if (!source_board_window->game_root) return;

    // Snapshot the source window's current game tree from its current node
    GameNode *src = source_board_window->current_node;
    if (!src) return;

    // Walk to the root of the source tree and regenerate SGF, then reload here
    QString sgf = source_board_window->generateSGF();
    QString temp_file = QString("/tmp/xgospel2_update_%1.sgf").arg(source_board_window->observed_game_id);

    QFile f(temp_file);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Update Error", "Could not write temporary SGF file.");
        return;
    }
    QTextStream out(&f);
    out << sgf;
    f.close();

    SGFParser parser;
    QString error;
    GameNode *new_root = parser.parseFile(temp_file, error);
    if (!new_root) {
        QMessageBox::warning(this, "Update Error", QString("Failed to parse updated SGF:\n%1").arg(error));
        return;
    }

    current_node = nullptr;     // prevent dangling pointer during delete
    delete game_root;
    game_root = nullptr;        // prevent dangling pointer
    game_root = new_root;
    current_node = game_root;
    current_move_index = 0;

    goToLastMove();
    updateMoveNavigation();
}

void BoardWindow::onPassClicked() {
    if (local_play_mode) {
        // Local engine game: emit signal so the main window can forward to KataGo
        emit passRequested(observed_game_id);
        return;
    }

    // Edit window: insert a pass node after the current node
    GameNode *pass_node = current_node->addMove(-1, -1, board_widget->getNextPlayerColor());
    pass_node->setBoard(current_node->getBoard().copy());
    pass_node->setEdited(true);
    current_node = pass_node;
    current_move_index = current_node->moveNumber();

    // Alternate next player color
    StoneColor next = (board_widget->getNextPlayerColor() == BLACK_STONE) ? WHITE_STONE : BLACK_STONE;
    board_widget->setNextPlayerColor(next);

    updateMoveNavigation();
}

void BoardWindow::onScoreClicked() {
    if (!current_node) return;

    // In the edit window, white_captures/black_captures start at 0 because they
    // are not copied from the source board on loadSGF.  Recompute them by walking
    // the game tree from root to current_node and diffing consecutive board states.
    // Stones of color C present in the parent but absent in the child were captured
    // by the opponent of C.
    if (is_edit_window) {
        // Build the ancestor chain root→…→current_node
        QList<GameNode*> path;
        for (GameNode *n = current_node; n != nullptr; n = n->prevMove())
            path.prepend(n);

        int white_stones_taken = 0;  // white stones removed from board = black's prisoners
        int black_stones_taken = 0;  // black stones removed from board = white's prisoners
        for (int i = 1; i < path.size(); i++) {
            const GoBoard &prev = path[i-1]->getBoard();
            const GoBoard &curr = path[i]->getBoard();
            for (int x = 0; x < 19; x++) {
                for (int y = 0; y < 19; y++) {
                    StoneColor was = prev.getStone(x, y);
                    StoneColor now = curr.getStone(x, y);
                    if (was == WHITE_STONE && now != WHITE_STONE) white_stones_taken++;
                    if (was == BLACK_STONE && now != BLACK_STONE) black_stones_taken++;
                }
            }
        }
        // white_captures = prisoners held by white = black stones white captured
        // black_captures = prisoners held by black = white stones black captured
        white_captures = black_stones_taken;
        black_captures = white_stones_taken;
    }

    // In edit position mode the widget holds the live edited board, not current_node
    GoBoard score_board;
    if (in_edit_position_mode) {
        for (int x = 0; x < 19; x++)
            for (int y = 0; y < 19; y++) {
                StoneColor sc = board_widget->getStoneAt(x, y);
                if (sc != EMPTY) score_board.placeStone(x, y, sc);
            }
    } else {
        score_board = current_node->getBoard();
    }

    QMap<QPair<int,int>, StoneColor> territory_map;
    QSet<QPair<int,int>> dead_set;
    QSet<QPair<int,int>> disputed_set;
    int b_score = 0, w_score = 0;

    ScoringMethod method = (settings->getScoringMethod() == "complex")
                           ? ScoringMethod::Complex : ScoringMethod::Simple;
    ScoreEngine::estimate(score_board, territory_map, dead_set, disputed_set, b_score, w_score, method);

    dead_stones      = dead_set;
    black_territory  = b_score;
    white_territory  = w_score;

    // Compute final score with komi (white gets komi)
    double w_total = w_score + komi;
    double b_total = b_score;
    double diff    = b_total - w_total;
    if (diff > 0)
        game_result = QString("B+%1").arg(diff, 0, 'f', 1);
    else if (diff < 0)
        game_result = QString("W+%1").arg(-diff, 0, 'f', 1);
    else
        game_result = "Jigo";

    // Store territory on the node so displayNode() can restore it on navigation
    current_node->setTerritoryMap(territory_map);

    is_scoring_mode = true;
    // Suspend edit placement so clicks toggle dead stones instead of placing stones
    if (in_edit_position_mode)
        board_widget->setGameMode(MODE_NORMAL);
    board_widget->setScoringMode(true);
    board_widget->setTerritoryMap(territory_map);
    board_widget->setDeadStones(dead_set);
    board_widget->setDisputedPoints(disputed_set);
    updateLabels();
}

void BoardWindow::setScoringModeWithTerritory(const QMap<QPair<int,int>, StoneColor> &tmap)
{
    is_scoring_mode = true;
    // Attach the territory map to the current node so displayNode() can restore it
    // when subsequent CMD15 moves re-render the board (otherwise displayNode() clears
    // scoring mode on every non-final node, wiping the overlay immediately).
    if (current_node)
        current_node->setTerritoryMap(tmap);
    board_widget->setScoringMode(true);
    board_widget->setTerritoryMap(tmap);
}

void BoardWindow::onEditPositionClicked() {
    // Set next player color to the alternate of whoever played the current node
    StoneColor last_color = current_node->getColor();
    StoneColor next_color;
    if (last_color == BLACK_STONE)
        next_color = WHITE_STONE;
    else if (last_color == WHITE_STONE)
        next_color = BLACK_STONE;
    else
        next_color = BLACK_STONE; // Root/setup node — black plays first
    board_widget->setNextPlayerColor(next_color);
    switchToEditPositionMode();
}

void BoardWindow::onCancelEditClicked() {
    // Discard any edits by re-displaying the current node's stored board state
    board_widget->clearEditMarker();
    displayNode(current_node);
    switchToViewMode();
}

void BoardWindow::onAppendClicked() {
    // Build a new GoBoard from whatever is currently drawn on the board widget
    GoBoard new_board;
    new_board.clear();
    for (int x = 0; x < 19; x++)
        for (int y = 0; y < 19; y++) {
            StoneColor c = board_widget->getBoardState(x, y);
            if (c != EMPTY)
                new_board.placeStone(x, y, c);
        }

    // Append as a setup node (x=-1, y=-1 signals no single move — just a board state)
    GameNode *new_node = current_node->addMove(-1, -1, EMPTY);
    new_node->setBoard(new_board);
    new_node->setEdited(true);
    current_node = new_node;
    current_move_index = current_node->moveNumber();

    updateMoveNavigation();
    switchToViewMode();
}

void BoardWindow::onUndoEditClicked() {
    if (!in_edit_position_mode) {
        // Default mode: undo = step back to parent node in the game tree
        if (!current_node || current_node->isRoot()) return;
        GameNode *parent = current_node->prevMove();
        if (!parent) return;
        current_node = parent;
        current_move_index = current_node->moveNumber();
        // Restore next-player color
        StoneColor last = current_node->getColor();
        StoneColor next = (last == BLACK_STONE) ? WHITE_STONE :
                          (last == WHITE_STONE) ? BLACK_STONE : BLACK_STONE;
        board_widget->setNextPlayerColor(next);
        displayNode(current_node);
        updateMoveNavigation();
        updateGameTreeStrip();
        undo_edit_button->setEnabled(!current_node->isRoot());
        return;
    }

    // Free-placement mode: restore from snapshot stack
    if (edit_undo_stack.isEmpty())
        return;

    EditSnapshot snap = edit_undo_stack.takeLast();

    // Restore the board widget to the saved snapshot
    board_widget->clearBoard();  // wipes last_move_x/y — restore below
    for (int bx = 0; bx < 19; bx++)
        for (int by = 0; by < 19; by++) {
            StoneColor sc = snap.board.getStone(bx, by);
            if (sc != EMPTY)
                board_widget->placeMoveAt(bx, by, sc);
        }

    // Restore red circle (original game last move)
    if (current_node->getX() >= 0 && current_node->getY() >= 0)
        board_widget->setLastMove(current_node->getX(), current_node->getY());

    // Restore blue circle to where it was before the undone stone
    if (snap.edit_x >= 0 && snap.edit_y >= 0)
        board_widget->setLastEditMove(snap.edit_x, snap.edit_y);
    else
        board_widget->clearEditMarker();

    // Alternate the next-player color back
    StoneColor cur = board_widget->getNextPlayerColor();
    board_widget->setNextPlayerColor(cur == BLACK_STONE ? WHITE_STONE : BLACK_STONE);

    undo_edit_button->setEnabled(!edit_undo_stack.isEmpty());
}

// q5Go-style tree navigation: Navigate to a specific move number
void BoardWindow::goToMove(int move_number) {
 DEBUG_MOVE_PROCESSING << "🎯 goToMove: Called with move_number:" << move_number << "slider_update_in_progress:" << slider_update_in_progress;
 if (slider_update_in_progress) {
 qDebug() << "🚫 goToMove: Blocked by slider_update_in_progress flag";
 return; // Prevent recursion
 }

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
 current_move_index = target->moveNumber();

 // AUTO-FOLLOW MODE MANAGEMENT (q5Go behavior):
 // Use leaf-node test for consistency with displayNode's is_final_position check.
 bool at_end = !current_node->nextMove();

 if (at_end) {
 // Re-enable auto-follow when user returns to the end
 if (!auto_follow_mode) {
 auto_follow_mode = true;
 qDebug() << "AUTO-FOLLOW: Re-enabled (at end of game)";
 }
 } else {
 // Disable auto-follow when user navigates to an earlier position
 if (auto_follow_mode) {
 auto_follow_mode = false;
 qDebug() << "AUTO-FOLLOW: Disabled (viewing earlier position)";
 }
 }

 // Display this node's board state
 displayNode(current_node);

 // Update navigation controls
 updateMoveNavigation();
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
 goToMove(getTotalMoves());
}

void BoardWindow::updateMoveNavigation() {
 // Get total moves from game tree
 int total_moves = getTotalMoves();

 // Block slider signals to prevent recursion
 slider_update_in_progress = true;

 // Update slider
 move_slider->setMaximum(total_moves);
 move_slider->setValue(current_move_index);

 // Unblock signals
 slider_update_in_progress = false;

 // Update label
 move_number_label->setText(QString("Move: %1/%2")
 .arg(current_move_index)
 .arg(total_moves));

 // Update button states
 first_move_button->setEnabled(current_move_index > 0);
 prev_move_button->setEnabled(current_move_index > 0);
 next_move_button->setEnabled(current_move_index < total_moves);
 last_move_button->setEnabled(current_move_index < total_moves);

 // Update game tree strip if present (edit window only)
 if (game_tree_strip)
     updateGameTreeStrip();
}

void BoardWindow::updateGameTreeStrip() {
 // Walk the active variation from root and collect move colors
 // Edited nodes are flagged so the strip can show them differently
 QList<StoneColor> moves;
 QList<bool> edited_flags;
 GameNode *node = game_root;
 while (node) {
     moves.append(node->getColor());
     edited_flags.append(node->isEdited());
     node = node->nextMove();
 }
 if (game_tree_strip)
     game_tree_strip->setMoves(moves, current_move_index, edited_flags);

 // Scroll to keep the active node visible
 if (game_tree_scroll) {
     int x = current_move_index * 24; // m_node_size
     game_tree_scroll->ensureVisible(x, 0, 36, 0);
 }
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

// IGS territory marking functions (following q5Go protocol)
void BoardWindow::receiveScoreBegin() {
 DEBUG_SCORING << "*** IGS TERRITORY: Beginning score data reception for game" << observed_game_id;
 receiving_territory_data = true;
 territory_data_row = 0;
 territory_ownership.clear();
 dead_stones.clear(); // Clear old dead stone marks from previous scoring sessions
 DEBUG_SCORING << "*** IGS TERRITORY: Cleared old territory and dead stone data for game" << observed_game_id;
}

void BoardWindow::receiveScoreLine(int row, const QString &line) {
 DEBUG_SCORING << "*** IGS TERRITORY: Row" << row << "data:" << line;

 if (!receiving_territory_data) {
 DEBUG_SCORING << "*** IGS TERRITORY: ERROR - Not in territory data mode";
 return;
 }

 // Enable scoring mode when Command 22 starts (row 0)
 // This allows markStoneAsDead() to work when Command 15 messages arrive
 if (row == 0) {
 is_scoring_mode = true;
 board_widget->setScoringMode(true);
 DEBUG_SCORING << "*** IGS TERRITORY: Enabled scoring mode for game" << observed_game_id;
 }

 // Process each character in the line
 // 0=black stone, 1=white stone, 2=free, 3=neutral, 4=white territory, 5=black territory
 for (int col = 0; col < line.length() && col < 19; col++) {
 int digit = line[col].digitValue();
 QPair<int, int> pos(row, col);

 // Store ALL territory data for later analysis
 territory_ownership[pos] = digit;

 if (digit == 4 || digit == 5) {
 DEBUG_SCORING << "*** IGS TERRITORY: Position" << row << col << "=" << (digit == 4 ? "WHITE" : "BLACK") << "territory";
 } else if (digit == 0 || digit == 1) {
 DEBUG_SCORING << "*** IGS TERRITORY: Position" << row << col << "=" << (digit == 0 ? "BLACK" : "WHITE") << "stone";
 }
 }

 territory_data_row++;
}

void BoardWindow::receiveScoreEnd() {
 DEBUG_SCORING << "*** IGS TERRITORY: Score data reception completed for game" << observed_game_id;
 receiving_territory_data = false;

 // Process Command 22 territory data
 // ONLY mark empty territory points (digits 4 and 5)
 //
 // IGS Command 22 format:
 // 0 = Black stone (alive or dead - still on board)
 // 1 = White stone (alive or dead - still on board)
 // 2 = Empty / dame
 // 3 = Unknown / neutral
 // 4 = White territory (EMPTY point)
 // 5 = Black territory (EMPTY point)
 //
 // IMPORTANT: Dead stones are NOT marked in Command 22 data.
 // They are marked interactively by players after typing "done".
 // For observation mode, we simply show empty territory marking
 // as provided by the server in Command 22.
 //
 // This matches q5Go's approach - see qgo_interface.cpp:receive_score_line()

 QMap<QPair<int, int>, StoneColor> territory_map;
 int white_territory_count = 0, black_territory_count = 0;

 int dame_count = 0;

 for (auto it = territory_ownership.begin(); it != territory_ownership.end(); ++it) {
 QPair<int, int> pos = it.key();
 int digit = it.value();

 if (digit == 4) { // White territory (empty point)
 territory_map[pos] = WHITE_STONE;
 white_territory_count++;
 } else if (digit == 5) { // Black territory (empty point)
 territory_map[pos] = BLACK_STONE;
 black_territory_count++;
 } else if (digit == 2 || digit == 3) { // Dame / neutral territory (empty point)
 territory_map[pos] = EMPTY; // Use EMPTY to represent dame/neutral
 dame_count++;
 }
 // Ignore digits 0, 1 - they represent stones, not territory
 }

 DEBUG_SCORING << "*** IGS TERRITORY: Empty territory marked - White:" << white_territory_count
 << "Black:" << black_territory_count << "Dame:" << dame_count;
 DEBUG_SCORING << "*** IGS TERRITORY: Dead stones count from Command 15:" << dead_stones.size();

 // Detect dead stones from Command 22 territory data
 // Strategy: Stones located in opponent's territory are dead
 // This mirrors q5Go's approach - after scoring completes, Command 22 territory
 // data reflects final agreed-upon territories including dead stone positions
 DEBUG_SCORING << "*** IGS TERRITORY: Detecting dead stones from Command 22 data...";

 QSet<QPair<int, int>> cmd22_dead_stones;

 // Walk to the final node in the main line so dead stone detection uses the
 // actual end-of-game board, not whatever position the slider is currently at.
 const GoBoard *final_board = nullptr;
 GameNode *leaf = game_root;
 if (leaf) {
     while (leaf->nextMove()) leaf = leaf->nextMove();
     final_board = &leaf->getBoard();
 }

 for (auto it = territory_ownership.begin(); it != territory_ownership.end(); ++it) {
 QPair<int, int> pos = it.key();
 int digit = it.value();

 // Read stone color from the final game tree position, not the widget (which
 // reflects the slider-selected move and may not be the end of the game).
 StoneColor actual_stone = final_board
     ? final_board->getStone(pos.first, pos.second)
     : board_widget->getStoneAt(pos.first, pos.second);

 // Detect dead stones: black stones in white territory, or white stones in black territory
 if (digit == 4 && actual_stone == BLACK_STONE) {
 // Black stone in white territory = dead black stone
 qDebug() << " Found dead BLACK stone at" << pos.first << pos.second << "(in white territory)";
 cmd22_dead_stones.insert(pos);
 } else if (digit == 5 && actual_stone == WHITE_STONE) {
 // White stone in black territory = dead white stone
 qDebug() << " Found dead WHITE stone at" << pos.first << pos.second << "(in black territory)";
 cmd22_dead_stones.insert(pos);
 }
 }

 DEBUG_SCORING << "*** IGS TERRITORY: Detected" << cmd22_dead_stones.size() << "dead stones from Command 22 territory data";

 // Merge with any Command 15 dead stones (if we received them during active scoring)
 if (!dead_stones.isEmpty()) {
 DEBUG_SCORING << "*** IGS TERRITORY: Merging" << dead_stones.size() << "Command 15 dead stones with" << cmd22_dead_stones.size() << "Command 22 detected dead stones";
 dead_stones.unite(cmd22_dead_stones);
 } else {
 dead_stones = cmd22_dead_stones;
 }

 DEBUG_SCORING << "*** IGS TERRITORY: Final dead stone count:" << dead_stones.size();

 // Apply empty territory visualization
 board_widget->setTerritoryMap(territory_map);

 // Set dead stones (either from Command 15 or from algorithmic detection)
 board_widget->setDeadStones(dead_stones);

 // Update stored values
 white_territory = white_territory_count;
 black_territory = black_territory_count;
 updateLabels();
}

// Board click handler - routes to appropriate action based on game mode
void BoardWindow::onBoardClicked(int x, int y) {
 qDebug() << "Board clicked at" << x << "," << y << "- is_playing:" << is_playing << "is_scoring_mode:" << is_scoring_mode;

 // In local engine play, ignore clicks until the engine has finished initialising
 if (local_play_mode && !engine_ready) {
     qDebug() << "Board click ignored - engine not ready yet";
     return;
 }

 if (is_scoring_mode) {
 markStoneAsDead(x, y);
 } else if (in_edit_position_mode) {
 if (x < 0 || x >= 19 || y < 0 || y >= 19) return;
 StoneColor color = board_widget->getNextPlayerColor();
 StoneColor existing = board_widget->getBoardState(x, y);
 if (existing == color) {
     // Same color — remove it as a correction, don't alternate
     board_widget->removeStoneAt(x, y);
     board_widget->setLastEditMove(-1, -1);
 } else {
     // Snapshot current board widget state into a GoBoard, apply the move
     // with capture logic, then render the result back to the widget
     GoBoard work;
     work.clear();
     for (int bx = 0; bx < 19; bx++)
         for (int by = 0; by < 19; by++) {
             StoneColor sc = board_widget->getBoardState(bx, by);
             if (sc != EMPTY) work.placeStone(bx, by, sc);
         }
     // Save pre-move snapshot (board + current blue marker position) for per-stone undo
     edit_undo_stack.append({work, board_widget->getLastEditX(), board_widget->getLastEditY()});
     undo_edit_button->setEnabled(true);
     work.placeStone(x, y, color);

     // Remove opponent groups with no liberties
     StoneColor opponent = (color == BLACK_STONE) ? WHITE_STONE : BLACK_STONE;
     int dx[] = {-1, 1, 0, 0};
     int dy[] = {0, 0, -1, 1};
     for (int dir = 0; dir < 4; dir++) {
         int nx = x + dx[dir], ny = y + dy[dir];
         if (nx >= 0 && nx < 19 && ny >= 0 && ny < 19 &&
             work.getStone(nx, ny) == opponent &&
             countLiberties(work, nx, ny) == 0)
             removeGroup(work, nx, ny);
     }

     // Render updated GoBoard back to widget
     board_widget->clearBoard();  // clears last_move_x/y — restore it below
     for (int bx = 0; bx < 19; bx++)
         for (int by = 0; by < 19; by++) {
             StoneColor sc = work.getStone(bx, by);
             if (sc != EMPTY) board_widget->placeMoveAt(bx, by, sc);
         }
     // Restore red circle (clearBoard wiped it)
     if (current_node->getX() >= 0 && current_node->getY() >= 0)
         board_widget->setLastMove(current_node->getX(), current_node->getY());
     board_widget->setLastEditMove(x, y);
     StoneColor next = (color == BLACK_STONE) ? WHITE_STONE : BLACK_STONE;
     board_widget->setNextPlayerColor(next);
 }
 } else if (is_edit_window && !in_edit_position_mode) {
 // Default edit window behavior: append a single move to the game tree
 if (x < 0 || x >= 19 || y < 0 || y >= 19) return;
 if (board_widget->getStoneAt(x, y) != EMPTY) return;  // occupied

 // Clear scoring overlay if active
 if (is_scoring_mode) {
     is_scoring_mode = false;
     board_widget->setScoringMode(false);
     board_widget->setTerritoryMap(QMap<QPair<int,int>, StoneColor>());
     board_widget->setDeadStones(QSet<QPair<int,int>>());
 }

 StoneColor color = board_widget->getNextPlayerColor();

 // Apply move with capture logic on a copy of the current board
 GoBoard new_board = current_node->getBoard().copy();
 new_board.placeStone(x, y, color);
 StoneColor opponent = (color == BLACK_STONE) ? WHITE_STONE : BLACK_STONE;
 int dx[] = {-1, 1, 0, 0};
 int dy[] = {0, 0, -1, 1};
 for (int dir = 0; dir < 4; dir++) {
     int nx = x + dx[dir], ny = y + dy[dir];
     if (nx >= 0 && nx < 19 && ny >= 0 && ny < 19 &&
         new_board.getStone(nx, ny) == opponent &&
         countLiberties(new_board, nx, ny) == 0)
         removeGroup(new_board, nx, ny);
 }

 // Append as a new child node of current_node
 GameNode *new_node = current_node->addMove(x, y, color);
 new_node->setBoard(new_board);
 new_node->setEdited(true);
 current_node = new_node;
 current_move_index = current_node->moveNumber();

 // Render
 board_widget->clearBoard();
 for (int bx = 0; bx < 19; bx++)
     for (int by = 0; by < 19; by++) {
         StoneColor sc = new_board.getStone(bx, by);
         if (sc != EMPTY) board_widget->placeMoveAt(bx, by, sc);
     }
 board_widget->setLastMove(x, y);
 board_widget->setNextPlayerColor(opponent);

 undo_edit_button->setEnabled(true);
 updateMoveNavigation();
 updateGameTreeStrip();
 } else if (is_playing) {
 makeMove(x, y);
 } else {
 qDebug() << "Board click ignored - not playing or scoring";
 }
}

// Handle move making for actual gameplay
void BoardWindow::makeMove(int x, int y) {
 qDebug() << "Making move at" << x << "," << y << "for game" << observed_game_id;
 
 // Basic validation - assume 19x19 board for now
 if (x < 0 || y < 0 || x >= 19 || y >= 19) {
 qDebug() << "Invalid move coordinates:" << x << "," << y;
 return;
 }
 
 // Check if position is empty
 StoneColor stone_at_pos = board_widget->getStoneAt(x, y);
 if (stone_at_pos != EMPTY) {
 qDebug() << "Position occupied - cannot place stone at" << x << "," << y;
 return;
 }
 
 // Emit signal to main window to send move to server
 emit moveRequested(observed_game_id, x, y);
 
 qDebug() << "Move request sent to main window for game" << observed_game_id << "at" << x << "," << y;
}

// Set playing mode for the board
void BoardWindow::setPlayingMode(bool playing) {
 is_playing = playing;
 is_observing = !playing; // Playing and observing are mutually exclusive

 qDebug() << "Board window game" << observed_game_id << "set to" << (playing ? "PLAYING" : "OBSERVING") << "mode";

 // Update UI elements to reflect the new mode
 updateWindowTitle();
 updateCommentButtonText(); // Update say/kibitz button based on mode

 // Show/hide appropriate buttons based on mode
 if (playing) {
     resign_button->setVisible(true);  // Show Resign when playing
     pass_button->setVisible(true);    // Show Pass when playing IGS game
     close_button->setVisible(false);  // Hide Close when playing (use title bar X)
     // Done button shown only when scoring phase starts (enterScoringMode)
 } else {
     resign_button->setVisible(false); // Hide Resign when observing
     pass_button->setVisible(false);   // Hide Pass when observing
     if (done_button) done_button->setVisible(false); // Never show Done in observe mode
     close_button->setVisible(true);   // Show Close when observing
 }
}

void BoardWindow::setLocalPlayMode(bool enabled) {
    local_play_mode = enabled;
    if (enabled) {
        if (pass_button)            pass_button->setVisible(true);
        if (undo_button)            undo_button->setVisible(true);
        if (score_button)           score_button->setVisible(true);
        if (engine_console_panel)
            engine_console_panel->setVisible(true);
        // No IGS history exchange in local play — release the mv_counter guard
        // so processMove() accepts stones immediately.
        mv_counter = 0;
    }
}

void BoardWindow::setEngineReady(bool ready) {
    engine_ready = ready;
    if (ready) {
        QString t = windowTitle();
        t.replace("  [Engine starting...]", "");
        t.replace(" [Engine starting...]", "");
        setWindowTitle(t + "  [Ready]");
        if (engine_go_btn)    engine_go_btn->setEnabled(true);
        if (engine_clear_btn) engine_clear_btn->setEnabled(true);
    } else {
        if (engine_go_btn)    engine_go_btn->setEnabled(false);
    }
}

void BoardWindow::stepBackOneMove()
{
    if (!current_node || current_node->isRoot()) return;
    GameNode *parent = current_node->prevMove();
    if (!parent) return;
    current_node = parent;
    current_move_index = current_node->moveNumber();
    StoneColor last = current_node->getColor();
    StoneColor next = (last == WHITE_STONE) ? BLACK_STONE : BLACK_STONE;
    Q_UNUSED(next);
    displayNode(current_node);
    updateMoveNavigation();
    updateGameTreeStrip();
}

void BoardWindow::enableUndoButton(bool on)
{
    if (undo_button) undo_button->setEnabled(on);
}

void BoardWindow::appendEngineLog(const QString &text, bool is_sent)
{
    if (!engine_log) return;
    // is_sent (>>> commands) in cyan; responses (<<<) in light green; errors in red
    QString color = is_sent ? "#5dade2" : "#58d68d";
    if (text.startsWith("?"))
        color = "#e74c3c";
    engine_log->append(
        QString("<span style=\"color:%1; font-family:monospace;\">%2</span>")
            .arg(color)
            .arg(text.toHtmlEscaped())
    );
    // Auto-scroll to bottom
    QScrollBar *sb = engine_log->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void BoardWindow::setEngineStatus(const QString &name, bool ready)
{
    if (!engine_status_label) return;
    QString dot   = ready ? "<span style=\"color:#2ecc71\">&#9679;</span>"
                          : "<span style=\"color:#e74c3c\">&#9679;</span>";
    engine_status_label->setText(QString("%1 %2").arg(dot).arg(name.toHtmlEscaped()));
    engine_status_label->setTextFormat(Qt::RichText);
}

void BoardWindow::onEngineCmdSend()
{
    if (!engine_cmd_input) return;
    QString cmd = engine_cmd_input->text().trimmed();
    if (cmd.isEmpty()) return;
    engine_cmd_input->clear();
    appendEngineLog(">>> " + cmd, true);
    emit engineCmdRequested(cmd);
}

// Resign the current game
void BoardWindow::resignGame() {
 if (!is_playing) {
 qDebug() << "Cannot resign - not playing in game" << observed_game_id;
 return;
 }

 // Confirm resignation with user
 QMessageBox::StandardButton reply = QMessageBox::question(
 this,
 "Resign Game",
 "Are you sure you want to resign this game?",
 QMessageBox::Yes | QMessageBox::No
 );

 if (reply == QMessageBox::Yes) {
 qDebug() << "Player resigned game" << observed_game_id;
 emit resignRequested(observed_game_id);
 }
}

// Game tree navigation helpers (q5Go-style implementation)
void BoardWindow::displayNode(GameNode* node) {
 if (!node) return;

 // Get complete board state from this node
 const GoBoard& board = node->getBoard();

 // Clear the board and rebuild from stored state
 board_widget->clearBoard();

 int total_moves = getTotalMoves();

 // Check if this node has territory markers (SGF TW/TB or user Score button)
 if (node->hasTerritory()) {
 board_widget->setTerritoryMap(node->getTerritory());
 board_widget->setScoringMode(true);
 } else {
 // Use leaf-node test: immune to getTotalMoves() being wrong due to game ID recycling.
 bool is_final_position = (node->childCount() == 0) || (node->moveNumber() == total_moves);

 if (!is_final_position) {
 // Clear all scoring markers (territory, dead stones, dame) when viewing historical
 // positions — these only apply at the final board position.
 board_widget->setTerritoryMap(QMap<QPair<int, int>, StoneColor>());
 board_widget->setDeadStones(QSet<QPair<int, int>>());
 board_widget->setScoringMode(false);
 } else if (!territory_ownership.isEmpty() && is_scoring_mode) {
 // Restore territory markers at final position if we have them from live observation
 QMap<QPair<int, int>, StoneColor> territory_map;
 for (auto it = territory_ownership.begin(); it != territory_ownership.end(); ++it) {
 int digit = it.value();
 if (digit == 4) {
 territory_map[it.key()] = WHITE_STONE;
 } else if (digit == 5) {
 territory_map[it.key()] = BLACK_STONE;
 } else if (digit == 2 || digit == 3) {
 territory_map[it.key()] = EMPTY;
 }
 }
 board_widget->setTerritoryMap(territory_map);
 board_widget->setDeadStones(dead_stones);
 board_widget->setScoringMode(true);
 }
 }

 // Place all stones from the stored board state
 for (int x = 0; x < 19; x++) {
 for (int y = 0; y < 19; y++) {
 StoneColor color = board.getStone(x, y);
 if (color != EMPTY_STONE) {
 board_widget->placeMoveAt(x, y, color);
 }
 }
 }

 // Red marker always tracks the node's move coordinate (original game)
 // Blue marker is only set during active Edit Position mode via onBoardClicked
 if (node->getX() >= 0 && node->getY() >= 0)
     board_widget->setLastMove(node->getX(), node->getY());
 else
     board_widget->setLastMove(-1, -1);
 // Clear blue marker when navigating — it only applies during live editing
 if (!in_edit_position_mode)
     board_widget->clearEditMarker();

 // Update the move number label
 move_number_label->setText(QString("Move: %1/%2")
 .arg(node->moveNumber())
 .arg(total_moves));

 board_widget->update();
}

int BoardWindow::getTotalMoves() const {
 if (!game_root) return 0;
 return game_root->activeVariationMax();
}

// Helper functions for capture calculation during live move processing
int BoardWindow::countLiberties(const GoBoard& board, int x, int y) {
 StoneColor color = board.getStone(x, y);
 if (color == EMPTY_STONE) {
 return 0;
 }

 // Find all stones in this group using flood fill
 bool visited[19][19] = {{false}};
 QList<QPair<int,int>> group;
 floodFill(board, x, y, color, visited, group);

 // Count unique liberties (empty points adjacent to group)
 QSet<QPair<int,int>> liberties;
 int dx[] = {-1, 1, 0, 0};
 int dy[] = {0, 0, -1, 1};

 for (const auto& stone : group) {
 for (int dir = 0; dir < 4; dir++) {
 int nx = stone.first + dx[dir];
 int ny = stone.second + dy[dir];

 if (nx >= 0 && nx < 19 && ny >= 0 && ny < 19 &&
 board.getStone(nx, ny) == EMPTY_STONE) {
 liberties.insert(QPair<int,int>(nx, ny));
 }
 }
 }

 return liberties.size();
}

void BoardWindow::removeGroup(GoBoard& board, int x, int y) {
 StoneColor color = board.getStone(x, y);
 if (color == EMPTY_STONE) {
 return;
 }

 // Find all stones in this group
 bool visited[19][19] = {{false}};
 QList<QPair<int,int>> group;
 floodFill(board, x, y, color, visited, group);

 // Remove all stones in the group
 for (const auto& stone : group) {
 board.removeStone(stone.first, stone.second);
 }
}

void BoardWindow::floodFill(const GoBoard& board, int x, int y, StoneColor color,
 bool visited[19][19], QList<QPair<int,int>>& group) {
 if (x < 0 || x >= 19 || y < 0 || y >= 19) {
 return;
 }

 if (visited[x][y] || board.getStone(x, y) != color) {
 return;
 }

 visited[x][y] = true;
 group.append(QPair<int,int>(x, y));

 // Recursively visit all 4 neighbors
 floodFill(board, x - 1, y, color, visited, group);
 floodFill(board, x + 1, y, color, visited, group);
 floodFill(board, x, y - 1, color, visited, group);
 floodFill(board, x, y + 1, color, visited, group);
}

#include "board_window.moc"