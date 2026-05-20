#include "local_game_dialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QLabel>

LocalGameDialog::LocalGameDialog(const QList<EngineProfile> &profiles,
                                 const QString &defaultProfileId,
                                 QWidget *parent)
    : QDialog(parent)
{
    setupUI(profiles, defaultProfileId);
    setWindowTitle("Play vs Engine");
    setFixedWidth(340);
}

void LocalGameDialog::setupUI(const QList<EngineProfile> &profiles,
                               const QString &defaultProfileId)
{
    QVBoxLayout *main_layout = new QVBoxLayout(this);

    // Engine selection
    QGroupBox *engine_group = new QGroupBox("Engine", this);
    QHBoxLayout *engine_layout = new QHBoxLayout(engine_group);
    m_engine_combo = new QComboBox(engine_group);
    int default_idx = 0;
    for (int i = 0; i < profiles.size(); ++i) {
        m_engine_combo->addItem(profiles[i].name, profiles[i].id);
        if (profiles[i].id == defaultProfileId)
            default_idx = i;
    }
    if (!defaultProfileId.isEmpty())
        m_engine_combo->setCurrentIndex(default_idx);
    engine_layout->addWidget(m_engine_combo);
    main_layout->addWidget(engine_group);

    // Color selection
    QGroupBox *color_group = new QGroupBox("Your Color", this);
    QHBoxLayout *color_layout = new QHBoxLayout(color_group);
    m_black_btn  = new QRadioButton("Black",  color_group);
    m_white_btn  = new QRadioButton("White",  color_group);
    m_random_btn = new QRadioButton("Random", color_group);
    m_black_btn->setChecked(true);
    color_layout->addWidget(m_black_btn);
    color_layout->addWidget(m_white_btn);
    color_layout->addWidget(m_random_btn);
    main_layout->addWidget(color_group);

    // Game parameters
    QGroupBox *params_group = new QGroupBox("Game Parameters", this);
    QFormLayout *form = new QFormLayout(params_group);

    m_handicap_spin = new QSpinBox(params_group);
    m_handicap_spin->setRange(0, 9);
    m_handicap_spin->setValue(0);
    form->addRow("Handicap:", m_handicap_spin);

    m_komi_spin = new QDoubleSpinBox(params_group);
    m_komi_spin->setRange(0.0, 9.5);
    m_komi_spin->setSingleStep(0.5);
    m_komi_spin->setValue(6.5);
    m_komi_spin->setDecimals(1);
    form->addRow("Komi:", m_komi_spin);

    m_boardsize_spin = new QSpinBox(params_group);
    m_boardsize_spin->setRange(9, 19);
    m_boardsize_spin->setSingleStep(4);
    m_boardsize_spin->setValue(19);
    form->addRow("Board size:", m_boardsize_spin);

    m_time_spin = new QSpinBox(params_group);
    m_time_spin->setRange(1, 300);
    m_time_spin->setSingleStep(5);
    m_time_spin->setValue(5);
    m_time_spin->setSuffix(" s/move");
    form->addRow("Time per move:", m_time_spin);

    main_layout->addWidget(params_group);

    // Buttons
    QHBoxLayout *btn_row = new QHBoxLayout();
    btn_row->addStretch();
    m_ok_btn     = new QPushButton("Start Game", this);
    m_cancel_btn = new QPushButton("Cancel",     this);
    m_ok_btn->setDefault(true);
    btn_row->addWidget(m_ok_btn);
    btn_row->addWidget(m_cancel_btn);
    main_layout->addLayout(btn_row);

    connect(m_handicap_spin, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &LocalGameDialog::onHandicapChanged);
    connect(m_ok_btn,     &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancel_btn, &QPushButton::clicked, this, &QDialog::reject);
}

void LocalGameDialog::onHandicapChanged(int value)
{
    if (value > 0)
        m_komi_spin->setValue(0.5);
    else
        m_komi_spin->setValue(6.5);
}

QString LocalGameDialog::selectedProfileId() const
{
    return m_engine_combo->currentData().toString();
}

StoneColor LocalGameDialog::userColor() const
{
    if (m_black_btn->isChecked()) return BLACK_STONE;
    if (m_white_btn->isChecked()) return WHITE_STONE;
    return EMPTY;
}

double LocalGameDialog::komi() const      { return m_komi_spin->value(); }
int    LocalGameDialog::handicap() const  { return m_handicap_spin->value(); }
int    LocalGameDialog::boardsize() const { return m_boardsize_spin->value(); }
int    LocalGameDialog::timePerMove() const { return m_time_spin->value(); }

#include "local_game_dialog.moc"
