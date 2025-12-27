#include "q5go_parser.h"
#include <QDebug>
#include <QRegExp>

// Override the parsePlayerLine method to handle real IGS format
bool Q5GoParser::parsePlayerLine(const QString &line, Q5GoPlayer &player)
{
    // Reset player data
    player = Q5GoPlayer();

    // Skip obviously non-player lines
    if (line.length() < 20) return false;
    if (line.contains("Idle") || line.contains("Info")) return false;
    if (line.contains("**")) return false;
    if (line.contains("players listed")) return false;

    // Handle both IGS formats:
    // 1. userlist (cmd42) format: Fixed-width format with detailed player info
    // 2. who format: [number] [flags] [name] [idle] [rank] | [number] [flags] [name] [idle] [rank]

    // Try cmd42 (userlist) format first - this has the full player data including correct status flags
    if (line.contains("42 ") || line.startsWith("< 42")) {
        if (parseIGSPlayer(line, player)) {
            return true;
        }
    }

    // Handle "who" format which can have multiple players per line separated by "|"
    // Example: "17 -- guest4959 7s NR | Q -- 25 Kuchi 1s 1k*"
    if (line.contains("|")) {
        return parseWhoFormatLine(line, player);
    }
    
    // Handle userlist format
    QStringList parts = line.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
    if (parts.size() < 6) return false;
    
    // Skip if first part is not a number (header lines, etc.)
    bool isNumber;
    parts[0].toInt(&isNumber);
    if (!isNumber) return false;
    
    // Parse the userlist format
    if (parts.size() >= 6) {
        player.name = parts[1].trimmed();  // Real player name (gp1138, etc.)
        
        // Better parsing: look for rank patterns more carefully
        QString country = "";
        QString rank = "NR";  // Default to NR
        
        // Look for rank pattern (more specific: 1-2 digits followed by d/k/p, possibly with *)
        int rankIndex = -1;
        for (int i = 2; i < parts.size(); i++) {
            QString part = parts[i];
            // More precise rank pattern: number + rank letter + optional *
            if (part.contains(QRegExp("^[0-9]{1,2}[dkp][*+?]?$|^[0-9]+[dkp]$|^BC$|^NR$"))) {
                rank = part;
                rankIndex = i;
                break;
            }
        }
        
        // Look for country: should be alphabetic and NOT a rank, typically before rank
        // Common countries: Japan, USA, China, Korea, Taiwan, etc.
        for (int i = 2; i < parts.size() && i < rankIndex; i++) {
            QString part = parts[i];
            // Country should be alphabetic, not contain < or >, and not be a rank
            if (part.contains(QRegExp("^[A-Za-z]+$")) && 
                !part.contains("<") && !part.contains(">") &&
                !part.contains(QRegExp("^[0-9]{1,2}[dkp]"))) {
                country = part;
                break;
            }
        }
        
        // Check for "+" designation based on BWN pattern
        // Players without BWN string in their userlist entry get "+" designation
        QString finalRank = rank;
        if (rank.contains(QRegExp("^[0-9]{1,2}[dk][*?]?$"))) {
            // Check if this is a plus rank by looking for absence of BWN
            // BWN typically appears after "T" in the format "T BWN"
            if (!line.contains("BWN")) {
                // Special cases: Pro ranks and 10d never have "+"
                QString lowerRank = rank.toLower();
                if (!lowerRank.contains('p') && !lowerRank.startsWith("10d")) {
                    // Convert to plus rank by replacing * with +
                    finalRank = rank;
                    finalRank.replace('*', '+');
                }
            }
        }
        
        player.rank = finalRank;
        player.country = country;
        
        // Extract Info field (descriptive text like <None>, <Playing>, etc.)
        QString info_text = "";
        for (int i = 2; i < parts.size() && i < 6; i++) {
            if (parts[i].contains("<") || parts[i].contains(">")) {
                info_text = parts[i];
                break;
            }
        }
        
        // Extract Stat field (status flags)
        // IGS status flags are exactly 2 characters: first char + second char
        // Examples: "-!", "-X", "Q-", "S-", "--"
        // Each char can be: ! (looking ON), X (open OFF), Q (quiet ON), S (shout suppress ON), - (default/off)
        // Note: "--" means all flags in default state (not looking, open, not quiet, not shout-suppress)
        QString stat_flags = "";
        for (int i = parts.size() - 1; i >= 0 && i > parts.size() - 4; i--) {
            QString part = parts[i];
            // Look for exactly 2-character status flags
            if (part.length() == 2 && part.contains(QRegExp("[!XQS-]")) &&
                !part.contains(QRegExp("[0-9]+[smh]"))) {
                stat_flags = part;
                break;
            }
        }

        // Assign to correct fields - ensure exactly 2 chars
        player.info = stat_flags.isEmpty() || stat_flags.length() != 2 ? "--" : stat_flags;  // Stat column gets status flags
        player.extInfo = info_text;  // This will be used for Info column
        
        // Look for idle time at the end (format like "0s", "5m", "2h")
        for (int i = parts.size() - 1; i >= 0; i--) {
            if (parts[i].contains(QRegExp("^[0-9]+[smh]?$"))) {
                player.idle = parts[i];
                break;
            }
        }
        if (player.idle.isEmpty()) player.idle = "0s";
        
        // Set defaults for other fields
        player.play_str = "--";
        player.obs_str = "--";
        player.sort_rk = player.rankToSortKey();
        player.won = "0";
        player.lost = "0";
        player.nmatch_settings = "";
        
        return true;
    }
    
    // Method 3: Very permissive - any line with a reasonable username
    QRegExp namePattern("([a-zA-Z][a-zA-Z0-9_]{2,15})");
    if (namePattern.indexIn(line) >= 0) {
        QString name = namePattern.cap(1);
        
        // Skip obvious non-usernames
        if (name == "IGS" || name == "Type" || name == "players" || 
            name == "Game" || name == "Info" || name == "Idle") {
            return false;
        }
        
        player.name = name;
        
        // Try to find a rank after the name
        int namePos = namePattern.pos();
        QString afterName = line.mid(namePos + name.length());
        QRegExp rankPattern("([0-9]*[dkp*]+|BC|NR)");
        
        if (rankPattern.indexIn(afterName) >= 0) {
            player.rank = rankPattern.cap(1);
        } else {
            player.rank = "NR";
        }
        
        // Try to extract idle time
        QRegExp idlePattern("([0-9]+[smh]?)");
        if (idlePattern.indexIn(afterName) >= 0) {
            player.idle = idlePattern.cap(1);
        } else {
            player.idle = "0s";
        }
        
        // Set defaults
        player.info = "--";
        player.play_str = "--";
        player.obs_str = "--";
        player.sort_rk = player.rankToSortKey();
        player.won = "0";
        player.lost = "0";
        player.country = "";
        player.nmatch_settings = "";
        
        // Player parsed successfully
        return true;
    }
    
    return false;
}

