/******************************************************************************
 * XGospel2 IGS Parser - Adapted from q5Go
 *
 * Original code from q5Go: https://github.com/bernds/q5Go
 * Adapted for XGospel2 IGS client by Claude AI assistant (2025-2026)
 *
 * This file contains IGS protocol parsers extracted and modified from q5Go
 * source code to handle IGS player list (userlist/WHO) responses.
 ******************************************************************************/

#ifndef XGOSPEL2_PARSER_H
#define XGOSPEL2_PARSER_H

#include <QString>
#include <vector>

// Player structure - extracted from q5Go source
struct Q5GoPlayer
{
    QString info;
    QString name;
    QString idle;
    QString rank;
    QString play_str;
    QString obs_str;
    QString extInfo;
    QString won;
    QString lost;
    QString country;
    QString nmatch_settings;
    QString rated;
    QString address;
    QString mark;
    QString sort_rk;
    QString playing;
    QString observing;
    bool nmatch;
    
    // Helper function for rank sorting
    QString rankToSortKey() const;
};

// Extracted from q5Go source - Game structure  
struct Q5GoGame
{
    QString nr;
    QString wname;
    QString wrank;
    QString bname;
    QString brank;
    QString mv;
    QString Sz;
    QString H;
    QString K;
    QString By;
    QString FR;
    QString ob;
    QString sort_rk_w, sort_rk_b;
};

// Parser class to handle IGS responses
class Q5GoParser
{
public:
    Q5GoParser();

    // Parse a single line from IGS
    bool parseGameLine(const QString &line, Q5GoGame &game);
    bool parsePlayerLine(const QString &line, Q5GoPlayer &player);
    std::vector<Q5GoPlayer> parsePlayersLine(const QString &line);
    bool parseWhoFormatLine(const QString &line, Q5GoPlayer &player);

private:
    // From q5Go cmd7 - games parsing
    bool parseIGSGame(const QString &line, Q5GoGame &game);

    // Player parsing helpers
    bool parseIGSPlayer(const QString &line, Q5GoPlayer &player);
};

#endif // XGOSPEL2_PARSER_H