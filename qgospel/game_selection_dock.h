#ifndef GAME_SELECTION_DOCK_H
#define GAME_SELECTION_DOCK_H

#include <QDockWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QList>
#include <QPixmap>
#include <QString>

#include "game_button_widget.h"

// ---------------------------------------------------------------------------
// GameSelectionDock — left-side dockable pane listing all observed games.
//
// Contains a scrollable column of GameButtonWidget items.  One button is
// marked active (blue border) at a time.  The dock is hidden in non-docked
// mode and shown when use_docked_game_pane = true.
// ---------------------------------------------------------------------------
class GameSelectionDock : public QDockWidget {
    Q_OBJECT

public:
    explicit GameSelectionDock(QWidget *parent = nullptr);

    // Add a button for a newly observed game.
    void addGame(int game_id,
                 const QString &black, const QString &b_rank,
                 const QString &white, const QString &w_rank);

    // Remove the button for a game that has been unobserved / closed.
    void removeGame(int game_id);

    // Mark one button as active (blue border), clear all others.
    void setActiveGame(int game_id);

    // Update ranks on an existing button (called when who response arrives after addGame).
    void updateGameRanks(int game_id, const QString &b_rank, const QString &w_rank);

    // Push a fresh board pixmap to the correct button's hover popup.
    void updateHoverPixmap(int game_id, const QPixmap &px);

    int  activeGameId() const { return m_active_game_id; }
    bool hasGame(int game_id) const;

signals:
    // Emitted when the user clicks a game button.
    void gameSelected(int game_id);

private:
    GameButtonWidget *findButton(int game_id) const;

    QScrollArea          *m_scroll_area;
    QWidget              *m_container;
    QVBoxLayout          *m_layout;
    QList<GameButtonWidget*> m_buttons;
    int                   m_active_game_id = -1;
};

#endif // GAME_SELECTION_DOCK_H