// Constructor
Q5GoParser::Q5GoParser() {
}

// Parse game line (simplified)
bool Q5GoParser::parseGameLine(const QString &line, Q5GoGame &game) {
    Q_UNUSED(line);
    Q_UNUSED(game);
    return false; // Not implemented yet
}

// Parse players line (wrapper for parsePlayerLine)
std::vector<Q5GoPlayer> Q5GoParser::parsePlayersLine(const QString &line) {
    std::vector<Q5GoPlayer> players;
    Q5GoPlayer player;
    if (parsePlayerLine(line, player)) {
        players.push_back(player);
    }
    return players;
}

// IGS player parsing - cmd42 userlist format (exact q5Go regex from parser.cpp:1965)
// Format: 42      -----  <none>          Japan    15k+ 2108/2455  -   -    1m    Q- default  T BWN 0-9 19-19 600-600 1200-1200 25-25 0-0 0-0 0-0
bool Q5GoParser::parseIGSPlayer(const QString &txt, Q5GoPlayer &player) {
    // Skip header lines (exact q5Go logic from parser.cpp:1957)
    if (txt.length() > 40 && txt[40] == 'R') {
        return false; // skip header
    }

    // Skip end markers
    if (txt.contains("**")) {
        return false;
    }

    // Strip "< " prefix if present (IGS sends "< 42" but regex expects "42")
    QString line = txt;
    if (line.startsWith("< ")) {
        line = line.mid(2);  // Remove first 2 characters
    }

    // Exact q5Go regex from parser.cpp lines 1965-1980
    // Status flags (capture 10): 2 chars, each can be ! X Q S or -
    QRegExp re = QRegExp("42 ([A-Za-z0-9 ]{,10})  (.{1,14})  "
                         "([a-zA-Z][a-zA-Z. /]{,6}|--     )  "
                         "([0-9 ][0-9][kdp].?| +BC) +"
                         "([0-9 ]+[0-9]+)/([0-9 ]+[0-9]+) +"
                         "([0-9]+|-) +([0-9]+|-) +"
                         "([A-Za-z0-9]+) +([^ ]{,2}) +default  ([TF])(.*)?");

    if (re.indexIn(line) < 0) {
        qDebug() << "[IGS PARSE] FAIL:" << line.left(80);
        return false;  // No match
    }

    // Extract fields exactly like q5Go (parser.cpp lines 1989-2001)
    player.name = re.cap(1).trimmed();
    player.extInfo = re.cap(2);
    player.country = re.cap(3).trimmed();
    if (player.country == "--") {
        player.country = "";
    }
    player.rank = re.cap(4).trimmed();
    player.won = re.cap(5).trimmed();
    player.lost = re.cap(6).trimmed();
    player.obs_str = re.cap(7).trimmed();
    player.play_str = re.cap(8).trimmed();
    player.idle = re.cap(9).trimmed();
    player.info = re.cap(10).trimmed();  // This is the status flags (Q-, -X, etc.)
    player.nmatch = re.cap(11) == "T";

    // Debug: Show what we captured for status flags
    if (player.name == "ktl" || player.name == "kansai2" || player.info != "--") {
        qDebug() << "[IGS PARSE] Name:" << player.name
                 << "Cap10:'" << re.cap(10) << "'"
                 << "Trimmed:'" << player.info << "'";
    }

    // Calculate rated games count (wins + losses)
    int wins_int = player.won.toInt();
    int losses_int = player.lost.toInt();
    player.rated = QString::number(wins_int + losses_int);

    // Set defaults
    player.nmatch_settings = "";
    player.sort_rk = player.rankToSortKey();

    return true;
}

