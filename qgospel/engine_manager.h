#ifndef ENGINE_MANAGER_H
#define ENGINE_MANAGER_H

#include <QObject>
#include <QDialog>
#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QList>

#include "katago_engine.h"

// ---------------------------------------------------------------------------
// EngineProfile — persistent engine registration record
// ---------------------------------------------------------------------------

struct EngineProfile {
    QString id;     // UUID — stable across renames; used as Settings key
    QString name;   // Display name, e.g. "KataGo (local)"
    QString path;   // Path to binary or wrapper script
    QString args;   // Extra args, e.g. "-model foo.bin.gz -config gtp.cfg"
                    // For a wrapper script this is typically empty.
    QString workdir; // Working directory; empty = derive from executable path
};

// ---------------------------------------------------------------------------
// EngineManager — owns the engine registry and the active KataGoEngine instance
// ---------------------------------------------------------------------------

class EngineManager : public QObject {
    Q_OBJECT

public:
    explicit EngineManager(QObject *parent = nullptr);
    ~EngineManager();

    // Profile registry
    QList<EngineProfile> profiles() const { return m_profiles; }
    void setProfiles(const QList<EngineProfile> &profiles);
    void saveProfiles();
    void loadProfiles();

    // Active engine
    KataGoEngine *currentEngine() const { return m_engine; }
    bool          isAttached()    const { return m_engine != nullptr; }

    // Attach an engine from a profile.  Stops any currently running engine first.
    // Returns false if the profile is not found.
    bool attach(const QString &profileId, double komi, int handicap,
                int boardsize = 19, int timePerMove = 5);

    // Detach (stop) the current engine.
    void detach();

signals:
    void engineAttached(const QString &profileName);
    void engineDetached();

private:
    QList<EngineProfile> m_profiles;
    KataGoEngine        *m_engine = nullptr;
    QString              m_active_profile_id;
};

// ---------------------------------------------------------------------------
// EngineEditDialog — Add / Edit a single engine profile
// ---------------------------------------------------------------------------

class EngineEditDialog : public QDialog {
    Q_OBJECT

public:
    explicit EngineEditDialog(QWidget *parent = nullptr,
                              const EngineProfile &profile = EngineProfile());

    EngineProfile result() const;

private slots:
    void onBrowsePath();
    void onBrowseWorkdir();

private:
    void setupUI(const EngineProfile &profile);

    QLineEdit *m_name_edit;
    QLineEdit *m_path_edit;
    QLineEdit *m_args_edit;
    QLineEdit *m_workdir_edit;
    QPushButton *m_browse_path_btn;
    QPushButton *m_browse_workdir_btn;
    QPushButton *m_ok_btn;
    QPushButton *m_cancel_btn;
};

// ---------------------------------------------------------------------------
// EnginesPrefsWidget — "Engines" tab in PreferencesDialog
// ---------------------------------------------------------------------------

class EnginesPrefsWidget : public QWidget {
    Q_OBJECT

public:
    explicit EnginesPrefsWidget(EngineManager *manager, QWidget *parent = nullptr);

    // Call from PreferencesDialog::onApply() / onOk()
    void apply();

private slots:
    void onAdd();
    void onEdit();
    void onRemove();
    void onSelectionChanged();
    void onBrowseLogDir();

private:
    void setupUI();
    void refreshList();

    EngineManager  *m_manager;
    QList<EngineProfile> m_profiles; // working copy

    QListWidget    *m_list;
    QPushButton    *m_add_btn;
    QPushButton    *m_edit_btn;
    QPushButton    *m_remove_btn;
    QCheckBox      *m_log_check;
    QLineEdit      *m_log_dir_edit;
    QPushButton    *m_browse_log_btn;
};

#endif // ENGINE_MANAGER_H
