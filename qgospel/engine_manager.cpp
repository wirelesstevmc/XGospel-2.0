#include "engine_manager.h"
#include "settings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QUuid>
#include <QFileInfo>
#include <QDebug>

// ---------------------------------------------------------------------------
// EngineManager
// ---------------------------------------------------------------------------

EngineManager::EngineManager(QObject *parent)
    : QObject(parent)
{
    loadProfiles();
}

EngineManager::~EngineManager()
{
    detach();
}

void EngineManager::setProfiles(const QList<EngineProfile> &profiles)
{
    m_profiles = profiles;
}

void EngineManager::saveProfiles()
{
    // Serialise as: engine_count, then for each: id, name, path, args, workdir
    settings->writeIntEntry("engine_profile_count", m_profiles.size());
    for (int i = 0; i < m_profiles.size(); ++i) {
        const EngineProfile &p = m_profiles[i];
        QString prefix = QString("engine_%1_").arg(i);
        settings->writeEntry(prefix + "id",      p.id);
        settings->writeEntry(prefix + "name",    p.name);
        settings->writeEntry(prefix + "path",    p.path);
        settings->writeEntry(prefix + "args",    p.args);
        settings->writeEntry(prefix + "workdir", p.workdir);
    }
    settings->save();
}

void EngineManager::loadProfiles()
{
    m_profiles.clear();
    int count = settings->readIntEntry("engine_profile_count", 0);
    for (int i = 0; i < count; ++i) {
        QString prefix = QString("engine_%1_").arg(i);
        EngineProfile p;
        p.id      = settings->readEntry(prefix + "id");
        p.name    = settings->readEntry(prefix + "name");
        p.path    = settings->readEntry(prefix + "path");
        p.args    = settings->readEntry(prefix + "args");
        p.workdir = settings->readEntry(prefix + "workdir");
        if (!p.id.isEmpty() && !p.name.isEmpty() && !p.path.isEmpty())
            m_profiles.append(p);
    }
}

bool EngineManager::attach(const QString &profileId, double komi, int handicap,
                           int boardsize, int timePerMove)
{
    EngineProfile found;
    bool ok = false;
    for (const EngineProfile &p : m_profiles) {
        if (p.id == profileId) { found = p; ok = true; break; }
    }
    if (!ok) {
        qWarning() << "[EngineManager] Profile not found:" << profileId;
        return false;
    }

    detach();

    m_engine = new KataGoEngine(this);
    m_active_profile_id = profileId;

    // Derive working directory: explicit > directory of executable > current dir
    QString workdir = found.workdir.trimmed();
    if (workdir.isEmpty()) {
        QFileInfo fi(found.path);
        workdir = fi.absolutePath();
    }

    m_engine->start(found.path, found.args, workdir, komi, handicap, boardsize, timePerMove);
    emit engineAttached(found.name);
    return true;
}

void EngineManager::detach()
{
    if (m_engine) {
        m_engine->stop();
        delete m_engine;
        m_engine = nullptr;
        m_active_profile_id.clear();
        emit engineDetached();
    }
}

// ---------------------------------------------------------------------------
// EngineEditDialog
// ---------------------------------------------------------------------------

EngineEditDialog::EngineEditDialog(QWidget *parent, const EngineProfile &profile)
    : QDialog(parent)
{
    setupUI(profile);
    setWindowTitle(profile.name.isEmpty() ? "Add Engine" : "Edit Engine");
    setMinimumWidth(500);
}

