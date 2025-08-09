/*
 * modern_parser.h - Enhanced parser for modern server protocols
 * 
 * This extends the existing flex/bison parser with modern protocol features
 * from q5Go while maintaining compatibility with the original gointer.y
 */

#ifndef MODERN_PARSER_H
#define MODERN_PARSER_H

#include "modern_connect.h"

/* Enhanced command structure */
typedef struct {
    int command_id;
    const char *command_name;
    int (*handler)(ModernConnection conn, const char *line);
    int supported_servers;  /* Bitmask of supported server types */
} ModernCommand;

/* Server support flags */
#define SUPPORT_IGS     (1 << SERVER_IGS)
#define SUPPORT_NNGS    (1 << SERVER_NNGS)
#define SUPPORT_LGS     (1 << SERVER_LGS)
#define SUPPORT_PANDANET (1 << SERVER_PANDANET)
#define SUPPORT_ALL     0xFFFF

/* Message type enumeration - extended from q5Go */
typedef enum {
    MSG_UNKNOWN = 0,
    MSG_PROMPT,         /* 1 */
    MSG_BEEP,           /* 2 */
    MSG_BOARD,          /* 3 */
    MSG_DOWN,           /* 4 */
    MSG_ERROR,          /* 5 */
    MSG_FILE,           /* 6 */
    MSG_GAMES,          /* 7 */
    MSG_HELP,           /* 8 */
    MSG_INFO,           /* 9 */
    MSG_LAST,           /* 10 */
    MSG_KIBITZ,         /* 11 */
    MSG_LOAD,           /* 12 */
    MSG_LOOK,           /* 13 */
    MSG_MESSAGE,        /* 14 */
    MSG_MOVE,           /* 15 */
    MSG_OBSERVE,        /* 16 */
    MSG_REFRESH,        /* 17 */
    MSG_SAVED,          /* 18 */
    MSG_SAY,            /* 19 */
    MSG_SCORE,          /* 20 */
    MSG_SHOUT,          /* 21 */
    MSG_STATUS,         /* 22 */
    MSG_TELL,           /* 23 */
    MSG_WHO,            /* 24 */
    /* Extended message types */
    MSG_MATCH_REQUEST = 27,
    MSG_MATCH_CREATE = 28,
    MSG_UNDO_REQUEST = 32,
    MSG_TIME_ADDED = 42,
    MSG_SEEK_LIST = 48,
    MSG_CHANNEL_TELL = 49,
    MSG_STATS_PLAYER = 63
} MessageType;

/* Function prototypes */
extern int InitModernParser(void);
extern void CleanupModernParser(void);
extern MessageType ParseModernMessage(ModernConnection conn, const char *line);
extern const char *GetMessageTypeName(MessageType type);

/* Enhanced command handlers - implementations of the q5Go cmd functions */
extern int HandleLogin(ModernConnection conn, const char *line);
extern int HandleBeep(ModernConnection conn, const char *line);
extern int HandleError(ModernConnection conn, const char *line);
extern int HandleGamesList(ModernConnection conn, const char *line);
extern int HandleHelp(ModernConnection conn, const char *line);
extern int HandleInfo(ModernConnection conn, const char *line);
extern int HandleKibitz(ModernConnection conn, const char *line);
extern int HandleMessage(ModernConnection conn, const char *line);
extern int HandleMove(ModernConnection conn, const char *line);
extern int HandleSay(ModernConnection conn, const char *line);
extern int HandleScore(ModernConnection conn, const char *line);
extern int HandleShout(ModernConnection conn, const char *line);
extern int HandleStatus(ModernConnection conn, const char *line);
extern int HandleMatchRequest(ModernConnection conn, const char *line);
extern int HandleTimeAdded(ModernConnection conn, const char *line);
extern int HandleSeekList(ModernConnection conn, const char *line);
extern int HandleChannelTell(ModernConnection conn, const char *line);

/* Utility functions for parsing */
extern char **SplitLine(const char *line, const char *delim, int *count);
extern void FreeSplitLine(char **parts, int count);
extern int ExtractNumber(const char *str, int *number);
extern int ExtractTime(const char *str, int *minutes, int *seconds);
extern int ExtractRank(const char *str, char *rank, size_t rank_size);

/* Regular expression patterns for common parsing */
extern const char *PATTERN_MOVE;
extern const char *PATTERN_GAME_INFO;
extern const char *PATTERN_PLAYER_INFO;
extern const char *PATTERN_TIME_INFO;

#endif /* MODERN_PARSER_H */