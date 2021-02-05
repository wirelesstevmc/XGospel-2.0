! Sample Xgospel resource file
! Modify at least user and password here, then the window sizes
! If you modify a boolean resource, don't put spaces after "False" or "True".

xgospel*User:                     yourigsname
xgospel*Password:                 yourpassword

! put here the location (full path name) of board.xpm:
!xgospel*board.backgroundPixmap:      pixmap(board.xpm)

!xgospel*games.?.width:        520
!xgospel*games.collect.height: 600
!xgospel*games.collect.viewport.preferredPaneSize: 500
!xgospel*games.collect.stripform.preferredPaneSize: 20

!xgospel*players.?.width:      816
!xgospel*players.collect.height: 600
!xgospel*players.collect.viewport.preferredPaneSize: 500
!xgospel*players.collect.stripform.preferredPaneSize: 20

!xgospel*observe.?.width:      580
!xgospel*observe.?.height:     663

!xgospel*broadcasts.?.width:   550
!xgospel*broadcasts.?.height:  210

!xgospel*igsMessages.?.width:   600
!xgospel*igsMessages.?.height:  150

!xgospel*tell.?.width:         600
!xgospel*tell.?.height:        210

!xgospel*main.?.width:         655
!xgospel*main.?.height:        350

!xgospel.geometry:		-1450+900
!xgospel.broadcasts.geometry:   +1160+700
!xgospel.players.geometry:	+5+60
!xgospel.games.geometry:	+1780+150
!xgospel.observe.geometry:	+10+10

!xgospel*font: -adobe-courier-bold-r-normal--*-120-75-75-m-70-*-1
!xgospel*font: fixed

! Message to be sent for tells and beeps when the "auto reply" option is set:
!xgospel*autoReplyMessage: I'm away for a moment, this is an automatic reply.

! list of your IGS friends, enemies, marked players, etc... You can
! create new categories. Only the "Marked" category is predefined in
! xgospel (a player can be marked with control-left click in the players
! window, but such marking does not last across IGS sessions. Use the "Marked"
! category here to make marking permanent).
xgospel*playerToWidget: \
   jl,   Friend\n\
   xgospel, Friend\n\
   escaper, Enemy\n\
   impolite, Marked

! If want to have different colors depending on the player status and to sort
! players by status (looking, open or other), you can set *simpleNames to
! False and use the following instead:
!xgospel*playerToWidget: \
!   ^tobe, Friend\n\
!   ^xgospel, Friend\n\
!   !.*[^0-9]$, Looking\n\
!   \\]. .[^X] .*[^0-9]$, Open
! If you want to highlight high dans specially, you can use:
!   \\[ [456789]d\\][*], HighDan\n\
! Remove [*] from the HighDan definition if you also want to include
! 4d without a *.

! Set *simpleNames to True if playerToWidget contains complete user names
! and no regular expression. In this case xgospel is much faster and
! highlights games using player name matching instead of game name matching.
! If you want to change simpleNames dynamically with the "program settings"
! panel, remove ^ before the friends names.
!xgospel*simpleNames: True

! Player importance. Players and games are now by default sorted
! by importance instead of strength. The importance of a player is the strength
! by default. The importance of a set of players can be changed by defining
! the set in *playerToWidget (such as all players with a Friend entry), and
! by defining the importance of this set in *playerImportance. An importance
! of 10p ensures that the set always appears first, but you can also assign
! an importance equal to your own strength to let all your IGS friends
! appear close to you in the players window.
! Alternatively, you can sort players by status: if you want to put all
! "looking" players first, define the set "Looking" and set its importance
! above 10p (and set also *simpleNames to False).
xgospel*playerImportance: \
   Friend,  13p\n\
   Looking, 11p\n\
   Open,    10p
! If you want to see your enemies at the top too, you can add after Friend:
!  Enemy,  12p\n\

! By default, players 5d and above are highlighted as pros. You can change
! this limit to 4d or 1p for example.
!xgospel*minProRank: 5d

