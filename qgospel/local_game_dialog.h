#ifndef LOCAL_GAME_DIALOG_H
#define LOCAL_GAME_DIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QList>
#include <QRadioButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QString>

#include "game_types.h"
#include "engine_manager.h"

// ---------------------------------------------------------------------------
// LocalGameDialog — presented when user chooses "Play vs Engine…"
// Collects: engine selection, color choice, komi, handicap, board size, time.
// ---------------------------------------------------------------------------

class LocalGameDialog : public QDialog {
    Q_OBJECT

public:
    explicit LocalGameDialog(const QList<EngineProfile> &profiles,
                             const QString &defaultProfileId = QString(),
                             QWidget *parent = nullptr);

    QString    selectedProfileId() const;
    StoneColor userColor()         const;
    double     komi()              const;
    int        handicap()          const;
    int        boardsize()         const;
    int        timePerMove()       const;

private slots:
    void onHandicapChanged(int value);

private:
    void setupUI(const QList<EngineProfile> &profiles,
                 const QString &defaultProfileId);

    QComboBox      *m_engine_combo;
    QRadioButton   *m_black_btn;
    QRadioButton   *m_white_btn;
    QRadioButton   *m_random_btn;
    QDoubleSpinBox *m_komi_spin;
    QSpinBox       *m_handicap_spin;
    QSpinBox       *m_boardsize_spin;
    QSpinBox       *m_time_spin;
    QPushButton    *m_ok_btn;
    QPushButton    *m_cancel_btn;
};

#endif // LOCAL_GAME_DIALOG_H
