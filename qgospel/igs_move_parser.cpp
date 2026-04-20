#include "igs_move_parser.h"
#include <QDebug>

IGSMoveParser::IGSMoveParser() {
    // IGS move format from q5Go: "15 144(B): B12"
    // Pattern: move_number(color): coordinate
    // More specific: coordinate should be 1-3 chars starting with A-T, followed by 1-2 digits
    moveRegex = QRegExp("(\\d+)\\s*\\(([BW])\\):\\s*(.+)\\s*$");
    
    // Observe start format: various patterns from IGS
    observeRegex = QRegExp("Observing\\s+game\\s+(\\d+)");
}

bool IGSMoveParser::parseMoveLine(const QString& line, GameMove& move) {
    qDebug() << "DEBUG: parseMoveLine input:" << line;
    
    // Enhanced debugging for pass moves
    if (line.contains("PASS", Qt::CaseInsensitive) || line.contains("Pass")) {
        qDebug() << "*** PASS MOVE CANDIDATE in parseMoveLine:" << line;
    }
    
    // Check for handicap stone placement first
    if (line.contains("Handicap", Qt::CaseInsensitive)) {
        // Extract handicap number: "0(B): Handicap 2" -> handicap = 2
        QRegExp handicapRegex("(\\d+)\\s*\\(([BW])\\):\\s*Handicap\\s+(\\d+)");
        if (handicapRegex.indexIn(line) != -1) {
            move.move_number = handicapRegex.cap(1).toInt();
            move.color = BLACK_STONE; // Handicap stones are always black
            move.game_id = -1; // Will be set by caller
            move.time_info = "";
            move.captured = "";
            
            // Signal that this is a handicap placement, not a regular move
            // We'll use special coordinates to indicate handicap
            int handicap_count = handicapRegex.cap(3).toInt();
            move.x = -2; // Special flag for handicap
            move.y = handicap_count; // Number of handicap stones
            
            qDebug() << "DEBUG: Detected handicap placement:" << handicap_count << "stones";
            return true;
        }
        return false;
    }
    
    if (moveRegex.indexIn(line) == -1) {
        qDebug() << "DEBUG: Regex failed to match line:" << line;
        // Special debugging for pass moves that fail regex
        if (line.contains("PASS", Qt::CaseInsensitive) || line.contains("Pass")) {
            qDebug() << "*** REGEX FAILED FOR PASS MOVE:" << line;
        }
        return false;
    }
    
    // From regex: (\\d+)\\s*\\(([BW])\\):\\s*([^\\s].*)
    // Cap 1: move number
    // Cap 2: color (B or W)  
    // Cap 3: coordinate (like "B12")
    
    move.move_number = moveRegex.cap(1).toInt();
    QString color_char = moveRegex.cap(2);
    QString coordinates = moveRegex.cap(3).trimmed();
    
    // Convert B/W to stone color
    move.color = (color_char == "B") ? BLACK_STONE : WHITE_STONE;
    
    // Check for Pass moves - handle various IGS pass formats
    QString coord_lower = coordinates.toLower().trimmed();
    if (coord_lower == "pass" || coord_lower == "Pass" || coord_lower == "PASS" || 
        coordinates.contains("Pass", Qt::CaseInsensitive) || coordinates.contains("PASS")) {
        move.x = -1;
        move.y = -1;
        qDebug() << "DEBUG: Pass move detected from coordinates:" << coordinates;
        return true;
    }
    
    // Check for stone removal notation during counting phase (e.g., "R8 R7" or "D7 B4 B5 C4 C5 E6 C6 D5 D6")
    QStringList coords = coordinates.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    if (coords.size() >= 2) {
        // First coordinate is the move, rest are captured stones
        parseCoordinates(coords[0], move.x, move.y);
        
        // Process all captured stone coordinates
        QStringList captured_coords;
        for (int i = 1; i < coords.size(); i++) {
            int cap_x, cap_y;
            parseCoordinates(coords[i], cap_x, cap_y);
            captured_coords << QString("%1,%2").arg(cap_x).arg(cap_y);
        }
        move.captured = captured_coords.join(";");
        
        qDebug() << "DEBUG: Multi-capture move detected - played at" << coords[0] 
                 << "captured" << (coords.size()-1) << "stones:" << captured_coords;
        return true;
    }
    
    // Regular single coordinate move
    parseCoordinates(coordinates, move.x, move.y);
    
    // For now, we don't have game_id from the move line itself
    // It will be set by the calling code based on which game is being observed
    move.game_id = -1;  // Will be filled in by caller
    
    // Extract additional info if available
    move.time_info = "";
    move.captured = "";
    
    return true;
}

