#include "preferences_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>
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

    main_layout->addWidget(server_group);

    // Bottom buttons
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

void PreferencesDialog::onApply() {
    if (!saveCurrentHost()) {
        return;
    }

    // Save to settings
    settings->setHosts(m_hosts);
    settings->save();

    qDebug() << "Preferences applied";
    QMessageBox::information(this, "Preferences", "Settings saved successfully.");
}

void PreferencesDialog::onOk() {
    if (!saveCurrentHost()) {
        return;
    }

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
