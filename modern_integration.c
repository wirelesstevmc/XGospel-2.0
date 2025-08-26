/*
 * modern_integration.c - Integration layer for modern protocol support
 * 
 * This file provides integration between the existing XGospel-2.0 architecture
 * and the modern protocol handling from q5Go. It maintains backward compatibility
 * while adding enhanced server support.
 */

#include "modern_connect.h"
#include "connect.h"
#include "gointer.h"
#include "utils.h"
#include "xgospel.h"
#include "messages.h"

/* Include for HandleGamesList */
extern int HandleGamesList(ModernConnection conn, const char *line);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

/* Global modern connection instance */
static ModernConnection g_modern_conn = NULL;

/* Authentication state machine like q5Go - use existing AuthState from header */
static AuthState auth_state = AUTH_LOGIN;

/* q5Go-style authentication buffer for partial data */
static char *saved_auth_data = NULL;
static int len_saved_auth_data = 0;

/* Reset authentication state for new connection */
void ResetAuthState(void)
{
    printf("AUTHENTICATION_DEBUG: ResetAuthState called - resetting to AUTH_LOGIN\n");
    fflush(stdout);
    auth_state = AUTH_LOGIN;
    if (saved_auth_data) {
        free(saved_auth_data);
        saved_auth_data = NULL;
    }
    len_saved_auth_data = 0;
}

/* q5Go-style partial data authentication handler */
int HandlePartialAuthData(const char *data, int data_len)
{
    printf("AUTHENTICATION_DEBUG: HandlePartialAuthData called - state=%d, data_len=%d\n", auth_state, data_len);
    if (data_len > 0 && data_len < 50) {
        printf("AUTHENTICATION_DEBUG: Data content: '");
        for (int i = 0; i < data_len; i++) {
            if (data[i] >= 32 && data[i] <= 126) {
                printf("%c", data[i]);
            } else {
                printf("\\%d", data[i]);
            }
        }
        printf("'\n");
    }
    fflush(stdout);
    
    if (auth_state == AUTH_LOGIN && data_len == 7) {
        if (strncmp(data, "Login: ", 7) == 0) {
            extern char *MyName;
            extern char *MyPassword;
            extern void ForceCommand(Connection conn, const char *Command, ...);
            
            printf("AUTHENTICATION_DEBUG: Login prompt detected!\n");
            printf("AUTHENTICATION_DEBUG: MyName='%s', MyPassword=%s\n", 
                   MyName ? MyName : "(null)", MyPassword ? "(set)" : "(null)");
            fflush(stdout);
            
            /* Send username or default to guest */
            if (MyName) {
                printf("AUTHENTICATION: Sending username: %s\n", MyName);
                fflush(stdout);
                ForceCommand(NULL, "%s", MyName);
                /* If we have a password, expect password prompt, otherwise expect direct session */
                auth_state = MyPassword ? AUTH_PASSWORD : AUTH_SESSION;
                printf("AUTHENTICATION_DEBUG: Transitioned to state %d\n", auth_state);
                fflush(stdout);
            } else {
                printf("AUTHENTICATION: Sending guest login\n");
                fflush(stdout);
                ForceCommand(NULL, "guest");
                /* Guest login goes directly to session - no password required */
                auth_state = AUTH_SESSION;
                printf("AUTHENTICATION_DEBUG: Transitioned to AUTH_SESSION\n");
                fflush(stdout);
            }
            return 1; /* Handled */
        }
    } else if (auth_state == AUTH_PASSWORD && data_len == 10) {
        if (strncmp(data, "Password: ", 10) == 0) {
            extern char *MyPassword;
            extern void ForceCommand(Connection conn, const char *Command, ...);
            
            printf("AUTHENTICATION_DEBUG: Password prompt detected!\n");
            printf("AUTHENTICATION_DEBUG: MyPassword=%s\n", MyPassword ? "(set)" : "(null)");
            fflush(stdout);
            
            if (MyPassword) {
                printf("AUTHENTICATION: Sending password\n");
                fflush(stdout);
                ForceCommand(NULL, "%s", MyPassword);
            } else {
                printf("AUTHENTICATION_ERROR: No password available!\n");
                fflush(stdout);
            }
            auth_state = AUTH_SESSION;
            printf("AUTHENTICATION_DEBUG: Transitioned to AUTH_SESSION after password\n");
            fflush(stdout);
            return 1; /* Handled */
        }
    }
    
    return 0; /* Not handled */
}

