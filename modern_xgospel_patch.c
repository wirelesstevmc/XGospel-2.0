/*
 * modern_xgospel_patch.c - Integration patch for XGospel-2.0
 * 
 * This file contains patches to integrate modern protocol support
 * into the existing XGospel application without breaking compatibility.
 */

#include "modern_integration.h"
#include "modern_parser.h"
#include "xgospel.h"
#include "connect.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* Function to initialize modern features - call from main() */
void InitModernXGospel(void)
{
    printf("Initializing XGospel-2.0 with modern protocol support...\n");
    
    /* Initialize modern connection system */
    InitModernConnection();
    
    /* Initialize modern parser */
    InitModernParser();
    
    printf("Modern features initialized successfully\n");
}

/* Enhanced connection function - replacement for original Connect() */
Connection ModernConnect_Wrapper(const char *site, int port)
{
    const char *username;
    const char *password; 
    Connection conn;
    ModernConnection modern_conn;
    char stats[512];
    
    printf("Attempting enhanced connection to %s:%d\n", site, port);
    
    /* Get credentials from application data */
    username = appdata.User ? appdata.User : "";
    password = appdata.Password ? appdata.Password : "";
    
    /* Try modern connection first */
    modern_conn = ModernConnect(site, port, username, password);
    
    if (modern_conn && modern_conn->base_conn) {
        conn = modern_conn->base_conn;
        printf("Enhanced connection established\n");
        
        /* Set up the global modern connection for integration */
        SetModernConnection(modern_conn);
        
        /* Display connection info */
        GetConnectionStats(stats, sizeof(stats));
        ServerMessage("Connection Details:\n%s\n", stats);
        
        return conn;
    } else {
        printf("Enhanced connection failed, using basic connection\n");
        /* Fall back to original connection method */
        return Connect(site, port);
    }
}

/* Enhanced output function with modern protocol awareness */
void ModernOutput(const char *text)
{
    if (!text) return;
    
    /* Try modern protocol processing first */
    ProcessServerMessage(text);
    
    /* Always pass through to original output system */
    Output(text);
}

/* Enhanced command sending with server detection */
void ModernSendCommand(const char *command, ...)
{
    va_list args;
    char buffer[1024];
    
    if (!command) return;
    
    va_start(args, command);
    vsnprintf(buffer, sizeof(buffer), command, args);
    
    /* Use enhanced command sending if available */
    if (IsModernConnectionActive()) {
        SendEnhancedCommand("%s", buffer);
        
        /* Log command for debugging */
        printf("Modern command sent: %s\n", buffer);
    } else {
        /* Fall back to original command sending */
        Connection conn = Conn;
        if (conn) {
            SendCommand(conn, NULL, "%s", buffer);
        }
    }
    
    va_end(args);
}

/* Status display function for UI */
void ShowModernConnectionStatus(void)
{
    if (IsModernConnectionActive()) {
        char stats[512];
        GetConnectionStats(stats, sizeof(stats));
        
        /* Display in a message dialog or console */
        ServerMessage("Enhanced Connection Status:\n%s\n", stats);
    } else {
        ServerMessage("Enhanced connection not active\n");
    }
}

/* Server capabilities check for UI features */
int GetModernServerCapabilities(void)
{
    if (IsModernConnectionActive()) {
        return DetectServerCapabilities();
    }
    return 0; /* No modern features available */
}

/* Modern protocol version information */
const char *GetModernProtocolVersion(void)
{
    return "XGospel-2.0 with q5Go Protocol Extensions v1.0";
}

/* Cleanup function - call on exit */
void CleanupModernXGospel(void)
{
    printf("Cleaning up modern XGospel features...\n");
    
    CleanupModernParser();
    CleanupModernConnection();
    
    printf("Modern cleanup complete\n");
}

/*
 * Function prototypes for integration with existing XGospel code:
 * 
 * To use these functions, you can add the following calls to existing code:
 * 
 * 1. In main() function, add:
 *    InitModernXGospel();
 *    atexit(CleanupModernXGospel);
 * 
 * 2. Replace Connect() calls with:
 *    ModernConnect_Wrapper(site, port)
 * 
 * 3. Replace Output() calls with:
 *    ModernOutput(text)
 * 
 * 4. Replace SendCommand() calls with:
 *    ModernSendCommand(command, args...)
 * 
 * 5. Add menu item or button to call:
 *    ShowModernConnectionStatus()
 */