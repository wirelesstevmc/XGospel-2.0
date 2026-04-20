#ifndef SGF_PARSER_H
#define SGF_PARSER_H

#include <QString>
#include <QMap>
#include "game_tree.h"

// SGF Parser - converts SGF files to GameNode tree structure
// Based on q5Go's SGF parsing approach
class SGFParser {
public:
    SGFParser();

    // Parse an SGF file and return the root node of the game tree
    // Returns nullptr on failure
    GameNode* parseFile(const QString& filename, QString& error);

    // Parse SGF content from a string
    GameNode* parseContent(const QString& sgf_content, QString& error);

    // Get game metadata after parsing
    QString getPlayerWhite() const { return white_name; }
    QString getPlayerBlack() const { return black_name; }
    QString getWhiteRank() const { return white_rank; }
    QString getBlackRank() const { return black_rank; }
    double getKomi() const { return komi; }
    int getHandicap() const { return handicap; }
    int getBoardSize() const { return board_size; }
    QString getGameResult() const { return result; }
    QString getGameDate() const { return date; }
    QString getGameName() const { return game_name; }

private:
    // Parse a single SGF node (properties between ; and next ;)
    bool parseNode(const QString& node_text, GameNode* parent, GameNode*& new_node);

    // Extract property value from SGF property like "B[pd]"
    QString extractProperty(const QString& text, const QString& property);

    // Parse SGF coordinates like "pd" to board coordinates (15, 3)
    bool parseCoordinates(const QString& coords, int& x, int& y);

    // Parse root node properties (game metadata)
    void parseRootProperties(const QString& node_text);

    // Parse territory markers (TW/TB properties)
    void parseTerritory(const QString& node_text, QMap<QPair<int,int>, StoneColor>& territory);

    // Capture calculation helpers
    int countLiberties(const GoBoard& board, int x, int y);
    void removeGroup(GoBoard& board, int x, int y);
    void floodFill(const GoBoard& board, int x, int y, StoneColor color,
                   bool visited[19][19], QList<QPair<int,int>>& group);

    // Game metadata
    QString white_name;
    QString black_name;
    QString white_rank;
    QString black_rank;
    double komi;
    int handicap;
    int board_size;
    QString result;
    QString date;
    QString game_name;
};

#endif // SGF_PARSER_H
