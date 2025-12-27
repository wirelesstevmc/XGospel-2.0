#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
#include <QMap>
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
    QString readEntry(const QString &key, const QString &defaultValue = QString());
    void writeIntEntry(const QString &key, int value);
    int readIntEntry(const QString &key, int defaultValue = 0);
    void writeBoolEntry(const QString &key, bool value);
    bool readBoolEntry(const QString &key, bool defaultValue = false);

    // Host/Account management
    std::vector<Host> getHosts() const { return m_hosts; }
    void setHosts(const std::vector<Host> &hosts);
    QString getActiveHost() const;
    void setActiveHost(const QString &title);

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