bool IGSMoveParser::parseObserveStart(const QString& line, int& game_id, QString& white, QString& black) {
    // Look for pattern: "9 Observing game 123 (white vs. black) :"
    QRegExp observePattern("Observing\\s+game\\s+(\\d+)\\s+\\(([^\\s]+)\\s+vs\\.\\s+([^\\)]+)\\)");
    
    if (observePattern.indexIn(line) != -1) {
        game_id = observePattern.cap(1).toInt();
        white = observePattern.cap(2);
        black = observePattern.cap(3);
        return true;
    }
    
    return false;
}

bool IGSMoveParser::parseGameInfo(const QString& line, int& game_id, QString& white_name, QString& black_name, 
                                  int& white_captures, int& black_captures, int& white_time, int& black_time,
                                  int& white_byo_moves, int& black_byo_moves, QString& game_type) {
    // Parse lines like: 
    // "15 Game 560 I: PandaBot1 (0 859 10) vs youi88 (0 541 3)"     (rated game)
    // "15 Game 123 FI: player1 (0 300 25) vs player2 (1 280 25)"    (free game)  
    // "15 Game 456 TI: teacher (0 300 25) vs student (0 300 25)"    (teaching game)
    // Format: "15 Game <game_id> (FI|I|TI): <white_name> (<white_captures> <white_time> <white_byo_moves>) vs <black_name> (<black_captures> <black_time> <black_byo_moves>)"
    
    // Pattern handles optional spaces around parentheses: "weakkyu (0 60 -1)" or "weakkyu(0 60 -1)"
    // Format: "15 Game 10 I: woodnstone (0 60 -1) vs weakkyu (0 60 -1)" - note space after colon
    QRegExp gameInfoPattern("^15 Game (\\d+) ([^:]+):\\s+([^\\(]+)\\s*\\((\\d+) (\\d+) (-?\\d+)\\) vs ([^\\(]+)\\s*\\((\\d+) (\\d+) (-?\\d+)\\)");

    qDebug() << ">>> REGEX TEST: Trying to parse Command 15 line:" << line;
    if (gameInfoPattern.indexIn(line) != -1) {
        qDebug() << ">>> REGEX SUCCESS: Pattern matched!";
        game_id = gameInfoPattern.cap(1).toInt();
        QString type_indicator = gameInfoPattern.cap(2);
        white_name = gameInfoPattern.cap(3).trimmed();
        white_captures = gameInfoPattern.cap(4).toInt();
        white_time = gameInfoPattern.cap(5).toInt();
        white_byo_moves = gameInfoPattern.cap(6).toInt();
        black_name = gameInfoPattern.cap(7).trimmed();
        black_captures = gameInfoPattern.cap(8).toInt();
        black_time = gameInfoPattern.cap(9).toInt();
        black_byo_moves = gameInfoPattern.cap(10).toInt();
        
        // Determine game type based on protocol indicator
        // Trim whitespace since regex capture might include spaces
        QString trimmed_indicator = type_indicator.trimmed();

        if (trimmed_indicator == "FI") {
            game_type = "Free";
        } else if (trimmed_indicator == "TI") {
            game_type = "Teaching";
        } else if (trimmed_indicator == "I") {
            game_type = "Rated";
        } else {
            game_type = QString("Unknown(%1)").arg(trimmed_indicator);
            qDebug() << ">>> RESEARCH: Found unknown game type indicator: [" << trimmed_indicator << "] in line:" << line;
        }
        qDebug() << "🎯 Command 15 Game Type: indicator=[" << trimmed_indicator << "] -> type=" << game_type;
        
        qDebug() << "DEBUG: Parsed game info - Game:" << game_id 
                 << "Type:" << game_type << "(" << type_indicator << ")"
                 << "White:" << white_name << "captures:" << white_captures << "time:" << white_time << "byo:" << white_byo_moves
                 << "Black:" << black_name << "captures:" << black_captures << "time:" << black_time << "byo:" << black_byo_moves;

        return true;
    } else {
        qDebug() << ">>> REGEX FAILED: Pattern did not match Command 15 line!";
    }

    return false;
}

bool IGSMoveParser::isObserveCommand(const QString& line) {
    return line.contains("observe", Qt::CaseInsensitive) || 
           line.contains("Observing game", Qt::CaseInsensitive);
}

StoneColor IGSMoveParser::charToColor(const QString& color_char) {
    QString lower = color_char.toLower();
    
    // This is a simplification - in practice, we'd need more context
    // to determine if "white" means white player or white stone
    // For now, we'll assume alternating moves starting with black
    static bool last_was_black = false;
    last_was_black = !last_was_black;
    return last_was_black ? BLACK_STONE : WHITE_STONE;
}

