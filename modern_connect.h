/*
 * modern_connect.h - Enhanced connection handling for XGospel-2.0
 * 
 * Incorporates modern protocol improvements from q5Go while maintaining
 * XGospel's architecture. This provides better server compatibility,
 * authentication handling, and protocol parsing.
 */

#ifndef MODERN_CONNECT_H
#define MODERN_CONNECT_H

#include "connect.h"
#include <time.h>

/* Server types - expanded from q5Go */
typedef enum {
    SERVER_UNKNOWN = 0,
    SERVER_IGS,
    SERVER_NNGS, 
    SERVER_LGS,
    SERVER_PANDANET,
    SERVER_WBG,
    SERVER_TYGEM
} ServerType;

/* Authentication states */
typedef enum {
    AUTH_LOGIN = 0,
    AUTH_PASSWORD,
    AUTH_GUEST,
    AUTH_SESSION,
    AUTH_FAILED
} AuthState;

/* Enhanced connection structure */
typedef struct _ModernConnection {
    /* Base connection */
    Connection base_conn;
    
    /* Server information */
    ServerType server_type;
    char *server_host;
    int server_port;
    
    /* Authentication */
    AuthState auth_state;
    char *username;
    char *password;
    time_t login_time;
    
    /* Protocol handling */
    int protocol_version;
    char *codec_name;          /* Character encoding */
    int supports_unicode;
    int supports_extended_info;
    
    /* Connection state */
    int connection_stable;
    time_t last_keepalive;
    int reconnect_attempts;
    
    /* Buffer management - improved from original */
    char *input_buffer;
    size_t input_buffer_size;
    size_t input_data_length;
    
    /* Command tracking */
    int last_command_id;
    time_t last_command_time;
    
} *ModernConnection;

/* Function prototypes */
extern ModernConnection ModernConnect(const char *site, int port, 
                                     const char *username, const char *password);
extern void ModernDisconnect(ModernConnection conn);
extern int ModernIsConnected(ModernConnection conn);
extern ServerType DetectServerType(const char *response);
extern int HandleAuthenticationResponse(ModernConnection conn, const char *line);
extern int ParseModernProtocol(ModernConnection conn, const char *line);
extern void SendModernCommand(ModernConnection conn, const char *command, ...);
extern void SetServerEncoding(ModernConnection conn, const char *encoding);

/* Protocol command handlers - based on q5Go cmd functions */
extern int HandleServerCommand1(ModernConnection conn, const char *line);  /* Login info */
extern int HandleServerCommand2(ModernConnection conn, const char *line);  /* Beep */
extern int HandleServerCommand5(ModernConnection conn, const char *line);  /* Error messages */
extern int HandleServerCommand7(ModernConnection conn, const char *line);  /* Games list */
extern int HandleServerCommand8(ModernConnection conn, const char *line);  /* Help */
extern int HandleServerCommand9(ModernConnection conn, const char *line);  /* Info */
extern int HandleServerCommand11(ModernConnection conn, const char *line); /* Kibitz */
extern int HandleServerCommand14(ModernConnection conn, const char *line); /* Messages */
extern int HandleServerCommand15(ModernConnection conn, const char *line); /* Moves */
extern int HandleServerCommand19(ModernConnection conn, const char *line); /* Say */
extern int HandleServerCommand20(ModernConnection conn, const char *line); /* Score */
extern int HandleServerCommand21(ModernConnection conn, const char *line); /* Shout */
extern int HandleServerCommand22(ModernConnection conn, const char *line); /* Status */

/* Utility functions */
extern char *SafeStrdup(const char *str);
extern void SafeFree(void **ptr);
extern int IsValidUTF8(const char *str);
extern char *ConvertEncoding(const char *input, const char *from_encoding, 
                            const char *to_encoding);

/* Constants */
#define MODERN_BUFFER_SIZE 8192
#define MAX_RECONNECT_ATTEMPTS 5
#define KEEPALIVE_INTERVAL 300  /* 5 minutes */
#define COMMAND_TIMEOUT 30      /* 30 seconds */

/* Modern protocol command prefixes - expanded from q5Go */
#define CONSOLE_CMD_PREFIX "#>"
#define SERVER_PROMPT_IGS "Login:"
#define SERVER_PROMPT_NNGS "login:"  
#define SERVER_PROMPT_LGS "login:"

#endif /* MODERN_CONNECT_H */