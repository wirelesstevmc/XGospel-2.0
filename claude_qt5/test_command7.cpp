#include <QtCore/QDebug>
#include <QtCore/QRegExp>

int main() {
    QString line = "7 [255] YK [7d*] vs. SOJ [7d*] ( 56 19 0 -5.5 4 I) ( 18)";
    
    QRegExp gamesre("\\[\\s*(\\d+)\\s*\\]\\s+"
        "([^\\s]+)\\s+\\[\\s*([^\\]\\s]*)\\s*\\]\\s+vs\\.\\s+"
        "([^\\s]+)\\s+\\[\\s*([^\\]\\s]*)\\s*\\]\\s+"
        "\\(\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+([\\d\\-.]+)\\s+(\\d+)\\s+([^\\s\\)]+)\\)\\s+"
        "\\(\\s*(\\d+)\\).*");
    
    qDebug() << "Testing line:" << line;
    
    if (gamesre.indexIn(line) != -1) {
        int game_id = gamesre.cap(1).toInt();
        QString white = gamesre.cap(2);
        QString white_rank = gamesre.cap(3);
        QString black = gamesre.cap(4);
        QString black_rank = gamesre.cap(5);
        int move_count = gamesre.cap(6).toInt();
        int board_size = gamesre.cap(7).toInt();
        int handicap_val = gamesre.cap(8).toInt();
        double komi_val = gamesre.cap(9).toDouble();
        int byoyomi_val = gamesre.cap(10).toInt();
        QString game_flags = gamesre.cap(11);
        int observers = gamesre.cap(12).toInt();
        
        qDebug() << "SUCCESS! Parsed:";
        qDebug() << "  game_id:" << game_id;
        qDebug() << "  white:" << white << white_rank;
        qDebug() << "  black:" << black << black_rank;
        qDebug() << "  move_count:" << move_count;
        qDebug() << "  board_size:" << board_size;
        qDebug() << "  handicap:" << handicap_val;
        qDebug() << "  KOMI:" << komi_val;
        qDebug() << "  byoyomi:" << byoyomi_val;
        qDebug() << "  flags:" << game_flags;
        qDebug() << "  observers:" << observers;
        
    } else {
        qDebug() << "FAILED: Regex did not match";
    }
    
    return 0;
}