// Implementation of rankToSortKey for Q5GoPlayer
QString Q5GoPlayer::rankToSortKey() const {
    // Create sort key: pro="a"+reverse, dan="b"+reverse, kyu="c"+normal
    // Special cases:
    // - Pro ranks (1p-9p): No half-ranks exist
    // - 10d: Highest amateur rank, no 10d+ exists  
    // - Other dan/kyu: + indicates half-rank, * indicates evaluated rank
    QString trimmed = rank.trimmed().toLower();
    
    if (trimmed.contains('p')) {
        int num = trimmed.left(trimmed.indexOf('p')).toInt();
        // Pro ranks: higher number = stronger, so reverse for ascending sort
        // Pro ranks don't have half-ranks (ignore + or *)
        double rankValue = num;
        return QString("a%1").arg((int)((100.0 - rankValue) * 10), 4, 10, QChar('0'));
    } else if (trimmed.contains('d')) {
        int num = trimmed.left(trimmed.indexOf('d')).toInt();
        // Dan ranks: higher number = stronger, so reverse for ascending sort
        bool hasHalf = false;
        bool isProvisional = trimmed.contains('?'); // ? indicates provisional rank
        if (num < 10) {
            // Only ranks below 10d can have half-ranks
            hasHalf = trimmed.contains('+');
        }
        // 10d is special: highest amateur rank, no 10d+ exists
        double rankValue = num + (hasHalf ? 0.5 : 0.0);
        // Provisional ranks sort slightly after their equivalent rank
        if (isProvisional) rankValue -= 0.1; // For dan ranks, lower value means higher position
        return QString("b%1").arg((int)((100.0 - rankValue) * 10), 4, 10, QChar('0'));
    } else if (trimmed.contains('k')) {
        int num = trimmed.left(trimmed.indexOf('k')).toInt();
        // Kyu ranks: lower number = stronger, so normal order for ascending sort
        bool hasHalf = trimmed.contains('+');  // + indicates half-rank
        bool isProvisional = trimmed.contains('?'); // ? indicates provisional rank
        double rankValue = num + (hasHalf ? -0.5 : 0.0); // + means stronger (lower effective kyu)
        // Provisional ranks sort slightly after their equivalent rank
        if (isProvisional) rankValue += 0.1;
        return QString("c%1").arg((int)(rankValue * 10), 4, 10, QChar('0'));
    }
    return "d9999"; // Unranked at bottom
}

// Parse IGS "who" command format which has multiple players per line separated by |
// Example: "34 -- chengdu1   1m    2k* |  --   27 Prima     0s   13k*"
bool Q5GoParser::parseWhoFormatLine(const QString &line, Q5GoPlayer &player) {
    // Split on "|" to get individual player sections
    QStringList sections = line.split("|");
    if (sections.isEmpty()) return false;
    
    // Parse the first player in the line (before the first |)
    QString firstSection = sections[0].trimmed();
    QStringList parts = firstSection.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
    
    if (parts.size() < 5) return false;
    
    // Skip if first part is not a number
    bool isNumber;
    parts[0].toInt(&isNumber);
    if (!isNumber) return false;
    
    // WHO format: [number] [flags] [name] [idle] [rank]
    // Example: "34 -- chengdu1 1m 2k*"
    // Flags are 2 characters: each can be ! (looking), X (open off), Q (quiet), S (shout suppress), - (default)
    // Examples: "-!", "-X", "Q-", "S-", "--"
    player.name = parts[2].trimmed();     // Player name
    player.idle = parts[3].trimmed();     // Idle time
    player.rank = parts[4].trimmed();     // Rank
    QString flags = parts[1].trimmed();
    // Ensure flags are exactly 2 characters, default to "--" if invalid
    if (flags.length() != 2) {
        player.info = "--";
    } else {
        player.info = flags;
    }
    
    // Set defaults for WHO format (limited info compared to userlist)
    player.country = "";        // WHO command doesn't provide country
    player.play_str = "--";
    player.obs_str = "--";
    player.extInfo = "";
    player.won = "0";
    player.lost = "0";
    player.nmatch_settings = "";
    player.sort_rk = player.rankToSortKey();
    
    return true;
}