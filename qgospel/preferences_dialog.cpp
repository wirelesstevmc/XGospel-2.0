#include "preferences_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QDebug>

PreferencesDialog::PreferencesDialog(EngineManager *engine_manager, QWidget *parent)
    : QDialog(parent), m_current_host_index(-1), m_engines_widget(nullptr)
{
    setWindowTitle("xgospel2 Preferences");
    setMinimumSize(600, 400);

    setupUI(engine_manager);
    loadHosts();
}

PreferencesDialog::~PreferencesDialog() {
}

void PreferencesDialog::setupUI(EngineManager *engine_manager) {
    QVBoxLayout *main_layout = new QVBoxLayout(this);

    // Create tab widget
    m_tab_widget = new QTabWidget();
    main_layout->addWidget(m_tab_widget);

    // ========== Server Connections Tab ==========
    QWidget *server_tab = new QWidget();
    QVBoxLayout *server_tab_layout = new QVBoxLayout(server_tab);

    // Host/Server section
    QGroupBox *server_group = new QGroupBox("Server Connections");
    QHBoxLayout *server_layout = new QHBoxLayout();

    // Left side: Host list
    QVBoxLayout *list_layout = new QVBoxLayout();
    list_layout->addWidget(new QLabel("Accounts:"));

    m_host_list = new QListWidget();
    list_layout->addWidget(m_host_list);

    QHBoxLayout *list_buttons = new QHBoxLayout();
    m_new_btn = new QPushButton("New");
    m_delete_btn = new QPushButton("Delete");
    list_buttons->addWidget(m_new_btn);
    list_buttons->addWidget(m_delete_btn);
    list_layout->addLayout(list_buttons);

    server_layout->addLayout(list_layout);

    // Right side: Host details
    QFormLayout *form_layout = new QFormLayout();

    m_title_edit = new QLineEdit();
    m_host_edit = new QLineEdit();
    m_port_edit = new QLineEdit();
    m_port_edit->setText("7777");
    m_login_edit = new QLineEdit();
    m_password_edit = new QLineEdit();
    m_password_edit->setEchoMode(QLineEdit::Password);

    m_codec_combo = new QComboBox();
    m_codec_combo->addItems({"US-ASCII", "ISO-8859-1", "SJIS", "UTF-8"});

    form_layout->addRow("Title:", m_title_edit);
    form_layout->addRow("Host:", m_host_edit);
    form_layout->addRow("Port:", m_port_edit);
    form_layout->addRow("Login:", m_login_edit);
    form_layout->addRow("Password:", m_password_edit);
    form_layout->addRow("Codec:", m_codec_combo);

    server_layout->addLayout(form_layout);
    server_group->setLayout(server_layout);
    server_tab_layout->addWidget(server_group);

    // Save Game Directory section
    QGroupBox *savegame_group = new QGroupBox("Game Files");
    QHBoxLayout *savegame_layout = new QHBoxLayout();

    QLabel *savegame_label = new QLabel("Save Directory:");
    m_savegame_dir_edit = new QLineEdit();
    m_savegame_dir_edit->setPlaceholderText("$HOME/Claude_Projects/64-bit/game_files");

    m_browse_btn = new QPushButton("Browse...");
    m_browse_btn->setMaximumWidth(100);

    savegame_layout->addWidget(savegame_label);
    savegame_layout->addWidget(m_savegame_dir_edit);
    savegame_layout->addWidget(m_browse_btn);

    savegame_group->setLayout(savegame_layout);
    server_tab_layout->addWidget(savegame_group);

    // Load current save directory
    m_savegame_dir_edit->setText(settings->readEntry("SAVEGAMEDIR", "$HOME/Claude_Projects/64-bit/game_files"));

    server_tab_layout->addStretch();
    m_tab_widget->addTab(server_tab, "Server Connections");

    // ========== Application Settings Tab ==========
    QWidget *app_settings_tab = new QWidget();
    QVBoxLayout *app_tab_layout = new QVBoxLayout(app_settings_tab);

    QGroupBox *console_group = new QGroupBox("Console Settings");
    QFormLayout *console_form = new QFormLayout();

    m_console_buffer_spin = new QSpinBox();
    m_console_buffer_spin->setRange(0, 100000);
    m_console_buffer_spin->setSingleStep(1000);
    m_console_buffer_spin->setValue(settings->getConsoleBufferSize());
    m_console_buffer_spin->setSpecialValueText("Unlimited");
    m_console_buffer_spin->setSuffix(" lines");
    console_form->addRow("Console Buffer Size:", m_console_buffer_spin);

    QLabel *console_help = new QLabel("(Maximum number of lines in console. 0 = unlimited. Default: 30000)");
    console_help->setWordWrap(true);
    console_help->setStyleSheet("color: gray; font-size: 9pt;");
    console_form->addRow("", console_help);

    // Console dump directory — shown here so it is always visible
    m_consoledump_dir_edit = new QLineEdit();
    m_consoledump_dir_edit->setPlaceholderText("$HOME/Claude_Projects/64-bit/xgospel2_console_dumps");
    m_consoledump_dir_edit->setText(settings->readEntry("CONSOLEDUMPDIR",
        "$HOME/Claude_Projects/64-bit/xgospel2_console_dumps"));
    m_consoledump_browse_btn = new QPushButton("Browse...");
    m_consoledump_browse_btn->setMaximumWidth(100);
    QHBoxLayout *consoledump_row = new QHBoxLayout();
    consoledump_row->addWidget(m_consoledump_dir_edit);
    consoledump_row->addWidget(m_consoledump_browse_btn);
    console_form->addRow("Console Dump Folder:", consoledump_row);

    console_group->setLayout(console_form);
    app_tab_layout->addWidget(console_group);

    QGroupBox *games_group = new QGroupBox("Games Window");
    QFormLayout *games_form = new QFormLayout();

    m_games_refresh_spin = new QSpinBox();
    m_games_refresh_spin->setRange(0, 600);
    m_games_refresh_spin->setSingleStep(10);
    m_games_refresh_spin->setValue(settings->getGamesWindowRefreshInterval());
    m_games_refresh_spin->setSpecialValueText("Disabled");
    m_games_refresh_spin->setSuffix(" seconds");
    games_form->addRow("Games Auto-Refresh Interval:", m_games_refresh_spin);

    QLabel *games_help = new QLabel("(Automatic refresh interval for games list. 0 = disabled. Default: 90 seconds)");
    games_help->setWordWrap(true);
    games_help->setStyleSheet("color: gray; font-size: 9pt;");
    games_form->addRow("", games_help);

    m_players_refresh_spin = new QSpinBox();
    m_players_refresh_spin->setRange(0, 600);
    m_players_refresh_spin->setSingleStep(10);
    m_players_refresh_spin->setValue(settings->getPlayersWindowRefreshInterval());
    m_players_refresh_spin->setSpecialValueText("Disabled");
    m_players_refresh_spin->setSuffix(" seconds");
    games_form->addRow("Players Auto-Refresh Interval:", m_players_refresh_spin);

    QLabel *players_help = new QLabel("(Automatic refresh interval for players list. 0 = disabled. Default: 120 seconds)");
    players_help->setWordWrap(true);
    players_help->setStyleSheet("color: gray; font-size: 9pt;");
    games_form->addRow("", players_help);

    m_use_focus_colors_check = new QCheckBox("Use focus-based highlight colors");
    m_use_focus_colors_check->setChecked(settings->getUseFocusColors());
    games_form->addRow("", m_use_focus_colors_check);

    QLabel *focus_help = new QLabel("(When enabled, observed games change to lighter color when window has focus. Disable for q5Go-style behavior.)");
    focus_help->setWordWrap(true);
    focus_help->setStyleSheet("color: gray; font-size: 9pt;");
    games_form->addRow("", focus_help);

    games_group->setLayout(games_form);
    app_tab_layout->addWidget(games_group);

    // ---- Scoring group ----
    QGroupBox *scoring_group = new QGroupBox("Scoring");
    QFormLayout *scoring_form = new QFormLayout();

    m_scoring_method_combo = new QComboBox();
    m_scoring_method_combo->addItem("Simple  (flood-fill, fast)", "simple");
    m_scoring_method_combo->addItem("Complex  (false-eye + seki detection)", "complex");

    QString cur_method = settings->getScoringMethod();
    int method_idx = m_scoring_method_combo->findData(cur_method);
    if (method_idx >= 0) m_scoring_method_combo->setCurrentIndex(method_idx);

    scoring_form->addRow("Score estimation method:", m_scoring_method_combo);

    QLabel *scoring_help = new QLabel(
        "Simple: fast flood-fill, accurate for most positions.  "
        "Complex: adds false-eye detection and seki exclusion; marks disputed points with a grey square.  "
        "Both methods can be compared by switching and re-scoring the same position.");
    scoring_help->setWordWrap(true);
    scoring_help->setStyleSheet("color: gray; font-size: 9pt;");
    scoring_form->addRow("", scoring_help);

    scoring_group->setLayout(scoring_form);
    app_tab_layout->addWidget(scoring_group);

    // ---- Game Pane group ----
    QGroupBox *game_pane_group = new QGroupBox("Game Pane");
    QFormLayout *game_pane_form = new QFormLayout();

    m_docked_game_pane_check = new QCheckBox("Use docked game selection pane");
    m_docked_game_pane_check->setChecked(settings->getUseDockedGamePane());
    game_pane_form->addRow("", m_docked_game_pane_check);

    QLabel *docked_help = new QLabel(
        "When enabled, all observed games share a single board window with a dockable "
        "selection pane on the left.  Click a game button to switch the board view.  "
        "Hover over a button to preview that game's current position.  "
        "Requires restart to take effect.");
    docked_help->setWordWrap(true);
    docked_help->setStyleSheet("color: gray; font-size: 9pt;");
    game_pane_form->addRow("", docked_help);

    m_hover_board_size_spin = new QSpinBox();
    m_hover_board_size_spin->setRange(100, 600);
    m_hover_board_size_spin->setSingleStep(25);
    m_hover_board_size_spin->setValue(settings->getHoverBoardSize());
    m_hover_board_size_spin->setSuffix(" px");
    game_pane_form->addRow("Board preview size:", m_hover_board_size_spin);

    QLabel *preview_help = new QLabel("(Size of the board popup shown on hover. Default: 200 px)");
    preview_help->setWordWrap(true);
    preview_help->setStyleSheet("color: gray; font-size: 9pt;");
    game_pane_form->addRow("", preview_help);

    game_pane_group->setLayout(game_pane_form);
    app_tab_layout->addWidget(game_pane_group);

    // Shout window group
    QGroupBox *shout_group = new QGroupBox("Shout Window");
    QFormLayout *shout_form = new QFormLayout();

    m_auto_launch_shout_check = new QCheckBox("Auto-launch Shout window on login");
    m_auto_launch_shout_check->setChecked(settings->getAutoLaunchShoutWindow());
    shout_form->addRow("", m_auto_launch_shout_check);

    QLabel *shout_help = new QLabel(
        "When enabled, the Shout window opens minimized automatically on login.  "
        "Shout messages are always cached from login regardless of this setting — "
        "open the window via Windows > Show Shouts to read them.");
    shout_help->setWordWrap(true);
    shout_help->setStyleSheet("color: gray; font-size: 9pt;");
    shout_form->addRow("", shout_help);

    shout_group->setLayout(shout_form);
    app_tab_layout->addWidget(shout_group);

    app_tab_layout->addStretch();
    m_tab_widget->addTab(app_settings_tab, "Application Settings");

    // ========== Bot Settings Tab ==========
    QWidget *bot_tab = new QWidget();
    QVBoxLayout *bot_tab_layout = new QVBoxLayout(bot_tab);

    QGroupBox *blacklist_group = new QGroupBox("Opponent Blacklist");
    QFormLayout *blacklist_form = new QFormLayout();

    m_bot_blacklist_edit = new QLineEdit();
    m_bot_blacklist_edit->setPlaceholderText("e.g. feeder,spammer,cheater");
    m_bot_blacklist_edit->setText(settings->getBotBlacklist().join(", "));
    blacklist_form->addRow("Blocked players:", m_bot_blacklist_edit);

    QLabel *blacklist_help = new QLabel(
        "Comma-separated IGS usernames. Match requests from these players will be\n"
        "automatically declined. Names are case-insensitive.");
    blacklist_help->setWordWrap(true);
    blacklist_help->setStyleSheet("color: gray; font-size: 9pt;");
    blacklist_form->addRow("", blacklist_help);

    blacklist_group->setLayout(blacklist_form);
    bot_tab_layout->addWidget(blacklist_group);

    // ---- Greylist group ----
    QGroupBox *greylist_group = new QGroupBox("Opponent Greylist (Handicap Limits)");
    QVBoxLayout *greylist_layout = new QVBoxLayout();

    QLabel *greylist_help = new QLabel(
        "Per-opponent handicap limit. Positive value: decline if offered handicap exceeds limit (weaker opponent asking too many stones). "
        "Negative value: decline if opponent offers an even or reverse-handicap game (stronger opponent offering unfavorable conditions). "
        "Example: -1 declines any even or handicap game from a consistently stronger opponent. Names are case-insensitive.");
    greylist_help->setWordWrap(true);
    greylist_help->setStyleSheet("color: gray; font-size: 9pt;");
    greylist_layout->addWidget(greylist_help);

    m_greylist_table = new QTableWidget(0, 3);
    m_greylist_table->setHorizontalHeaderLabels({"Player", "HC Limit", "Custom Tell"});
    m_greylist_table->horizontalHeader()->setStretchLastSection(true);
    m_greylist_table->horizontalHeader()->resizeSection(0, 120);
    m_greylist_table->horizontalHeader()->resizeSection(1, 60);
    m_greylist_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_greylist_table->setMinimumHeight(120);

    // Populate from saved settings
    for (const auto &e : settings->getBotGreylist()) {
        int row = m_greylist_table->rowCount();
        m_greylist_table->insertRow(row);
        m_greylist_table->setItem(row, 0, new QTableWidgetItem(e.name));
        m_greylist_table->setItem(row, 1, new QTableWidgetItem(QString::number(e.max_hc)));
        m_greylist_table->setItem(row, 2, new QTableWidgetItem(e.tell));
    }
    greylist_layout->addWidget(m_greylist_table);

    QHBoxLayout *greylist_btn_layout = new QHBoxLayout();
    QPushButton *greylist_add_btn    = new QPushButton("Add Row");
    QPushButton *greylist_remove_btn = new QPushButton("Remove Row");
    greylist_btn_layout->addWidget(greylist_add_btn);
    greylist_btn_layout->addWidget(greylist_remove_btn);
    greylist_btn_layout->addStretch();
    greylist_layout->addLayout(greylist_btn_layout);

    connect(greylist_add_btn,    &QPushButton::clicked, this, &PreferencesDialog::onGreylistAddRow);
    connect(greylist_remove_btn, &QPushButton::clicked, this, &PreferencesDialog::onGreylistRemoveRow);

    greylist_group->setLayout(greylist_layout);
    bot_tab_layout->addWidget(greylist_group);
    bot_tab_layout->addStretch();
    m_tab_widget->addTab(bot_tab, "Bot Settings");

    // ========== Engines Tab ==========
    if (engine_manager) {
        m_engines_widget = new EnginesPrefsWidget(engine_manager, this);
        m_tab_widget->addTab(m_engines_widget, "Engines");
    }

    // ========== Bottom Buttons ==========
    QHBoxLayout *button_layout = new QHBoxLayout();
    button_layout->addStretch();

    m_apply_btn = new QPushButton("Apply");
    m_ok_btn = new QPushButton("OK");
    m_cancel_btn = new QPushButton("Cancel");

    button_layout->addWidget(m_apply_btn);
    button_layout->addWidget(m_ok_btn);
    button_layout->addWidget(m_cancel_btn);

    main_layout->addLayout(button_layout);

    // Connect signals
    connect(m_host_list, &QListWidget::currentRowChanged, this, &PreferencesDialog::onHostSelectionChanged);
    connect(m_new_btn, &QPushButton::clicked, this, &PreferencesDialog::onNewHost);
    connect(m_delete_btn, &QPushButton::clicked, this, &PreferencesDialog::onDeleteHost);
    connect(m_browse_btn, &QPushButton::clicked, this, &PreferencesDialog::onBrowseSaveDir);
    connect(m_consoledump_browse_btn, &QPushButton::clicked, this, &PreferencesDialog::onBrowseConsoleDumpDir);
    connect(m_apply_btn, &QPushButton::clicked, this, &PreferencesDialog::onApply);
    connect(m_ok_btn, &QPushButton::clicked, this, &PreferencesDialog::onOk);
    connect(m_cancel_btn, &QPushButton::clicked, this, &PreferencesDialog::onCancel);

    // Initially disable fields until a host is selected
    clearHostFields();
}

