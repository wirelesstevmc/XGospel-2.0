/*
 * modern_connect.c - Enhanced connection handling for XGospel-2.0
 * 
 * Based on protocol improvements from q5Go, this provides:
 * - Better server type detection
 * - Improved authentication handling  
 * - Enhanced protocol parsing
 * - UTF-8/encoding support
 * - Robust error handling
 */

#include "modern_connect.h"
#include "connect.h"
#include "utils.h"
#include "xgospel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>

#ifdef HAVE_REGEX_H
#include <regex.h>
#endif

/* Global connection - maintaining XGospel architecture */
static ModernConnection modern_conn = NULL;

/* Server detection patterns - from q5Go parser.cpp */
static const struct {
    const char *pattern;
    ServerType type;
} server_patterns[] = {
    {"IGS entry on", SERVER_IGS},
    {"NNGS #", SERVER_NNGS}, 
    {"LGS #", SERVER_LGS},
    {"pandanet", SERVER_PANDANET},
    {"WBG", SERVER_WBG},
    {"Tygem", SERVER_TYGEM},
    {NULL, SERVER_UNKNOWN}
};

/* Utility Functions */
char *SafeStrdup(const char *str)
{
    if (!str) return NULL;
    
    char *result = malloc(strlen(str) + 1);
    if (!result) {
        fprintf(stderr, "Memory allocation failed in SafeStrdup\n");
        return NULL;
    }
    strcpy(result, str);
    return result;
}

void SafeFree(void **ptr)
{
    if (ptr && *ptr) {
        free(*ptr);
        *ptr = NULL;
    }
}

/* Basic UTF-8 validation - simplified version */
int IsValidUTF8(const char *str)
{
    if (!str) return 0;
    
    /* Simple ASCII check for now - full UTF-8 validation would be more complex */
    while (*str) {
        if ((*str & 0x80) != 0) {
            /* Non-ASCII character found, assume valid UTF-8 for now */
            return 1;
        }
        str++;
    }
    return 1; /* All ASCII is valid UTF-8 */
}

/* Server Type Detection */
ServerType DetectServerType(const char *response)
{
    if (!response) return SERVER_UNKNOWN;
    
    for (int i = 0; server_patterns[i].pattern; i++) {
        if (strstr(response, server_patterns[i].pattern)) {
            return server_patterns[i].type;
        }
    }
    
    return SERVER_UNKNOWN;
}

/* Enhanced Connection Creation */
ModernConnection ModernConnect(const char *site, int port, 
                              const char *username, const char *password)
{
    if (!site) return NULL;
    
    /* Allocate modern connection structure */
    ModernConnection conn = calloc(1, sizeof(struct _ModernConnection));
    if (!conn) {
        fprintf(stderr, "Failed to allocate ModernConnection\n");
        return NULL;
    }
    
    /* Initialize connection */
    conn->base_conn = Connect(site, port);
    if (!conn->base_conn) {
        SafeFree((void**)&conn);
        return NULL;
    }
    
    /* Set up modern connection fields */
    conn->server_type = SERVER_UNKNOWN;
    conn->server_host = SafeStrdup(site);
    conn->server_port = port;
    conn->auth_state = AUTH_LOGIN;
    conn->username = SafeStrdup(username);
    conn->password = SafeStrdup(password);
    conn->login_time = time(NULL);
    conn->protocol_version = 1;
    conn->codec_name = SafeStrdup("UTF-8");
    conn->supports_unicode = 1;
    conn->connection_stable = 0;
    conn->last_keepalive = time(NULL);
    conn->reconnect_attempts = 0;
    
    /* Initialize input buffer */
    conn->input_buffer_size = MODERN_BUFFER_SIZE;
    conn->input_buffer = malloc(conn->input_buffer_size);
    if (!conn->input_buffer) {
        ModernDisconnect(conn);
        return NULL;
    }
    conn->input_data_length = 0;
    
    /* Store global reference */
    modern_conn = conn;
    
    return conn;
}

/* Enhanced Disconnection */
void ModernDisconnect(ModernConnection conn)
{
    if (!conn) return;
    
    /* Clean up base connection first */
    if (conn->base_conn && ConnectedP(conn->base_conn)) {
        /* Send clean disconnect if possible */
        if (conn->auth_state == AUTH_SESSION) {
            SendCommand(conn->base_conn, NULL, "quit");
        }
    }
    
    /* Clean up allocated memory */
    SafeFree((void**)&conn->server_host);
    SafeFree((void**)&conn->username);
    SafeFree((void**)&conn->password);
    SafeFree((void**)&conn->codec_name);
    SafeFree((void**)&conn->input_buffer);
    
    /* Clear global reference */
    if (modern_conn == conn) {
        modern_conn = NULL;
    }
    
    free(conn);
}

/* Connection Status Check */
int ModernIsConnected(ModernConnection conn)
{
    if (!conn || !conn->base_conn) return 0;
    return ConnectedP(conn->base_conn) && (conn->auth_state == AUTH_SESSION);
}

