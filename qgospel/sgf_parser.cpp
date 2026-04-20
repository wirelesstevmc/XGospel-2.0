#include "sgf_parser.h"
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include <QDebug>
#include <QSet>

SGFParser::SGFParser()
    : komi(6.5), handicap(0), board_size(19)
{
}

GameNode* SGFParser::parseFile(const QString& filename, QString& error) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = QString("Cannot open file: %1").arg(filename);
        return nullptr;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    return parseContent(content, error);
}

GameNode* SGFParser::parseContent(const QString& sgf_content, QString& error) {
    // Basic SGF validation
    if (!sgf_content.contains("(;") && !sgf_content.startsWith("(;")) {
        error = "Invalid SGF format: must start with (;";
        return nullptr;
    }

    // Create root node
    GameNode* root = new GameNode(nullptr, 0);

    // Remove outer parentheses and split into nodes
    QString content = sgf_content.trimmed();
    if (content.startsWith("(")) {
        content = content.mid(1);
    }
    if (content.endsWith(")")) {
        content.chop(1);
    }

    // Split by semicolons to get individual nodes
    // SGF format: (;property[value]property[value];B[pd];W[dd];...)
    QStringList nodes = content.split(";", QString::SkipEmptyParts);

    if (nodes.isEmpty()) {
        error = "No nodes found in SGF";
        delete root;
        return nullptr;
    }

    // Parse root node (game properties)
    QString root_text = nodes[0];
    parseRootProperties(root_text);

    // Set up initial board state
    GoBoard initial_board;
    initial_board.clear();

    // Parse handicap setup stones (AB[...] properties in root node)
    // Format: AB[pd][dp][pp]... (Add Black stones)
    QRegExp ab_regex("AB((?:\\[[a-z]{2}\\])+)");
    if (ab_regex.indexIn(root_text) != -1) {
        QString coords_list = ab_regex.cap(1);
        QRegExp coord_regex("\\[([a-z]{2})\\]");
        int pos = 0;
        while ((pos = coord_regex.indexIn(coords_list, pos)) != -1) {
            QString coord = coord_regex.cap(1);
            int x, y;
            if (parseCoordinates(coord, x, y)) {
                initial_board.placeStone(x, y, BLACK_STONE);
                qDebug() << "SGF: Loaded handicap stone at" << x << "," << y;
            }
            pos += coord_regex.matchedLength();
        }
    }

    // Parse white setup stones (AW[...] properties in root node)
    QRegExp aw_regex("AW((?:\\[[a-z]{2}\\])+)");
    if (aw_regex.indexIn(root_text) != -1) {
        QString coords_list = aw_regex.cap(1);
        QRegExp coord_regex("\\[([a-z]{2})\\]");
        int pos = 0;
        while ((pos = coord_regex.indexIn(coords_list, pos)) != -1) {
            QString coord = coord_regex.cap(1);
            int x, y;
            if (parseCoordinates(coord, x, y)) {
                initial_board.placeStone(x, y, WHITE_STONE);
                qDebug() << "SGF: Loaded white setup stone at" << x << "," << y;
            }
            pos += coord_regex.matchedLength();
        }
    }

    root->setBoard(initial_board);

    // Current node and board state
    GameNode* current_node = root;
    GoBoard current_board = initial_board.copy();
    int move_number = 0;

    // Territory markers for final position (will be populated from TW/TB properties)
    QMap<QPair<int,int>, StoneColor> territory_markers;

    // Parse remaining nodes (moves)
    for (int i = 1; i < nodes.size(); i++) {
        QString node_text = nodes[i].trimmed();

        // Check for territory markers (TW[...] or TB[...])
        // These appear at the end of the game to mark scored territory
        if (node_text.contains("TW[") || node_text.contains("TB[")) {
            parseTerritory(node_text, territory_markers);
            continue;  // Don't process as a move
        }

        // Extract move (B[...] or W[...])
        // Use word boundary to avoid matching TW[...] or TB[...] territory markers
        QRegExp move_regex("\\b([BW])\\[([a-z]*)\\]");
        if (move_regex.indexIn(node_text) != -1) {
            QString color_char = move_regex.cap(1);
            QString coords = move_regex.cap(2);

            StoneColor color = (color_char == "B") ? BLACK_STONE : WHITE_STONE;
            int x = -1, y = -1;

            // Empty coords means pass
            if (!coords.isEmpty()) {
                if (!parseCoordinates(coords, x, y)) {
                    qDebug() << "Warning: Invalid coordinates" << coords << "in move" << i;
                    continue;
                }
            } else {
                // Pass move
                // qDebug() << "SGF: Pass move" << move_number + 1 << "color:" << color_char;
            }

            move_number++;

            // Create new node for this move
            GameNode* new_node = current_node->addMove(x, y, color);

            if (x >= 0 && y >= 0) {
                // qDebug() << "SGF: Regular move" << move_number << "at (" << x << "," << y << ") color:" << color_char;
            }

            // Update board state and calculate captures
            if (x >= 0 && y >= 0) {
                current_board.placeStone(x, y, color);

                // Calculate and remove captured stones
                StoneColor opponent_color = (color == BLACK_STONE) ? WHITE_STONE : BLACK_STONE;

                // Check all 4 neighbors for captured opponent groups
                int dx[] = {-1, 1, 0, 0};
                int dy[] = {0, 0, -1, 1};

                for (int dir = 0; dir < 4; dir++) {
                    int nx = x + dx[dir];
                    int ny = y + dy[dir];

                    if (nx >= 0 && nx < 19 && ny >= 0 && ny < 19 &&
                        current_board.getStone(nx, ny) == opponent_color) {
                        // Check if this opponent group has no liberties
                        if (countLiberties(current_board, nx, ny) == 0) {
                            removeGroup(current_board, nx, ny);
                        }
                    }
                }

                // Check for suicide (own group has no liberties after placement)
                if (countLiberties(current_board, x, y) == 0) {
                    removeGroup(current_board, x, y);
                }
            }
            new_node->setBoard(current_board.copy());

            // Extract comment if present
            QRegExp comment_regex("C\\[([^\\]]*)\\]");
            if (comment_regex.indexIn(node_text) != -1) {
                QString comment = comment_regex.cap(1);
                new_node->setComment(comment);
            }

            current_node = new_node;
        }
    }

    // Store territory markers in the final node (current_node is now the last move)
    if (!territory_markers.isEmpty() && current_node) {
        current_node->setTerritoryMap(territory_markers);
        // qDebug() << "SGF: Stored" << territory_markers.size() << "territory markers in final position";
    }

    // qDebug() << "SGF parsed:" << move_number << "moves,"
    //          << white_name << "vs" << black_name
    //          << "komi:" << komi << "handicap:" << handicap;

    return root;
}

