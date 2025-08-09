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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global modern connection instance */
static ModernConnection g_modern_conn = NULL;

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
    if (!command) return;
    
    va_list args;
    va_start(args, command);
    
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), command, args);
    
    if (g_modern_conn && g_modern_conn->base_conn) {
        /* Use modern command sending */
        SendModernCommand(g_modern_conn, "%s", buffer);
    } else {
        /* Fall back to original command sending */
        Connection conn = Connections.Next;
        if (conn != &Connections) {
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
    if (!buffer || buffer_size == 0) return;
    
    if (!g_modern_conn) {
        snprintf(buffer, buffer_size, "No modern connection active");
        return;
    }
    
    time_t current_time = time(NULL);
    time_t uptime = current_time - g_modern_conn->login_time;
    
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
    if (!g_modern_conn) return 0;
    
    int capabilities = 0;
    
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