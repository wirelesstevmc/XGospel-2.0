#include "preferences_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent), m_current_host_index(-1)
{
    setWindowTitle("xgospel2 Preferences");
    setMinimumSize(600, 400);

    setupUI();
    loadHosts();
}

PreferencesDialog::~PreferencesDialog() {
}

void PreferencesDialog::setupUI() {
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

    app_tab_layout->addStretch();
    m_tab_widget->addTab(app_settings_tab, "Application Settings");

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

void PreferencesDialog::onApply() {
    if (!saveCurrentHost()) {
        return;
    }

    // Save save game directory
    QString saveDir = m_savegame_dir_edit->text().trimmed();
    if (saveDir.isEmpty()) {
        saveDir = "$HOME/Claude_Projects/64-bit/game_files";
    }
    settings->setSaveGameDirectory(saveDir);

    // Save application settings
    settings->setConsoleBufferSize(m_console_buffer_spin->value());
    settings->setGamesWindowRefreshInterval(m_games_refresh_spin->value());
    settings->setPlayersWindowRefreshInterval(m_players_refresh_spin->value());
    settings->setUseFocusColors(m_use_focus_colors_check->isChecked());
    settings->setScoringMethod(m_scoring_method_combo->currentData().toString());

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
    if (saveDir.isEmpty()) {
        saveDir = "$HOME/Claude_Projects/64-bit/game_files";
    }
    settings->setSaveGameDirectory(saveDir);

    // Save application settings
    settings->setConsoleBufferSize(m_console_buffer_spin->value());
    settings->setGamesWindowRefreshInterval(m_games_refresh_spin->value());
    settings->setPlayersWindowRefreshInterval(m_players_refresh_spin->value());
    settings->setUseFocusColors(m_use_focus_colors_check->isChecked());
    settings->setScoringMethod(m_scoring_method_combo->currentData().toString());

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
