#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
#include <QMap>
#include <QRect>
#include <QList>
#include <vector>

// Host/Server account information
struct Host {
    QString title;        // Descriptive title for this account
    QString host;         // Server address (e.g., igs.joyjoy.net)
    QString login_name;   // Username for login
    QString password;     // Password for login
    QString codec;        // Text encoding (e.g., SJIS, US-ASCII, ISO-8859-1)
    unsigned int port;    // Server port (typically 7777 for IGS)

    Host() : port(7777) {}

    Host(const QString &t, const QString &h, unsigned int p,
         const QString &l, const QString &pw, const QString &c)
        : title(t), host(h), login_name(l), password(pw), codec(c), port(p)
    {
    }
};

// Settings manager - reads/writes configuration file
class Settings {
public:
    Settings();
    ~Settings();

    // Read/write individual settings
    void writeEntry(const QString &key, const QString &value);
    QString readEntry(const QString &key, const QString &defaultValue = QString()) const;
    void writeIntEntry(const QString &key, int value);
    int readIntEntry(const QString &key, int defaultValue = 0) const;
    void writeBoolEntry(const QString &key, bool value);
    bool readBoolEntry(const QString &key, bool defaultValue = false) const;

    // Host/Account management
    std::vector<Host> getHosts() const { return m_hosts; }
    void setHosts(const std::vector<Host> &hosts);
    QString getActiveHost() const;
    void setActiveHost(const QString &title);

    // Save game directory management
    QString getSaveGameDirectory() const;
    void setSaveGameDirectory(const QString &directory);

    // Console dump directory management
    QString getConsoleDumpDirectory() const;
    void setConsoleDumpDirectory(const QString &directory);

    // Window geometry management (xgospel1 .Xdefaults style)
    void saveWindowGeometry(const QString &windowName, const QRect &geometry);
    QRect loadWindowGeometry(const QString &windowName, const QRect &defaultGeometry = QRect()) const;

    // Splitter state management (for board window panel sizes)
    void saveSplitterSizes(const QString &splitterName, const QList<int> &sizes);
    QList<int> loadSplitterSizes(const QString &splitterName) const;

    // Binary state storage (hex-encoded) — used for QMainWindow::saveState()
    void saveByteArray(const QString &key, const QByteArray &data);
    QByteArray loadByteArray(const QString &key) const;

    // Debug flag management (all default to false)
    bool getDebugObservationState() const { return readBoolEntry("DEBUG_OBSERVATION_STATE", false); }
    void setDebugObservationState(bool value) { writeBoolEntry("DEBUG_OBSERVATION_STATE", value); }

    bool getDebugMoveProcessing() const { return readBoolEntry("DEBUG_MOVE_PROCESSING", false); }
    void setDebugMoveProcessing(bool value) { writeBoolEntry("DEBUG_MOVE_PROCESSING", value); }

    bool getDebugEditMode() const { return readBoolEntry("DEBUG_EDIT_MODE", false); }
    void setDebugEditMode(bool value) { writeBoolEntry("DEBUG_EDIT_MODE", value); }

    bool getDebugScoring() const { return readBoolEntry("DEBUG_SCORING", false); }
    void setDebugScoring(bool value) { writeBoolEntry("DEBUG_SCORING", value); }

    // Scoring method: "simple" (default) or "complex"
    QString getScoringMethod() const { return readEntry("scoring_method", "simple"); }
    void setScoringMethod(const QString &method) { writeEntry("scoring_method", method); }

    bool getDebugProtocol() const { return readBoolEntry("DEBUG_PROTOCOL", false); }
    void setDebugProtocol(bool value) { writeBoolEntry("DEBUG_PROTOCOL", value); }

    bool getDebugMatch() const { return readBoolEntry("DEBUG_MATCH", false); }
    void setDebugMatch(bool value) { writeBoolEntry("DEBUG_MATCH", value); }

    bool getDebugCmd15() const { return readBoolEntry("DEBUG_CMD15", false); }
    void setDebugCmd15(bool value) { writeBoolEntry("DEBUG_CMD15", value); }