void PreferencesDialog::loadHosts() {
    m_hosts = settings->getHosts();

    m_host_list->clear();
    for (const Host &host : m_hosts) {
        m_host_list->addItem(host.title);
    }

    if (!m_hosts.empty()) {
        m_host_list->setCurrentRow(0);
    }
}

void PreferencesDialog::onHostSelectionChanged() {
    if (!saveCurrentHost()) {
        return;  // Validation failed, stay on current host
    }

    int row = m_host_list->currentRow();
    if (row >= 0 && row < (int)m_hosts.size()) {
        m_current_host_index = row;
        updateHostFields();
        m_delete_btn->setEnabled(true);
    } else {
        m_current_host_index = -1;
        clearHostFields();
        m_delete_btn->setEnabled(false);
    }
}

void PreferencesDialog::updateHostFields() {
    if (m_current_host_index >= 0 && m_current_host_index < (int)m_hosts.size()) {
        const Host &host = m_hosts[m_current_host_index];

        m_title_edit->setText(host.title);
        m_host_edit->setText(host.host);
        m_port_edit->setText(QString::number(host.port));
        m_login_edit->setText(host.login_name);
        m_password_edit->setText(host.password);

        int codec_index = m_codec_combo->findText(host.codec);
        if (codec_index >= 0) {
            m_codec_combo->setCurrentIndex(codec_index);
        }

        // Enable all fields
        m_title_edit->setEnabled(true);
        m_host_edit->setEnabled(true);
        m_port_edit->setEnabled(true);
        m_login_edit->setEnabled(true);
        m_password_edit->setEnabled(true);
        m_codec_combo->setEnabled(true);
    }
}

