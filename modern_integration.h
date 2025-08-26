/*
 * modern_integration.h - Integration layer for modern protocol support
 * 
 * This header provides the integration API between existing XGospel-2.0 
 * and the modern protocol handling from q5Go.
 */

#ifndef MODERN_INTEGRATION_H
#define MODERN_INTEGRATION_H

#include "modern_connect.h"
#include "connect.h"
#include <stddef.h>

/* Integration status functions */
extern int IsModernConnectionActive(void);
extern void ShowModernConnectionStatus(void);
extern int GetModernServerCapabilities(void);

/* Enhanced parsing integration */
extern int ParseWithModernProtocol(const char *line);

/* q5Go-style partial data authentication */
extern int HandlePartialAuthData(const char *data, int data_len);
extern void ResetAuthState(void);

/* Enhanced command interface */
extern void SendEnhancedCommand(const char *command, ...);

/* Connection management */
extern void InitializeModernIntegration(void);
extern void CleanupModernIntegration(void);

/* Status and statistics */
extern void GetConnectionStats(char *buffer, size_t buffer_size);
extern const char *GetServerTypeName(void);
extern const char *GetAuthStatusName(void);

/* Server capabilities detection */
extern int DetectServerCapabilities(void);

/* Wrapper functions for backward compatibility */
extern void ModernOutput(const char *text);
extern Connection ModernConnect_Wrapper(const char *site, int port);

/* Global connection management */
extern void SetModernConnection(ModernConnection conn);
extern ModernConnection GetModernConnection(void);

/* Modern XGospel initialization functions */
extern void InitModernXGospel(void);
extern void CleanupModernXGospel(void);

#endif /* MODERN_INTEGRATION_H */