/*
 * modern_parser.c - Enhanced parser implementation
 * 
 * This implements modern protocol parsing based on q5Go's parser.cpp
 * while integrating with XGospel's existing architecture.
 */

#include "modern_parser.h"
#include "modern_connect.h"
#include "utils.h"
#include "messages.h"
#include "xgospel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Regular expression patterns */
const char *PATTERN_MOVE = "^15 \\(([BW])\\): ([A-T][0-9]+|PASS)";
const char *PATTERN_GAME_INFO = "^Game ([0-9]+) (\\w+): (\\w+) \\[([0-9]+k|[0-9]+d)\\] vs (\\w+) \\[([0-9]+k|[0-9]+d)\\]";
const char *PATTERN_PLAYER_INFO = "^(\\w+) \\[([0-9]+k|[0-9]+d|NR)\\] (\\w+)";
const char *PATTERN_TIME_INFO = "^([0-9]+):([0-9]+)";

/* Command lookup table - based on q5Go's command structure */
static const ModernCommand modern_commands[] = {
    {1, "LOGIN", HandleLogin, SUPPORT_ALL},
    {2, "BEEP", HandleBeep, SUPPORT_ALL},
    {5, "ERROR", HandleError, SUPPORT_ALL},
    {7, "GAMES", HandleGamesList, SUPPORT_ALL},
    {8, "HELP", HandleHelp, SUPPORT_ALL},
    {9, "INFO", HandleInfo, SUPPORT_ALL},
    {11, "KIBITZ", HandleKibitz, SUPPORT_ALL},
    {14, "MESSAGE", HandleMessage, SUPPORT_ALL},
    {15, "MOVE", HandleMove, SUPPORT_ALL},
    {19, "SAY", HandleSay, SUPPORT_ALL},
    {20, "SCORE", HandleScore, SUPPORT_ALL},
    {21, "SHOUT", HandleShout, SUPPORT_ALL},
    {22, "STATUS", HandleStatus, SUPPORT_ALL},
    {27, "MATCH_REQUEST", HandleMatchRequest, SUPPORT_IGS | SUPPORT_NNGS},
    {42, "TIME_ADDED", HandleTimeAdded, SUPPORT_ALL},
    {48, "SEEK_LIST", HandleSeekList, SUPPORT_IGS | SUPPORT_NNGS},
    {49, "CHANNEL_TELL", HandleChannelTell, SUPPORT_IGS | SUPPORT_NNGS},
    {0, NULL, NULL, 0} /* Terminator */
};

/* Initialize modern parser */
int InitModernParser(void)
{
    printf("Modern parser initialized\n");
    return 1;
}

/* Cleanup modern parser */
void CleanupModernParser(void)
{
    printf("Modern parser cleaned up\n");
}

/* Main parsing function */
MessageType ParseModernMessage(ModernConnection conn, const char *line)
{
    const char *trimmed;
    int cmd_num, i, server_flag;
    
    if (!conn || !line) return MSG_UNKNOWN;
    
    /* Skip empty lines */
    trimmed = line;
    while (*trimmed && isspace(*trimmed)) trimmed++;
    if (!*trimmed) return MSG_UNKNOWN;
    
    /* Extract command number if present */
    cmd_num = -1;
    if (sscanf(trimmed, "%d ", &cmd_num) == 1) {
        /* Find and execute command handler */
        for (i = 0; modern_commands[i].command_name; i++) {
            if (modern_commands[i].command_id == cmd_num) {
                /* Check server compatibility */
                server_flag = 1 << conn->server_type;
                if (modern_commands[i].supported_servers & server_flag) {
                    if (modern_commands[i].handler(conn, line)) {
                        return (MessageType)cmd_num;
                    }
                }
                break;
            }
        }
    }
    
    /* Handle non-numbered messages */
    if (strstr(line, "Login:") || strstr(line, "login:")) {
        HandleLogin(conn, line);
        return MSG_PROMPT;
    }
    
    if (strstr(line, "Password:")) {
        return MSG_PROMPT;
    }
    
    return MSG_UNKNOWN;
}

/* Get message type name for debugging */
const char *GetMessageTypeName(MessageType type)
{
    switch (type) {
        case MSG_PROMPT: return "PROMPT";
        case MSG_BEEP: return "BEEP";
        case MSG_BOARD: return "BOARD";
        case MSG_ERROR: return "ERROR";
        case MSG_GAMES: return "GAMES";
        case MSG_HELP: return "HELP";
        case MSG_INFO: return "INFO";
        case MSG_KIBITZ: return "KIBITZ";
        case MSG_MOVE: return "MOVE";
        case MSG_SAY: return "SAY";
        case MSG_SHOUT: return "SHOUT";
        case MSG_STATUS: return "STATUS";
        default: return "UNKNOWN";
    }
}

