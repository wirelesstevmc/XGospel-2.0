/* Stub implementations for event functions to get basic build working */

#include <X11/Intrinsic.h>
#include "events.h"
#include "players.h" 
#include "stats.h"
#include "tell.h"
#include "connect.h"
#include "xgospel.h"

/* Stub event functions */
Widget EventsButton = NULL;
void InitEvents(Widget TopLevel) { (void)TopLevel; }
void CleanEvents(void) { }

/* Player event stubs */
int EnterServer(const Player *player) { (void)player; return 0; }
int Logon(const Player *player) { (void)player; return 0; }
int Logoff(const Player *player) { (void)player; return 0; }

/* Stats event stubs */
int maintainerRating = 0;
int GotStats(const Player *player, const NameVal *stats) { (void)player; (void)stats; return 0; }

/* Tell event stubs */
int GotNoTell(Player *player, const char *message) { (void)player; (void)message; return 0; }
int GotTell(Player *player, const char *message) { (void)player; (void)message; return 0; }

/* Modern connection stubs that delegate to original functions */
Connection ModernConnect_Wrapper(const char *site, int port) { 
    extern Connection Connect(const char *Site, int Port);
    return Connect(site, port);
}
int IsModernConnectionActive(void) { return 0; }
int ParseWithModernProtocol(const char *line) { (void)line; return 0; }