void PreferencesDialog::clearHostFields() {
    m_title_edit->clear();
    m_title_edit->setEnabled(false);

    m_host_edit->clear();
    m_host_edit->setEnabled(false);

    m_port_edit->clear();
    m_port_edit->setEnabled(false);

    m_login_edit->clear();
    m_login_edit->setEnabled(false);

    m_password_edit->clear();
    m_password_edit->setEnabled(false);

    m_codec_combo->setCurrentIndex(0);
    m_codec_combo->setEnabled(false);
}

bool PreferencesDialog::saveCurrentHost() {
    if (m_current_host_index >= 0 && m_current_host_index < (int)m_hosts.size()) {
        // Validate required fields
        if (m_title_edit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validation Error", "Title cannot be empty.");
            return false;
        }
        if (m_host_edit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validation Error", "Host cannot be empty.");
            return false;
        }
        if (m_login_edit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validation Error", "Login name cannot be empty.");
            return false;
        }

        bool ok;
        unsigned int port = m_port_edit->text().toUInt(&ok);
        if (!ok || port == 0 || port > 65535) {
            QMessageBox::warning(this, "Validation Error", "Port must be between 1 and 65535.");
            return false;
        }

        // Save to host object
        Host &host = m_hosts[m_current_host_index];
        host.title = m_title_edit->text().trimmed();
        host.host = m_host_edit->text().trimmed();
        host.port = port;
        host.login_name = m_login_edit->text().trimmed();
        host.password = m_password_edit->text();
        host.codec = m_codec_combo->currentText();

        // Update list item
        m_host_list->item(m_current_host_index)->setText(host.title);
    }

    return true;
}