/* Authentication Response Handler - based on q5Go igsconnection.cpp */
int HandleAuthenticationResponse(ModernConnection conn, const char *line)
{
    if (!conn || !line) return 0;
    
    switch (conn->auth_state) {
        case AUTH_LOGIN:
            if (strstr(line, "Login:") || strstr(line, "login:")) {
                if (conn->username && strlen(conn->username) > 0) {
                    SendCommand(conn->base_conn, NULL, "%s", conn->username);
                    conn->auth_state = (conn->password && strlen(conn->password) > 0) 
                                      ? AUTH_PASSWORD : AUTH_GUEST;
                    return 1;
                }
            }
            break;
            
        case AUTH_PASSWORD:
            if (strstr(line, "Password:") || strstr(line, "1 1")) {
                if (conn->password && strlen(conn->password) > 0) {
                    SendCommand(conn->base_conn, NULL, "%s", conn->password);
                    conn->auth_state = AUTH_SESSION;
                    conn->connection_stable = 1;
                    return 1;
                }
            } else if (strstr(line, "guest account")) {
                conn->auth_state = AUTH_SESSION;
                conn->connection_stable = 1;
                return 1;
            }
            break;
            
        case AUTH_GUEST:
            /* Guest login completed */
            conn->auth_state = AUTH_SESSION;
            conn->connection_stable = 1;
            return 1;
            
        case AUTH_FAILED:
            /* Authentication failed - may need to reconnect */
            return 0;
            
        case AUTH_SESSION:
            /* Already authenticated */
            return 1;
    }
    
    /* Check for authentication failure */
    if (strstr(line, "wrong password") || strstr(line, "invalid login") ||
        strstr(line, "login incorrect")) {
        conn->auth_state = AUTH_FAILED;
        return 0;
    }
    
    return 0;
}

/* Modern Protocol Parser - based on q5Go parser structure */
int ParseModernProtocol(ModernConnection conn, const char *line)
{
    if (!conn || !line) return 0;
    
    /* Skip empty lines */
    if (strlen(line) == 0) return 0;
    
    /* Detect server type if unknown */
    if (conn->server_type == SERVER_UNKNOWN) {
        ServerType detected = DetectServerType(line);
        if (detected != SERVER_UNKNOWN) {
            conn->server_type = detected;
            printf("Detected server type: %d\n", detected);
        }
    }
    
    /* Handle authentication first */
    if (conn->auth_state != AUTH_SESSION) {
        if (HandleAuthenticationResponse(conn, line)) {
            return 1; /* Authentication handled */
        }
    }
    
    /* Skip console commands */
    if (strstr(line, CONSOLE_CMD_PREFIX)) {
        return 0;
    }
    
    /* Check for connection status messages */
    if (strstr(line, "Connection closed")) {
        conn->connection_stable = 0;
        return 0;
    }
    
    /* Parse numbered commands - extract command number */
    int cmd_num = -1;
    if (sscanf(line, "%d ", &cmd_num) == 1) {
        switch (cmd_num) {
            case 1: return HandleServerCommand1(conn, line);
            case 2: return HandleServerCommand2(conn, line);
            case 5: return HandleServerCommand5(conn, line);
            case 7: return HandleServerCommand7(conn, line);
            case 8: return HandleServerCommand8(conn, line);
            case 9: return HandleServerCommand9(conn, line);
            case 11: return HandleServerCommand11(conn, line);
            case 14: return HandleServerCommand14(conn, line);
            case 15: return HandleServerCommand15(conn, line);
            case 19: return HandleServerCommand19(conn, line);
            case 20: return HandleServerCommand20(conn, line);
            case 21: return HandleServerCommand21(conn, line);
            case 22: return HandleServerCommand22(conn, line);
            default:
                /* Unknown command - pass through to original parser */
                return 0;
        }
    }
    
    return 0; /* Not handled by modern parser */
}

/* Enhanced Command Sending */
void SendModernCommand(ModernConnection conn, const char *command, ...)
{
    if (!conn || !command || !conn->base_conn) return;
    
    va_list args;
    va_start(args, command);
    
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), command, args);
    
    /* Update command tracking */
    conn->last_command_id++;
    conn->last_command_time = time(NULL);
    
    /* Send through base connection */
    SendCommand(conn->base_conn, NULL, "%s", buffer);
    
    va_end(args);
}

/* Encoding Support */
void SetServerEncoding(ModernConnection conn, const char *encoding)
{
    if (!conn || !encoding) return;
    
    SafeFree((void**)&conn->codec_name);
    conn->codec_name = SafeStrdup(encoding);
    
    /* Update unicode support flag */
    conn->supports_unicode = (strcmp(encoding, "UTF-8") == 0 || 
                             strcmp(encoding, "UTF-16") == 0);
}

/* Placeholder command handlers - to be implemented based on specific needs */
int HandleServerCommand1(ModernConnection conn, const char *line) { return 0; }
int HandleServerCommand2(ModernConnection conn, const char *line) { return 0; }
int HandleServerCommand5(ModernConnection conn, const char *line) { return 0; }
int HandleServerCommand7(ModernConnection conn, const char *line) { return 0; }
int HandleServerCommand8(ModernConnection conn, const char *line) { return 0; }
int HandleServerCommand9(ModernConnection conn, const char *line) { return 0; }
int HandleServerCommand11(ModernConnection conn, const char *line) { return 0; }
int HandleServerCommand14(ModernConnection conn, const char *line) { return 0; }
int HandleServerCommand15(ModernConnection conn, const char *line) { return 0; }
int HandleServerCommand19(ModernConnection conn, const char *line) { return 0; }
int HandleServerCommand20(ModernConnection conn, const char *line) { return 0; }
int HandleServerCommand21(ModernConnection conn, const char *line) { return 0; }
int HandleServerCommand22(ModernConnection conn, const char *line) { return 0; }

/* Character encoding conversion placeholder */
char *ConvertEncoding(const char *input, const char *from_encoding, 
                     const char *to_encoding)
{
    /* Simplified version - full implementation would use iconv or similar */
    if (!input) return NULL;
    return SafeStrdup(input);
}