void SGFParser::parseRootProperties(const QString& node_text) {
    // Extract various SGF properties from root node

    // Player names
    white_name = extractProperty(node_text, "PW");
    black_name = extractProperty(node_text, "PB");

    // Ranks
    white_rank = extractProperty(node_text, "WR");
    black_rank = extractProperty(node_text, "BR");

    // Komi
    QString komi_str = extractProperty(node_text, "KM");
    if (!komi_str.isEmpty()) {
        komi = komi_str.toDouble();
    }

    // Handicap
    QString handicap_str = extractProperty(node_text, "HA");
    if (!handicap_str.isEmpty()) {
        handicap = handicap_str.toInt();
    }

    // Board size
    QString size_str = extractProperty(node_text, "SZ");
    if (!size_str.isEmpty()) {
        board_size = size_str.toInt();
    }

    // Result
    result = extractProperty(node_text, "RE");

    // Date
    date = extractProperty(node_text, "DT");

    // Game name
    game_name = extractProperty(node_text, "GN");
}

QString SGFParser::extractProperty(const QString& text, const QString& property) {
    // Look for property[value] pattern
    QRegExp regex(property + "\\[([^\\]]*)\\]");
    if (regex.indexIn(text) != -1) {
        return regex.cap(1);
    }
    return QString();
}