void PreferencesDialog::onNewHost() {
    // Save current host before creating new one
    if (!saveCurrentHost()) {
        return;
    }

    // Create new host with defaults
    Host new_host;
    new_host.title = "New Account";
    new_host.host = "igs.joyjoy.net";
    new_host.port = 7777;
    new_host.login_name = "";
    new_host.password = "";
    new_host.codec = "US-ASCII";

    m_hosts.push_back(new_host);

    // Add to list and select it
    m_host_list->addItem(new_host.title);
    m_host_list->setCurrentRow(m_hosts.size() - 1);
}

void PreferencesDialog::onDeleteHost() {
    int row = m_host_list->currentRow();
    if (row >= 0 && row < (int)m_hosts.size()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "Confirm Delete",
            QString("Delete account '%1'?").arg(m_hosts[row].title),
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            m_hosts.erase(m_hosts.begin() + row);
            delete m_host_list->takeItem(row);
            m_current_host_index = -1;

            if (m_hosts.empty()) {
                clearHostFields();
                m_delete_btn->setEnabled(false);
            }
        }
    }
}

void PreferencesDialog::onBrowseSaveDir() {
    QString currentDir = m_savegame_dir_edit->text();

    // Expand $HOME for display
    if (currentDir.startsWith("$HOME/")) {
        currentDir = QDir::homePath() + currentDir.mid(5);
    } else if (currentDir == "$HOME") {
        currentDir = QDir::homePath();
    }

    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Select Save Game Directory",
        currentDir.isEmpty() ? QDir::homePath() : currentDir,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!dir.isEmpty()) {
        // Optionally replace home path with $HOME for portability
        QString homePath = QDir::homePath();
        if (dir.startsWith(homePath)) {
            dir = "$HOME" + dir.mid(homePath.length());
        }
        m_savegame_dir_edit->setText(dir);
    }
}

