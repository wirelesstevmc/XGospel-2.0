#include "game_selection_dock.h"

#include <QScrollBar>

GameSelectionDock::GameSelectionDock(QWidget *parent)
    : QDockWidget("Games", parent)
{
    setAllowedAreas(Qt::AllDockWidgetAreas);
    setFeatures(QDockWidget::DockWidgetMovable |
                QDockWidget::DockWidgetFloatable);

    // Container widget inside the scroll area
    m_container = new QWidget;
    m_layout = new QVBoxLayout(m_container);
    m_layout->setSpacing(2);
    m_layout->setContentsMargins(3, 3, 3, 3);
    m_layout->addStretch();   // pushes buttons to the top

    m_scroll_area = new QScrollArea;
    m_scroll_area->setWidget(m_container);
    m_scroll_area->setWidgetResizable(true);
    m_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    // Suppress the scroll area's own frame — the dock provides the border
    m_scroll_area->setFrameShape(QFrame::NoFrame);

    setWidget(m_scroll_area);

    // Default width hint — matches the button design (~220px)
    setMinimumWidth(160);
    resize(220, height());
}

void GameSelectionDock::addGame(int game_id,
                                const QString &black, const QString &b_rank,
                                const QString &white, const QString &w_rank)
{
    if (findButton(game_id)) return;   // already present — ignore duplicate

    auto *btn = new GameButtonWidget(game_id, black, b_rank, white, w_rank, m_container);
    connect(btn, &GameButtonWidget::clicked, this, &GameSelectionDock::gameSelected);

    // Insert before the trailing stretch (second-to-last position)
    const int insert_pos = std::max(0, m_layout->count() - 1);
    m_layout->insertWidget(insert_pos, btn);
    m_buttons.append(btn);

    // Auto-activate the first game added
    if (m_buttons.size() == 1)
        setActiveGame(game_id);
}

void GameSelectionDock::removeGame(int game_id)
{
    GameButtonWidget *btn = findButton(game_id);
    if (!btn) return;

    m_buttons.removeOne(btn);
    m_layout->removeWidget(btn);
    btn->deleteLater();

    if (m_active_game_id == game_id) {
        m_active_game_id = -1;
        // Auto-select the first remaining game if any
        if (!m_buttons.isEmpty())
            setActiveGame(m_buttons.first()->gameId());
    }
}

void GameSelectionDock::setActiveGame(int game_id)
{
    m_active_game_id = game_id;
    for (GameButtonWidget *btn : m_buttons)
        btn->setActive(btn->gameId() == game_id);
}

void GameSelectionDock::updateGameRanks(int game_id, const QString &b_rank, const QString &w_rank)
{
    if (GameButtonWidget *btn = findButton(game_id))
        btn->updateRanks(b_rank, w_rank);
}

void GameSelectionDock::updateHoverPixmap(int game_id, const QPixmap &px)
{
    if (GameButtonWidget *btn = findButton(game_id))
        btn->updateBoardPixmap(px);
}

bool GameSelectionDock::hasGame(int game_id) const
{
    return findButton(game_id) != nullptr;
}

GameButtonWidget *GameSelectionDock::findButton(int game_id) const
{
    for (GameButtonWidget *btn : m_buttons)
        if (btn->gameId() == game_id)
            return btn;
    return nullptr;
}

#include "game_selection_dock.moc"
