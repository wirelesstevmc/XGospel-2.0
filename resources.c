#include "players.h"
#include "games.h"

/* #define META    "Mod1"         The modifier you like as META or ALT key */
#define META    "Meta"         /* The modifier you like as META or ALT key */
#define DEFTEXT \
  "\"Press shift + left mouse button on windows or buttons for help\""
#define SETDEFTEXT "setvalues(*shortHelp, label," DEFTEXT ")"
#define ONENTER(action)             \
        "Ctrl<Key>j:" action "\\n"  \
        "Ctrl<Key>m:" action "\\n"  \
        "Ctrl<Key>o:" action "\\n"  \
        "<Key>Return:" action "\\n" \
        "<Key>KP_Enter:" action
#define ONTAB(action)                    \
        "~Shift Ctrl<Key>i:" action "\\n"\
        "~Shift<Key>Tab:"    action
#define ONBACKTAB(action)                \
        "Shift Ctrl<Key>i:" action "\\n" \
        "Shift<Key>Tab:" action
#define ASKTEXTTRAN()                        \
        ONTAB("asknexttext()") "\\n"         \
        ONBACKTAB("askprevioustext()") "\\n" \
        ONENTER("askendtext()") "\\n"        \
        "<Btn1Down>:asksettext() select-start()\\n" \
        "<Btn2Down>:asksettext() insert-selection(\"PRIMARY\",\"CUT_BUFFER0\")\\n" \
        "<Btn3Down>:asksettext() extend-start()\\n" \
        META "<Key>c:change() \\n"   \
        META "<Key>w:widgets()\\n"   \
        META "<Key>h:info()"
#define USER_TRANS \
        META "<Key>a: usercommand(autoreply) \\n" \
        META "<Key>o: usercommand(toggle open) \\n" \
        META "<Key>l: usercommand(toggle looking) \\n" \
        META "<Key>q: usercommand(quit) \\n"

#define SHADOWTEXT(data)                \
        " textForm.Form"                \
            "(text.AsciiText" data      \
            ")"

/* The next 4 are a (tiny) bit nicer to the colormap, but don't work very well
   on b/w displays
#define GREEN1 "Green"
#define GREEN2 "LimeGreen"
#define GREEN3 "#40B040"
#define GREEN4 "DarkSeaGreen"
*/
#define GREEN1 "Green"
#define GREEN2 "#32CD32"
#define GREEN3 "#40B040"
#define GREEN4 "#8FBC8F"