void PreferencesDialog::onGreylistAddRow() {
    int row = m_greylist_table->rowCount();
    m_greylist_table->insertRow(row);
    m_greylist_table->setItem(row, 0, new QTableWidgetItem(""));
    m_greylist_table->setItem(row, 1, new QTableWidgetItem("8"));
    m_greylist_table->setItem(row, 2, new QTableWidgetItem(""));
    m_greylist_table->editItem(m_greylist_table->item(row, 0));
}

void PreferencesDialog::onGreylistRemoveRow() {
    int row = m_greylist_table->currentRow();
    if (row >= 0)
        m_greylist_table->removeRow(row);
}

static QList<Settings::GreylistEntry> greylistFromTable(QTableWidget *t) {
    QList<Settings::GreylistEntry> entries;
    for (int r = 0; r < t->rowCount(); ++r) {
        QString name = t->item(r, 0) ? t->item(r, 0)->text().trimmed().toLower() : QString();
        int     hc   = t->item(r, 1) ? t->item(r, 1)->text().trimmed().toInt()  : 0;
        QString tell = t->item(r, 2) ? t->item(r, 2)->text().trimmed()          : QString();
        if (!name.isEmpty())
            entries.append({name, hc, tell});
    }
    return entries;
}

void PreferencesDialog::onBrowseConsoleDumpDir() {
    QString currentDir = m_consoledump_dir_edit->text();
    if (currentDir.startsWith("$HOME/"))
        currentDir = QDir::homePath() + currentDir.mid(5);
    else if (currentDir == "$HOME")
        currentDir = QDir::homePath();

    QString dir = QFileDialog::getExistingDirectory(
        this, "Select Console Dump Directory",
        currentDir.isEmpty() ? QDir::homePath() : currentDir,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty()) {
        QString homePath = QDir::homePath();
        if (dir.startsWith(homePath))
            dir = "$HOME" + dir.mid(homePath.length());
        m_consoledump_dir_edit->setText(dir);
    }
}

