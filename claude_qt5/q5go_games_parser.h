#ifndef Q5GO_GAMES_PARSER_H
#define Q5GO_GAMES_PARSER_H

#include <QString>
#include <QStringList>
#include <QRegExp>

struct Q5GoGameInfo {
    QString nr;           // Game number
    QString wname;        // White player name
    QString wrank;        // White player rank
    QString bname;        // Black player name
    QString brank;        // Black player rank
    QString mv;           // Move number
    QString Sz;           // Board size
    QString H;            // Handicap
    QString K;            // Komi
    QString By;           // Byoyomi
    QString FR;           // Final result
    QString ob;           // Observers count
    QString sort_rk_w;    // Sortable white rank key
    QString sort_rk_b;    // Sortable black rank key
    
    QString rankToSortKey(const QString& rank);
};

class Q5GoGamesParser {
public:
    Q5GoGamesParser();
    bool parseGameLine(const QString& line, Q5GoGameInfo& game);
    
private:
    QRegExp gamesRegex;
};

#endif // Q5GO_GAMES_PARSER_H