/* Integration wrapper for enhanced parsing */
static const char *ParseWithModernSupport(Connection conn, const char *input)
{
    if (!g_modern_conn || !input) {
        /* Fall back to original parsing */
        return Parsing(conn);
    }
    
    /* Try modern protocol parsing first */
    if (ParseModernProtocol(g_modern_conn, input)) {
        /* Modern parser handled this message */
        return input;
    }
    
    /* Fall back to original parser */
    return Parsing(conn);
}

/* Enhanced connection establishment */
Connection ModernConnectWrapper(const char *site, int port, 
                               const char *username, const char *password)
{
    /* Reset authentication state for new connection */
    ResetAuthState();
    
    /* Create base connection using original method */
    Connection base_conn = Connect(site, port);
    if (!base_conn) {
        return NULL;
    }
    
    /* Create modern connection wrapper */
    g_modern_conn = ModernConnect(site, port, username, password);
    if (!g_modern_conn) {
        /* Modern connection failed, but base connection might work */
        fprintf(stderr, "Warning: Modern connection failed, using basic connection\n");
        return base_conn;
    }
    
    /* Associate modern connection with base connection */
    g_modern_conn->base_conn = base_conn;
    
    printf("Enhanced connection established to %s:%d\n", site, port);
    return base_conn;
}

/* Enhanced server message processing */
void ProcessServerMessage(const char *message)
{
    if (!message) return;
    
    /* Let modern parser handle authentication and protocol detection */
    if (g_modern_conn && ParseModernProtocol(g_modern_conn, message)) {
        return; /* Modern parser handled it */
    }
    
    /* Pass through to original message processing */
    ServerMessage("%s", message);
}

/* Server type information for UI */
const char *GetServerTypeName(void)
{
    if (!g_modern_conn) return "Unknown";
    
    switch (g_modern_conn->server_type) {
        case SERVER_IGS: return "IGS (Internet Go Server)";
        case SERVER_NNGS: return "NNGS (No Name Go Server)";
        case SERVER_LGS: return "LGS (Local Go Server)";
        case SERVER_PANDANET: return "PandaNet";
        case SERVER_WBG: return "WBG";
        case SERVER_TYGEM: return "Tygem";
        default: return "Unknown Server";
    }
}

/* Authentication status for UI */
const char *GetAuthStatusName(void)
{
    if (!g_modern_conn) return "Disconnected";
    
    switch (g_modern_conn->auth_state) {
        case AUTH_LOGIN: return "Waiting for login prompt";
        case AUTH_PASSWORD: return "Waiting for password prompt";
        case AUTH_GUEST: return "Guest login";
        case AUTH_SESSION: return "Authenticated";
        case AUTH_FAILED: return "Authentication failed";
        default: return "Unknown";
    }
}

/* Enhanced command sending with modern features */
void SendEnhancedCommand(const char *command, ...)
{
    va_list args;
    char buffer[1024];
    Connection conn;
    
    if (!command) return;
    
    va_start(args, command);
    vsnprintf(buffer, sizeof(buffer), command, args);
    
    if (g_modern_conn && g_modern_conn->base_conn) {
        /* Use modern command sending */
        SendModernCommand(g_modern_conn, "%s", buffer);
    } else {
        /* Fall back to original command sending */
        conn = Conn;
        if (conn) {
            SendCommand(conn, NULL, "%s", buffer);
        }
    }
    
    va_end(args);
}

/* Connection status checking */
int IsModernConnectionActive(void)
{
    return (g_modern_conn && ModernIsConnected(g_modern_conn));
}