void PreferencesDialog::onApply() {
    if (!saveCurrentHost()) {
        return;
    }

    // Save save game directory
    QString saveDir = m_savegame_dir_edit->text().trimmed();
    if (saveDir.isEmpty())
        saveDir = "$HOME/Claude_Projects/64-bit/game_files";
    settings->setSaveGameDirectory(saveDir);

    // Save console dump directory
    QString dumpDir = m_consoledump_dir_edit->text().trimmed();
    if (dumpDir.isEmpty())
        dumpDir = "$HOME/Claude_Projects/64-bit/xgospel2_console_dumps";
    settings->setConsoleDumpDirectory(dumpDir);

    // Save application settings
    settings->setConsoleBufferSize(m_console_buffer_spin->value());
    settings->setGamesWindowRefreshInterval(m_games_refresh_spin->value());
    settings->setPlayersWindowRefreshInterval(m_players_refresh_spin->value());
    settings->setUseFocusColors(m_use_focus_colors_check->isChecked());
    settings->setAutoLaunchShoutWindow(m_auto_launch_shout_check->isChecked());
    settings->setScoringMethod(m_scoring_method_combo->currentData().toString());
    settings->setUseDockedGamePane(m_docked_game_pane_check->isChecked());
    settings->setHoverBoardSize(m_hover_board_size_spin->value());

    // Save engine profiles
    if (m_engines_widget) m_engines_widget->apply();

    // Save bot settings
    settings->setBotBlacklist(m_bot_blacklist_edit->text().split(',', Qt::SkipEmptyParts));
    settings->setBotGreylist(greylistFromTable(m_greylist_table));

    // Save to settings
    settings->setHosts(m_hosts);
    settings->save();

    qDebug() << "Preferences applied";
    QMessageBox::information(this, "Preferences", "Settings saved successfully.\n\nNote: Console buffer size and refresh interval changes will take effect after restarting the application.");
}

