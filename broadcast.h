#ifndef BROADCAST_H
# define BROADCAST_H

# include <X11/Intrinsic.h>
# include "players.h"
# include "gointer.h"

extern Widget BroadcastButton, YellButton;

extern void InitBroadcast(Widget Toplevel);
extern void CleanBroadcast(void);
extern void ShowBroadcast(const Player *player, const char *separator,
                          const char *What);
extern void InitYell(Widget Toplevel);
extern void CleanYell(void);
extern void ChannelJoin( int channel, const Player *Who);
extern void ChannelLeave(int channel, const Player *Who);
extern void ChannelTitle(int channel, const Player *Who, const char *Title);
extern void WrongChannel(const char *Message);
extern void ChannelDisallowed(void);
extern void JoinChannel(int channel);
extern void RejoinChannel(void);
extern void ShowYell(int channel, const Player *Who, const char *What);

extern ChannelData *OpenChannelData(void);
extern void         AddChannelData(ChannelData *Data, char *Name,
                                   char *Moderator, char *Title,
                                   char *state, NameList *Names);
extern void         CloseChannelData(ChannelData *Data);
extern void         ChannelList(const ChannelData *Data);
#endif /* BROADCAST_H */
