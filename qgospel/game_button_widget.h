#ifndef GAME_BUTTON_WIDGET_H
#define GAME_BUTTON_WIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QTimer>
#include <QString>
#include "stone_renderer.h"

// ---------------------------------------------------------------------------
// GameButtonWidget — one entry in the GameSelectionDock.
//
// Displays black player / white player as a raised 3D button with stone-dot
// indicators.  Hover shows a frameless board-position popup.  The active game
// gets a blue border.  Game ID is not displayed but is carried for signal use.
// ---------------------------------------------------------------------------
class GameButtonWidget : public QWidget {
    Q_OBJECT

public:
    GameButtonWidget(int game_id,
                     const QString &black,  const QString &b_rank,
                     const QString &white,  const QString &w_rank,
                     QWidget *parent = nullptr);
    ~GameButtonWidget();

    int     gameId()   const { return m_game_id; }
    bool    isActive() const { return m_active; }

    void setActive(bool active);
    void updateRanks(const QString &b_rank, const QString &w_rank);
    // Called by FixedXGospelWindow whenever this game's board state changes.
    void updateBoardPixmap(const QPixmap &px);

signals:
    void clicked(int game_id);

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEvent *event)      override;
    void leaveEvent(QEvent *event)      override;
    void mousePressEvent(QMouseEvent *event)   override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void showPopup();

private:
    void hidePopup();

    int      m_game_id;
    QString  m_black,  m_b_rank;
    QString  m_white,  m_w_rank;
    bool     m_active       = false;
    bool     m_pressed      = false;

    QPixmap  m_board_pixmap;    // cached hover board render — updated on each move
    QPixmap  m_black_stone;     // board-quality black stone icon
    QPixmap  m_white_stone;     // board-quality white stone icon
    QTimer  *m_hover_timer;
    QWidget *m_popup        = nullptr;
};

#endif // GAME_BUTTON_WIDGET_H
