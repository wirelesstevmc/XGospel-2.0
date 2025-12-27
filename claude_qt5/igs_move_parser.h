#ifndef IGS_MOVE_PARSER_H
#define IGS_MOVE_PARSER_H

#include <QString>
#include <QRegExp>
#include "board_window.h"

class IGSMoveParser {
public:
    IGSMoveParser();
    
    bool parseMoveLine(const QString& line, GameMove& move);
    bool parseObserveStart(const QString& line, int& game_id, QString& white, QString& black);
    bool parseGameInfo(const QString& line, int& game_id, QString& white_name, QString& black_name, 
                       int& white_captures, int& black_captures, int& white_time, int& black_time,
                       int& white_byo_moves, int& black_byo_moves, QString& game_type);
    bool isObserveCommand(const QString& line);
    
    // Handicap stone placement
    static QList<QPair<int, int>> getHandicapPositions(int handicap_count);
    
private:
    QRegExp moveRegex;
    QRegExp observeRegex;
    
    StoneColor charToColor(const QString& color_char);
    void parseCoordinates(const QString& coord_str, int& x, int& y);
    QString extractGameNumber(const QString& line);
};

#endif // IGS_MOVE_PARSER_H