void EngineEditDialog::setupUI(const EngineProfile &profile)
{
    QVBoxLayout *main_layout = new QVBoxLayout(this);

    QFormLayout *form = new QFormLayout();

    m_name_edit = new QLineEdit(profile.name, this);
    m_name_edit->setPlaceholderText("e.g. KataGo (local)");
    form->addRow("Name:", m_name_edit);

    QHBoxLayout *path_row = new QHBoxLayout();
    m_path_edit = new QLineEdit(profile.path, this);
    m_path_edit->setPlaceholderText("/path/to/katago or /path/to/engine_wrapper.sh");
    m_browse_path_btn = new QPushButton("Browse…", this);
    path_row->addWidget(m_path_edit);
    path_row->addWidget(m_browse_path_btn);
    form->addRow("Path:", path_row);

    m_args_edit = new QLineEdit(profile.args, this);
    m_args_edit->setPlaceholderText("gtp -model model.bin.gz -config gtp.cfg");
    form->addRow("Arguments:", m_args_edit);

    QHBoxLayout *workdir_row = new QHBoxLayout();
    m_workdir_edit = new QLineEdit(profile.workdir, this);
    m_workdir_edit->setPlaceholderText("Leave blank to use directory of executable");
    m_browse_workdir_btn = new QPushButton("Browse…", this);
    workdir_row->addWidget(m_workdir_edit);
    workdir_row->addWidget(m_browse_workdir_btn);
    form->addRow("Working dir:", workdir_row);

    main_layout->addLayout(form);

    QLabel *hint = new QLabel(
        "<small>Tip: for a remote engine via SSH, set Path to a shell script:<br>"
        "<tt>#!/bin/sh<br>ssh -t -t user@host '/path/to/katago gtp'</tt></small>", this);
    hint->setTextFormat(Qt::RichText);
    hint->setWordWrap(true);
    main_layout->addWidget(hint);

    QHBoxLayout *btn_row = new QHBoxLayout();
    btn_row->addStretch();
    m_ok_btn     = new QPushButton("OK",     this);
    m_cancel_btn = new QPushButton("Cancel", this);
    m_ok_btn->setDefault(true);
    btn_row->addWidget(m_ok_btn);
    btn_row->addWidget(m_cancel_btn);
    main_layout->addLayout(btn_row);

    connect(m_browse_path_btn,    &QPushButton::clicked, this, &EngineEditDialog::onBrowsePath);
    connect(m_browse_workdir_btn, &QPushButton::clicked, this, &EngineEditDialog::onBrowseWorkdir);
    connect(m_ok_btn,     &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancel_btn, &QPushButton::clicked, this, &QDialog::reject);
}

EngineProfile EngineEditDialog::result() const
{
    EngineProfile p;
    p.id      = QUuid::createUuid().toString();
    p.name    = m_name_edit->text().trimmed();
    p.path    = m_path_edit->text().trimmed();
    p.args    = m_args_edit->text().trimmed();
    p.workdir = m_workdir_edit->text().trimmed();
    return p;
}

void EngineEditDialog::onBrowsePath()
{
    QString path = QFileDialog::getOpenFileName(
        this, "Select Engine Executable or Script",
        m_path_edit->text().isEmpty() ? QDir::homePath() : m_path_edit->text());
    if (!path.isEmpty())
        m_path_edit->setText(path);
}

void EngineEditDialog::onBrowseWorkdir()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, "Select Working Directory",
        m_workdir_edit->text().isEmpty() ? QDir::homePath() : m_workdir_edit->text());
    if (!dir.isEmpty())
        m_workdir_edit->setText(dir);
}

// ---------------------------------------------------------------------------
// EnginesPrefsWidget
// ---------------------------------------------------------------------------

EnginesPrefsWidget::EnginesPrefsWidget(EngineManager *manager, QWidget *parent)
    : QWidget(parent)
    , m_manager(manager)
    , m_profiles(manager->profiles())
{
    setupUI();
    refreshList();
}

void EnginesPrefsWidget::setupUI()
{
    QVBoxLayout *main_layout = new QVBoxLayout(this);

    // GTP logging row
    QGroupBox *log_group = new QGroupBox("GTP Communication Logging", this);
    QHBoxLayout *log_layout = new QHBoxLayout(log_group);
    m_log_check    = new QCheckBox("Enable GTP logging to directory:", log_group);
    m_log_dir_edit = new QLineEdit(log_group);
    m_log_dir_edit->setPlaceholderText("/path/to/log/dir");
    m_browse_log_btn = new QPushButton("Browse…", log_group);
    log_layout->addWidget(m_log_check);
    log_layout->addWidget(m_log_dir_edit, 1);
    log_layout->addWidget(m_browse_log_btn);
    main_layout->addWidget(log_group);

    // Load logging settings
    m_log_check->setChecked(settings->readBoolEntry("engine_gtp_logging", false));
    m_log_dir_edit->setText(settings->readEntry("engine_gtp_log_dir"));
    m_log_dir_edit->setEnabled(m_log_check->isChecked());
    m_browse_log_btn->setEnabled(m_log_check->isChecked());
    connect(m_log_check, &QCheckBox::toggled, m_log_dir_edit, &QLineEdit::setEnabled);
    connect(m_log_check, &QCheckBox::toggled, m_browse_log_btn, &QPushButton::setEnabled);
    connect(m_browse_log_btn, &QPushButton::clicked, this, &EnginesPrefsWidget::onBrowseLogDir);

    // Engine list
    QGroupBox *list_group = new QGroupBox("Registered Engines", this);
    QVBoxLayout *list_layout = new QVBoxLayout(list_group);

    m_list = new QListWidget(list_group);
    m_list->setAlternatingRowColors(true);
    list_layout->addWidget(m_list);

    QHBoxLayout *btn_row = new QHBoxLayout();
    m_add_btn    = new QPushButton("Add",    list_group);
    m_edit_btn   = new QPushButton("Edit",   list_group);
    m_remove_btn = new QPushButton("Remove", list_group);
    m_edit_btn->setEnabled(false);
    m_remove_btn->setEnabled(false);
    btn_row->addWidget(m_add_btn);
    btn_row->addWidget(m_edit_btn);
    btn_row->addWidget(m_remove_btn);
    btn_row->addStretch();
    list_layout->addLayout(btn_row);
    main_layout->addWidget(list_group, 1);

    QLabel *note = new QLabel(
        "<small>Each engine must support the GTP protocol. "
        "You may use a shell script as the path to connect to a remote engine via SSH.</small>",
        this);
    note->setWordWrap(true);
    main_layout->addWidget(note);

    connect(m_list,       &QListWidget::itemSelectionChanged,
            this, &EnginesPrefsWidget::onSelectionChanged);
    connect(m_list,       &QListWidget::itemDoubleClicked,
            this, &EnginesPrefsWidget::onEdit);
    connect(m_add_btn,    &QPushButton::clicked, this, &EnginesPrefsWidget::onAdd);
    connect(m_edit_btn,   &QPushButton::clicked, this, &EnginesPrefsWidget::onEdit);
    connect(m_remove_btn, &QPushButton::clicked, this, &EnginesPrefsWidget::onRemove);
}