! For important games, the game window pops up if *games*raise.state is true.
! A game is important if the importance of either player >= minImportantRank
!xgospel*minImportantRank: 5d

! If you wish more control on highlighting of high dans, set
! simpleNames to False and use a resource such as the following:
!xgospel*gameToWidget: [ ][3456789]d[ *], HighDan


! You can but need not modify the rest of the resources. Other resources not
! mentionned here are defined in the source file resources.c
! -----------------------------------------------------------------------------

!xgospel*site: 210.134.78.91
!xgospel*site: igs.joyjoy.net
!xgospel*port: 7777

!Turning on debugging functions can be done with:
! xgospel*debug:                  True
! xgospel*debugFun:               1
!
!You can save a full log of the server-client interaction in a file. This
!produces a lot of output, so don't use this if you don't need it. It's great
!for identifying bugs though....
!Or if you want to have a check, you can do: tail -f filename while running.
!(you can get about the same effect by selecting stdout in the options menu)
!The %N will be replaced by your login name on the server
!Also remember that this log file will contain your IGS password !
! xgospel*debugFile:                /tmp/%N.log
!
!If you want the observe title or iconName to give some other text, you can
!set a pattern (%B and %W will be replaced by the name of the black and the
!white player, while %b and %w will be replaced by their strength). %A is
!'*' if black has at least 20 rated games, ' ' otherwise; %a is the same for
! white. %G will be replaced with the game number.
! So you can set something like (colons do not work at the moment):
! xgospel*observe.title:    Game %G      %B %b%A (B)  vs  %W %w%a (W)
!

! Set this to True to get more warnings:
!xgospel*verbose.state:            False
! Set this to True to allow overwriting .sgf files:
!xgospel*sgfOverwrite.state:       True
!The next line means that the board cursor does not blink by default:
!xgospel*blink.state:             False
!The next line means that all Beep widgets will be off by default.
!xgospel*SmeBell.state:            False
!But I do want a beep in case of error:
!xgospel*errorBeep.state:          True
!And I want it on on the broadcast window:
!xgospel*broadcasts*SmeBell.state: True

! I don't want the main window to map and raise itself if a message appears
!xgospel*main*raise.state:         False
! If fact, I want this behavior for IGS messages, broadcasts and
! channels (this makes the previous line unnecessary).
!xgospel*raise.state:              False
! I don't want a map from the events window
! xgospel*events*raise.state:       False
! and I don't want my observed games to map in case of a move or kibitz
!xgospel*moveRaise.state:          False
!xgospel*kibitzRaise.state:        False
! The "IGS Messages" window pops up by default:
! xgospel*igsMessages*raise.state:  True
! Raise on the Broadcast (shout) window:
!xgospel*broadcasts*raise.state:   False
! Raise on tell windows. WARNING: if you set this to False, you may not
! see the tells sent to you. You should really keep it as True.
!xgospel*tell*raise.state:         True
! Raise the games window on important games (see *minImportantRank above):
!xgospel*games*raise.state:        False
! Raise the players window on important players (see *minImportantRank above):
!xgospel*players*raise.state:      False

