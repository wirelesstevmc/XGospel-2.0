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

QString Settings::readEntry(const QString &key, const QString &defaultValue) const {
    return m_params.value(key, defaultValue);
}

void Settings::writeIntEntry(const QString &key, int value) {
    m_params[key] = QString::number(value);
}

int Settings::readIntEntry(const QString &key, int defaultValue) const {
    bool ok;
    int result = m_params.value(key).toInt(&ok);
    return ok ? result : defaultValue;
}

void Settings::writeBoolEntry(const QString &key, bool value) {
    m_params[key] = value ? "1" : "0";
}

bool Settings::readBoolEntry(const QString &key, bool defaultValue) const {
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

QString Settings::getSaveGameDirectory() const {
    // Default to $HOME/Claude_Projects/64-bit/game_files if not configured
    QString defaultDir = QDir::homePath() + "/Claude_Projects/64-bit/game_files";
    QString saveDir = readEntry("SAVEGAMEDIR", defaultDir);

    // Expand $HOME if present in the path
    if (saveDir.startsWith("$HOME/")) {
        saveDir = QDir::homePath() + saveDir.mid(5);  // Remove "$HOME" and append rest
    } else if (saveDir == "$HOME") {
        saveDir = QDir::homePath();
    }

    // Ensure directory exists
    QDir dir;
    if (!dir.exists(saveDir)) {
        dir.mkpath(saveDir);
        qDebug() << "Created save game directory:" << saveDir;
    }

    return saveDir;
}

void Settings::setSaveGameDirectory(const QString &directory) {
    writeEntry("SAVEGAMEDIR", directory);
}

QString Settings::getConsoleDumpDirectory() const {
    QString defaultDir = QDir::homePath() + "/Claude_Projects/64-bit/xgospel2_console_dumps";
    QString dir = readEntry("CONSOLEDUMPDIR", defaultDir);
    if (dir.startsWith("$HOME/"))
        dir = QDir::homePath() + dir.mid(5);
    else if (dir == "$HOME")
        dir = QDir::homePath();
    QDir d;
    if (!d.exists(dir))
        d.mkpath(dir);
    return dir;
}

void Settings::setConsoleDumpDirectory(const QString &directory) {
    writeEntry("CONSOLEDUMPDIR", directory);
}

void Settings::saveWindowGeometry(const QString &windowName, const QRect &geometry) {
    // Format: "WINDOWNAMEgeometry [x,y,width,height]"
    // Following xgospel1 .Xdefaults style: xgospel.console.geometry: 800x600+100+50
    QString key = windowName + "geometry";
    QString value = QString("%1,%2,%3,%4")
        .arg(geometry.x())
        .arg(geometry.y())
        .arg(geometry.width())
        .arg(geometry.height());
    writeEntry(key, value);
}

QRect Settings::loadWindowGeometry(const QString &windowName, const QRect &defaultGeometry) const {
    QString key = windowName + "geometry";
    QString value = readEntry(key);

    if (value.isEmpty()) {
        return defaultGeometry;
    }

    // Parse "x,y,width,height" format
    QStringList parts = value.split(',');
    if (parts.size() != 4) {
        qDebug() << "Invalid geometry format for" << windowName << "- using default";
        return defaultGeometry;
    }

    bool ok1, ok2, ok3, ok4;
    int x = parts[0].toInt(&ok1);
    int y = parts[1].toInt(&ok2);
    int width = parts[2].toInt(&ok3);
    int height = parts[3].toInt(&ok4);

    if (!ok1 || !ok2 || !ok3 || !ok4 || width <= 0 || height <= 0) {
        qDebug() << "Invalid geometry values for" << windowName << "- using default";
        return defaultGeometry;
    }

    return QRect(x, y, width, height);
}

void Settings::saveByteArray(const QString &key, const QByteArray &data)
{
    writeEntry(key, QString::fromLatin1(data.toHex()));
}

QByteArray Settings::loadByteArray(const QString &key) const
{
    QString hex = readEntry(key);
    if (hex.isEmpty()) return QByteArray();
    return QByteArray::fromHex(hex.toLatin1());
}

void Settings::saveSplitterSizes(const QString &splitterName, const QList<int> &sizes) {
    // Format: "SPLITTERNAMEsizes [size1,size2,...]"
    QString key = splitterName + "sizes";
    QStringList sizeStrings;
    for (int size : sizes) {
        sizeStrings << QString::number(size);
    }
    QString value = sizeStrings.join(",");
    writeEntry(key, value);
}

QList<int> Settings::loadSplitterSizes(const QString &splitterName) const {
    QString key = splitterName + "sizes";
    QString value = readEntry(key);

    QList<int> sizes;
    if (value.isEmpty()) {
        return sizes;  // Return empty list, caller will use defaults
    }

    // Parse "size1,size2,..." format
    QStringList parts = value.split(',');
    for (const QString &part : parts) {
        bool ok;
        int size = part.toInt(&ok);
        if (ok && size >= 0) {
            sizes.append(size);
        } else {
            qDebug() << "Invalid splitter size in" << splitterName << "- ignoring";
            return QList<int>();  // Return empty on error
        }
    }

    return sizes;
}

double Settings::getUiFontScale() const {
    bool ok;
    double v = m_params.value("ui_font_scale", "1.0").toDouble(&ok);
    if (!ok || v < 0.5 || v > 4.0) return 1.0;
    return v;
}

void Settings::setUiFontScale(double scale) {
    m_params["ui_font_scale"] = QString::number(scale, 'f', 2);
}

QStringList Settings::getBotBlacklist() const {
    QString raw = m_params.value("bot_blacklist", "");
    if (raw.trimmed().isEmpty()) return QStringList();
    QStringList names = raw.split(',', Qt::SkipEmptyParts);
    for (auto &n : names) n = n.trimmed().toLower();
    return names;
}

void Settings::setBotBlacklist(const QStringList &names) {
    QStringList lower;
    for (const auto &n : names) { QString t = n.trimmed(); if (!t.isEmpty()) lower << t.toLower(); }
    m_params["bot_blacklist"] = lower.join(',');
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