bool SGFParser::parseCoordinates(const QString& coords, int& x, int& y) {
    // SGF coordinates: "aa" = (0,0), "ab" = (0,1), "pd" = (15,3), etc.
    // Letter 'a' = 0, 'b' = 1, ..., 's' = 18
    if (coords.length() != 2) {
        return false;
    }

    QChar col_char = coords[0];
    QChar row_char = coords[1];

    if (col_char < 'a' || col_char > 's' || row_char < 'a' || row_char > 's') {
        return false;
    }

    x = col_char.toLatin1() - 'a';
    y = row_char.toLatin1() - 'a';

    return (x >= 0 && x < 19 && y >= 0 && y < 19);
}

int SGFParser::countLiberties(const GoBoard& board, int x, int y) {
    StoneColor color = board.getStone(x, y);
    if (color == EMPTY_STONE) {
        return 0;
    }

    // Find all stones in this group using flood fill
    bool visited[19][19] = {{false}};
    QList<QPair<int,int>> group;
    floodFill(board, x, y, color, visited, group);

    // Count unique liberties (empty points adjacent to group)
    QSet<QPair<int,int>> liberties;
    int dx[] = {-1, 1, 0, 0};
    int dy[] = {0, 0, -1, 1};

    for (const auto& stone : group) {
        for (int dir = 0; dir < 4; dir++) {
            int nx = stone.first + dx[dir];
            int ny = stone.second + dy[dir];

            if (nx >= 0 && nx < 19 && ny >= 0 && ny < 19 &&
                board.getStone(nx, ny) == EMPTY_STONE) {
                liberties.insert(QPair<int,int>(nx, ny));
            }
        }
    }

    return liberties.size();
}

void SGFParser::removeGroup(GoBoard& board, int x, int y) {
    StoneColor color = board.getStone(x, y);
    if (color == EMPTY_STONE) {
        return;
    }

    // Find all stones in this group
    bool visited[19][19] = {{false}};
    QList<QPair<int,int>> group;
    floodFill(board, x, y, color, visited, group);

    // Remove all stones in the group
    for (const auto& stone : group) {
        board.removeStone(stone.first, stone.second);
    }
}

void SGFParser::floodFill(const GoBoard& board, int x, int y, StoneColor color,
                          bool visited[19][19], QList<QPair<int,int>>& group) {
    if (x < 0 || x >= 19 || y < 0 || y >= 19) {
        return;
    }

    if (visited[x][y] || board.getStone(x, y) != color) {
        return;
    }

    visited[x][y] = true;
    group.append(QPair<int,int>(x, y));

    // Recursively visit all 4 neighbors
    floodFill(board, x - 1, y, color, visited, group);
    floodFill(board, x + 1, y, color, visited, group);
    floodFill(board, x, y - 1, color, visited, group);
    floodFill(board, x, y + 1, color, visited, group);
}

void SGFParser::parseTerritory(const QString& node_text, QMap<QPair<int,int>, StoneColor>& territory) {
    // Parse TW[aa][bb][cc]... and TB[aa][bb][cc]... territory markers
    // TW = White territory, TB = Black territory

    // Extract all TW properties (white territory)
    QRegExp tw_regex("TW((?:\\[[a-z]*\\])+)");
    if (tw_regex.indexIn(node_text) != -1) {
        QString coords_list = tw_regex.cap(1);
        // Extract individual coordinates [aa][bb][cc]...
        QRegExp coord_regex("\\[([a-z]{2})\\]");
        int pos = 0;
        while ((pos = coord_regex.indexIn(coords_list, pos)) != -1) {
            QString coord = coord_regex.cap(1);
            int x, y;
            if (parseCoordinates(coord, x, y)) {
                territory[QPair<int,int>(x, y)] = WHITE_STONE;
            }
            pos += coord_regex.matchedLength();
        }
    }

    // Extract all TB properties (black territory)
    QRegExp tb_regex("TB((?:\\[[a-z]*\\])+)");
    if (tb_regex.indexIn(node_text) != -1) {
        QString coords_list = tb_regex.cap(1);
        QRegExp coord_regex("\\[([a-z]{2})\\]");
        int pos = 0;
        while ((pos = coord_regex.indexIn(coords_list, pos)) != -1) {
            QString coord = coord_regex.cap(1);
            int x, y;
            if (parseCoordinates(coord, x, y)) {
                territory[QPair<int,int>(x, y)] = BLACK_STONE;
            }
            pos += coord_regex.matchedLength();
        }
    }
}