void PreferencesDialog::onOk() {
    if (!saveCurrentHost()) {
        return;
    }

    // Save save game directory
    QString saveDir = m_savegame_dir_edit->text().trimmed();
    if (saveDir.isEmpty())
        saveDir = "$HOME/Claude_Projects/64-bit/game_files";
    settings->setSaveGameDirectory(saveDir);

    // Save console dump directory
    QString dumpDir = m_consoledump_dir_edit->text().trimmed();
    if (dumpDir.isEmpty())
        dumpDir = "$HOME/Claude_Projects/64-bit/xgospel2_console_dumps";
    settings->setConsoleDumpDirectory(dumpDir);

    // Save application settings
    settings->setConsoleBufferSize(m_console_buffer_spin->value());
    settings->setGamesWindowRefreshInterval(m_games_refresh_spin->value());
    settings->setPlayersWindowRefreshInterval(m_players_refresh_spin->value());
    settings->setUseFocusColors(m_use_focus_colors_check->isChecked());
    settings->setAutoLaunchShoutWindow(m_auto_launch_shout_check->isChecked());
    settings->setScoringMethod(m_scoring_method_combo->currentData().toString());

    // Save engine profiles
    if (m_engines_widget) m_engines_widget->apply();

    // Save bot settings
    settings->setBotBlacklist(m_bot_blacklist_edit->text().split(',', Qt::SkipEmptyParts));
    settings->setBotGreylist(greylistFromTable(m_greylist_table));

    // Save and close
    settings->setHosts(m_hosts);
    settings->save();

    qDebug() << "Preferences saved";
    accept();
}

void PreferencesDialog::onCancel() {
    reject();
}

#include "preferences_dialog.moc"