! Set to true if you like players sorted by name instead of importance:
!xgospel*players*sortName.state:  False
! Set to true if you like games sorted by server number instead of importance:
!xgospel*games*sortNumber.state:   False
!
!I used the following when I work on a single color X server:
!   xgospel*players*set.playerEntryk.background: White
!   xgospel*players*set.playerEntryd.background: White
!   xgospel*players*set.playerEntryp.background: White
!But now I prefer:
!   xgospel*players*set.playerEntryk.background: White
!   xgospel*players*set.playerEntryd.foreground: White
!   xgospel*players*set.playerEntryd.background: Black
!   xgospel*players*set.playerEntryp.backgroundPixmap: bitmap(gray3,White, Black)
!   xgospel*players*set.playerEntry__.foreground: White
!   xgospel*players*set.playerEntry__.background: Black
!   xgospel*players*set.playerEntryNR.backgroundPixmap: bitmap(gray3)
!   xgospel*players*set.playerEntryIGS92.foreground: White
!   xgospel*players*set.playerEntryIGS92.background: Black
!
!The program converts a strength to a name by removing all leading
!digits, and by then replacing each weird characters by _ . So if
!you want IGS92 or NR handled in a special way:
!xgospel*players*set.playerEntryNR.background:    Blue
!xgospel*players*set.playerEntryIGS92.background: Red
!
! You can in fact yourself decide what text is put after playerEntry by using
! the playerToWidget resource to give a set of pairs of regular expressions
! [except if resource *simpleNames is True, see above]
! and strings terminated by newlines. The full text that will appear in the
! player widget will be matched against this, and the first match will get
! its corresponding string tucked to playerEntry. So a pattern like:
!xgospel*playerToWidget: ^ *AshaiRey,   Me\n\
!                        \\[ [456]k\\], MyStrength
!xgospel*players*set*playerEntryMe.background:    Red
!xgospel*players*set*playerEntryMe.foreground:    yellow
!xgospel*players*set*playerEntryMyStrength.background:    Blue
!xgospel*players*set*playerEntryMyStrength.foreground:    yellow
! will have as result:
!  ^ *AshaiRey will match an entry that starts with any number of spaces
!   followed by at least the text AshaiRey. In that case xgospel will use
!   playerEntryMe as widget name. And the following two lines say that this
!   widget should be displayed with background red and foreground yellow. And
!   that is indeed the way my entry (IGS userid AshaiRey) will be displayed.
!   in fact, userid AshaiRey2 will also be so displayed. If I had only wanted
!   to match AshaiRey, I should have used ^ *AshaiRey\\[
!
!  \\[ [456]k\\] will match any of the strings [ 4k], [ 5k] and [ 6k], so it
!   will match all players with a strength around mine (5k), and give them the
!   name playerEntryMyStrength. And the last two lines say that these entries
!   should be displayed with yellow letters on a blue background.
! some extra notes:
! -if you want a \ in your regular expression, you have to escape it with \,
!  since \ is also an escape character in a resource file. That is why in the
!  example I used \\[ when I wanted \[ in the regular expression.
! -Entries are separated by newlines. You can get a newline in a resource file
!  with the special sequence \n\ (with nothing, not even spaces, behind the
!  last \ (as in the example))
! -Leading spaces in the regular expression and widget name are dropped,
!  trailing spaces are not (that is why the , and \n\ in the example directly
!  follow the preceding text, without spaces in between)
! You can use this to give special colors/fonts/background pictures/borders
! (everything that is settable as a resource) to people with any visible
! property (e.g. friend, looking for a game, playing......).

xgospel*players*set*playerEntryFriend.background:  cornflowerblue
xgospel*players*set*playerEntryFriend.foreground:  #FDEDC8

xgospel*players*set*playerEntryEnemy.background:  red
xgospel*players*set*playerEntryEnemy.foreground:  yellow

xgospel*players*set*playerEntryLooking.background: green
xgospel*players*set*playerEntryOpen.background: green2
xgospel*players*set*playerEntryHighDan.background: green2

xgospel*games*set*gameEntryFriend.background: cornflowerblue
xgospel*games*set*gameEntryFriend.foreground: #FDEDC8
xgospel*games*set*gameEntryHighDan.background:  green2

! Click with control-left button on a player in the players window to
! mark this player, changing its widget color. Marking a player can be
! used to remember a player who declined a game request, or who escaped, etc...
!xgospel*players*set*playerEntryMarked.background:  #FDEDC8
!xgospel*players*set*playerEntryMarked.foreground:  cornflowerblue


!depending on the resolution of your screen, you could add bitmap
!patterns like:
!xgospel*board.backgroundPixmap:      bitmap(gray3)
! (for most sites: look in /usr/include/X11/bitmaps for more 
!  possible patterns, or make your own)
! In case you compiled with xpm, you can also add pixmaps [see above
! pixmap(board.xpm)]
!
! If you want just black and white stones, set simpleStones to true:
!xgospel*board.simpleStones: false

! mark dame and/or territories in scoring mode:
!xgospel*markTerritories:  true
!xgospel*markDame:         true
!xgospel*board.dameColor:  green

! switch to scoring mode automatically after 3 passes:
!xgospel*autoScore: true

!The default line width for drawing things on the board is 0. This tells X to 
!use the fastest way to draw lines on that particular piece of hardware. But
!sometimes this results in badly drawn stones. In that case, you might try the
!slower, but more dependable line width of one. This is not used if
!simpleStones is false.
! xgospel*board.lineWidth:           1
!
! By default xgospel will send to the account "xgospel" (the xgospel
! maintainer) a message with the version number of xgospel. It will also do
! a "stats xgospel" to get some info about
! the current version (it will pop up a message if your version is not the
! most recent one). If you don't like this, set the resource maintainer to the
! empty string:
! xgospel*maintainer: xgospel

!
! xgospel can also let a stone click instead of beeping if your computer has
! a sparc-like /dev/audio device that you can access. So if you are sitting
! behind a sparc console, you might try setting:
! xgospel*board.audioFile: /dev/audio
! On linux I patched my kernel with the pc speaker patches, which also gives me
! a /dev/audio. Be sure to use at least version 0.7 (0.6 contained a bug that
! caused xgospel not to make a click sound most of the time).
! (the default for audioFile is NULL, which means don't beep)
! (sound contributed by Nici Schraudolph (schraudo@cs.ucsd.edu, nic on IGS))

! xgospel*version: 1.12a
! xgospel*inactiveTimeout: 2700

! Number of seconds for each move when replaying a game automatically:
!xgospel*replayTimeout: 1

! Rate (in seconds) at which to update the players window:
!xgospel*playersUpdateTimeout: 15

! Rate (in seconds) at which xgospel sends "who" and "games" commands.
! Increase them if you have too much netlag. If *gamesTimeout is zero,
! xgospel will not automatically ask for the games list; click on the Update
! button to refresh the list. Same for *whoTimeout and the player window.
! If you set either resource to zero, *verbose.state (see below) should
! be false otherwise you get too many warnings. Also in this case xgospel
! is sometimes confused about your playing or observing state; click on
! the Update button in the games window or type "games" in the main window
! when this occurs.
!xgospel*whoTimeout: 300
!xgospel*gamesTimeout: 310

! To avoid mouse clickos: the button up event must be at the same intersection
! as the button down event and separated by at least minButtonTime
! milliseconds (default 50 ms) to be sent as a move. If you are subject to
! frequent clickos, increase this to 200 or 300. If you want to force yourself
! to think at least one second before a move, set it to 1000.
!xgospel*minButtonTime: 50

! Default times for a challenge are 10 10 (10 minutes then 25 stones per
! 10 minutes byoyomi period):
! xgospel*defaultTime: 10
! xgospel*defaultByoYomi: 10

! xgospel warns when your time per move is getting low. For an observed
! game, it warns when the time for either player is getting low.
! minLagMargin is number of seconds left as safety margin to take netlag
! into account. lowTimeSet is 0 for no warning, 1 for my time only,
! 2 for opponent time only, 3 for both players, 4 for observed games only,
! 5 for my time and observed games, 7 for everything. myLowTimeBackground is
! for my own time only, lowTimeBackground for other players.
! *myLowTimeBackground: red
! *myLowTimeForeground: yellow
! *lowTimeBackground: cornflowerblue
! *lowTimeForeground: yellow
! *lowTimeSet: 3
! *minSecPerMove: 5
! *minLagMargin: 15

! Add move number in kibitzes:
!xgospel*numberKibitzes: true

! use "say" when talking to opponent. Set to false if you do not want
! your conversation with opponent to be saved in the game record.
!xgospel*useSay:  true