void IGSMoveParser::parseCoordinates(const QString& coord_str, int& x, int& y) {
    if (coord_str.length() < 2) {
        x = y = -1;
        return;
    }
    
    // Parse format like "Q16" or "A1" 
    QChar letter = coord_str[0].toUpper();
    QString number_str = coord_str.mid(1);
    
    // Convert letter to x-coordinate (A=0, B=1, ... skipping I)
    // Standard Go: A=0, B=1, C=2, D=3, E=4, F=5, G=6, H=7, J=8, K=9, L=10, M=11, N=12, O=13, P=14, Q=15, R=16, S=17, T=18
    if (letter >= 'A' && letter <= 'H') {
        x = letter.unicode() - 'A';  // A=0, B=1, ..., H=7
    } else if (letter >= 'J' && letter <= 'T') {
        x = letter.unicode() - 'A' - 1;  // J=8, K=9, ..., T=18 (skip I)
    } else {
        x = -1;
    }
    
    // Convert number to y-coordinate 
    // Standard Go: 1=bottom of board, 19=top of board
    // For array indexing: y=0 should be top, y=18 should be bottom
    int board_number = number_str.toInt();
    if (board_number >= 1 && board_number <= 19) {
        y = 19 - board_number;  // 1->18, 2->17, ..., 19->0
    } else {
        y = -1;
    }
    
    // Debug output
    qDebug() << "parseCoordinates:" << coord_str << "-> (" << x << "," << y << ")";
}

QString IGSMoveParser::extractGameNumber(const QString& line) {
    QRegExp gameNumRegex("(\\d+)");
    if (gameNumRegex.indexIn(line) != -1) {
        return gameNumRegex.cap(1);
    }
    return "";
}

QList<QPair<int, int>> IGSMoveParser::getHandicapPositions(int handicap_count) {
    QList<QPair<int, int>> positions;
    
    // Standard handicap positions on 19x19 board (0-based coordinates)
    // Star points (hoshi) at: (3,3), (9,3), (15,3), (3,9), (9,9), (15,9), (3,15), (9,15), (15,15)
    
    switch (handicap_count) {
    case 2:
        positions << QPair<int, int>(15, 3) << QPair<int, int>(3, 15);  // Q16 (upper right), D4 (lower left)
        break;
    case 3:
        positions << QPair<int, int>(15, 3) << QPair<int, int>(3, 15) << QPair<int, int>(3, 3);  // + D16 (upper left)
        break;
    case 4:
        positions << QPair<int, int>(3, 3) << QPair<int, int>(15, 3) << QPair<int, int>(3, 15) << QPair<int, int>(15, 15);  // All corners: D16, Q16, D4, Q4
        break;
    case 5:
        positions << QPair<int, int>(3, 3) << QPair<int, int>(15, 3) << QPair<int, int>(3, 15) << QPair<int, int>(15, 15) << QPair<int, int>(9, 9);  // Corners + center
        break;
    case 6:
        positions << QPair<int, int>(3, 3) << QPair<int, int>(15, 3) << QPair<int, int>(3, 15) << QPair<int, int>(15, 15) 
                 << QPair<int, int>(3, 9) << QPair<int, int>(15, 9);  // Corners + left/right sides
        break;
    case 7:
        positions << QPair<int, int>(3, 3) << QPair<int, int>(15, 3) << QPair<int, int>(3, 15) << QPair<int, int>(15, 15) 
                 << QPair<int, int>(3, 9) << QPair<int, int>(15, 9) << QPair<int, int>(9, 9);  // 6 + center
        break;
    case 8:
        positions << QPair<int, int>(3, 3) << QPair<int, int>(15, 3) << QPair<int, int>(3, 15) << QPair<int, int>(15, 15) 
                 << QPair<int, int>(3, 9) << QPair<int, int>(15, 9) << QPair<int, int>(9, 3) << QPair<int, int>(9, 15);  // Corners + all sides
        break;
    case 9:
        positions << QPair<int, int>(3, 3) << QPair<int, int>(15, 3) << QPair<int, int>(3, 15) << QPair<int, int>(15, 15) 
                 << QPair<int, int>(3, 9) << QPair<int, int>(15, 9) << QPair<int, int>(9, 3) << QPair<int, int>(9, 15) << QPair<int, int>(9, 9);  // All hoshi points
        break;
    default:
        qDebug() << "WARNING: Invalid handicap count:" << handicap_count;
        break;
    }
    
    return positions;
}