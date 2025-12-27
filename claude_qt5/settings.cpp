#include "settings.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>

// Global settings instance
Settings *settings = nullptr;

Settings::Settings() {
    load();
}

Settings::~Settings() {
    save();
}

QString Settings::getConfigFilePath() const {
    // Follow q5Go pattern: ~/.config/xgospel2/xgospel2rc
    QString configDir = QDir::homePath() + "/.config/xgospel2";

    // Create directory if it doesn't exist
    QDir dir;
    if (!dir.exists(configDir)) {
        dir.mkpath(configDir);
        qDebug() << "Created config directory:" << configDir;
    }

    return configDir + "/xgospel2rc";
}

void Settings::writeEntry(const QString &key, const QString &value) {
    m_params[key] = value;
}

QString Settings::readEntry(const QString &key, const QString &defaultValue) {
    return m_params.value(key, defaultValue);
}

void Settings::writeIntEntry(const QString &key, int value) {
    m_params[key] = QString::number(value);
}

int Settings::readIntEntry(const QString &key, int defaultValue) {
    bool ok;
    int result = m_params.value(key).toInt(&ok);
    return ok ? result : defaultValue;
}

void Settings::writeBoolEntry(const QString &key, bool value) {
    m_params[key] = value ? "1" : "0";
}

bool Settings::readBoolEntry(const QString &key, bool defaultValue) {
    QString value = m_params.value(key);
    if (value.isEmpty()) {
        return defaultValue;
    }
    return value == "1" || value.toLower() == "true";
}

void Settings::setHosts(const std::vector<Host> &hosts) {
    m_hosts = hosts;
}

QString Settings::getActiveHost() const {
    return m_active_host;
}

void Settings::setActiveHost(const QString &title) {
    m_active_host = title;
    writeEntry("ACTIVEHOST", title);
}

void Settings::load() {
    QString configFile = getConfigFilePath();
    QFile file(configFile);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Config file not found (first run?), using defaults:" << configFile;
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        // Skip empty lines and comments
        if (line.isEmpty() || line.startsWith("#")) {
            continue;
        }

        // Parse "KEY [value]" format (q5Go format)
        int start = line.indexOf('[');
        int end = line.indexOf(']');

        if (start != -1 && end != -1 && end > start) {
            QString key = line.left(start).trimmed();
            QString value = line.mid(start + 1, end - start - 1);
            m_params[key] = value;
        }
    }

    file.close();

    // Load hosts after reading params
    loadHosts();

    // Load active host
    m_active_host = readEntry("ACTIVEHOST");

    qDebug() << "Loaded configuration from:" << configFile;
    qDebug() << "Loaded" << m_hosts.size() << "host(s)";
}

void Settings::save() {
    // Save hosts to params first
    saveHosts();

    QString configFile = getConfigFilePath();
    QFile file(configFile);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "ERROR: Could not write config file:" << configFile;
        return;
    }

    QTextStream out(&file);

    // Write all parameters in q5Go format: KEY [value]
    QMap<QString, QString>::const_iterator i;
    for (i = m_params.constBegin(); i != m_params.constEnd(); ++i) {
        out << i.key() << " [" << i.value() << "]\n";
    }

    file.close();
    qDebug() << "Saved configuration to:" << configFile;
}

void Settings::loadHosts() {
    m_hosts.clear();

    // Follow q5Go format: HOST1a, HOST1b, etc.
    // HOSTna = title
    // HOSTnb = host address
    // HOSTnc = port
    // HOSTnd = login name
    // HOSTne = password
    // HOSTnf = codec

    for (int i = 1; i <= 20; i++) {  // Support up to 20 hosts
        QString prefix = QString("HOST%1").arg(i);
        QString title = readEntry(prefix + "a");

        if (title.isEmpty()) {
            continue;  // No more hosts
        }

        Host host;
        host.title = title;
        host.host = readEntry(prefix + "b", "igs.joyjoy.net");
        host.port = readIntEntry(prefix + "c", 7777);
        host.login_name = readEntry(prefix + "d");
        host.password = readEntry(prefix + "e");
        host.codec = readEntry(prefix + "f", "US-ASCII");

        m_hosts.push_back(host);
    }
}

void Settings::saveHosts() {
    // Clear old host entries first
    QStringList keysToRemove;
    for (const QString &key : m_params.keys()) {
        if (key.startsWith("HOST") && key.length() > 4) {
            keysToRemove.append(key);
        }
    }
    for (const QString &key : keysToRemove) {
        m_params.remove(key);
    }

    // Write hosts in q5Go format
    for (size_t i = 0; i < m_hosts.size(); i++) {
        QString prefix = QString("HOST%1").arg(i + 1);
        const Host &host = m_hosts[i];

        writeEntry(prefix + "a", host.title);
        writeEntry(prefix + "b", host.host);
        writeIntEntry(prefix + "c", host.port);
        writeEntry(prefix + "d", host.login_name);
        writeEntry(prefix + "e", host.password);
        writeEntry(prefix + "f", host.codec);
    }
}