/* Modern protocol parsing hook */
int ParseWithModernProtocol(const char *line)
{
    static int test_games_sent = 0;
    
    if (!line) {
        return 0; /* Not handled by modern parser */
    }
    
    
    /* q5Go-style authentication - ONLY handle auth during partial data phase, ignore all complete lines during banner */
    switch (auth_state) {
        case AUTH_LOGIN:
            /* During AUTH_LOGIN phase, completely ignore ALL complete lines like q5Go does */
            /* Authentication is handled only via partial data buffering in connect.c */
            return 1; /* HANDLED - completely block traditional parser during auth */
            
        case AUTH_PASSWORD:
            /* During AUTH_PASSWORD phase, check for password prompts like q5Go */
            if (strstr(line, "1 1") != NULL || strstr(line, "Password:") != NULL) {
                extern char *MyPassword;
                extern void ForceCommand(Connection conn, const char *Command, ...);
                if (MyPassword) {
                    ForceCommand(NULL, "%s", MyPassword);
                }
                auth_state = AUTH_SESSION;
                return 1; /* HANDLED */
            }
            /* Block all other complete lines during password phase */
            return 1; /* HANDLED - completely block traditional parser during auth */
            
        case AUTH_SESSION:
            /* Already authenticated, let normal protocol handling proceed */
            break;
            
        case AUTH_FAILED:
            /* Authentication failed, reset on next connection */
            break;
    }
    
    /* Handle successful login messages */
    if (strstr(line, "You have entered IGS") != NULL || 
        strstr(line, "account name is") != NULL) {
        printf("AUTHENTICATION: Login successful! Transitioning to session mode\n");
        fflush(stdout);
        auth_state = AUTH_SESSION;
        return 0; /* Let this important message pass through to normal processing */
    }
    
    if (!g_modern_conn) {
        return 0; /* Not handled by modern parser */
    }
    
    /* Try to process with modern protocol */
    if (ParseModernProtocol(g_modern_conn, line)) {
        return 1; /* Handled by modern parser */
    }
    
    return 0; /* Not handled by modern parser */
}

/* Set global modern connection for integration */
void SetModernConnection(ModernConnection conn)
{
    g_modern_conn = conn;
}

/* Get global modern connection */
ModernConnection GetModernConnection(void)
{
    return g_modern_conn;
}

/* Clean shutdown of modern connection */
void CleanupModernConnection(void)
{
    if (g_modern_conn) {
        ModernDisconnect(g_modern_conn);
        g_modern_conn = NULL;
    }
}

/* Get connection statistics */
void GetConnectionStats(char *buffer, size_t buffer_size)
{
    time_t current_time;
    time_t uptime;
    
    if (!buffer || buffer_size == 0) return;
    
    if (!g_modern_conn) {
        snprintf(buffer, buffer_size, "No modern connection active");
        return;
    }
    
    current_time = time(NULL);
    uptime = current_time - g_modern_conn->login_time;
    
    snprintf(buffer, buffer_size,
        "Server: %s\n"
        "Host: %s:%d\n" 
        "Auth: %s\n"
        "Uptime: %ld seconds\n"
        "Commands sent: %d\n"
        "Encoding: %s\n",
        GetServerTypeName(),
        g_modern_conn->server_host ? g_modern_conn->server_host : "unknown",
        g_modern_conn->server_port,
        GetAuthStatusName(),
        uptime,
        g_modern_conn->last_command_id,
        g_modern_conn->codec_name ? g_modern_conn->codec_name : "ASCII");
}

/* Compatibility layer for existing XGospel functions */

/* Override the original Connect function to use modern connection */
Connection ModernConnect_Override(const char *Site, int Port)
{
    /* Get username/password from application data if available */
    const char *username = appdata.User ? appdata.User : "";
    const char *password = appdata.Password ? appdata.Password : "";
    
    return ModernConnectWrapper(Site, Port, username, password);
}

/* Enhanced parsing hook that can be called from the original parser */
int TryModernParsing(const char *line)
{
    if (g_modern_conn) {
        return ParseModernProtocol(g_modern_conn, line);
    }
    return 0; /* Not handled */
}

/* Server compatibility detection */
int DetectServerCapabilities(void)
{
    int capabilities;
    
    if (!g_modern_conn) return 0;
    
    capabilities = 0;
    
    switch (g_modern_conn->server_type) {
        case SERVER_IGS:
            capabilities = 1; /* Basic IGS support */
            break;
        case SERVER_NNGS:
        case SERVER_LGS:
            capabilities = 2; /* Enhanced protocol support */
            break;
        case SERVER_PANDANET:
        case SERVER_TYGEM:
            capabilities = 3; /* Modern server features */
            break;
        default:
            capabilities = 0;
    }
    
    return capabilities;
}

/* Initialize modern connection system */
void InitModernConnection(void)
{
    /* Set up any global state needed for modern connections */
    g_modern_conn = NULL;
    
    /* Register cleanup function */
    atexit(CleanupModernConnection);
    
    printf("Modern connection system initialized\n");
}