String fallback_resources[] = {
#include "helptext.h"
    (String) "*analyzer.?.width:		   382",
    (String) "*analyzer.?.height:                  425",
    (String) "*analyzer*score.label:               Score",
    (String) "*analyzer.translations:              #override\\n"
        "<ClientMessage>WM_PROTOCOLS:   WMprotocol()\\n"
        "<ClientMessage>DELETE:         destroy()",
    (String) "*analyzer.collect*resizeToPreferred: True",
/*  (String) "*analyzer.collect.?.skipAdjust:      True",
    (String) "*analyzer.collect.scrollboard.skipAdjust:     False", */
    (String) "*analyzer*close.translations:        #override\\n"
        "Shift<Btn1Down>: help(analyze_close)\\n"
/*      "Shift<Btn3Down>: help(analyze_close, 1)\\n" */
        "<Btn1Up>: notify() unset() destroy(\"\")\\n"
        "<EnterWindow>: helpup(analyze_close,"
            "\"Quit this analyze session.\")\\n"
        "<LeaveWindow>: helpdown(analyze_close)",
    (String) "*analyzer*undo.translations:         #override\\n"
        "Shift<Btn1Down>: help(analyze_undo)\\n"
        "<EnterWindow>: helpup(analyze_undo,"
            "\"Undo the last move.\")\\n"
        "<LeaveWindow>: helpdown(analyze_undo)",
    (String) "*analyzer*copy.translations:         #override\\n"
        "Shift<Btn1Down>: help(analyze_copy)\\n"
        "<EnterWindow>: helpup(analyze_copy,"
            "\"Make a new copy of the board being analyzed.\")\\n"
        "<LeaveWindow>: helpdown(analyze_copy)",
    (String) "*analyzer*score.translations:        #override\\n"
        "Shift<Btn1Down>: help(analyze_score)\\n"
        "<EnterWindow>: helpup(analyze_score,"
            "\"Start scoring the board being analyzed.\")\\n"
        "<LeaveWindow>: helpdown(analyze_score)",
    (String) "*analyzer*scroll.translations:       #override\\n"
        "Shift<Btn1Down>: help(analyze_scroll)\\n"
        "<EnterWindow>: helpup(analyze_scroll,"
            "\"Scroll to older and/or more recent positions.\")\\n"
        "<LeaveWindow>: helpdown(analyze_scroll)",
    (String) "*analyzer*board.translations:        #override\\n"
        USER_TRANS
        "Shift<Key>Up:   analyzegoto(-1000)\\n"
        "Shift<Key>Down: analyzegoto(1000)\\n"
        "<Key>Up:        analyzegoto(-1)\\n"
        "<Key>Down:      analyzegoto(1)\\n"
        "Shift<Btn1Down>: help(analyze_board)\\n"
        "<EnterWindow>: helpup(analyze_board,"
            "\"View a position you are analyzing.\")",

    (String) "*observe.title:      Game %G   %B %b%A (B)  vs  %W %w%a (W)",
    (String) "*observe.iconName:                   %B-%W",
    (String) "*observe.?.width:                    580",
    (String) "*observe.?.height:                   663",
    (String) "*observe.translations:               #override\\n"
        "<ClientMessage>WM_PROTOCOLS:   WMprotocol()\\n"
        "<ClientMessage>DELETE:         destroy()",
    (String) "*observe.collect.info.height:        80",
    (String) "*observe.collect*resizeToPreferred:  True",
    (String) "*observe.collect.?.skipAdjust:       True",
    (String) "*observe.collect.scrollboard.skipAdjust:      False",
    (String) "*observe*titlePlay*editType:         edit",
    (String) "*observe*titlePlay.translations:     #override\\n"
        "Shift<Btn1Down>: help(observe_titlePlay)\\n"
        "<EnterWindow>: setfocus() helpup(observe_titlePlay,"
            "\"Enter title you want to give this game.\")\\n"
        "<LeaveWindow>: setfocus(\"*input\") "
            "helpdown(observe_titlePlay)\\n"
        ONENTER("changetitle()") "\\n"
        USER_TRANS
        META "<Key>c:   change()  \\n"
        META "<Key>w:   widgets() \\n"
        META "<Key>h:   info()",
    (String) "*observe*titleObserve.displayCaret:  False",
    (String) "*observe*titleObserve.translations:  #override\\n"
        "Shift<Btn1Down>: help(observe_titleObserve)\\n"
        "<EnterWindow>: helpup(observe_titleObserve,"
            "\"Title of teaching game.\")\\n"
        "<LeaveWindow>: helpdown(observe_titleObserve)\\n"
        ONENTER("changetitle()") "\\n"
        USER_TRANS
        META "<Key>c:   change()  \\n"
        META "<Key>w:   widgets() \\n"
        META "<Key>h:   info()",
    (String) "*observe*close.translations:         #override\\n"
        "Shift<Btn1Down>: help(observe_close)\\n"
        "<Btn1Up>: notify() unset() destroy(\"\")\\n"
        "<EnterWindow>: helpup(observe_close,"
            "\"Close board widget.\")\\n"
        "<LeaveWindow>: helpdown(observe_close)",
    (String) "*observe*undo.translations:          #override\\n"
        "Shift<Btn1Down>: help(observe_undo)\\n"
        "<EnterWindow>: helpup(observe_undo,"
            "\"Undo your opponents last move.\")\\n"
        "<LeaveWindow>: helpdown(observe_undo)",
    (String) "*observe*pass.translations:          #override\\n"
        "Shift<Btn1Down>: help(observe_pass)\\n"
        "<EnterWindow>: helpup(observe_pass,"
            "\"Pass.\")\\n"
        "<LeaveWindow>: helpdown(observe_pass)",
    (String) "*observe*done.translations:          #override\\n"
        "Shift<Btn1Down>: help(observe_done)\\n"
        "<EnterWindow>: helpup(observe_done,"
            "\"Stop scoring.\")\\n"
        "<LeaveWindow>: helpdown(observe_done)",
    (String) "*observe*komi.translations:          #override\\n"
        "Shift<Btn1Down>: help(observe_komi)\\n"
        "<EnterWindow>: helpup(observe_komi,"
            "\"Komi.\")\\n"
        "<LeaveWindow>: helpdown(observe_komi)",
    (String) "*observe*handicap.translations:      #override\\n"
        "Shift<Btn1Down>: help(observe_handicap)\\n"
        "<EnterWindow>: helpup(observe_handicap,"
            "\"Handicap.\")\\n"
        "<LeaveWindow>: helpdown(observe_handicap)",
    (String) "*observe*captures.translations:      #override\\n"
        "Shift<Btn1Down>: help(observe_captures)\\n"
        "<EnterWindow>: helpup(observe_captures,"
            "\"Number of stones captured by black and white.\")\\n"
        "<LeaveWindow>: helpdown(observe_captures)",
    (String) "*observe*move.translations:          #override\\n"
        "Shift<Btn1Down>: help(observe_moves)\\n"
        "<EnterWindow>: helpup(observe_moves,"
            "\"Move number of current position / "
            "Number of moves played.\")\\n"
        "<LeaveWindow>: helpdown(observe_moves)",
    (String) "*observe*time.translations:          #override\\n"
        "Shift<Btn1Down>: help(observe_time)\\n"
        "<EnterWindow>: helpup(observe_time,"
            "\"Remaining time for black and white.\")\\n"
        "<LeaveWindow>: helpdown(observe_time)",
    (String) "*observe*scroll.translations:        #override\\n"
        "Shift<Btn1Down>: help(observe_scroll)\\n"
        "<EnterWindow>: helpup(observe_scroll,"
            "\"Scroll to older and/or more recent positions.\")\\n"
        "<LeaveWindow>: helpdown(observe_scroll)",
    (String) "*observe*board.translations:         #override\\n"
        "Shift<Btn1Down>: help(observe_board)\\n"
        "<EnterWindow>: helpup(observe_board,"
            "\"View a position in a certain game.\")",
    (String) "*observe*info.translations:          #override\\n"
        "Shift<Btn1Down>: help(observe_info)\\n"
        "<EnterWindow>: helpup(observe_info,"
            "\"Shows the kibitzes in this game.\")\\n"
        "<LeaveWindow>: helpdown(observe_info)",
    (String) "*observe.collect*chatter.translations: #override\\n"
        "Shift<Btn1Down>: help(observe_chatter)\\n"
        "<EnterWindow>: helpup(observe_chatter,"
            "\"Send a chatter about this game.\")\\n"
        "<LeaveWindow>: helpdown(observe_chatter)\\n"
        ONENTER("chatter()") "\\n"
        ONTAB("nexttext()")"\\n"
        ONBACKTAB("previoustext()")"\\n"
        "Shift<Key>Up:   observegoto(-1000)\\n"
        "Shift<Key>Down: observegoto(1000)\\n"
        "<Key>Up:        observegoto(-1)\\n"
        "<Key>Down:      observegoto(1)\\n"
        "<Btn1Down>: thistext() select-start()\\n"
        "<Btn2Down>: thistext() insert-selection(\"PRIMARY\", \"CUT_BUFFER0\")\\n"
        "<Btn3Down>: thistext() extend-start()\\n"
        USER_TRANS
        META "<Key>c:   change() \\n"
        META "<Key>w:   widgets()\\n"
        META "<Key>h:   info()",
    (String) "*observe*serverScore.label: server score",
    (String) "*observe.collect*chatterPrefixLabel.label: Chatter:",
    (String) "*observe.collect*chatter.displayCaret: False",
    (String) "*observe.collect*input.translations: #override\\n"
        "Shift<Btn1Down>: help(observe_input)\\n"
        "<EnterWindow>: helpup(observe_input,"
            "\"Send a kibitz about this game.\")\\n"
        "<LeaveWindow>: helpdown(observe_input)\\n"
        ONENTER("kibitz()") "\\n"
        ONTAB("nexttext()")"\\n"
        ONBACKTAB("previoustext()")"\\n"
        "Shift<Key>Up:   observegoto(-1000)\\n"
        "Shift<Key>Down: observegoto(1000)\\n"
        "<Key>Up:        observegoto(-1)\\n"
        "<Key>Down:      observegoto(1)\\n"
        "<Btn1Down>: thistext() select-start()\\n"
        "<Btn2Down>: thistext() insert-selection(\"PRIMARY\", \"CUT_BUFFER0\")\\n"
        "<Btn3Down>: thistext() extend-start()\\n"
        USER_TRANS
        META "<Key>c:   change() \\n"
        META "<Key>w:   widgets()\\n"
        META "<Key>h:   info()",
    (String) "*observe.collect*input.displayCaret: False",
    (String) "*observe.collect*kibitzPrefixLabel.label: Kibitz :",
    (String) "*chatterPanes.orientation:           horizontal",
    (String) "*kibitzPanes.orientation:            horizontal",
    (String) "*observe.collect*kibitzChatter*Label.background: Green",
    (String) "*observe.collect*kibitzChatter*Command.background: Green",

/*  (String) "*review.title:                       %T", */
    (String) "*review.title:                       %B [%b]%A vs [%w]%a %W",
    (String) "*review.iconName:                    go review",
    (String) "*review.?.width:                     396",
    (String) "*review.?.height:                    540",
    (String) "*review.translations:                #override\\n"
        "<ClientMessage>WM_PROTOCOLS:   WMprotocol()\\n"
        "<ClientMessage>DELETE:         destroy()",
    (String) "*review.collect.info.height:         80",
    (String) "*review.collect*resizeToPreferred:   True",
    (String) "*review.collect.?.skipAdjust:        True",
    (String) "*review.collect.scrollboard.skipAdjust: False",
    (String) "*review*titleObserve.displayCaret:   False",
    (String) "*review*titleObserve.translations:   #override\\n"
        "Shift<Btn1Down>: help(review_title)\\n"
        "<EnterWindow>: helpup(review_title,"
            "\"Title of reviewed game.\")\\n"
        "<LeaveWindow>: helpdown(review_title)\\n"
        ONENTER("changetitle()") "\\n"
        USER_TRANS
        META "<Key>c:   change()  \\n"
        META "<Key>w:   widgets() \\n"
        META "<Key>h:   info()",
    (String) "*review*close.translations:          #override\\n"
        "Shift<Btn1Down>: help(review_close)\\n"
        "<Btn1Up>: notify() unset() popdown()\\n"
        "<EnterWindow>: helpup(review_close,"
            "\"Close board widget.\")\\n"
        "<LeaveWindow>: helpdown(review_close)",
    (String) "*review*komi.translations:           #override\\n"
        "Shift<Btn1Down>: help(observe_komi)\\n"
        "<EnterWindow>: helpup(observe_komi,"
            "\"Komi.\")\\n"
        "<LeaveWindow>: helpdown(observe_komi)",
    (String) "*review*handicap.translations:       #override\\n"
        "Shift<Btn1Down>: help(observe_handicap)\\n"
        "<EnterWindow>: helpup(observe_handicap,"
            "\"Handicap.\")\\n"
        "<LeaveWindow>: helpdown(observe_handicap)",
    (String) "*review*captures.translations:       #override\\n"
        "Shift<Btn1Down>: help(observe_captures)\\n"
        "<EnterWindow>: helpup(observe_captures,"
            "\"Number of stones captured by black and white.\")\\n"
        "<LeaveWindow>: helpdown(observe_captures)",
    (String) "*review*move.translations:           #override\\n"
        "Shift<Btn1Down>: help(observe_moves)\\n"
        "<EnterWindow>: helpup(observe_moves,"
            "\"Move number of current position / "
            "Number of moves played.\")\\n"
        "<LeaveWindow>: helpdown(observe_moves)",
    (String) "*review*time.translations:           #override\\n"
        "Shift<Btn1Down>: help(review_time)\\n"
        "<EnterWindow>: helpup(review_time,"
            "\"Remaining time for black and white.\")\\n"
        "<LeaveWindow>: helpdown(review_time)",
    (String) "*review*scroll.translations:         #override\\n"
        "Shift<Btn1Down>: help(observe_scroll)\\n"
        "<EnterWindow>: helpup(observe_scroll,"
            "\"Scroll to older and/or more recent positions.\")\\n"
        "<LeaveWindow>: helpdown(observe_scroll)",
    (String) "*review*board.translations:          #override\\n"
        "Shift<Btn1Down>: help(observe_board)\\n"
        "<EnterWindow>: helpup(observe_board,"
            "\"View a position in a certain game.\")",
    (String) "*review*info.translations:           #override\\n"
        "Shift<Btn1Down>: help(review_info)\\n"
        "<EnterWindow>: helpup(review_info,"
            "\"Shows the comments for this game.\")\\n"
        "<LeaveWindow>: helpdown(review_info)",

    (String) "*players.title:                      Players",
    (String) "*players.iconName:                   go players",
    (String) "*players.translations:               #override\\n"
        "<ClientMessage>WM_PROTOCOLS:   WMprotocol()\\n"
        "<ClientMessage>DELETE:         popdown()",
#ifndef   XAW3D
    (String) "*players.?.width:                    816",
#else  /* XAW3D */
    (String) "*players.?.width:                    834",
#endif /* XAW3D */
    (String) "*players.?.height:                   600",
    (String) "*players*set.height:                 50",
/*  (String) "*players*set.resizeToPreferred:      True", */
/*  (String) "*players.collect.strip.allowResize:  True",
    (String) "*players.collect.strip.resizeToPreferred:     True", */
    (String) "*players.collect.?.skipAdjust:       True",
    (String) "*players.collect.info.skipAdjust:    False",
    (String) "*players.collect.viewport.preferredPaneSize: 500",
    (String) "*players.collect.stripform.preferredPaneSize: 20",
/*  (String) "*players.collect.info.height:        80", */
    (String) "*players.collect.info.preferredPaneSize:      80",
    (String) "*players.collect.stats.label:        Players may exist",
    (String) "*players*allowResize.state:          True",
    (String) "*players*close.translations:         #override\\n"
        "Shift<Btn1Down>: help(players_close)\\n"
        "<Btn1Up>: notify() unset() popdown()\\n"
        "<EnterWindow>: helpup(players_close,"
            "\"Popdown the players widget.\")\\n"
        "<LeaveWindow>: helpdown(players_close)",
    (String) "*players*update.translations:        #override\\n"
        "Shift<Btn1Down>: help(players_update)\\n"
        "<Btn1Down>,<Btn1Up>: sendcommand(who) unset()\\n"
        "<EnterWindow>: helpup(players_update,"
            "\"Ask the server for the current users.\")\\n"
        "<LeaveWindow>: helpdown(players_update)",
    (String) "*players*set*Command.foreground:     XtDefaultForeground",
    (String) "*players*set*Command.background:     "GREEN4,
    (String) "*players*set*playerEntryk.background:"GREEN3,
    (String) "*players*set*playerEntryd.background:"GREEN2,
    (String) "*players*set*playerEntryp.background:"GREEN1,
    (String) "*players*set*playerEntryMarked.background: #FDEDC8",
    (String) "*players*set*playerEntryMarked.foreground: cornflowerblue",
    (String) "*players*set*Command.translations:   #override\\n"
        "Shift<Btn1Down>: help(players_player)\\n"
        "<EnterWindow>: helpup(players_player,"
            "\"Shows info on a logged on player.\")\\n"
        "<Btn2Down>: set()\\n"
        "<Btn3Down>: set()\\n"
        "Ctrl<Btn1Up>:  unset() doplayer(" MARKPLAYER ") \\n"
        "<Btn1Up>:      doplayer(" TELLPLAYER ") unset()\\n"
        "<Btn2Up>:      doplayer(" OBSERVEPLAYER ") unset()\\n"
        "Ctrl<Btn3Up>:  doplayer(" CHALLENGEPLAYER ") unset()\\n"
        "<Btn3Up>:      doplayer(" STATSPLAYER ") unset()",
    (String) "*players*stats.translations:         #override\\n"
        "Shift<Btn1Down>: help(players_stats)\\n"
        "<EnterWindow>: helpup(players_stats,"
            "\"Shows the number of players and games.\")\\n"
        "<LeaveWindow>: helpdown(players_stats)",
    (String) "*players*strip.translations:         #override\\n"
        "Shift<Btn1Down>: help(players_strip)\\n"
        "<EnterWindow>: helpup(players_strip,"
            "\"Shows a graph of the number of logged on players.\")\\n"
        "<LeaveWindow>: helpdown(players_strip)",
    (String) "*players*info.translations:          #override\\n"
        "Shift<Btn1Down>: help(players_info)\\n"
        "<EnterWindow>: helpup(players_info,"
            "\"Shows a history of connect/disconnect messages.\")\\n"
        "<LeaveWindow>: helpdown(players_info)",

    (String) "*games.title:                        Games",
    (String) "*games.iconName:                     go games",
    (String) "*games.translations:                 #override\\n"
        "<ClientMessage>WM_PROTOCOLS:   WMprotocol()\\n"
        "<ClientMessage>DELETE:         popdown()",
#ifndef   XAW3D
    (String) "*games.?.width:                      520",
#else  /* XAW3D */
    (String) "*games.?.width:                      529",
#endif /* XAW3D */
    (String) "*games.?.height:                     600",
    (String) "*games*set.height:                   50",
/*  (String) "*games*set.resizeToPreferred:        True", */
/*  (String) "*games.collect.strip.allowResize:    True",
    (String) "*games.collect.strip.resizeToPreferred: True", */
    (String) "*games.collect.viewport.preferredPaneSize: 500",
    (String) "*games.collect.stripform.preferredPaneSize: 20",
/*  (String) "*games.collect.info.height:          80", */
    (String) "*games.collect.info.preferredPaneSize: 80",
    (String) "*games.collect.?.skipAdjust:         True",
    (String) "*games.collect.info.skipAdjust:      False",
    (String) "*games.collect.strip.skipAdjust:     False",
    (String) "*games*allowResize.state:            True",
    (String) "*games*close.translations:           #override\\n"
        "Shift<Btn1Down>: help(games_close)\\n"
        "<Btn1Up>: notify() unset() popdown()\\n"
        "<EnterWindow>: helpup(games_close,"
            "\"Popdown the games widget.\")\\n"
        "<LeaveWindow>: helpdown(games_close)",
    (String) "*games*update.translations:          #override\\n"
        "Shift<Btn1Down>: help(games_update)\\n"
        "<Btn1Down>,<Btn1Up>: sendcommand(games) unset()\\n"
        "<EnterWindow>: helpup(games_update,"
            "\"Ask the server for the current games.\")\\n"
        "<LeaveWindow>: helpdown(games_update)",
    (String) "*games*set*Toggle.translations:      #override\\n"
        "Shift<Btn1Down>: help(games_game)\\n"
   META "<Btn3Down>:  dogame(" DUMPGAME ")\\n"
        "<EnterWindow>: helpup(games_game,"
            "\"Shows information about a game in progress.\")\\n"
        "Ctrl<Btn1Down>:   toggle() dogame(" GAME_PLAYERS ") toggle()\\n"
        "Alt<Btn1Down>:    toggle() dogame(" GAME_PLAYERS_MAIN ") toggle()\\n"
        "<Btn1Down>,<Btn1Up>: toggle() dogame(" GAME_OBSERVE ")         \\n"
        "<Btn2Down>,<Btn2Up>: toggle() dogame(" GAME_STATUS ") toggle()\\n"
        "<Btn3Down>,<Btn3Up>: toggle() dogame(" GAME_OBSERVERS ") toggle()",
    (String) "*games*set*Toggle.foreground:        XtDefaultForeground",
    (String) "*games*set*Toggle.background:        "GREEN4,
    (String) "*games*set*gameEntryk.background:    "GREEN3,
    (String) "*games*set*gameEntryd.background:    "GREEN2,
    (String) "*games*set*gameEntryp.background:    "GREEN1,
    (String) "*games*strip.translations:           #override\\n"
        "Shift<Btn1Down>: help(games_strip)\\n"
        "<EnterWindow>: helpup(games_strip,"
            "\"Shows a graph of the number of games in progress.\")\\n"
        "<LeaveWindow>: helpdown(games_strip)",
    (String) "*games*info.translations:            #override\\n"
        "Shift<Btn1Down>: help(games_info)\\n"
        "<EnterWindow>: helpup(games_info,"
            "\"Shows a history of messages about the games.\")\\n"
        "<LeaveWindow>: helpdown(games_info)",

    (String) "*reviews.title:                      Reviews",
    (String) "*reviews.iconName:                   go reviews",
    (String) "*reviews.translations:               #override\\n"
        "<ClientMessage>WM_PROTOCOLS:   WMprotocol()\\n"
        "<ClientMessage>DELETE:         popdown()",
#ifndef   XAW3D
    (String) "*reviews.?.width:                    516",
#else  /* XAW3D */
    (String) "*reviews.?.width:                    527",
#endif /* XAW3D */
    (String) "*reviews*set*Command.width:          490",
    (String) "*reviews.?.height:                   150",
    (String) "*reviews*set.height:                 50",
    (String) "*reviews*allowResize.state:          False",
    (String) "*reviews*close.translations:         #override\\n"
        "Shift<Btn1Down>: help(reviews_close)\\n"
        "<Btn1Up>: notify() unset() popdown()\\n"
        "<EnterWindow>: helpup(reviews_close,"
            "\"Pop down the reviews widget.\")\\n"
        "<LeaveWindow>: helpdown(reviews_close)",
    (String) "*reviews*update.translations:        #override\\n"
        "Shift<Btn1Down>: help(reviews_update)\\n"
        "<Btn1Down>,<Btn1Up>: sendcommand(reviews)\\n"
        "<EnterWindow>: helpup(reviews_update,"
            "\"Ask the server for the currently available reviews.\")\\n"
        "<LeaveWindow>: helpdown(reviews_update)",
/*  (String) "*reviews*set*resizeToPreferred:True", */
    (String) "*reviews*set*Command.translations:   #override\\n"
        "Shift<Btn1Down>: help(reviews_game)\\n"
        "Ctrl<Btn1Down>:  dogame(" DUMPGAME ")\\n"
        "<EnterWindow>: helpup(reviews_game,"
            "\"Shows information about a game you can review.\")\\n"
        "<LeaveWindow>: helpdown(reviews_game)\\n"
        "<Btn1Down>,<Btn1Up>: doreview(" GAME_OBSERVE ")\\n"
        "<Btn2Down>,<Btn2Up>: doreview(" GAME_STATUS ")\\n"
        "<Btn3Down>,<Btn3Up>: doreview(" GAME_OBSERVERS ") ",
    (String) "*reviews*set*Command.foreground:     XtDefaultForeground",
    (String) "*reviews*set*Command.background:     "GREEN4,
    (String) "*reviews*set*reviewEntry.background: "GREEN1,
    (String) "*reviews*set*reviewEntryPending.background: "GREEN2,
    (String) "*reviews*set*reviewEntryDone.background:    "GREEN3,

    (String) "*igsMessages.translations:           #override\\n"
        "<ClientMessage>WM_PROTOCOLS:   WMprotocol()\\n"
        "<ClientMessage>DELETE:         popdown()",
    (String) "*igsMessages.title:                  IGS messages",
    (String) "*igsMessages.iconName:               IGS messages",
    (String) "*igsMessages.?.width:                600",
    (String) "*igsMessages.?.height:               150",
    (String) "*igsMessages*close.translations:     #override\\n"
        "Shift<Btn1Down>: help(messages_close)\\n"
        "<Btn1Up>: notify() unset() popdown()\\n"
        "<EnterWindow>: helpup(messages_close,"
            "\"Popdown the window with IGS messages.\")\\n"
        "<LeaveWindow>: helpdown(messages_close)",
    (String) "*igsMessages*save.translations:      #override\\n"
        "Shift<Btn1Down>: help(messages_save)\\n"
        "<EnterWindow>: helpup(messages_save,"
            "\"Save the current IGS messages to a file.\")\\n"
        "<LeaveWindow>: helpdown(messages_save)",
    (String) "*igsMessages*info.translations:      #override\\n"
        "Shift<Btn1Down>: help(messages_info)\\n"
        "<EnterWindow>: helpup(messages_info,"
            "\"Shows important IGS related messages.\")",

    (String) "*events.translations:                #override\\n"
        "<ClientMessage>WM_PROTOCOLS:   WMprotocol()\\n"
        "<ClientMessage>DELETE:         popdown()",
    (String) "*events.title:                       Events",
    (String) "*events.iconName:                    go events",
    (String) "*events.?.width:                     700",
    (String) "*events.?.height:                    210",
    (String) "*events*close.translations:          #override\\n"
        "Shift<Btn1Down>: help(events_close)\\n"
        "<Btn1Up>: notify() unset() popdown()\\n"
        "<EnterWindow>: helpup(events_close,"
            "\"Popdown the window with event related messages.\")\\n"
        "<LeaveWindow>: helpdown(events_close)",
    (String) "*events*save.translations:           #override\\n"
        "Shift<Btn1Down>: help(events_save)\\n"
        "<EnterWindow>: helpup(events_save,"
            "\"Save the current event related messages to a file.\")\\n"
        "<LeaveWindow>: helpdown(events_save)",
    (String) "*events*info.translations:           #override\\n"
        "Shift<Btn1Down>: help(events_info)\\n"
        "<EnterWindow>: helpup(events_info,"
            "\"Shows event related messages.\")",

    (String) "*broadcasts.translations:            #override\\n"
        "<ClientMessage>WM_PROTOCOLS:   WMprotocol()\\n"
        "<ClientMessage>DELETE:         popdown()",
    (String) "*broadcasts.title:                   Broadcasts",
    (String) "*broadcasts.iconName:                go broadcasts",
    (String) "*broadcasts.?.width:                 550",
    (String) "*broadcasts.?.height:                210",
    (String) "*broadcasts.collect.?.resizeToPreferred: True",
    (String) "*broadcasts.collect.?.skipAdjust:    True",
    (String) "*broadcasts.collect.info.skipAdjust: False",
    (String) "*broadcasts*close.translations:      #override\\n"
        "Shift<Btn1Down>: help(broadcasts_close)\\n"
        "<Btn1Up>: notify() unset() popdown()\\n"
        "<EnterWindow>: helpup(broadcasts_close,"
            "\"Popdown the broadcasts widget.\")\\n"
        "<LeaveWindow>: helpdown(broadcasts_close)",
    (String) "*broadcasts*save.translations:       #override\\n"
        "Shift<Btn1Down>: help(broadcasts_save)\\n"
        "<EnterWindow>: helpup(broadcasts_save,"
            "\"Save the current broadcasts (shouts) to a file.\")\\n"
        "<LeaveWindow>: helpdown(broadcasts_save)",
    (String) "*broadcasts*info.translations:       #override\\n"
        "Shift<Btn1Down>: help(broadcasts_info)\\n"
        "<EnterWindow>: helpup(broadcasts_info,"
            "\"Shows the current broadcasts (shouts).\")",
    (String) "*broadcasts.collect.input.translations: #override\\n"
        "Shift<Btn1Down>: help(broadcast_input)\\n"
        "<EnterWindow>: helpup(broadcast_input,"
            "\"Send a message to everybody on the server (shout).\")\\n"
        "<LeaveWindow>: helpdown(broadcast_input)\\n"
        ONENTER("broadcast()") "\\n"
        USER_TRANS
        META "<Key>c:   change()  \\n"
        META "<Key>w:   widgets() \\n"
        META "<Key>h:   info()",

    (String) "*yells.translations:                 #override\\n"
        "<ClientMessage>WM_PROTOCOLS:   WMprotocol()\\n"
        "<ClientMessage>DELETE:         popdown()",
    (String) "*yells.title:                        Channels",
    (String) "*yells.iconName:                     go channels",
    (String) "*yells.?.width:                      530",
    (String) "*yells.?.height:                     210",
    (String) "*yells.collect.?.resizeToPreferred:  True",
    (String) "*yells.collect.?.skipAdjust:         True",
    (String) "*yells.collect.info.skipAdjust:      False",
    (String) "*yells*close.translations:           #override\\n"
        "Shift<Btn1Down>: help(yells_close)\\n"
        "<Btn1Up>: notify() unset() popdown()\\n"
        "<EnterWindow>: helpup(yells_close,"
            "\"Popdown the channels widget.\")\\n"
        "<LeaveWindow>: helpdown(yells_close)",
    (String) "*yells*save.translations:            #override\\n"
        "Shift<Btn1Down>: help(yells_save)\\n"
        "<EnterWindow>: helpup(yells_save,"
            "\"Save the current channel messages (yells) to a file.\")\\n"
        "<LeaveWindow>: helpdown(yells_save)",
    (String) "*yells*channels.translations:        #override\\n"
        "Shift<Btn1Down>: help(yells_channels)\\n"
        "<EnterWindow>: helpup(yells_channels,"
            "\"Show who is on which channel.\")\\n"
        "<LeaveWindow>: helpdown(yells_channels)",
    (String) "*yells*channel*string:               (None)",
    (String) "*yells*channel*editType:             edit",
    (String) "*yells*channel.displayCaret:         True",
    (String) "*yells*channel.resize:               never",
    (String) "*yells*channel.translations:         #override\\n"
        "Shift<Btn1Down>: help(yells_channel)\\n"
        "<EnterWindow>: setfocus() "
            "setvalues(\"textSource(*yells*channel)\", string,\"\") "
            "helpup(yells_channel,"
            "\"Enter channel you want to move to.\")\\n"
        "<LeaveWindow>:  setfocus(\"*input\") "
            "changechannel() "
            "helpdown(yells_channel)\\n"
        ONENTER("changechannel()") "\\n"
        USER_TRANS
        META "<Key>c:   change()    \\n"
        META "<Key>w:   widgets()   \\n"
        META "<Key>h:   info()",
    (String) "*yells*title.translations:           #override\\n"
        "Shift<Btn1Down>: help(yells_title)\\n"
        "<EnterWindow>:  setfocus() "
            "setvalues(*yells*title, displayCaret, True) "
            "helpup(yells_title,"
            "\"Shows the title of the channel you are on.\")\\n"
        "<LeaveWindow>:  setfocus(\"*input\") "
            "setvalues(*yells*title, displayCaret, False) "
            "helpdown(yells_title)\\n"
        ONENTER("changechanneltitle()"),
    (String) "*yells*title*editType:               edit",
    (String) "*yells*title.displayCaret:           False",
    (String) "*yells*moderator.translations:       #override\\n"
        "Shift<Btn1Down>: help(yells_moderator)\\n"
        "<EnterWindow>: helpup(yells_moderator,"
            "\"Shows the moderator of the channel you are on.\")\\n"
        "<LeaveWindow>: helpdown(yells_moderator)",
    (String) "*yells*moderator.width:              1",
    (String) "*yells*moderator.displayCaret:       False",
    (String) "*yells*state.translations:           #override\\n"
        "Shift<Btn1Down>: help(yells_state)\\n"
        "<EnterWindow>: helpup(yells_state,"
            "\"Shows the state of the channel you are on.\")\\n"
        "<LeaveWindow>: helpdown(yells_state)",
    (String) "*yells*state.width:                  1",
    (String) "*yells*state.displayCaret:           False",
    (String) "*yells*info.translations:            #override\\n"
        "Shift<Btn1Down>: help(yells_info)\\n"
        "<EnterWindow>: helpup(yells_info,"
            "\"Shows the current channel messages (yells).\")",
    (String) "*yells.collect.input.translations:   #override\\n"
        "Shift<Btn1Down>: help(yells_input)\\n"
        "<EnterWindow>: helpup(yells_input,"
            "\"Sends a message (yell) on the current channel.\")\\n"
        "<LeaveWindow>: helpdown(yells_input)\\n"
        ONENTER("yell()") "\\n"
        USER_TRANS
        META "<Key>c:   change()  \\n"
        META "<Key>w:   widgets() \\n"
        META "<Key>h:   info()",

    (String) "*tell.title:                         Talking to %N[%n]%A",
    (String) "*tell.iconName:                      %N[%n]%A",
    (String) "*tell.?.width:                       600",
    (String) "*tell.?.height:                      210",
    (String) "*tell.translations:                  #override\\n"
        "<ClientMessage>WM_PROTOCOLS:   WMprotocol()\\n"
        "<ClientMessage>DELETE:         destroy()",
    (String) "*tell*bug.label:                     Beep",
    (String) "*tell.collect.?.allowResize:         True",
    (String) "*tell.collect*resizeToPreferred:     True",
    (String) "*tell.collect.?.skipAdjust:          True",
    (String) "*tell.collect.info.skipAdjust:       False",
    (String) "*tell*close.translations:            #override\\n"
        "Shift<Btn1Down>: help(tell_close)\\n"
        "<Btn1Up>: notify() unset() destroy(\"\")\\n"
        "<EnterWindow>: helpup(tell_close,"
            "\"Quit talking and destroy widget.\")\\n"
        "<LeaveWindow>: helpdown(tell_close)",
    (String) "*tell*save.translations:             #override\\n"
        "Shift<Btn1Down>: help(tell_save)\\n"
        "<EnterWindow>: helpup(tell_save,"
            "\"Save the current messages to a file.\")\\n"
        "<LeaveWindow>: helpdown(tell_save)",
    (String) "*tell*bug.translations:              #override\\n"
        "Shift<Btn1Down>: help(tell_beep)\\n"
        "<EnterWindow>: helpup(tell_beep,"
            "\"Send a beep command to the other person.\")\\n"
        "<LeaveWindow>: helpdown(tell_beep)",
    (String) "*tell*getStats.translations:         #override\\n"
        "Shift<Btn1Down>: help(tell_stats)\\n"
        "<EnterWindow>: helpup(tell_stats,"
            "\"Get extra info about the other person.\")\\n"
        "<LeaveWindow>: helpdown(tell_stats)",
    (String) "*tell*playerObserve.translations:      #override\\n"
        "Shift<Btn1Down>: help(tell_observe)\\n"
        "<EnterWindow>: helpup(tell_observe,"
            "\"Observe the other person.\")\\n"
        "<LeaveWindow>: helpdown(tell_observe)",
    (String) "*tell*getChallenge.translations:         #override\\n"
        "Shift<Btn1Down>: help(tell_challenge)\\n"
        "<EnterWindow>: helpup(tell_challenge,"
            "\"Popup a widget to challenge the other person.\")\\n"
        "<LeaveWindow>: helpdown(tell_challenge)",
    (String) "*tell*info.translations:             #override\\n"
        "Shift<Btn1Down>: help(tell_info)\\n"
        "<EnterWindow>: helpup(tell_info,"
            "\"Shows the current messages.\")",
    (String) "*tell.collect.input.translations:    #override\\n"
        "Shift<Btn1Down>: help(tell_input)\\n"
        "<EnterWindow>: helpup(tell_input,"
            "\"Send a message to the other person.\")\\n"
        "<LeaveWindow>: helpdown(tell_input)\\n"
        ONENTER("tell()") "\\n"
        USER_TRANS
        META "<Key>c:   change() \\n"
        META "<Key>w:   widgets()\\n"
        META "<Key>h:   info()",

    (String) "*stats.title:                        Stats of %N[%n]%A",
    (String) "*stats.iconName:                     stats of %N[%n]%A",
    (String) "*stats.allowShellResize:             True",
    (String) "*stats.?.width:                      570",
/* high enough to allow text widgets to grow */
    (String) "*stats.?.height:                     220",
    (String) "*stats.translations:                 #override\\n"
        "<ClientMessage>WM_PROTOCOLS:   WMprotocol()\\n"
        "<ClientMessage>DELETE:         destroy(\"\")",
    (String) "*stats.collect.?*foreground:         XtDefaultForeground",
    (String) "*stats.collect.?*background:         Green",
    (String) "*stats.collect.close.foreground:     Yellow",
    (String) "*stats.collect.close.background:     Red",
#ifndef   XAW3D
    (String) "*stats.collect.close.shapeStyle:     roundedRectangle",
    (String) "*stats.collect.getTell.shapeStyle:   roundedRectangle",
    (String) "*stats.collect.playerObserve.shapeStyle: roundedRectangle",
#endif /* XAW3D */
    (String) "*stats*Text.resize:                  both",
    (String) "*stats*info.scrollVertical:          never",
    (String) "*stats*Text.width:                   1",
    (String) "*stats*Text.wrap:                    never",
    (String) "*stats*Text.displayCaret:            False",
    (String) "*stats*Text.translations:            #override\\n"
        USER_TRANS
        META "<Key>c:   change() \\n"
        META "<Key>w:   widgets()\\n"
        META "<Key>h:   info()",
    (String) "*stats*statsMyInfo*editType:         edit",
    (String) "*stats*statsMyInfo.displayCaret:     True",
    (String) "*stats*statsMyInfo.translations:     #override\\n"
        "Shift<Btn1Down>: help(stats_myinfo)\\n"
        "<EnterWindow>: helpup(stats_myinfo,"
            "\"Update your info and press return to set the new info.\")\\n"
        ONENTER("setinfo()") "\\n"
        USER_TRANS
        META "<Key>c:   change() \\n"
        META "<Key>w:   widgets()\\n"
        META "<Key>h:   info()",
    (String) "*stats*close.translations:           #override\\n"
        "Shift<Btn1Down>: help(stats_close)\\n"
        "<Btn1Up>: notify() unset() destroy(\"\")\\n"
        "<EnterWindow>: helpup(stats_close,"
            "\"Destroy this stats widget.\")\\n"
        "<LeaveWindow>: helpdown(stats_close)",
    (String) "*stats*getTell.translations:     #override\\n"
        "Shift<Btn1Down>: help(stats_tell)\\n"
        "<EnterWindow>: helpup(stats_tell,"
            "\"Open a tell window to talk to this player.\")\\n"
        "<LeaveWindow>: helpdown(stats_tell)",
    (String) "*stats*playerObserve.translations:      #override\\n"
        "Shift<Btn1Down>: help(stats_observe)\\n"
        "<EnterWindow>: helpup(stats_observe,"
            "\"Observe the other person.\")\\n"
        "<LeaveWindow>: helpdown(stats_observe)",
    (String) "*stats*statsVerbose.translations:      #override\\n"
        "Shift<Btn1Down>: help(stats_verbose)\\n"
        "<EnterWindow>: helpup(stats_verbose,"
            "\"Toggle verbose.\")\\n"
        "<LeaveWindow>: helpdown(stats_verbose)",
    (String) "*stats*statsBell.translations:      #override\\n"
        "Shift<Btn1Down>: help(stats_bell)\\n"
        "<EnterWindow>: helpup(stats_bell,"
            "\"Toggle bell.\")\\n"
        "<LeaveWindow>: helpdown(stats_bell)",
    (String) "*stats*statsQuiet.translations:      #override\\n"
        "Shift<Btn1Down>: help(stats_quiet)\\n"
        "<EnterWindow>: helpup(stats_quiet,"
            "\"Toggle quiet.\")\\n"
        "<LeaveWindow>: helpdown(stats_quiet)",
    (String) "*stats*statsShout.translations:      #override\\n"
        "Shift<Btn1Down>: help(stats_shout)\\n"
        "<EnterWindow>: helpup(stats_shout,"
            "\"Toggle shout.\")\\n"
        "<LeaveWindow>: helpdown(stats_shout)",
    (String) "*stats*statsAutomail.translations:      #override\\n"
        "Shift<Btn1Down>: help(stats_automail)\\n"
        "<EnterWindow>: helpup(stats_automail,"
            "\"Toggle automail.\")\\n"
        "<LeaveWindow>: helpdown(stats_automail)",
    (String) "*stats*statsOpen.translations:      #override\\n"
        "Shift<Btn1Down>: help(stats_open)\\n"
        "<EnterWindow>: helpup(stats_open,"
            "\"Toggle open.\")\\n"
        "<LeaveWindow>: helpdown(stats_open)",
    (String) "*stats*statsLooking.translations:      #override\\n"
        "Shift<Btn1Down>: help(stats_looking)\\n"
        "<EnterWindow>: helpup(stats_looking,"
            "\"Toggle looking.\")\\n"
        "<LeaveWindow>: helpdown(stats_looking)",
    (String) "*stats*statsClient.translations:      #override\\n"
        "Shift<Btn1Down>: help(stats_client)\\n"
        "<EnterWindow>: helpup(stats_client,"
            "\"Toggle client.\")\\n"
        "<LeaveWindow>: helpdown(stats_client)",
    (String) "*stats*statsKibitz.translations:      #override\\n"
        "Shift<Btn1Down>: help(stats_kibitz)\\n"
        "<EnterWindow>: helpup(stats_kibitz,"
            "\"Toggle kibitz.\")\\n"
        "<LeaveWindow>: helpdown(stats_kibitz)",
    (String) "*stats*statsChatter.translations:      #override\\n"
        "Shift<Btn1Down>: help(stats_chatter)\\n"
        "<EnterWindow>: helpup(stats_chatter,"
            "\"Toggle chatter.\")\\n"
        "<LeaveWindow>: helpdown(stats_chatter)",

    (String) "*title:                              %N v %v",
    (String) "*iconName:                           %N",
    (String) "?.translations:                      #override\\n"
/*
        "<LeaveWindow>: setvalues(*shortHelp, label,"
            "\"Hey, where is your pointer going ?\")\\n"
        "<EnterWindow>: setvalues(*shortHelp, label," DEFTEXT ")\\n"
*/
        "<ClientMessage>WM_PROTOCOLS:   WMprotocol()\\n"
        "<ClientMessage>DELETE:         output(Please log out in a clean way)\\n"
        "<ClientMessage>XGOSPEL_TRIP_PROBE: tripmessage()",
#ifdef HAVE_XPM
    /* Some window managers cannot do this. In that case use the bitmap */
    (String) "?.iconPixmap:                        pixmap(builtin(XgospelIcon))",
#else /* HAVE_XPM */
    (String) "?.iconPixmap:                        bitmap(builtin(XgospelIcon))",
#endif /* HAVE_XPM */

    (String) "*main.?.width:                       655",
    (String) "*main.?.height:                      350",
    (String) "*main.collect.?.resizeToPreferred:   True",
    (String) "*main.collect.?.skipAdjust:          True",
    (String) "*main.collect.info.skipAdjust:       False",
    (String) "*main*shortHelp.label: "
        "Press shift left mouse button for more info on a widget.",
#ifdef    XAW3D
    (String) "*main*shortHelp.shadowWidth:         0",
#endif /* XAW3D */
    (String) "*main*info.translations:             #override\\n"
        "Shift<Btn1Down>: help(main_info)\\n"
        "<EnterWindow>: helpup(main_info," DEFTEXT ")",
    /* (String) "*main*options.cursorName:            leftbutton", */
    (String) "*main*commands.cursorName:           hand2",
    (String) "*main*gamesButton.cursorName:        circle",
    (String) "*main*playersButton.cursorName:      gumby",
    (String) "*main*messageButton.cursorName:      pencil",
    (String) "*main*broadcastButton.cursorName:    spraycan",
    (String) "*main*yellButton.cursorName:         exchange",
    (String) "*main*userButton.cursorName:         heart",
    (String) "*main*eventsButton.cursorName:       spider",
    (String) "*main*helpButton.cursorName:         question_arrow",
    (String) "*main*quit.cursorName:               pirate",
    (String) "*main*quit.state:                    True",
    (String) "*main*quit.translations:             #override\\n"
        "Shift<Btn1Down>: help(main_quit)\\n"
        "<EnterWindow>: helpup(main_quit,"
            "\"Terminate with extreme prejudice.\")\\n"
        "<LeaveWindow>: helpdown(main_quit)",
    (String) "*main*gamesButton.translations:      #override\\n"
        "Shift<Btn1Down>: help(main_games)\\n"
        "<EnterWindow>: helpup(main_games,"
            "\"Toggle display of games in progress.\")\\n"
        "<LeaveWindow>: helpdown(main_games)\\n"
        "Ctrl<Btn3Down>,<Btn3Up>: setvalues(*games, iconic, true)\\n"
        "<Btn3Down>,<Btn3Up>: setvalues(,state, true)  popup(*games) setvalues(*games, iconic, false)",
    (String) "*main*playersButton.translations:    #override\\n"
        "Shift<Btn1Down>: help(main_players)\\n"
        "<EnterWindow>: helpup(main_players,"
            "\"Toggle display of players logged on.\")\\n"
        "<LeaveWindow>: helpdown(main_players)\\n"
        "Ctrl<Btn3Down>,<Btn3Up>: setvalues(*players, iconic, true)\\n"
        "<Btn3Down>,<Btn3Up>: setvalues(,state, true)  popup(*players) setvalues(*players, iconic, false)",
    (String) "*main*messageButton.translations:    #override\\n"
        "Shift<Btn1Down>: help(main_messages)\\n"
        "<EnterWindow>: helpup(main_messages,"
            "\"Toggle display of important server messages.\")\\n"
        "<LeaveWindow>: helpdown(main_messages)\\n"
        "Ctrl<Btn3Down>,<Btn3Up>: setvalues(*igsMessages, iconic, true)\\n"
        "<Btn3Down>,<Btn3Up>: setvalues(,state, true)  popup(*igsMessages) setvalues(*igsMessages, iconic, false)",
    (String) "*main*broadcastButton.translations:  #override\\n"
        "Shift<Btn1Down>: help(main_broadcasts)\\n"
        "<EnterWindow>: helpup(main_broadcasts,"
            "\"Toggle display of broadcast widget.\")\\n"
        "<LeaveWindow>: helpdown(main_broadcasts)\\n"
        "Ctrl<Btn3Down>,<Btn3Up>: setvalues(*broadcasts, iconic, true)\\n"
        "<Btn3Down>,<Btn3Up>: setvalues(,state, true)  popup(*broadcasts) setvalues(*broadcasts, iconic, false)",
    (String) "*main*yellButton.translations:       #override\\n"
        "Shift<Btn1Down>: help(main_channels)\\n"
        "<EnterWindow>: helpup(main_channels,"
            "\"Toggle display of channel widget.\")\\n"
        "<LeaveWindow>: helpdown(main_channels)\\n"
        "<Btn2Down>,<Btn2Up>: toggle() usercommand(channels) toggle()\\n"
        "Ctrl<Btn3Down>,<Btn3Up>: setvalues(*yells, iconic, true)\\n"
        "<Btn3Down>,<Btn3Up>: setvalues(,state, true)  popup(*yells) setvalues(*yells, iconic, false)",
#ifdef DO_REVIEW
    (String) "*main*reviewsButton.translations:    #override\\n"
        "Shift<Btn1Down>: help(main_reviews)\\n"
        "<EnterWindow>: helpup(main_reviews,"
            "\"Toggle display of available saved games.\")\\n"
        "<LeaveWindow>: helpdown(main_reviews)\\n"
        "Ctrl<Btn3Down>,<Btn3Up>: setvalues(*reviews, iconic, true)\\n"
        "<Btn3Down>,<Btn3Up>: setvalues(,state, true)  popup(*reviews) setvalues(*events, iconic, false)",
#endif
    (String) "*main*userButton.translations:    #override\\n"
        "Shift<Btn1Down>: help(main_user)\\n"
        "<EnterWindow>: helpup(main_user,"
            "\"Execute all IGS commands contained in ~/.xgospelrc\")\\n"
        "<LeaveWindow>: helpdown(main_user)",
    (String) "*main*eventsButton.translations:     #override\\n"
        "Shift<Btn1Down>: help(main_events)\\n"
        "<EnterWindow>: helpup(main_events,"
            "\"Toggle display of events.\")\\n"
        "<LeaveWindow>: helpdown(main_events)\\n"
        "Ctrl<Btn3Down>,<Btn3Up>: setvalues(*events, iconic, true)\\n"
        "<Btn3Down>,<Btn3Up>: setvalues(,state, true)  popup(*events) setvalues(*events, iconic, false)",
    (String) "*main*helpButton.translations:     #override\\n"
        "Shift<Btn1Down>: help(main_help)\\n"
        "<EnterWindow>: helpup(main_help,"
            "\"Show or hide help windows.\")\\n"
        "<LeaveWindow>: helpdown(main_help)",
    (String) "*main*localTime.translations:        #override\\n"
        "Shift<Btn1Down>: help(main_localTime)\\n"
        "<EnterWindow>: helpup(main_localTime,"
            "\"Displays your local time.\")\\n"
        "<LeaveWindow>: helpdown(main_localTime)",
    (String) "*main*universalTime.translations:    #override\\n"
        "Shift<Btn1Down>: help(main_universalTime)\\n"
        "<EnterWindow>: helpup(main_universalTime,"
            "\"Displays universal time.\")\\n"
        "<LeaveWindow>: helpdown(main_universalTime)",
    (String) "*main*serverTime.translations:       #override\\n"
        "Shift<Btn1Down>: help(main_serverTime)\\n"
        "<EnterWindow>: helpup(main_serverTime,"
            "\"Displays IGS server local time.\")\\n"
        "<LeaveWindow>: helpdown(main_serverTime)",
    (String) "*main*shortHelp.translations:        #override\\n"
        "Shift<Btn1Down>: help(main_shortHelp)\\n"
        "<EnterWindow>: helpup(main_shortHelp,"
            "\"Gives short help on widget.\")\\n"
        "<LeaveWindow>: helpdown(main_shortHelp)",
    (String) "*main.collect.input.translations:    #override\\n"
        "Shift<Btn1Down>: help(main_input)\\n"
        "<EnterWindow>: helpup(main_input,"
            "\"Enter server commands.\")\\n"
        "<LeaveWindow>: helpdown(main_input)\\n"
        ONENTER("igscommand()") "\\n"
        USER_TRANS
        META "<Key>c:   change()  \\n"
        META "<Key>w:   widgets() \\n"
        META "<Key>h:   info()",
    (String) "*main*wantConnect.state:             True",
    (String) "*main*connect.label:                 connection with %S %P:",
    (String) "*main*hasConnect.label:              have it",
    (String) "*main*wantConnect.label:             want it",

    (String) "*quitConfirm*confirm.label:          Do you really want to quit ?",
    (String) "*quitConfirm*Paned.?.showGrip:       False",
    (String) "*quitConfirm*Paned*Paned.orientation: horizontal",
    (String) "*quitConfirm*Paned*Paned.?.skipAdjust: True",
    (String) "*quitConfirm*Paned*Paned.filler.skipAdjust: False",
    (String) "*quitConfirm*ok.label:               Ok",
    (String) "*quitConfirm*ok.cursorName:          pirate",
    (String) "*quitConfirm*filler.width:           1",
    (String) "*quitConfirm*filler.height:          1",
    (String) "*quitConfirm*cancel.label:           Cancel",
    (String) "*quitConfirm*cancel.translations:    #override\\n"
        "<Btn1Up>: notify() popdown() unset()",
    (String) "*quitConfirm.title:                  Confirm",
    (String) "*quitConfirm.iconName:               Confirm",
    (String) "*quitConfirm.translations:           #override\\n"
        "<ClientMessage>WM_PROTOCOLS:   WMprotocol()\\n"
        "<ClientMessage>DELETE:         popdown()",

    (String) "*askString*allowShellResize:         True",
    (String) "*askString*Label.background:         Green",
    (String) "*askString*Command.background:       Green",
    (String) "*askString*userPassword.label:       Enter user and password",
    (String) "*askString*userLabel.label:          User:    ",
    (String) "*askString*passwordLabel.label:      Password:",
    (String) "*askString*sgfFilenameLabel.label:   sgf filename:",
    (String) "*askString*psFilenameLabel.label:    ps filename:",
    (String) "*askString*kibitzFilenameLabel.label:kibitz filename:",
    (String) "*askString*broadcastFilenameLabel.label: broadcasts filename:",
    (String) "*askString*yellFilenameLabel.label:  channels filename:",
    (String) "*askString*tellFilenameLabel.label:  tell filename:",
    (String) "*askString*serverFilenameLabel.label:igs messages filename:",
    (String) "*askString*eventsFilenameLabel.label:events filename:",
    (String) "*askString*mainFilenameLabel.label:  session filename:",
    (String) "*askString*autoReplyMessageLabel.label:  auto reply message:",
    (String) "*askString*analyzeSizeLabel.label:          analyzeboard size: ",
    (String) "*askString*allowSuicideLabel.label:         allow suicide:     ",
    (String) "*askString*simpleNamesLabel.label:          simple names:      ",
    (String) "*askString*numberKibitzesLabel.label:       number kibitzes:   ",
    (String) "*askString*minSecPerMoveLabel.label:        seconds per move:  ",
    (String) "*askString*minLagMarginLabel.label:         lag  margin:       ",
    (String) "*askString*replayTimeoutLabel.label:        replay rate:       ",
    (String) "*askString*whoTimeoutLabel.label:           who    rate:       ",
    (String) "*askString*gamesTimeoutLabel.label:         games  rate:       ",
#ifdef DO_REVIEW
    (String) "*askString*reviewsTimeoutLabel.label:       review rate:       ",
#endif
    (String) "*askString*clockTimeoutLabel.label:         clock  rate:       ",
    (String) "*askString*playersUpdateTimeoutLabel.label: playerwindow rate: ",
    (String) "*askString*gamesUpdateTimeoutLabel.label:   game  window rate: ",
#ifdef DO_REVIEW
    (String) "*askString*reviewsUpdateTimeoutLabel.label: reviewwindow rate: ",
#endif
    (String) "*askString*serverTimeoutLabel.label:        server timeout:    ",
    (String) "*askString*inactiveTimeoutLabel.label:      time to auto quit: ",
    (String) "*askString*quitTimeoutLabel.label:          quit timeout:      ",
    (String) "*askString*tersePlayLabel.label:            terse play traffic:",
    (String) "*askString*Text*editType:            edit",
    (String) "*askString*Text.resize:              both",
    (String) "*askString*Text.displayCaret:        False",
    (String) "*askString*password*Text*echo:       False",
    (String) "*askString*Paned.?.showGrip:         False",
    (String) "*askString*Paned.?.allowResize:      True",
    (String) "*askString*Paned.?.resizeToPreferred:True",
/*    (String) "*askString*collect.orientation:      vertical", */
    (String) "*askString*collect*Paned.orientation:horizontal",
    (String) "*askString*Text.translations:        #override\\n"
        ASKTEXTTRAN(),
    (String) "*askString*collect.translations:     #override\\n"
        "Shift<Btn1Down>: help(askstring_collect)\\n"
        "<EnterWindow>: helpup(askstring_collect,"
            "\"Enter value.\")",
    (String) "*askString*analyzeSize*Command.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_analyzeSize)\\n"
        "<Btn3Down>: set()\\n"
        "<Btn1Up>:notify() increase(*analyzeSize*text, -1, 2, 25) unset()\\n"
        "<Btn3Up>:notify() increase(*analyzeSize*text,  1, 2, 25) unset()\\n"
        "<EnterWindow>: helpup(askstring_analyzeSize,"
            "\"Enter new default size of analyze board.\")\\n"
        "<LeaveWindow>: helpdown(askstring_analyzeSize)",
    (String) "*askString*analyzeSize*Text.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_analyzeSize)\\n"
        "<EnterWindow>: helpup(askstring_analyzeSize,"
            "\"Enter new default size of analyze board.\")\\n"
        "<LeaveWindow>: helpdown(askstring_analyzeSize)\\n"
        ASKTEXTTRAN(),
    (String) "*askString*allowSuicide*Command.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_allowSuicide)\\n"
        "<Btn3Down>: set()\\n"
        "<Btn1Up>:notify() toggleboolean(*allowSuicide*text) unset()\\n"
        "<Btn3Up>:notify() toggleboolean(*allowSuicide*text) unset()\\n"
        "<EnterWindow>: helpup(askstring_allowSuicide,"
            "\"Should suicide be allowed by default in analyze boards ?\")\\n"
        "<LeaveWindow>: helpdown(askstring_allowSuicide)",
    (String) "*askString*allowSuicide*Text.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_allowSuicide)\\n"
        "<EnterWindow>: helpup(askstring_allowSuicide,"
            "\"Should suicide be allowed by default in analyze boards ?\")\\n"
        "<LeaveWindow>: helpdown(askstring_allowSuicide)\\n"
        ASKTEXTTRAN(),
    (String) "*askString*simpleNames*Command.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_simpleNames)\\n"
        "<Btn3Down>: set()\\n"
        "<Btn1Up>:notify() toggleboolean(*simpleNames*text) unset()\\n"
        "<Btn3Up>:notify() toggleboolean(*simpleNames*text) unset()\\n"
        "<EnterWindow>: helpup(askstring_simpleNames,"
            "\"Use simple player names or full player status for sort ?\")\\n"
        "<LeaveWindow>: helpdown(askstring_simpleNames)",
    (String) "*askString*simpleNames*Text.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_simpleNames)\\n"
        "<EnterWindow>: helpup(askstring_simpleNames,"
            "\"Use simple player names or full player status for sort ?\")\\n"
        "<LeaveWindow>: helpdown(askstring_simpleNames)\\n"
        ASKTEXTTRAN(),
    (String) "*askString*numberKibitzes*Command.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_numberKibitzes)\\n"
        "<Btn3Down>: set()\\n"
        "<Btn1Up>:notify() toggleboolean(*numberKibitzes*text) unset()\\n"
        "<Btn3Up>:notify() toggleboolean(*numberKibitzes*text) unset()\\n"
        "<EnterWindow>: helpup(askstring_numberKibitzes,"
            "\"Add move number before kibitzes ?\")\\n"
        "<LeaveWindow>: helpdown(askstring_numberKibitzes)",
    (String) "*askString*numberKibitzes*Text.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_numberKibitzes)\\n"
        "<EnterWindow>: helpup(askstring_numberKibitzes,"
            "\"Add move number before kibitzes ?\")\\n"
        "<LeaveWindow>: helpdown(askstring_numberKibitzes)\\n"
        ASKTEXTTRAN(),
    (String) "*askString*minSecPerMove*Command.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_minSecPerMove)\\n"
        "<Btn3Down>: set()\\n"
        "<Btn1Up>:notify() increase(*minSecPerMove*text, -1, 0, 120) unset()\\n"
        "<Btn3Up>:notify() increase(*minSecPerMove*text,  1, 0, 120) unset()\\n"
        "<EnterWindow>: helpup(askstring_minSecPerMove,"
            "\"Enter min seconds per move.\")\\n"
        "<LeaveWindow>: helpdown(askstring_minSecPerMove)",
    (String) "*askString*minSecPerMove*Text.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_minSecPerMove)\\n"
        "<EnterWindow>: helpup(askstring_minSecPerMove,"
            "\"Enter min seconds per move.\")\\n"
        "<LeaveWindow>: helpdown(askstring_minSecPerMove)\\n"
        ASKTEXTTRAN(),
    (String) "*askString*minLagMargin*Command.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_minLagMargin)\\n"
        "<Btn3Down>: set()\\n"
        "<Btn1Up>:notify() increase(*minLagMargin*text, -1, 0, 300) unset()\\n"
        "<Btn3Up>:notify() increase(*minLagMargin*text,  1, 0, 300) unset()\\n"
        "<EnterWindow>: helpup(askstring_minLagMargin,"
            "\"Enter seconds of safety margin for netlag.\")\\n"
        "<LeaveWindow>: helpdown(askstring_minLagMargin)",
    (String) "*askString*minLagMargin*Text.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_minLagMargin)\\n"
        "<EnterWindow>: helpup(askstring_minLagMargin,"
            "\"Enter seconds of safety margin for netlag.\")\\n"
        "<LeaveWindow>: helpdown(askstring_minLagMargin)\\n"
        ASKTEXTTRAN(),
    (String) "*askString*replayTimeout*Command.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_replayTimeout)\\n"
        "<Btn3Down>: set()\\n"
        "<Btn1Up>:notify() increase(*replayTimeout*text, -1, 1, 30) unset()\\n"
        "<Btn3Up>:notify() increase(*replayTimeout*text,  1, 1, 30) unset()\\n"
        "<EnterWindow>: helpup(askstring_replayTimeout,"
            "\"Enter new moverate for replay.\")\\n"
        "<LeaveWindow>: helpdown(askstring_replayTimeout)",
    (String) "*askString*replayTimeout*Text.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_replayTimeout)\\n"
        "<EnterWindow>: helpup(askstring_replayTimeout,"
            "\"Enter new moverate for replay.\")\\n"
        "<LeaveWindow>: helpdown(askstring_replayTimeout)\\n"
        ASKTEXTTRAN(),
    (String) "*askString*whoTimeout*Command.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_whoTimeout)\\n"
        "<Btn3Down>: set()\\n"
        "<Btn1Up>:notify() increase(*whoTimeout*text, -60, 120,960) unset()\\n"
        "<Btn3Up>:notify() increase(*whoTimeout*text,  60, 120,960) unset()\\n"
        "<EnterWindow>: helpup(askstring_whoTimeout,"
            "\"Enter minimum rate at which to send who commands.\")",
    (String) "*askString*whoTimeout*Text.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_whoTimeout)\\n"
        "<EnterWindow>: helpup(askstring_whoTimeout,"
            "\"Enter minimum rate at which to send who commands.\")\\n"
        ASKTEXTTRAN(),
    (String) "*askString*gamesTimeout*Command.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_gamesTimeout)\\n"
        "<Btn3Down>: set()\\n"
        "<Btn1Up>:notify() increase(*gamesTimeout*text,-60,120,960) unset()\\n"
        "<Btn3Up>:notify() increase(*gamesTimeout*text, 60,120,960) unset()\\n"
        "<EnterWindow>: helpup(askstring_gamesTimeout,"
            "\"Enter minimum rate at which to send games commands.\")",
    (String) "*askString*gamesTimeout*Text.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_gamesTimeout)\\n"
        "<EnterWindow>: helpup(askstring_gamesTimeout,"
            "\"Enter minimum rate at which to send games commands.\")\\n"
        ASKTEXTTRAN(),
#ifdef DO_REVIEW
    (String) "*askString*reviewsTimeout*Command.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_reviewsTimeout)\\n"
        "<Btn3Down>: set()\\n"
        "<Btn1Up>:notify() increase(*reviewsTimeout*text,-120,600,7200) unset()\\n"
        "<Btn3Up>:notify() increase(*reviewsTimeout*text, 120,600,7200) unset()\\n"
        "<EnterWindow>: helpup(askstring_reviewsTimeout,"
            "\"Enter minimum rate at which to send review commands.\")\\n"
        "<LeaveWindow>: helpdown(askstring_reviewsTimeout)",
    (String) "*askString*reviewsTimeout*Text.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_reviewsTimeout)\\n"
        "<EnterWindow>: helpup(askstring_reviewsTimeout,"
            "\"Enter minimum rate at which to send review commands.\")\\n"
        "<LeaveWindow>: helpdown(askstring_reviewsTimeout)\\n"
        ASKTEXTTRAN(),
#endif
    (String) "*askString*clockTimeout*Command.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_clockTimeout)\\n"
        "<Btn3Down>: set()\\n"
        "<Btn1Up>:notify() increase(*clockTimeout*text,-1,1,60) unset()\\n"
        "<Btn3Up>:notify() increase(*clockTimeout*text, 1,1,60) unset()\\n"
        "<EnterWindow>: helpup(askstring_clockTimeout,"
            "\"Enter rate at which to update the visible clicks.\")\\n"
        "<LeaveWindow>: helpdown(askstring_clockTimeout)",
    (String) "*askString*clockTimeout*Text.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_clockTimeout)\\n"
        "<EnterWindow>: helpup(askstring_clockTimeout,"
            "\"Enter rate at which to update the visible clicks.\")\\n"
        "<LeaveWindow>: helpdown(askstring_clockTimeout)\\n"
        ASKTEXTTRAN(),
    (String) "*askString*inactiveTimeout*Command.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_inactiveTimeout)\\n"
        "<Btn3Down>: set()\\n"
        "<Btn1Up>:notify() increase(*inactiveTimeout*text,-300,300,7200) unset()\\n"
        "<Btn3Up>:notify() increase(*inactiveTimeout*text, 300,300,7200) unset()\\n"
        "<EnterWindow>: helpup(askstring_inactiveTimeout,"
            "\"Enter after how many seconds of inactivity the program should quit.\")\\n"
        "<LeaveWindow>: helpdown(askstring_inactiveTimeout)",
    (String) "*askString*inactiveTimeout*Text.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_inactiveTimeout)\\n"
        "<EnterWindow>: helpup(askstring_inactiveTimeout,"
            "\"Enter after how many seconds of inactivity the program should quit.\")\\n"
        "<LeaveWindow>: helpdown(askstring_inactiveTimeout)\\n"
        ASKTEXTTRAN(),
    (String) "*askString*playersUpdateTimeout*Command.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_playersUpdateTimeout)\\n"
        "<Btn3Down>: set()\\n"
        "<Btn1Up>:notify() increase(*playersUpdateTimeout*text,-5,0,120) unset()\\n"
        "<Btn3Up>:notify() increase(*playersUpdateTimeout*text, 5,0,120) unset()\\n"
        "<EnterWindow>: helpup(askstring_playersUpdateTimeout,"
            "\"Enter rate at which to update the players widget.\")\\n"
        "<LeaveWindow>: helpdown(askstring_playersUpdateTimeout)",
    (String) "*askString*playersUpdateTimeout*Text.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_playersUpdateTimeout)\\n"
        "<EnterWindow>: helpup(askstring_playersUpdateTimeout,"
            "\"Enter rate at which to update the players widget.\")\\n"
        "<LeaveWindow>: helpdown(askstring_playersUpdateTimeout)\\n"
        ASKTEXTTRAN(),
    (String) "*askString*gamesUpdateTimeout*Command.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_gamesUpdateTimeout)\\n"
        "<Btn3Down>: set()\\n"
        "<Btn1Up>:notify() increase(*gamesUpdateTimeout*text,-1,0,30) unset()\\n"
        "<Btn3Up>:notify() increase(*gamesUpdateTimeout*text, 1,0,30) unset()\\n"
        "<EnterWindow>: helpup(askstring_gamesUpdateTimeout,"
            "\"Enter rate at which to update the games widget.\")\\n"
        "<LeaveWindow>: helpdown(askstring_gamesUpdateTimeout)",
    (String) "*askString*gamesUpdateTimeout*Text.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_gamesUpdateTimeout)\\n"
        "<EnterWindow>: helpup(askstring_gamesUpdateTimeout,"
            "\"Enter rate at which to update the games widget.\")\\n"
        "<LeaveWindow>: helpdown(askstring_gamesUpdateTimeout)\\n"
        ASKTEXTTRAN(),
#ifdef DO_REVIEW
    (String) "*askString*reviewsUpdateTimeout*Command.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_reviewsUpdateTimeout)\\n"
        "<Btn3Down>: set()\\n"
        "<Btn1Up>:notify() increase(*reviewsUpdateTimeout*text,-1,0,30) unset()\\n"
        "<Btn3Up>:notify() increase(*reviewsUpdateTimeout*text, 1,0,30) unset()\\n"
        "<EnterWindow>: helpup(askstring_reviewsUpdateTimeout,"
            "\"Enter rate at which to update the reviews widget.\")\\n"
        "<LeaveWindow>: helpdown(askstring_reviewsUpdateTimeout)",
    (String) "*askString*reviewsUpdateTimeout*Text.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_reviewsUpdateTimeout)\\n"
        "<EnterWindow>: helpup(askstring_reviewsUpdateTimeout,"
            "\"Enter rate at which to update the reviews widget.\")\\n"
        "<LeaveWindow>: helpdown(askstring_reviewsUpdateTimeout)\\n"
        ASKTEXTTRAN(),
#endif
    (String) "*askString*serverTimeout*Command.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_serverTimeout)\\n"
        "<Btn3Down>: set()\\n"
        "<Btn1Up>:notify() increase(*serverTimeout*text,-60,60,7200) unset()\\n"
        "<Btn3Up>:notify() increase(*serverTimeout*text, 60,60,7200) unset()\\n"
        "<EnterWindow>: helpup(askstring_serverTimeout,"
            "\"Enter waiting period before resending a failed command.\")\\n"
        "<LeaveWindow>: helpdown(askstring_serverTimeout)",
    (String) "*askString*serverTimeout*Text.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_serverTimeout)\\n"
        "<EnterWindow>: helpup(askstring_serverTimeout,"
            "\"Enter waiting period before resending a failed command.\")\\n"
        "<LeaveWindow>: helpdown(askstring_serverTimeout)\\n"
        ASKTEXTTRAN(),
    (String) "*askString*quitTimeout*Command.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_quitTimeout)\\n"
        "<Btn3Down>: set()\\n"
        "<Btn1Up>:notify() increase(*quitTimeout*text,-1,1,60) unset()\\n"
        "<Btn3Up>:notify() increase(*quitTimeout*text, 1,1,60) unset()\\n"
        "<EnterWindow>: helpup(askstring_quitTimeout,"
            "\"Enter waiting period before cutting connection while quitting.\")\\n"
        "<LeaveWindow>: helpdown(askstring_quitTimeout)",
    (String) "*askString*quitTimeout*Text.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_quitTimeout)\\n"
        "<EnterWindow>: helpup(askstring_quitTimeout,"
            "\"Enter waiting period before cutting connection while quitting.\")\\n"
        "<LeaveWindow>: helpdown(askstring_quitTimeout)\\n"
        ASKTEXTTRAN(),
    (String) "*askString*tersePlay*Command.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_tersePlay)\\n"
        "<Btn3Down>: set()\\n"
        "<Btn1Up>:notify() toggleboolean(*tersePlay*text) unset()\\n"
        "<Btn3Up>:notify() toggleboolean(*tersePlay*text) unset()\\n"
        "<EnterWindow>: helpup(askstring_tersePlay,"
            "\"Should we try to reduce server traffic while involved in a game ?\")\\n"
        "<LeaveWindow>: helpdown(askstring_tersePlay)",
    (String) "*askString*tersePlay*Text.translations: #override\\n"
        "Shift<Btn1Down>: help(askstring_tersePlay)\\n"
        "<EnterWindow>: helpup(askstring_tersePlay,"
            "\"Should we try to reduce server traffic while involved in a game ?\")\\n"
        "<LeaveWindow>: helpdown(askstring_tersePlay)\\n"
        ASKTEXTTRAN(),

    (String) "*stringList*allowShellResize:        True",
    (String) "*stringList*Text*editType:           edit",
    (String) "*stringList*Text.resize:             both",
    (String) "*stringList*Command.background:      Green",
#ifndef   XAW3D
    (String) "*stringList*Command*shapeStyle:      roundedRectangle",
#endif /* XAW3D */
    (String) "*stringList*cancel.background:       Red",
    (String) "*stringList*cancel.foreground:       Yellow",
    (String) "*stringList*cancel.translations:     #override\\n"
        "Shift<Btn1Down>: help(stringList_cancel)\\n"
        "<Btn1Up>: notify() unset() destroy(\"\")\\n"
        "<EnterWindow>: helpup(stringList_cancel,"
            "\"Forget it. Don't change the old values.\")\\n"
        "<LeaveWindow>: helpdown(stringList_cancel)",
    (String) "*stringList*ok.translations:         #override\\n"
        "Shift<Btn1Down>: help(stringList_ok)\\n"
        "<Btn1Up>: notify() unset()\\n"
        "<EnterWindow>: helpup(stringList_ok,"
            "\"Accept the entered values.\")\\n"
        "<LeaveWindow>: helpdown(stringList_ok)",
    (String) "*stringList*Text.translations: #override\\n"
        "Shift<Btn1Down>: help(stringList_text)\\n"
        "<EnterWindow>: helpup(stringList_text,"
            "\"Enter what you want as new string list.\")\\n"
        "<LeaveWindow>: helpdown(stringList_text)\\n"
        USER_TRANS
        META "<Key>c:change() \\n"
        META "<Key>w:widgets()\\n"
        META "<Key>h:info()",

    (String) "*challenge.title:                    Challenging %N[%n]%A",
    (String) "*challenge.iconName:                 challenging %N[%n]%A",
    (String) "*challenge.translations:             #override\\n"
        "<ClientMessage>WM_PROTOCOLS:   WMprotocol()\\n"
        "<ClientMessage>DELETE:         destroy()",
    (String) "*challenge*close.translations:       #override\\n"
        "Shift<Btn1Down>: help(challenge_close)\\n"
        "<Btn1Up>: notify() maydecline() unset() destroy(\"\")\\n"
        "<EnterWindow>: helpup(challenge_close,"
            "\"Destroy the challenge widget.\")\\n"
        "<LeaveWindow>: helpdown(challenge_close)",
    (String) "*challenge*ok.translations:          #override\\n"
        "Shift<Btn1Down>: help(challenge_ok)\\n"
        "<Btn1Up>: notify() unset()\\n"
        "<EnterWindow>: helpup(challenge_ok,"
            "\"Accept the current values and send proposal.\")\\n"
        "<LeaveWindow>: helpdown(challenge_ok)",
    (String) "*challenge*decline.translations:     #override\\n"
        "Shift<Btn1Down>: help(challenge_decline)\\n"
        "<Btn1Up>: notify() maydecline() unset() destroy(\"\")\\n"
        "<EnterWindow>: helpup(challenge_decline,"
            "\"Don't play against the other person.\")\\n"
        "<LeaveWindow>: helpdown(challenge_decline)",
    (String) "*challenge*getTell.translations:     #override\\n"
        "Shift<Btn1Down>: help(challenge_tell)\\n"
        "<EnterWindow>: helpup(challenge_tell,"
            "\"Open a tell window to talk to the other person.\")\\n"
        "<LeaveWindow>: helpdown(challenge_tell)",
    (String) "*challenge*getStats.translations:    #override\\n"
        "Shift<Btn1Down>: help(challenge_stats)\\n"
        "<EnterWindow>: helpup(challenge_stats,"
            "\"Get extra info about the other person.\")\\n"
        "<LeaveWindow>: helpdown(challenge_stats)",
    (String) "*challenge*colorWhite.translations:  #override\\n"
        "Shift<Btn1Down>: help(challenge_colorWhite)\\n"
        "<EnterWindow>: helpup(challenge_colorWhite,"
            "\"With this choice you will play white.\")\\n"
    /* "<LeaveWindow>: helpdown(challenge_colorWhite)\\n" */
        "<Btn1Up>: notify() setmanage(*colorBlack, 1, *colorWhite, 0) unset()",
    (String) "*challenge*colorBlack.translations:  #override\\n"
        "Shift<Btn1Down>: help(challenge_colorBlack)\\n"
        "<EnterWindow>: helpup(challenge_colorBlack,"
            "\"With this choice you will play black.\")\\n"
    /* "<LeaveWindow>: helpdown(challenge_colorBlack)\\n" */
        "<Btn1Up>: notify() setmanage(*colorBlack, 0, *colorWhite, 1) unset()",
    (String) "*challenge*rulesIgs.translations:  #override\\n"
        "Shift<Btn1Down>: help(challenge_rulesIgs)\\n"
        "<EnterWindow>: helpup(challenge_rulesIgs,"
            "\"The game will be played according to the IGS rules.\")\\n"
    /* "<LeaveWindow>: helpdown(challenge_rulesIgs)\\n" */
        "<Btn1Up>: notify() setmanage(*rulesGoe, 1, *rulesIgs, 0) unset()",
    (String) "*challenge*rulesGoe.translations:  #override\\n"
        "Shift<Btn1Down>: help(challenge_rulesGoe)\\n"
        "<EnterWindow>: helpup(challenge_rulesGoe,"
            "\"The game will be played according to the Ing rules.\")\\n"
    /* "<LeaveWindow>: helpdown(challenge_rulesGoe)\\n" */
        "<Btn1Up>: notify() setmanage(*rulesGoe, 0, *rulesIgs, 1) unset()",
    (String) "*challenge*tournamentYes.translations:  #override\\n"
        "Shift<Btn1Down>: help(challenge_tournamentYes)\\n"
        "<EnterWindow>: helpup(challenge_tournamentYes,"
            "\"The game will be a tournament game.\")\\n"
    /* "<LeaveWindow>: helpdown(challenge_tournamentYes)\\n" */
        "<Btn1Up>: notify() setmanage(*tournamentNo, 1, *tournamentYes, 0) unset()",
    (String) "*challenge*tournamentNo.translations:  #override\\n"
        "Shift<Btn1Down>: help(challenge_tournamentNo)\\n"
        "<EnterWindow>: helpup(challenge_tournamentNo,"
            "\"The game will be a non tournament game.\")\\n"
    /* "<LeaveWindow>: helpdown(challenge_tournamentNo)\\n" */
        "<Btn1Up>: notify() setmanage(*tournamentNo, 0, *tournamentYes, 1) unset()",
    (String) "*challenge*sizeLabel.translations:   #override\\n"
        "Shift<Btn1Down>: help(challenge_sizeLabel)\\n"
        "<Btn3Down>: set()\\n"
        "<Btn1Up>: notify() increase(*size*text, -1, 2, 25) unset()\\n"
        "<Btn3Up>: notify() increase(*size*text,  1, 2, 25) unset()\\n"
        "<EnterWindow>: helpup(challenge_sizeLabel,"
            "\"Enter the board size.\")",
    /* "<LeaveWindow>: helpdown(challenge_sizeLabel)", */
    (String) "*challenge*size*text.translations:   #override\\n"
        "Shift<Btn1Down>: help(challenge_size)\\n"
        ONTAB("nexttext()")"\\n"
        ONENTER("nexttext()")"\\n"
        ONBACKTAB("previoustext()")"\\n"
        "<Btn1Down>: thistext() select-start()\\n"
        "<Btn2Down>: thistext() insert-selection(\"PRIMARY\", \"CUT_BUFFER0\")\\n"
        "<Btn3Down>: thistext() extend-start()\\n"
        "<EnterWindow>: helpup(challenge_size,"
            "\"Enter the board size.\")\\n"
    /* "<LeaveWindow>: helpdown(challenge_size)\\n" */
        USER_TRANS
        META "<Key>c:change() \\n"
        META "<Key>w:widgets()\\n"
        META "<Key>h:info()",
    (String) "*challenge*timeLabel.translations:   #override\\n"
        "Shift<Btn1Down>: help(challenge_timeLabel)\\n"
        "<Btn3Down>: set()\\n"
        "<Btn1Up>: notify() increase(*time*text, -5, 0, 300) unset()\\n"
        "<Btn3Up>: notify() increase(*time*text,  5, 0, 300) unset()\\n"
        "<EnterWindow>: helpup(challenge_timeLabel,"
            "\"Enter game time in minutes.\")\\n"
        "<LeaveWindow>: helpdown(challenge_timeLabel)",
    (String) "*challenge*time*text.translations:   #override\\n"
        "Shift<Btn1Down>: help(challenge_time)\\n"
        ONTAB("nexttext()")"\\n"
        ONENTER("nexttext()")"\\n"
        ONBACKTAB("previoustext()")"\\n"
        "<Btn1Down>: thistext() select-start()\\n"
        "<Btn2Down>: thistext() insert-selection(\"PRIMARY\", \"CUT_BUFFER0\")\\n"
        "<Btn3Down>: thistext() extend-start()\\n"
        "<EnterWindow>: helpup(challenge_time,"
            "\"Enter game time in minutes.\")\\n"
        "<LeaveWindow>: helpdown(challenge_time)\\n"
        USER_TRANS
        META "<Key>c:change() \\n"
        META "<Key>w:widgets()\\n"
        META "<Key>h:info()",
    (String) "*challenge*byoYomiLabel.translations:   #override\\n"
        "Shift<Btn1Down>: help(challenge_byoYomiLabel)\\n"
        "<Btn3Down>: set()\\n"
        "<Btn1Up>: notify() increase(*byoYomi*text, -1, 0, 40) unset()\\n"
        "<Btn3Up>: notify() increase(*byoYomi*text,  1, 0, 40) unset()\\n"
        "<EnterWindow>: helpup(challenge_byoYomiLabel,"
            "\"Enter byo yomi.\")\\n"
        "<LeaveWindow>: helpdown(challenge_byoYomiLabel)",
    (String) "*challenge*byoYomi*text.translations:   #override\\n"
        "Shift<Btn1Down>: help(challenge_byoYomi)\\n"
        ONTAB("nexttext()")"\\n"
        ONENTER("nexttext()")"\\n"
        ONBACKTAB("previoustext()")"\\n"
        "<Btn1Down>: thistext() select-start()\\n"
        "<Btn2Down>: thistext() insert-selection(\"PRIMARY\", \"CUT_BUFFER0\")\\n"
        "<Btn3Down>: thistext() extend-start()\\n"
        "<EnterWindow>: helpup(challenge_byoYomi,"
            "\"Enter byo yomi.\")\\n"
        "<LeaveWindow>: helpdown(challenge_byoYomi)\\n"
        USER_TRANS
        META "<Key>c:change() \\n"
        META "<Key>w:widgets()\\n"
        META "<Key>h:info()",
    (String) "*challenge*allowShellResize:         True",
#ifndef XAW3D
    (String) "*challenge.?.width:                  238",
#else  /* XAW3D */
    (String) "*challenge.?.width:                  251",
#endif /* XAW3D */
    /* (String) "*challenge.collect.height:           157", */
    (String) "*challenge.collect.choices.height:   28",
    (String) "*challenge*Label.background:         Green",
    (String) "*challenge*Command.background:       Green",
    (String) "*challenge*Toggle.background:        Green",
    (String) "*challenge*close.background:         Red",
    (String) "*challenge*Text*editType:            edit",
    (String) "*challenge*Text.resize:              both",
    (String) "*challenge*Text.displayCaret:        False",
/*  (String) "*challenge*Text.displayCaret:        False", */
    (String) "*challenge*Paned.?.showGrip:         False",
    (String) "*challenge*Paned.?.allowResize:      True",
    (String) "*challenge*Paned.?.resizeToPreferred:True",
    (String) "*challenge.collect.choices.allowResize: False",
    (String) "*challenge.collect.choices.resizeToPreferred: False",
/*    (String) "*challenge*collect.orientation:      vertical", */
    (String) "*challenge*collect*Paned.orientation:horizontal",
    (String) "*challenge*stateChallenger.label:    You are challenging",
    (String) "*challenge*stateChallengee.label:    You are challenged",
    (String) "*challenge*stateDispute.label:       There is a dispute",
    (String) "*challenge*stateDecline.label:       You were declined",
    (String) "*challenge*sizeLabel.label:          Board size:",
    (String) "*challenge*timeLabel.label:          game time :",
    (String) "*challenge*byoYomiLabel.label:       byo yomi  :",
    (String) "*challenge*colorBlack.label:         Black",
    (String) "*challenge*colorWhite.label:         White",
    (String) "*challenge*colorBlack.foreground:    White",
    (String) "*challenge*colorBlack.background:    Black",
    (String) "*challenge*colorWhite.foreground:    Black",
    (String) "*challenge*colorWhite.background:    White",
    (String) "*challenge*rulesIgs.label:           IGS",
    (String) "*challenge*rulesGoe.label:           GOE",
    (String) "*challenge*tournamentYes.label:      \\   Tournament",
    (String) "*challenge*tournamentNo.label:       No tournament",

    (String) "*popMessage*Label.background:        Green",
    (String) "*popMessage*ok.foreground:           Yellow",
    (String) "*popMessage*ok.background:           Red",
    (String) "*popMessage*Paned.?.showGrip:        False",
    (String) "*popMessage*ok.translations:         #override\\n"
        "<Btn1Down>,<Btn1Up>: destroy(\"\")",
    (String) "*buttons*quit.foreground:            Yellow",
    (String) "*buttons*quit.background:            Red",
    (String) "*buttons*close.foreground:           Yellow",
    (String) "*buttons*close.background:           Red",
/*
    (String) "*buttons.bug.foreground:             Yellow",
    (String) "*buttons.bug.background:             #6B238E",
*/
    (String) "*buttons*?.foreground:               XtDefaultForeground",
    (String) "*buttons*?.background:               Green",
    (String) "*buttons*quit.label:                 Quit",
    (String) "*buttons*close.label:                Close",
    (String) "*buttons*update.label:               Update",
    (String) "*buttons*gamesButton.label:          Games",
    (String) "*buttons*playersButton.label:        Players",
    (String) "*buttons*messageButton.label:        Messages",
    (String) "*buttons*broadcastButton.label:      Broadcasts",
    (String) "*buttons*yellButton.label:           Channels",
    (String) "*buttons*analyzeButton.label:        Analyze",
    (String) "*buttons*eventsButton.label:         Events",
    (String) "*buttons*helpButton.label:           Help",
#ifdef DO_REVIEW
    (String) "*buttons*reviewsButton.label:        Reviews",
#endif
    (String) "*buttons*userButton.label:           User",
    (String) "*buttons*Text.resize:                both",
    (String) "*buttons*Text.displayCaret:          False",
#ifndef   XAW3D
    (String) "*buttons*shapeStyle:                 roundedRectangle",
#endif /* XAW3D */
    (String) "*times*?.foreground:                 XtDefaultForeground",
    (String) "*times*?.background:                 Green",
    (String) "*times*Text.resize:                  both",
    (String) "*times*Text.displayCaret:            False",
#ifndef   XAW3D
    (String) "*times*shapeStyle:                   roundedRectangle",
#endif /* XAW3D */

    (String) "*scrollboard.orientation:            horizontal",
    (String) "*scrollboard.?.showGrip:             False",

#ifdef    XAW3D
    (String) "*Label.borderWidth:                  0",
#endif /* XAW3D */
    (String) "*getStats.label:                     Stats",
    (String) "*playerObserve.label:                Observe",
    (String) "*getTell.label:                      Tell",
    (String) "*getChallenge.label:                 Challenge",
    (String) "*options.label:                      Options",
    (String) "*commands.label:                     Commands",
    (String) "*undo.label:                         Undo",
    (String) "*copy.label:                         Copy",
    (String) "*pass.label:                         Pass",
    (String) "*done.label:                         Done",
    (String) "*save.label:                         Save",
    (String) "*options.translations:               #override\\n"
        "~Shift<Btn1Down>: reset() popupmenu(optionMenu)\\n"
        "~Shift<Btn3Down>: reset() popupmenu(optionMenu, 1)\\n"
        " Shift<Btn1Down>: help(help_options)\\n"
        "<EnterWindow>: helpup(help_options,"
            "\"Press left mouse button for a menu with options.\")\\n"
        "<LeaveWindow>: helpdown(help_options)",
    (String) "*commands.translations:              #override\\n"
        "~Shift<Btn1Down>: reset() popupmenu(commandMenu)\\n"
        "~Shift<Btn3Down>: reset() popupmenu(commandMenu, 1)\\n"
        " Shift<Btn1Down>: help(help_commands)\\n"
        "<EnterWindow>: helpup(help_commands,"
            "\"Press left mouse button for a menu with commands.\")\\n"
        "<LeaveWindow>: helpdown(help_commands)",
    (String) "*kCom.translations:              #override\\n"
        "~Shift<Btn1Down>: reset() popupmenu(commandMenu)\\n"
        "~Shift<Btn3Down>: reset() popupmenu(commandMenu, 1)\\n"
        " Shift<Btn1Down>: help(help_commands)\\n"
        "<EnterWindow>: helpup(help_commands,"
            "\"Press left mouse button for a menu with commands.\")\\n"
        "<LeaveWindow>: helpdown(help_commands)",
    (String) "*hCom.translations:              #override\\n"
        "~Shift<Btn1Down>: reset() popupmenu(commandMenu)\\n"
        "~Shift<Btn3Down>: reset() popupmenu(commandMenu, 1)\\n"
        " Shift<Btn1Down>: help(help_commands)\\n"
        "<EnterWindow>: helpup(help_commands,"
            "\"Press left mouse button for a menu with commands.\")\\n"
        "<LeaveWindow>: helpdown(help_commands)",

    (String) "*optionMenu*?.HorizontalMargins:     30",
    (String) "*optionMenu*SmeLabel.HorizontalMargins: 4",
    (String) "*optionMenu*sgfFromStart.state:      True",
    (String) "*optionMenu*psFromStart.state:       True",
    (String) "*optionMenu*messageLabel.label:      On message:",
    (String) "*optionMenu*fileLabel.label:         Save file:",
    (String) "*optionMenu*file.label:              filename",

    (String) "*optionMenu*autoReplyLabel.label:    Tell auto-reply:",
    (String) "*optionMenu*autoReply.state:         False",
    (String) "*optionMenu*autoReply.label:         auto reply",
    (String) "*optionMenu*replyMessage.label:      reply message",
    (String) "*optionMenu*othersLabel.label:       Other options:",
    (String) "*optionMenu*patternsLabel.label:     Regex patterns:",
    (String) "*optionMenu*passPattern.label:       pass",
    (String) "*optionMenu*killPattern.label:       kill",
    (String) "*optionMenu*analyzeSize.label:       analyze size",
    (String) "*optionMenu*programSettings.label:   program settings",
    (String) "*optionMenu*observeMoveLabel.label:  On move:",
    (String) "*optionMenu*moveBeep.label:          beep",
    (String) "*optionMenu*moveRaise.label:         raise",
    (String) "*optionMenu*errorLabel.label:        On Error:",
    (String) "*optionMenu*errorBeep.label:         beep",
    (String) "*optionMenu*errorRaise.label:        raise",
    (String) "*optionMenu*sortLabel.label:         Sort by:",
    (String) "*optionMenu*outputLabel.label:       On Output:",
    (String) "*optionMenu*onImportantLabel.label:  On important:",
    (String) "*optionMenu*beepLabel.label:         On Beep:",
    (String) "*optionMenu*beepBeep.label:          beep",
    (String) "*optionMenu*beepRaise.label:         raise",
    (String) "*optionMenu*kibitzActionLabel.label: On Kibitz:",
    (String) "*optionMenu*kibitzBeep.label:        beep",
    (String) "*optionMenu*kibitzRaise.label:       raise",
    (String) "*optionMenu*sgfLabel.label:          Sgf file:",
    (String) "*optionMenu*sgfFile.label:           filename",
    (String) "*optionMenu*sgfOverwrite.label:      overwrite",
    (String) "*optionMenu*sgfFromStart.label:      from start",
    (String) "*optionMenu*psLabel.label:           Ps file:",
    (String) "*optionMenu*psFile.label:            filename",
    (String) "*optionMenu*psOverwrite.label:       overwrite",
    (String) "*optionMenu*psFromStart.label:       from start",
    (String) "*optionMenu*kibitzLabel.label:       Kibitz file:",
    (String) "*optionMenu*kibitzFile.label:        filename",
    (String) "*optionMenu*kibitzOverwrite.label:   overwrite",
    (String) "*optionMenu*allowSuicide.label:      allow suicide",
    (String) "*optionMenu*simpleNames.label:       simple names",
    (String) "*optionMenu*numberKibitzes.label:    number kibitzes",
    (String) "*optionMenu*sortName.label:          Name",
    (String) "*optionMenu*sortNumber.label:        Number",
    (String) "*optionMenu*sortStrength.label:      Strength",
    (String) "*optionMenu*sortImportance.label:    Importance",

    (String) "*players*optionMenu*sortName.radioGroup: *sortImportance",
    (String) "*players*optionMenu*sortStrength.radioGroup: *sortImportance",
    (String) "*games*optionMenu*sortNumber.radioGroup: *sortImportance",
    (String) "*games*optionMenu*sortStrength.radioGroup: *sortImportance",

    (String) "*commandMenu*declineAdjourn.label:   decline adjourn",
    (String) "*commandMenu*sgfSave.label:          save game",
    (String) "*commandMenu*psSave.label:           save postscript",
    (String) "*commandMenu*kibitzSave.label:       save kibitzes",
    (String) "*commandMenu*talkBlack.label:        talk to %B",
    (String) "*commandMenu*talkWhite.label:        talk to %W",
    (String) "*commandMenu*statsBlack.label:       stats %B",
    (String) "*commandMenu*statsWhite.label:       stats %W",
    (String) "*commandMenu*statsMe.label:          stats %N",
    (String) "*commandMenu*k0.label:               komi 0.5",
    (String) "*commandMenu*k5.label:               komi 5.5",
    (String) "*commandMenu*h2.label:               handicap 2",
    (String) "*commandMenu*h3.label:               handicap 3",
    (String) "*commandMenu*h4.label:               handicap 4",
    (String) "*commandMenu*h5.label:               handicap 5",
    (String) "*commandMenu*h6.label:               handicap 6",
    (String) "*commandMenu*h7.label:               handicap 7",
    (String) "*commandMenu*h8.label:               handicap 8",
    (String) "*commandMenu*h9.label:               handicap 9",
    (String) "*textForm.text*borderWidth:          0",
    (String) "*textForm.?.resizable:               True",
    (String) "*buttons*optionMenu*background:      XtDefaultBackground",
    (String) "*buttons*commandMenu*background:     XtDefaultBackground",
    (String) "*options.menuName:                   optionMenu",
    (String) "*raise.state:                        False",
    (String) "*tell*raise.state:                   True",
    (String) "*tell*beepRaise.state:               True",
    (String) "*moveRaise.state:                    False",
    (String) "*kibitzRaise.state:                  False",
    (String) "*errorRaise.state:                   True",
    (String) "*SmeBell.state:                      False",
    (String) "*errorBeep.state:                    True",
    (String) "*blink.state:                        False",
    (String) "*info.scrollVertical:                whenNeeded",
    (String) "*Text.wrap:                          word",
    (String) "*Viewport.allowHoriz:                True",
    (String) "*Viewport.allowVert:                 True",
    (String) "*Viewport.forceBars:                 True",
    (String) "*input*editType:                     edit",
    (String) "*chatter*editType:                   edit",
    (String) "*board.background:                   #FFA54F",
    (String) "*board.backgroundPixmap: pixmap(board.xpm)",
    (String) "*StripChart.jumpScroll:              1",
    (String) "*background:                         #BFD8D8",
    (String) "*font:"
/*	"-adobe-courier-bold-r-normal--12-120-75-75-m-70-iso8859-1", */
	"-adobe-courier-bold-r-normal--*-120-75-75-m-70-*-1",
    (String) "*?*accelerators:                     #augment\\n"
        USER_TRANS
        META "<Key>c: change() \\n"
        META "<Key>w: widgets()\\n"
        META "<Key>h: info()",
#ifdef    XAW3D
    (String) "*shadowWidth:                        3",
#endif /* XAW3D */

    (String) "*widgetTreeMacro.mainWidget:"
        "main.Paned"
            "(collect.Paned"
                "(buttons.Box"
                    "(quit.Toggle"
                    " #mainOptions"
                    " commands.Command"
                        "(commandMenu.TearofMenu"
                            "(analyzeButton.SmeBSB"
                            " save.SmeBSB"
                            " statsMe.SmeBSB.statsMe"
                            ")"
                        ")"
                    " gamesButton.Toggle"
                    " playersButton.Toggle"
                    " messageButton.Toggle"
                    " broadcastButton.Toggle"
                    " yellButton.Toggle"
#ifdef DO_REVIEW
                    " reviewsButton.Toggle"
#endif
                    " userButton.Command"
                    " eventsButton.Toggle"
                    " helpButton.Toggle"
                    ")"
                " times.Box"
                    "(localTime.AsciiText     [string.localTime]"
                    " universalTime.AsciiText [string.universalTime]"
                    " serverTime.AsciiText    [string.serverTime]"
                    ")"
                " shortHelp.Label"
                " info.AsciiText"
                " input.AsciiText"
                ")"
            ")",
    (String) "*widgetTreeMacro.mainOptions:"
        " options.Command"
            "(optionMenu.TearofMenu"
                "("
                " outputLabel.SmeLabel"
                " line.SmeLine"
                " beep.SmeBell"
                " raise.SmeToggle"
                " line.SmeLine"
                " #errorOptions"
                " line.SmeLine"
                " fileLabel.SmeLabel"
                " line.SmeLine"
                " file.SmeBSB"
                " overwrite.SmeToggle"
                " line.SmeLine"
                " autoReplyLabel.SmeLabel"
                " line.SmeLine"
                " autoReply.SmeToggle"
                " replyMessage.SmeBSB"
                " line.SmeLine"
                " connect.SmeLabel"
                " line.SmeLine"
                " hasConnect.SmeToggle"
                " wantConnect.SmeToggle"
                " line.SmeLine"
                " othersLabel.SmeLabel"
                " line.SmeLine"
                " programSettings.SmeBSB"
                " stdout.SmeToggle"
                " verbose.SmeToggle"
                ")"
            ")",
    (String) "*widgetTreeMacro.quitConfirm:"
        " quitConfirm.TopLevelShell"
            "(buttons.Paned"
                "(confirm.Label"
                " confirmContainer.Paned"
                    "(ok.Command"
                    " filler.Core"
                    " cancel.Command"
                    ")"
                ")"
            ")",
    (String) "*widgetTreeMacro.askText:"
        SHADOWTEXT(""),
    (String) "*widgetTreeMacro.askFilename:"
        SHADOWTEXT("[string.filename]"),
    (String) "*widgetTreeMacro.askTimeout:"
        SHADOWTEXT("[string.timeout]"),
    (String) "*widgetTreeMacro.askUserPass:"
        " user.Paned.user"
            "(userLabel.Label"
            " #askText"
            ")"
        " password.Paned.password"
            "(passwordLabel.Label"
            " #askText"
            ")",
    (String) "*widgetTreeMacro.askFiles:"
        " sgfFilename.Paned.sgfFilename"
            "(sgfFilenameLabel.Label"
            " #askFilename"
            ")"
        " psFilename.Paned.psFilename"
            "(psFilenameLabel.Label"
            " #askFilename"
            ")"
        " kibitzFilename.Paned.kibitzFilename"
            "(kibitzFilenameLabel.Label"
            " #askFilename"
            ")"
        " broadcastFilename.Paned.broadcastFilename"
            "(broadcastFilenameLabel.Label"
            " #askFilename"
            ")"
        " yellFilename.Paned.yellFilename"
            "(yellFilenameLabel.Label"
            " #askFilename"
            ")"
        " tellFilename.Paned.tellFilename"
            "(tellFilenameLabel.Label"
            " #askFilename"
            ")"
        " serverFilename.Paned.serverFilename"
            "(serverFilenameLabel.Label"
            " #askFilename"
            ")"
        " eventsFilename.Paned.eventsFilename"
            "(serverFilenameLabel.Label"
            " #askFilename"
            ")"
        " mainFilename.Paned.mainFilename"
            "(mainFilenameLabel.Label"
            " #askFilename"
            ")"
        " autoReplyMessage.Paned.autoReplyMessage"
            "(autoReplyMessageLabel.Label"
            " #askFilename"
            ")",
    (String) "*widgetTreeMacro.askTimeouts:"
        " minSecPerMove.Paned.minSecPerMove"
            "(minSecPerMoveLabel.Command"
            " #askTimeout"
            ")"
        " minLagMargin.Paned.minLagMargin"
            "(minLagMarginLabel.Command"
            " #askTimeout"
            ")"
        " replayTimeout.Paned.replayTimeout"
            "(replayTimeoutLabel.Command"
            " #askTimeout"
            ")"
        " whoTimeout.Paned.whoTimeout"
            "(whoTimeoutLabel.Command"
            " #askTimeout"
            ")"
        " gamesTimeout.Paned.gamesTimeout"
            "(gamesTimeoutLabel.Command"
            " #askTimeout"
            ")"
#ifdef DO_REVIEW
        " reviewsTimeout.Paned.reviewsTimeout"
            "(reviewsTimeoutLabel.Command"
            " #askTimeout"
            ")"
#endif
        " clockTimeout.Paned.clockTimeout"
            "(clockTimeoutLabel.Command"
            " #askTimeout"
            ")"
        " playersUpdateTimeout.Paned.playersUpdateTimeout"
            "(playersUpdateTimeoutLabel.Command"
            " #askTimeout"
            ")"
        " gamesUpdateTimeout.Paned.gamesUpdateTimeout"
            "(gamesUpdateTimeoutLabel.Command"
            " #askTimeout"
            ")"
#ifdef DO_REVIEW
        " reviewsUpdateTimeout.Paned.reviewsUpdateTimeout"
            "(reviewsUpdateTimeoutLabel.Command"
            " #askTimeout"
            ")"
#endif
        " serverTimeout.Paned.serverTimeout"
            "(serverTimeoutLabel.Command"
            " #askTimeout"
            ")"
        " inactiveTimeout.Paned.inactiveTimeout"
            "(inactiveTimeoutLabel.Command"
            " #askTimeout"
            ")"
        " quitTimeout.Paned.quitTimeout"
            "(quitTimeoutLabel.Command"
            " #askTimeout"
            ")",
    (String) "*widgetTreeMacro.askString:"
        " askString.TopLevelShell.askString [title.title][iconName.title]"
            "(collect.Box"
                "(#askUserPass"
                " #askFiles"
                " analyzeSize.Paned.analyzeSize"
                    "(analyzeSizeLabel.Command"
                      SHADOWTEXT("[string.size]")
                    ")"
                " allowSuicide.Paned.allowSuicide"
                    "(allowSuicideLabel.Command"
                      SHADOWTEXT("[string.boolean]")
                    ")"
                " simpleNames.Paned.simpleNames"
                    "(simpleNamesLabel.Command"
                      SHADOWTEXT("[string.boolean]")
                    ")"
                " numberKibitzes.Paned.numberKibitzes"
                    "(numberKibitzesLabel.Command"
                      SHADOWTEXT("[string.boolean]")
                    ")"
                " #askTimeouts"
                " tersePlay.Paned.tersePlay"
                    "(tersePlayLabel.Command"
                      SHADOWTEXT("[string.boolean]")
                    ")"
                ")"
            ")"
        " stringList.TopLevelShell.stringList"
            "(collect.Box"
                "(cancel.Command"
                " ok.Command"
                " text.AsciiText[string.text]"
                ")"
            ")",
    (String) "*widgetTreeMacro.popMessage:"
        " popMessage.TopLevelShell.popMessage [title.title][iconName.title]"
            "(collect.Paned"
                "(message.Label[label.text]"
                " ok.Command"
                ")"
            ")",
    (String) "*widgetTreeMacro.players:"
        " players.TopLevelShell.players"
            "(collect.Paned"
                "(buttons.Box"
                    "(close.Command"
                    " options.Command"
                        "(optionMenu.TearofMenu"
                            "("
                            " sortLabel.SmeLabel"
                            " line.SmeLine"
                            " sortImportance.SmeToggle"
                            " sortStrength.SmeToggle"
                            " sortName.SmeToggle"
                            " line.SmeLine"
                            " onImportantLabel.SmeLabel"
                            " line.SmeLine"
                            " beep.SmeBell"
                            " raise.SmeToggle"
                            " line.SmeLine"
                            " othersLabel.SmeLabel"
                            " line.SmeLine"
                            " allowResize.SmeToggle"
                            ")"
                        ")"
                    " update.Command"
                    ")"
#define SCROLL
#ifdef SCROLL
                " viewport.Viewport"
                    "("
#endif
                    "set.Box"
                        "(,Command.entry[name.name])"
#ifdef SCROLL
                    ")"
#endif
                " stats.Label"
                " stripform.Form"
                    "(strip.StripChart"
                    ")"
                " info.AsciiText"
                ")"
            ")",
    (String) "*widgetTreeMacro.games:"
        " games.TopLevelShell.games"
            "(collect.Paned"
                "(buttons.Box"
                    "(close.Command"
                    " options.Command"
                        "(optionMenu.TearofMenu"
                            "("
                            " sortLabel.SmeLabel"
                            " line.SmeLine"
                            " sortImportance.SmeToggle"
                            " sortStrength.SmeToggle"
                            " sortNumber.SmeToggle"
                            " line.SmeLine"
                            " onImportantLabel.SmeLabel"
                            " line.SmeLine"
                            " beep.SmeBell"
                            " raise.SmeToggle"
                            " line.SmeLine"
                            " othersLabel.SmeLabel"
                            " line.SmeLine"
                            " allowResize.SmeToggle"
                            ")"
                        ")"
                    " update.Command"
                    ")"
#ifdef SCROLL
                " viewport.Viewport"
                    "("
#endif
                    "set.Box"
                        "(,Toggle.entry[name.name])"
#ifdef SCROLL

                    ")"
#endif
                " stripform.Form"
                    "(strip.StripChart"
                    ")"
                " info.AsciiText"
                ")"
            ")",
    (String) "*widgetTreeMacro.reviews:"
        " reviews.TopLevelShell.reviews"
            "(collect.Paned"
                "(buttons.Box"
                    "(close.Command"
                    " options.Command"
                        "(optionMenu.TearofMenu"
                            "("
                            " othersLabel.SmeLabel"
                            " line.SmeLine"
                            " allowResize.SmeToggle"
                            ")"
                        ")"
                    " update.Command"
                    ")"
                " viewport.Viewport"
                    "(set.Box"
                        "(,Command.entry[name.name])"
                    ")"
                ")"
            ")",
    (String) "*widgetTreeMacro.igsMessages:"
        " igsMessages.TopLevelShell.igsMessages"
            "(collect.Paned"
                "(buttons.Box"
                    "(close.Command"
                    " options.Command"
                        "(optionMenu.TearofMenu"
                            "(messageLabel.SmeLabel"
                            " line.SmeLine"
                            " beep.SmeBell"
                            " raise.SmeToggle"
                            " line.SmeLine"
                            " #errorOptions"
                            " line.SmeLine"
                            " fileLabel.SmeLabel"
                            " line.SmeLine"
                            " file.SmeBSB"
                            " overwrite.SmeToggle"
                            ")"
                        ")"
                    " save.Command"
                    ")"
                " info.AsciiText"
                ")"
            ")",
    (String) "*widgetTreeMacro.events:"
        " events.TopLevelShell.events"
            "(collect.Paned"
                "(buttons.Box"
                    "(close.Command"
                    " options.Command"
                        "(optionMenu.TearofMenu"
                            "(messageLabel.SmeLabel"
                            " line.SmeLine"
                            " beep.SmeBell"
                            " raise.SmeToggle"
                            " line.SmeLine"
                            " #errorOptions"
                            " line.SmeLine"
                            " fileLabel.SmeLabel"
                            " line.SmeLine"
                            " file.SmeBSB"
                            " overwrite.SmeToggle"
                            ")"
                        ")"
                    " save.Command"
                    ")"
                " info.AsciiText"
                ")"
            ")",
    (String) "*widgetTreeMacro.broadcasts:"
        " broadcasts.TopLevelShell.broadcasts"
            "(collect.Paned"
                "(buttons.Box"
                    "(close.Command"
                    " options.Command"
                        "(optionMenu.TearofMenu"
                            "(messageLabel.SmeLabel"
                            " line.SmeLine"
                            " beep.SmeBell"
                            " raise.SmeToggle"
                            " line.SmeLine"
                            " #errorOptions"
                            " line.SmeLine"
                            " fileLabel.SmeLabel"
                            " line.SmeLine"
                            " file.SmeBSB"
                            " overwrite.SmeToggle"
                            " line.SmeLine"
                            " patternsLabel.SmeLabel"
                            " line.SmeLine"
                            " killPattern.SmeBSB"
                            " passPattern.SmeBSB"
                            ")"
                        ")"
                    " save.Command"
                    ")"
                " info.AsciiText"
                " input.AsciiText"
                ")"
            ")",
    (String) "*widgetTreeMacro.yells:"
        " yells.TopLevelShell.yells"
            "(collect.Paned"
                "(buttons.Box"
                    "(close.Command"
                    " options.Command"
                        "(optionMenu.TearofMenu"
                            "(messageLabel.SmeLabel"
                            " line.SmeLine"
                            " beep.SmeBell"
                            " raise.SmeToggle"
                            " line.SmeLine"
                            " #errorOptions"
                            " line.SmeLine"
                            " fileLabel.SmeLabel"
                            " line.SmeLine"
                            " file.SmeBSB"
                            " overwrite.SmeToggle"
                            " line.SmeLine"
                            " patternsLabel.SmeLabel"
                            " line.SmeLine"
                            " killPattern.SmeBSB"
                            " passPattern.SmeBSB"
                            ")"
                        ")"
                    " save.Command"
                    " channels.Command"
                    " channel.AsciiText"
                    " moderator.AsciiText"
                    " state.AsciiText"
                    ")"
                " title.AsciiText"
                " info.AsciiText"
                " input.AsciiText"
                ")"
            ")",
    (String) "*widgetTreeMacro.tell:"
        " tell.TopLevelShell.tell"
            "(collect.Paned"
                "(buttons.Box"
                    "(close.Command"
                    " options.Command"
                        "(optionMenu.TearofMenu"
                            "(messageLabel.SmeLabel"
                            " line.SmeLine"
                            " beep.SmeBell"
                            " raise.SmeToggle"
                            " line.SmeLine"
                            " beepLabel.SmeLabel"
                            " line.SmeLine"
                            " beepBeep.SmeBell"
                            " beepRaise.SmeToggle"
                            " line.SmeLine"
                            " #errorOptions"
                            " line.SmeLine"
                            " fileLabel.SmeLabel"
                            " line.SmeLine"
                            " file.SmeBSB"
                            " overwrite.SmeToggle"
                            " line.SmeLine"
                            " patternsLabel.SmeLabel"
                            " line.SmeLine"
                            " killPattern.SmeBSB"
                            " passPattern.SmeBSB"
                            ")"
                        ")"
                    " save.Command"
                    " bug.Command"
                    " getStats.Command"
                    " getChallenge.Toggle"
                    " playerObserve.Command"
                    ")"
                " info.AsciiText"
                " input.AsciiText"
                ")"
            ")",
    (String) "*widgetTreeMacro.stats:"
        " stats.TopLevelShell.stats"
            "(collect.Box"
                "(close.Command"
                " getTell.Toggle"
                " playerObserve.Command"
                " info.AsciiText.info[string.text][name.name]"
                " toggle.Toggle.toggle[label.text][name.name]"
                " results.AsciiText.results[string.text][name.name]"
                ")"
            ")",
    (String) "*widgetTreeMacro.sgfOptions:"
        " sgfLabel.SmeLabel"
        " line.SmeLine"
        " sgfFile.SmeBSB"
        " sgfFromStart.SmeToggle"
        " sgfOverwrite.SmeToggle",
    (String) "*widgetTreeMacro.psOptions:"
        " psLabel.SmeLabel"
        " line.SmeLine"
        " psFile.SmeBSB"
        " psFromStart.SmeToggle"
        " psOverwrite.SmeToggle",
    (String) "*widgetTreeMacro.moveOptions:"
        " observeMoveLabel.SmeLabel"
        " line.SmeLine"
        " moveBeep.SmeBell"
        " moveRaise.SmeToggle",
    (String) "*widgetTreeMacro.errorOptions:"
        " errorLabel.SmeLabel"
        " line.SmeLine"
        " errorBeep.SmeBell"
        " errorRaise.SmeToggle",
    (String) "*widgetTreeMacro.gameProperties:"
        " komi.Label     [label.komi]"
        " handicap.Label [label.handicap]"
        " captures.Label [label.captures]"
        " move.Label     [label.move]"
        " time.AsciiText [string.time]",
    (String) "*widgetTreeMacro.playProperties:"
	" captures.Label [label.captures]"
	" move.Label     [label.move]"
	" time.AsciiText [string.time]",
    /*
    (String) "*widgetTreeMacro.kibitzChatter:"
        " kibitzChatter.Paned"
            "(kibitzPanes.Paned"
                "(kibitzPrefixLabel.Label"
                " input.AsciiText"
                ")"
            " chatterPanes.Paned"
                "(chatterPrefixLabel.Label"
                " chatter.AsciiText"
                ")"
            ")",
    */
    (String) "*widgetTreeMacro.kibitzChatter:"
            " chatter.AsciiText"
            " input.AsciiText",
    (String) "*widgetTreeMacro.playArea:"
	    " titlePlay.AsciiText[string.title]"
	    " scrollboard.Paned"
		"(scroll.Scrollbar"
		" board.Board [boardSize.boardSize]"
		")"
	    " info.AsciiText"
            " chatter.AsciiText"
            " input.AsciiText",
    (String) "*widgetTreeMacro.review:"
        " review.TopLevelShell.review"
            "(collect.Paned"
                "(buttons.Box"
                    "(close.Command"
                    " options.Command"
                        "(optionMenu.TearofMenu"
                            "(#moveOptions"
                            " line.SmeLine"
                            " kibitzActionLabel.SmeLabel"
                            " line.SmeLine"
                            " kibitzBeep.SmeBell"
                            " kibitzRaise.SmeToggle"
                            " line.SmeLine"
                            " #errorOptions"
                            " line.SmeLine"
                            " #sgfOptions"
                            " line.SmeLine"
                            " #psOptions"
                            " line.SmeLine"
                            " kibitzLabel.SmeLabel"
                            " line.SmeLine"
                            " kibitzFile.SmeBSB"
                            " kibitzOverwrite.SmeToggle"
                            " line.SmeLine"
                            " othersLabel.SmeLabel"
                            " line.SmeLine"
                            " blink.SmeToggle"
                            " replay.SmeToggle"
                            ")"
                        ")"
                    " commands.Command"
                        "(commandMenu.TearofMenu"
                            "(dup.SmeBSB"
                            " analyze.SmeBSB"
                            " sgfSave.SmeBSB"
                            " psSave.SmeBSB"
                            " kibitzSave.SmeBSB"
                            ")"
                        ")"
                    " #gameProperties"
                    ")"
                " titleObserve.AsciiText[string.title]"
                " scrollboard.Paned"
                    "(scroll.Scrollbar"
                    " board.Board [boardSize.boardSize]"
                    ")"
                " info.AsciiText"
                ")"
            ")",
    (String) "*widgetTreeMacro.observeObserve:"
        " observe.TopLevelShell.observe"
            "(collect.Paned"
                "(buttons.Box"
                    "(close.Command"
                    " options.Command"
                        "(optionMenu.TearofMenu"
                            "(#moveOptions"
                            " line.SmeLine"
                            " kibitzActionLabel.SmeLabel"
                            " line.SmeLine"
                            " kibitzBeep.SmeBell"
                            " kibitzRaise.SmeToggle"
                            " line.SmeLine"
                            " #errorOptions"
                            " line.SmeLine"
                            " #sgfOptions"
                            " line.SmeLine"
                            " #psOptions"
                            " line.SmeLine"
                            " kibitzLabel.SmeLabel"
                            " line.SmeLine"
                            " kibitzFile.SmeBSB"
                            " kibitzOverwrite.SmeToggle"
                            " line.SmeLine"
                            " patternsLabel.SmeLabel"
                            " line.SmeLine"
                            " killPattern.SmeBSB"
                            " passPattern.SmeBSB"
                            " line.SmeLine"
                            " othersLabel.SmeLabel"
                            " line.SmeLine"
                            " blink.SmeToggle"
                            " replay.SmeToggle"
                            " score.SmeToggle"
                            ")"
                        ")"
                    " commands.Command"
                        "(commandMenu.TearofMenu"
                            "(refresh.SmeBSB"
                            " serverScore.SmeBSB"
                            " bets.SmeBSB"
                            " observers.SmeBSB"
                            " dup.SmeBSB"
                            " analyze.SmeBSB"
                            " sgfSave.SmeBSB"
                            " psSave.SmeBSB"
                            " kibitzSave.SmeBSB"
                            " talkBlack.SmeBSB"
                            " talkWhite.SmeBSB"
                            " statsBlack.SmeBSB"
                            " statsWhite.SmeBSB"
                            ")"
                        ")"
                    " #gameProperties"
                    ")"
                " titleObserve.AsciiText[string.title]"
                " scrollboard.Paned"
                    "(scroll.Scrollbar"
                    " board.Board [boardSize.boardSize]"
                    ")"
                " info.AsciiText"
                " #kibitzChatter"
                ")"
            ")",
    (String) "*widgetTreeMacro.observePlay:"
        " observe.TopLevelShell.play"
            "(collect.Paned"
                "(buttons.Box"
                    "(close.Command"
                    " options.Command"
                        "(optionMenu.TearofMenu"
                            "(#moveOptions"
                            " line.SmeLine"
                            " kibitzActionLabel.SmeLabel"
                            " line.SmeLine"
                            " kibitzBeep.SmeBell"
                            " kibitzRaise.SmeToggle"
                            " line.SmeLine"
                            " #errorOptions"
                            " line.SmeLine"
                            " #sgfOptions"
                            " line.SmeLine"
                            " #psOptions"
                            " line.SmeLine"
                            " kibitzLabel.SmeLabel"
                            " line.SmeLine"
                            " kibitzFile.SmeBSB"
                            " kibitzOverwrite.SmeToggle"
                            " line.SmeLine"
                            " othersLabel.SmeLabel"
                            " line.SmeLine"
                            " blink.SmeToggle"
                            " replay.SmeToggle"
                            " score.SmeToggle"
                            ")"
                         ")"
                    " commands.Command"
                        "(commandMenu.TearofMenu"
                            "(resume.SmeBSB"
                            " adjourn.SmeBSB"
                            " declineAdjourn.SmeBSB"
                            " refresh.SmeBSB"
                            " serverScore.SmeBSB"
                            " observers.SmeBSB"
                            " dup.SmeBSB"
                            " analyze.SmeBSB"
                            " sgfSave.SmeBSB"
                            " psSave.SmeBSB"
                            " kibitzSave.SmeBSB"
                            " talkBlack.SmeBSB"
                            " talkWhite.SmeBSB"
                            " statsBlack.SmeBSB"
                            " statsWhite.SmeBSB"
                            ")"
                        ")"
                    " undo.Command"
                    " pass.Command"
                    " done.Command"
		    " kCom.Command"
			"(commandMenu.TearofMenu"
			    "(k0.SmeBSB"
			    " k5.SmeBSB"
			    ")"
			")"
		    " hCom.Command"
			"(commandMenu.TearofMenu"
			    "(h2.SmeBSB"
			    " h3.SmeBSB"
			    " h4.SmeBSB"
			    " h5.SmeBSB"
			    " h6.SmeBSB"
			    " h7.SmeBSB"
			    " h8.SmeBSB"
			    " h9.SmeBSB"
			    ")"
			")"
                    " #playProperties"
                    ")"
                " #playArea"
                ")"
            ")",
    (String) "*widgetTreeMacro.analyzer:"
        " analyzer.TopLevelShell.analyzer"
            "(collect.Paned"
                "(buttons.Box"
                    "(close.Command"
                    " options.Command"
                        "(optionMenu.TearofMenu"
                            "(#moveOptions"
                            " line.SmeLine"
                            " #errorOptions"
                            " line.SmeLine"
                            " #sgfOptions"
                            " line.SmeLine"
                            " #psOptions"
                            " line.SmeLine"
                            " othersLabel.SmeLabel"
                            " line.SmeLine"
                            " allowSuicide.SmeToggle"
                            " blink.SmeToggle"
                            " replay.SmeToggle"
                            ")"
                        ")"
                    " commands.Command"
                        "(commandMenu.TearofMenu"
                            "(reset.SmeBSB"
                            " analyze.SmeBSB"
                            " sgfSave.SmeBSB"
                            " psSave.SmeBSB"
                            ")"
                        ")"
                    " undo.Command"
                    " copy.Command"
                    " score.Command"
                    ")"
                " scrollboard.Paned"
                    "(scroll.Scrollbar"
                    " board.Board [boardSize.boardSize]"
                    ")"
                ")"
            ")",

    (String) "*widgetTreeMacro.challenge:"
        " challenge.TopLevelShell.challenge"
            "(collect.Paned"
                "(buttons.Box"
                    "(close.Command"
                    " ok.Command"
                    " decline.Command"
                    " getTell.Toggle"
                    " getStats.Command"
                    ")"
                " state.Paned"
                    "(stateChallenger.Label"
                    " stateChallengee.Label"
                    " stateDispute.Label"
                    " stateDecline.Label"
                    ")"
                " choices.Box"
                    "(colorWhite.Command"
                    " colorBlack.Command"
                    " rulesIgs.Command"
                    " rulesGoe.Command"
                    " tournamentYes.Command"
                    " tournamentNo.Command"
                    ")"
                " size.Paned"
                    "(sizeLabel.Command"
                      SHADOWTEXT("[string.size]")
                    ")"
                " time.Paned"
                    "(timeLabel.Command"
                      SHADOWTEXT("[string.time]")
                    ")"
                " byoYomi.Paned"
                    "(byoYomiLabel.Command"
                      SHADOWTEXT("[string.byoyomi]")
                    ")"
                ")"
            ")",

    (String) "*widgetTree: (,ApplicationShell "
                     "(#mainWidget"
                     " #quitConfirm"
                     " #askString"
                     " #popMessage"
                     " #players"
                     " #games"
                     " #reviews"
                     " #igsMessages"
                     " #events"
                     " #broadcasts"
                     " #yells"
                     " #tell"
                     " #stats"
                     " #review"
                     " #observeObserve"
                     " #observePlay"
                     " #analyzer"
                     " #challenge"
                     ")"
                 ")",
    NULL
};
