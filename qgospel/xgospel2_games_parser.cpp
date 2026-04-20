/******************************************************************************
 * XGospel2 IGS Games Parser - Adapted from q5Go
 *
 * Original code from q5Go: https://github.com/bernds/q5Go
 * Adapted for XGospel2 IGS client by Claude AI assistant (2025-2026)
 *
 * This file contains IGS protocol parsers extracted and modified from q5Go
 * source code to handle IGS games list (command 7) responses.
 ******************************************************************************/

#include "xgospel2_games_parser.h"
#include <QDebug>

Q5GoGamesParser::Q5GoGamesParser() {
    // Same regex pattern as q5Go cmd7 parser
    // Format: [num] white [rank] vs. black [rank] (mv sz h komi by fr) (ob)
    // Example: [237] s2884 [ 3d*] vs. csc [ 2d*] (123 19 0 0.5 6 I) ( 0)
    gamesRegex = QRegExp("\\[\\s*(\\d+)\\s*\\]\\s+"
                        "([^\\s]+)\\s+\\[\\s*([^\\]\\s]*)\\s*\\]\\s+" "vs.\\s+"
                        "([^\\s]+)\\s+\\[\\s*([^\\]\\s]*)\\s*\\]\\s+"
                        "\\(\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+([\\d-.]+)\\s+(\\d+)\\s+([^\\s\\)]+)\\)\\s+"
                        "\\(\\s*(\\d+)\\).*");
}

bool Q5GoGamesParser::parseGameLine(const QString& line, Q5GoGameInfo& game) {
    // Skip header lines
    if (line.contains("##")) {
        return false;
    }
    
    // Try to match the games regex
    if (gamesRegex.indexIn(line) == -1) {
        return false;
    }
    
    // Extract all the captured data
    game.nr = gamesRegex.cap(1);
    game.wname = gamesRegex.cap(2);
    game.wrank = gamesRegex.cap(3);
    game.bname = gamesRegex.cap(4);
    game.brank = gamesRegex.cap(5);
    game.mv = gamesRegex.cap(6);
    game.Sz = gamesRegex.cap(7);
    game.H = gamesRegex.cap(8);
    game.K = gamesRegex.cap(9);
    game.By = gamesRegex.cap(10);
    game.FR = gamesRegex.cap(11);
    game.ob = gamesRegex.cap(12);
    
    // Create sortable rank keys for both players
    game.sort_rk_w = game.rankToSortKey(game.wrank);
    game.sort_rk_b = game.rankToSortKey(game.brank);
    
    return true;
}

QString Q5GoGameInfo::rankToSortKey(const QString& rank) {
    // Same rank sorting logic as players: pro="a"+reverse, dan="b"+reverse, kyu="c"+normal
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
        if (num < 10) {
            // Only ranks below 10d can have half-ranks
            hasHalf = trimmed.contains('+');
        }
        // 10d is special: highest amateur rank, no 10d+ exists
        double rankValue = num + (hasHalf ? 0.5 : 0.0);
        return QString("b%1").arg((int)((100.0 - rankValue) * 10), 4, 10, QChar('0'));
    } else if (trimmed.contains('k')) {
        int num = trimmed.left(trimmed.indexOf('k')).toInt();
        // Kyu ranks: lower number = stronger, so normal order for ascending sort
        bool hasHalf = trimmed.contains('+');  // + indicates half-rank
        double rankValue = num + (hasHalf ? -0.5 : 0.0); // + means stronger (lower effective kyu)
        return QString("c%1").arg((int)(rankValue * 10), 4, 10, QChar('0'));
    }
    return "d9999"; // Unranked at bottom
}