    // Games window auto-refresh interval (in seconds, 0 = disabled)
    int getGamesWindowRefreshInterval() const { return readIntEntry("games_window_refresh", 90); }
    void setGamesWindowRefreshInterval(int seconds) { writeIntEntry("games_window_refresh", seconds); }

    // Players window auto-refresh interval (in seconds, 0 = disabled)
    int getPlayersWindowRefreshInterval() const { return readIntEntry("players_window_refresh", 120); }
    void setPlayersWindowRefreshInterval(int seconds) { writeIntEntry("players_window_refresh", seconds); }

    // Console buffer size (in lines, 0 = unlimited)
    int getConsoleBufferSize() const { return readIntEntry("console_buffer_size", 30000); }
    void setConsoleBufferSize(int lines) { writeIntEntry("console_buffer_size", lines); }

    // Use focus-based highlighting colors (false = q5Go style, always use inactive color)
    bool getUseFocusColors() const { return readBoolEntry("use_focus_colors", true); }
    void setUseFocusColors(bool value) { writeBoolEntry("use_focus_colors", value); }

    // Dockable game pane (Phase 2 — multi-game observation mode)
    bool getUseDockedGamePane() const { return readBoolEntry("use_docked_game_pane", false); }
    void setUseDockedGamePane(bool value) { writeBoolEntry("use_docked_game_pane", value); }

    // Hover board preview size in pixels (100–600, default 200)
    int getHoverBoardSize() const { return readIntEntry("hover_board_size", 200); }
    void setHoverBoardSize(int px) { writeIntEntry("hover_board_size", px); }

    // Auto-launch Shout window minimized on login (default false — open manually via Windows menu)
    bool getAutoLaunchShoutWindow() const { return readBoolEntry("auto_launch_shout_window", false); }
    void setAutoLaunchShoutWindow(bool value) { writeBoolEntry("auto_launch_shout_window", value); }

    bool getAutoMinimizeOnLogin() const { return readBoolEntry("auto_minimize_on_login", true); }
    void setAutoMinimizeOnLogin(bool value) { writeBoolEntry("auto_minimize_on_login", value); }

    // Global UI font scale factor (1.0 = default; future font-scale feature)
    double getUiFontScale() const;
    void setUiFontScale(double scale);

    // Bot mode persistent state — restored automatically on next login
    bool getBotModeEnabled() const { return readBoolEntry("bot_mode_enabled", false); }
    void setBotModeEnabled(bool v) { writeBoolEntry("bot_mode_enabled", v); }

    // Last selected engine profile ID — restored at startup
    QString getSelectedEngineProfile() const { return readEntry("selected_engine_profile", ""); }
    void setSelectedEngineProfile(const QString &id) { writeEntry("selected_engine_profile", id); }

    // Bot opponent blacklist — comma-separated IGS usernames; match requests declined automatically
    QStringList getBotBlacklist() const;
    void setBotBlacklist(const QStringList &names);

    // Experimental: Sabaki-style areaMap dead stone detection (geometric flood-fill + KataGo confirm)
    bool getBotUseAreaMapDetection() const { return readBoolEntry("bot_use_area_map_detection", false); }
    void setBotUseAreaMapDetection(bool v) { writeBoolEntry("bot_use_area_map_detection", v); }

    // Bot opponent greylist — per-opponent max handicap + custom decline tell
    struct GreylistEntry {
        QString name;      // lowercase IGS username
        int     max_hc;    // accept if handicap <= max_hc; decline if > max_hc
        QString tell;      // message sent to opponent on decline (may be empty)
    };
    QList<GreylistEntry> getBotGreylist() const;
    void setBotGreylist(const QList<GreylistEntry> &entries);

    // Save all settings to disk
    void save();

    // Load all settings from disk
    void load();

private:
    QString getConfigFilePath() const;
    void loadHosts();
    void saveHosts();

    QMap<QString, QString> m_params;
    std::vector<Host> m_hosts;
    QString m_active_host;
};

// Global settings instance
extern Settings *settings;

#endif // SETTINGS_H
