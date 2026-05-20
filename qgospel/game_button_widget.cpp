#include "game_button_widget.h"

#include <QApplication>
#include <QFontMetrics>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QStyleOption>
#include <algorithm>

GameButtonWidget::GameButtonWidget(int game_id,
                                   const QString &black, const QString &b_rank,
                                   const QString &white, const QString &w_rank,
                                   QWidget *parent)
    : QWidget(parent)
    , m_game_id(game_id)
    , m_black(black),  m_b_rank(b_rank)
    , m_white(white),  m_w_rank(w_rank)
{
    // Stone diameter: match font cap-height so icons feel in proportion
    const QFontMetrics fm(QApplication::font());
    const int stone_d = fm.capHeight() > 0 ? fm.capHeight() + 6 : fm.height();

    StoneRenderer sr;
    sr.generateStones(stone_d);
    m_black_stone = sr.getBlackStone();
    m_white_stone = sr.getWhiteStone(0);

    // Row height must comfortably hold the stone icon plus a little padding
    const int row_h = qMax(fm.height() + 8, stone_d + 6);
    setFixedHeight(row_h * 2 + 10);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMouseTracking(true);

    m_hover_timer = new QTimer(this);
    m_hover_timer->setSingleShot(true);
    m_hover_timer->setInterval(400);
    connect(m_hover_timer, &QTimer::timeout, this, &GameButtonWidget::showPopup);
}

GameButtonWidget::~GameButtonWidget()
{
    hidePopup();
}

void GameButtonWidget::setActive(bool active)
{
    if (m_active == active) return;
    m_active = active;
    update();
}

void GameButtonWidget::updateRanks(const QString &b_rank, const QString &w_rank)
{
    m_b_rank = b_rank;
    m_w_rank = w_rank;
    update();
}

void GameButtonWidget::updateBoardPixmap(const QPixmap &px)
{
    m_board_pixmap = px;
    // If popup is already showing, refresh it in place
    if (m_popup) {
        if (QLabel *lbl = m_popup->findChild<QLabel*>())
            lbl->setPixmap(m_board_pixmap);
    }
}

void GameButtonWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    const QRect r = rect();

    // 3D raised button face via Qt style primitive
    QStyleOption opt;
    opt.initFrom(this);
    opt.rect  = r;
    opt.state = QStyle::State_Enabled |
                (m_pressed ? QStyle::State_Sunken : QStyle::State_Raised);
    style()->drawPrimitive(QStyle::PE_PanelButtonCommand, &opt, &p, this);

    // Active blue border — 2px inset from widget edge
    if (m_active) {
        p.setPen(QPen(QColor(0, 100, 220), 2));
        p.setBrush(Qt::NoBrush);
        p.drawRect(r.adjusted(2, 2, -3, -3));
    }

    // Layout constants
    const int border   = 5;
    const int stone_w  = m_black_stone.width();
    const int stone_h  = m_black_stone.height();
    const int text_x   = border + stone_w + 6;   // left edge of name text
    const int inner_w  = r.width() - text_x - border;
    const int half_h   = (r.height() - 2 * border) / 2;

    QFont f = QApplication::font();
    QFontMetrics fm(f);
    QFont bold_f = f;
    bold_f.setBold(true);

    // Board-quality stone icons, vertically centred in each row
    const int b_stone_y = border + (half_h - stone_h) / 2;
    const int w_stone_y = border + half_h + (half_h - stone_h) / 2;
    p.drawPixmap(border, b_stone_y, m_black_stone);
    p.drawPixmap(border, w_stone_y, m_white_stone);

    // Black player row (top)
    const int b_rw = fm.horizontalAdvance(m_b_rank) + 4;
    p.setFont(bold_f);
    p.setPen(Qt::black);
    p.drawText(QRect(text_x, border, inner_w - b_rw, half_h),
               Qt::AlignLeft | Qt::AlignVCenter,
               fm.elidedText(m_black, Qt::ElideRight, inner_w - b_rw));
    p.setFont(f);
    p.setPen(Qt::white);
    p.drawText(QRect(text_x + inner_w - b_rw, border, b_rw, half_h),
               Qt::AlignRight | Qt::AlignVCenter, m_b_rank);

    // White player row (bottom)
    const int w_rw = fm.horizontalAdvance(m_w_rank) + 4;
    p.setFont(bold_f);
    p.setPen(Qt::black);
    p.drawText(QRect(text_x, border + half_h, inner_w - w_rw, half_h),
               Qt::AlignLeft | Qt::AlignVCenter,
               fm.elidedText(m_white, Qt::ElideRight, inner_w - w_rw));
    p.setFont(f);
    p.setPen(Qt::white);
    p.drawText(QRect(text_x + inner_w - w_rw, border + half_h, w_rw, half_h),
               Qt::AlignRight | Qt::AlignVCenter, m_w_rank);
}

void GameButtonWidget::enterEvent(QEvent *)
{
    m_hover_timer->start();
}

void GameButtonWidget::leaveEvent(QEvent *)
{
    m_hover_timer->stop();
    hidePopup();
}

void GameButtonWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        update();
    }
}

void GameButtonWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_pressed) {
        m_pressed = false;
        update();
        if (rect().contains(event->pos()))
            emit clicked(m_game_id);
    }
}

void GameButtonWidget::showPopup()
{
    if (m_popup || m_board_pixmap.isNull()) return;

    const int sz  = m_board_pixmap.width();
    const int pad = 2;   // dark border thickness

    m_popup = new QWidget(nullptr,
                          Qt::ToolTip |
                          Qt::FramelessWindowHint |
                          Qt::WindowStaysOnTopHint);
    m_popup->setAttribute(Qt::WA_ShowWithoutActivating);
    m_popup->setAttribute(Qt::WA_DeleteOnClose);
    m_popup->setFixedSize(sz + 2 * pad, sz + 2 * pad);
    m_popup->setStyleSheet("background-color: #222;");

    QLabel *lbl = new QLabel(m_popup);
    lbl->setGeometry(pad, pad, sz, sz);
    lbl->setPixmap(m_board_pixmap);

    // Position to the right of the button, vertically centred.
    // Fall back to the left side if too close to the screen edge.
    QPoint gp = mapToGlobal(QPoint(width() + 4, height() / 2 - (sz + 2 * pad) / 2));
    QRect  sr = QApplication::primaryScreen()->availableGeometry();

    if (gp.x() + sz + 2 * pad > sr.right())
        gp.setX(mapToGlobal(QPoint(-(sz + 2 * pad + 4), 0)).x());
    gp.setY(std::max(sr.top(),
             std::min(gp.y(), sr.bottom() - sz - 2 * pad)));

    m_popup->move(gp);
    m_popup->show();
}

void GameButtonWidget::hidePopup()
{
    if (m_popup) {
        m_popup->close();
        m_popup = nullptr;
    }
}

#include "game_button_widget.moc"