/* Utility function to split lines */
char **SplitLine(const char *line, const char *delim, int *count)
{
    int max_parts;
    const char *p;
    char **parts;
    char *line_copy;
    char *token;
    
    if (!line || !delim || !count) return NULL;
    
    /* Count delimiters to estimate parts */
    max_parts = 1;
    p = line;
    while ((p = strstr(p, delim)) != NULL) {
        max_parts++;
        p += strlen(delim);
    }
    
    /* Allocate array for parts */
    parts = malloc(max_parts * sizeof(char*));
    if (!parts) return NULL;
    
    /* Copy line for tokenization */
    line_copy = SafeStrdup(line);
    if (!line_copy) {
        free(parts);
        return NULL;
    }
    
    /* Split the line */
    *count = 0;
    token = strtok(line_copy, delim);
    while (token && *count < max_parts) {
        parts[*count] = SafeStrdup(token);
        (*count)++;
        token = strtok(NULL, delim);
    }
    
    free(line_copy);
    return parts;
}

/* Free split line result */
void FreeSplitLine(char **parts, int count)
{
    int i;
    
    if (!parts) return;
    
    for (i = 0; i < count; i++) {
        SafeFree((void**)&parts[i]);
    }
    free(parts);
}

/* Extract number from string */
int ExtractNumber(const char *str, int *number)
{
    if (!str || !number) return 0;
    return (sscanf(str, "%d", number) == 1);
}

/* Extract time from string (MM:SS format) */
int ExtractTime(const char *str, int *minutes, int *seconds)
{
    if (!str || !minutes || !seconds) return 0;
    return (sscanf(str, "%d:%d", minutes, seconds) == 2);
}

/* Extract rank from string */
int ExtractRank(const char *str, char *rank, size_t rank_size)
{
    const char *p;
    
    if (!str || !rank || rank_size == 0) return 0;
    
    /* Look for patterns like "5k", "2d", "NR" */
    p = str;
    while (*p && !isdigit(*p) && *p != 'N') p++;
    
    if (*p) {
        int len = 0;
        while (*p && len < (int)rank_size - 1 && 
               (isdigit(*p) || *p == 'k' || *p == 'd' || *p == 'N' || *p == 'R')) {
            rank[len++] = *p++;
        }
        rank[len] = '\0';
        return (len > 0);
    }
    
    return 0;
}

/* Command handlers - implementations based on q5Go */

int HandleLogin(ModernConnection conn, const char *line)
{
    /* This is handled by the authentication system */
    return HandleAuthenticationResponse(conn, line);
}

int HandleBeep(ModernConnection conn, const char *line)
{
    /* Send beep to application - could trigger audio notification */
    printf("Server beep received\n");
    return 1;
}

int HandleError(ModernConnection conn, const char *line)
{
    /* Extract error message and display to user */
    const char *msg = strchr(line, ' ');
    if (msg) {
        msg++; /* Skip the space */
        ServerMessage("Server error: %s\n", msg);
    }
    return 1;
}

int HandleGamesList(ModernConnection conn, const char *line)
{
    /* Parse games list - format: "7 [###] white_name [rank] vs black_name [rank]" */
    printf("Games list update: %s\n", line);
    /* TODO: Parse and update games display */
    return 1;
}

int HandleHelp(ModernConnection conn, const char *line)
{
    /* Display help text */
    const char *help = strchr(line, ' ');
    if (help) {
        ServerMessage("%s\n", help + 1);
    }
    return 1;
}

int HandleInfo(ModernConnection conn, const char *line)
{
    /* General information message */
    const char *info = strchr(line, ' ');
    if (info) {
        ServerMessage("Info: %s\n", info + 1);
    }
    return 1;
}

int HandleKibitz(ModernConnection conn, const char *line)
{
    /* Parse kibitz: "11 game_id player_name: message" */
    printf("Kibitz received: %s\n", line);
    /* TODO: Parse and display kibitz */
    return 1;
}

int HandleMessage(ModernConnection conn, const char *line)
{
    /* Private message */
    printf("Message received: %s\n", line);
    /* TODO: Parse and display message */
    return 1;
}

int HandleMove(ModernConnection conn, const char *line)
{
    /* Parse move: "15 (B): A4" or "15 (W): PASS" */
    printf("Move received: %s\n", line);
    /* TODO: Parse and execute move */
    return 1;
}

int HandleSay(ModernConnection conn, const char *line)
{
    /* Say in game */
    printf("Say received: %s\n", line);
    return 1;
}

int HandleScore(ModernConnection conn, const char *line)
{
    /* Score report */
    printf("Score received: %s\n", line);
    return 1;
}

int HandleShout(ModernConnection conn, const char *line)
{
    /* Shout message */
    printf("Shout received: %s\n", line);
    return 1;
}

int HandleStatus(ModernConnection conn, const char *line)
{
    /* Game status update */
    printf("Status received: %s\n", line);
    return 1;
}

int HandleMatchRequest(ModernConnection conn, const char *line)
{
    /* Match request from another player */
    printf("Match request received: %s\n", line);
    return 1;
}

int HandleTimeAdded(ModernConnection conn, const char *line)
{
    /* Time added to game */
    printf("Time added: %s\n", line);
    return 1;
}

int HandleSeekList(ModernConnection conn, const char *line)
{
    /* Seek advertisements list */
    printf("Seek list: %s\n", line);
    return 1;
}

int HandleChannelTell(ModernConnection conn, const char *line)
{
    /* Channel tell message */
    printf("Channel tell: %s\n", line);
    return 1;
}