void EnginesPrefsWidget::refreshList()
{
    m_list->clear();
    for (const EngineProfile &p : m_profiles) {
        QString label = QString("%1  —  %2").arg(p.name).arg(p.path);
        QListWidgetItem *item = new QListWidgetItem(label, m_list);
        item->setData(Qt::UserRole, p.id);
    }
    onSelectionChanged();
}

void EnginesPrefsWidget::onSelectionChanged()
{
    bool has = !m_list->selectedItems().isEmpty();
    m_edit_btn->setEnabled(has);
    m_remove_btn->setEnabled(has);
}

void EnginesPrefsWidget::onAdd()
{
    EngineEditDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    EngineProfile p = dlg.result();
    if (p.name.isEmpty() || p.path.isEmpty()) {
        QMessageBox::warning(this, "Engine", "Name and path are required.");
        return;
    }
    m_profiles.append(p);
    refreshList();
    m_list->setCurrentRow(m_profiles.size() - 1);
}

void EnginesPrefsWidget::onEdit()
{
    QList<QListWidgetItem*> sel = m_list->selectedItems();
    if (sel.isEmpty()) return;
    QString id = sel.first()->data(Qt::UserRole).toString();
    int idx = -1;
    for (int i = 0; i < m_profiles.size(); ++i) {
        if (m_profiles[i].id == id) { idx = i; break; }
    }
    if (idx < 0) return;

    EngineEditDialog dlg(this, m_profiles[idx]);
    if (dlg.exec() != QDialog::Accepted) return;
    EngineProfile p = dlg.result();
    p.id = m_profiles[idx].id;  // preserve stable ID
    if (p.name.isEmpty() || p.path.isEmpty()) {
        QMessageBox::warning(this, "Engine", "Name and path are required.");
        return;
    }
    m_profiles[idx] = p;
    refreshList();
    m_list->setCurrentRow(idx);
}

void EnginesPrefsWidget::onRemove()
{
    QList<QListWidgetItem*> sel = m_list->selectedItems();
    if (sel.isEmpty()) return;
    QString id = sel.first()->data(Qt::UserRole).toString();
    for (int i = 0; i < m_profiles.size(); ++i) {
        if (m_profiles[i].id == id) {
            m_profiles.removeAt(i);
            break;
        }
    }
    refreshList();
}

void EnginesPrefsWidget::onBrowseLogDir()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, "Select GTP Log Directory",
        m_log_dir_edit->text().isEmpty() ? QDir::homePath() : m_log_dir_edit->text());
    if (!dir.isEmpty())
        m_log_dir_edit->setText(dir);
}

void EnginesPrefsWidget::apply()
{
    m_manager->setProfiles(m_profiles);
    m_manager->saveProfiles();
    settings->writeBoolEntry("engine_gtp_logging",  m_log_check->isChecked());
    settings->writeEntry("engine_gtp_log_dir", m_log_dir_edit->text().trimmed());
    settings->save();
}

#include "engine_manager.moc"
