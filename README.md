# XGospel-2.0
=======History and notes about building and findings S.Cahill=================

This is has been a hobby of mine since (~1991) after I got pretty excited
about all the features and capabilites of the xgospel Go client. I first
learned how to play Go sometime after I graduated with my BSEE back in 1981.
An elderly gentleman at who befriended me and showed me his array of puzzles
and games taught me the game. We played at work over the lunch hour. When I
really dug into all the features I was absolutely astonished. At the time the
internet was just coming into it's own and my thoughts were "Wow! what a way
to learn the game!". I was hooked.

Today the client is not fully supported on the IGS server but it does work. It
could really use a rewrite to conform to the newer server standards. Many
features back then have since been deprecated as internet privacy is much more
important. Back then you could see the email of anyone who was registered.

The code has not been touched in quite some time and I can only get it
to build/compile on a Linux OS that would not run on a modern PC!
Case in point the last OS I could actually get to compile was Slackware 8.0.
Slackware is my distribution of choice. That said the executable will run on
a modern day 64-bit OS with the support for 32-bit binaries (like multilib).

==============================================================================
Base code for XGospel 2.0 - a project to revamp the old XGospel Go client
This file describes version 1.12c1 of the xgospel program, an X11 (R5
or higher) IGS client for UNIX systems.

Please read the file README.1ST and in particular the section
Frequently Asked Questions.

Jean-loup Gailly
jloup@mail.dotcom.fr
jl and xgospel on IGS

==============================================================================
[Here is the original README file for xgospel 1.10d]

Hello world,

   this file describes  version 1.10c of the   xgospel program, an  X11 (R5 or
higher) IGS client for UNIX systems.

Compile instructions:
---------------------

See the INSTALL file.
Please note that xmkmf -a is NOT the preferred way of compiling anymore.

xgospel in use
--------------

   A lot of defaults have been specified in the resources.c file.  Go and look
at the  fallback_resources structure. If  you  don't like these  options, just
change  them.  This is done  easiest by  having your  own  defaults (they will
override these    in the  fallback_resources)  in an    applicationdefaults or
.Xdefaults file (or whatever you like to use, have a look in your local X docs
for the xrdb program, and the  environment variables XENVIRONMENT, XAPPLRESDIR
and XUSERFILESEARCHPATH).  You can just  take over any  line in the  C-program
(of  course  removing  the surrounding  quotes,   the comma  at the   end, and
replacing two backslashes by one). So you could get entries like:

    xgospel*broadcasts.collect.info.height:              110
    xgospel*broadcasts.collect.input.skipAdjust:         True
    xgospel*broadcasts.collect.input.translations:       #override \n\
        <Key>Return: broadcast()

derived from:

    "*broadcasts.collect.info.height:              110",
    "*broadcasts.collect.input.skipAdjust:         True",
    "*broadcasts.collect.input.translations:       #override \\n"
        "<Key>Return: broadcast()",

   Apart from that you can give  some more options  both from the command line
or in a defaults file. You  find these in  the file xgospel.c in the options[]
structure. The most important of these are:

    user     : The name you want to use on the igs server.
    password : The corresponding password
    site     : the site to which a connection will be tried. Defaults to
               igs.nuri.net.
    port     : The port to which a connection will be tried. Defaults to 6969.

   As an example, I could use this in my .Xdefaults  file (These are examples,
you don't have to set them):

xgospel*User:                     AshaiRey
xgospel*Password:                 Something
!
!The following line would cause a connection to the old server
! (the default is igs.nuri.net)
! xgospel*site: hellspark.wharton.upenn.edu
!
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
!    %P will be replaced by your password on the server
!    %p will be replaced by the port you connected to
!    %h will be replaced by the name of the host you connected to.
!    %H will be replaced by your hostname
!Also remember that this log file will contain your IGS password !
! xgospel*debugFile:                /tmp/%N.log
!
!If you want the observe title or iconName to give some other text, you can
!set a pattern (%B and %W will be replaced by the name of the black and the
!white player, while %b and %w will be replaced by their strength). So you can
!set something like (colons do not work at the moment):
! xgospel*observe.title:          (Black) %B[%b] - (White) %W[%w]
!
!The next line means that all Beep widgets will be off by default.
xgospel*SmeBell.state:            False
!But I do want a beep in case of error:
xgospel*errorBeep.state:          True
!And I want it on on the broadcast window:
xgospel*broadcasts*SmeBell.state: True
! Also, I want the main window to map and raise itself if a message appears
xgospel*main*raise.state:         True
! If fact, I want this behavior for IGS messages, broadcasts, tells and
! channels (this makes the previous line unnecessary)
xgospel*raise.state:              True
! However, I don't want a map from the events window
xgospel*events*raise.state:       False
! I also want my observed games to map in case of a move or kibitz
xgospel*moveRaise.state:          True
xgospel*kibitzRaise.state:        True
!If you like players sorted by name instead of by strength:
xgospel*players*sortName.state:  True
!If you like games sorted by server number instead of by strength
xgospel*games*sortNumber.state:   True
!
!I used the following when I work on a single color X server:
!   xgospel*players.collect.set.playerEntryk.background: White
!   xgospel*players.collect.set.playerEntryd.background: White
!   xgospel*players.collect.set.playerEntryp.background: White
!But now I prefer:
!   xgospel*players.collect.set.playerEntryk.background: White
!   xgospel*players.collect.set.playerEntryd.foreground: White
!   xgospel*players.collect.set.playerEntryd.background: Black
!   xgospel*players.collect.set.playerEntryp.backgroundPixmap: bitmap(gray3,White, Black)
!   xgospel*players.collect.set.playerEntry__.foreground: White
!   xgospel*players.collect.set.playerEntry__.background: Black
!   xgospel*players.collect.set.playerEntryNR.backgroundPixmap: bitmap(gray3)
!   xgospel*players.collect.set.playerEntryIGS92.foreground: White
!   xgospel*players.collect.set.playerEntryIGS92.background: Black
!
!The program converts a strength to a name by removing all leading
!digits, and by then replacing each weird characters by _ . So if
!you want IGS92 or NR handled in a special way:
xgospel*players.collect.set.playerEntryNR.background:    Blue
xgospel*players.collect.set.playerEntryIGS92.background: Red
!
! You can in fact yourself decide what text is put after playerEntry by using
! the playerToWidget resource to give a set of pairs of regular expressions
! and strings terminated by newlines. The full text that will appear in the
! player widget will be matched against this, and the first match will get
! its corresponding string tucked to playerEntry. So a pattern like:
xgospel*playerToWidget: ^ *AshaiRey,   Me\n\
                        \\[ [456]k\\], MyStrength\n
xgospel*set*playerEntryMe.background:    Red
xgospel*set*playerEntryMe.foreground:    yellow
xgospel*set*playerEntryMyStrength.background:    Blue
xgospel*set*playerEntryMyStrength.foreground:    yellow
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
!
!depending on the resolution of your screen, you could add bitmap
!patterns like:
xgospel*board.backgroundPixmap:      bitmap(gray3)
! (for most sites: look in /usr/include/X11/bitmaps for more 
!  possible patterns, or make your own)
! In case you compiled with xpm, you can also add pixmaps.
!
!The default line width for drawing things on the board is 0. This tells X to 
!use the fastest way to draw lines on that particular piece of hardware. But
!sometimes this results in badly drawn stones. In that case, you might try the
!slower, but more dependable line width of one:
! xgospel*board.lineWidth:           1
!
! By default xgospel will send AshaiRey (the author) a message with the version
! number of xgospel. It will also do a "stats AshaiRey" to get some info about
! the current version (it will pop up a message if your version is not the
! most recent one). If you don't like this, set the resource maintainer to the
! empty string:
! xgospel*maintainer:
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


   You can  do Meta-h or Meta-w  on  a widget  to get more  information on the
X-settings (Many places use  alt as their meta key.   If your system does  not
have a meta key defined, use the xmodmap program to define one).

   Some windows are allowed to resize themselves. If you don't like that, turn
off the allowShellResize resource for the  games, players and stats widget. If
you do this, you probably will want to change the width and height too.
                 
   The programs assumes Kibitz toggled on. Since  the program sets client mode
itself, you  can set it on  your first use  of xgospel. The  output of several
commands is unknown to  xgospel (yet), but that should  give no  problems.  In
fact, the  client is written in such  a way that  it should be able to recover
from any weird input from IGS.

   Sometimes  you will get warnings  about player or  game database corrupted.
They are just  that, warnings. xgospel will  send the appropriate who or  game
commands and recover.  (The player database  corrupted warnings  are caused by
the server sending a message that a  person who never  logged on suddenly logs
off,  the game database corrupted message  means  that things happened I never
considered. If you have a repeatable case of this,  I would like to know since
it probably signifies a bug (notice that the program only has a database after
it did  games at least one  time. So if  any  games stop before  this happened
(just after you logged on) you might get a few spurious messages)).

   The program will  keep a list of  games  and players. These  are updated at
least every five minutes. Until such an update,  placeholders (?) will be used
for the unknown information  (should be very little).  You can force an update
by typing who or games. If you want to know a person's idle  time (and force a
who at the same time) use the right mouse key on this person's name.

   All text  widgets in the  program will scroll automatically  as long as the
caret is at  the end of all text  (and it will  keep it at  the end). The text
widget  is supposed to keep  the caret visible, so  that the last line of text
always remains readable. Unfortunately the Athena  text widget has a bug which
breaks this rule when text wraps  in a widget, so sometimes  you won't see the
last line. As  long as this  bug  is not fixed,  you  might want to  make text
widgets as  wide as seems reasonable in  order to lower the  chances of a text
wrap.  (Oh, and if you don't want your text  to scroll when it's updated, just
click your mouse somewhere in the middle of the text. If you want scrolling to
start again, go to the end of the text and click somewhere behind it).

   The   program allows     you   to   save   your   games  in    smart     go
format.  Unfortunately, most programs that  convert to postscript are not able
to handle the (correct)  format in which xgospel saves  them. You will however
be able to view the games with mgt. I'll probably  write a PS converter myself
at some point in the future.

   You  can get more  impressions about functionality  by browsing through the
file  helptext.h (these are  the texts for the context  sensitive help you can
get by pressing shift left mouse button on a xgospel window)

The relay program
-----------------

   Sometimes people cannot directly connect to the IGS server, but can somehow
connect from the machine where they  plan to use  Xgospel through some gateway
in between.  In  that case you  can  try to  use the  relay program (the relay
program is the most  general way, but if  they work, term  or rport are almost
certainly easier to use). The trick is to let the  relay program function as a
sort local IGS server to which xgospel will connect.  Relay will just give you
a  shell from which  you can do your normal  gateway login. The only thing you
should have to do is to do a login to IGS in such a way that commands will not
echo (since xgospel will be  very confused if everything  it enters is  echoed
back to it).

So, the topology should be:

+-----------------------------+               +-----------------+ 
|                             |               |                 |
| +---------+    +---------+  |               |    +--------+   |
| | xgospel |----| relay   |-----------------------| relnet |-----------  IGS
| +---------+    +---------+  | ANY kind of   |    +--------+   |
|                             | remote login  |                 |
+-----------------------------+               +-----------------+ 
   machine with X                              machine from where you will
                                               connect to IGS

   The connection on the   machine that is allowed  to  connect to IGS can  be
anything  as long as it doesn't  echo.  Relnet is  just a  sample program that
shows how you can do a non-echoing telnet.

A typical session would be:
on machine with X:
 in one xterm:
  dorelay             (this will start relay with the correct stty settings
                       and use rlogin to your own machine to give you an
                       echoing tty. It will also say at which port it is
                       waiting for xgospel)
 in another xterm:
  xgospel -site <X-machine> -port <port given by relay>
                      (relay will report when the connection is established.
                       now use some remote login method to get to your
                       connection machine, and there do:)
on connection machine (CM5 ?):
  relnet igs.nuri.net
                      (this will send an escape code to relay telling it to
                       redirect stdin and stdout to xgospel, do some tty
                       settings to shut up the tty and use telnet to connect
                       to port 6969). Relnet is just a sample showing how it
                       could be done. Any connection can be used as long as
                       you first give the escape code and you can convince it
                       not to echo. In the current version, relnet uses 
                       cat -u - | telnet ... to get telnet to shut up. On
                       some unices this will give an error ("cat would block")
                       In that case, change it to cat - | telnet ...
  quit xgospel
  exit remote machine
  exit rlogin shell

   The  real order that is allowed  is  a bit more   flexible. and xgospel can
connect and disconnect from a running relay arbitrarily often. You can connect
and disconnect  at any time,  as long as   a connection is established  at the
moment you do relnet, and  the connection is broken when  relnet stops.   Also
notice that relay doesn't even have to run on the same machine as xgospel, but
can be put on yet another machine (though you will have to compile it there).

The term program
----------------
(documentation courtesy of Jean-loup Gailly)

   Term  is an  independent  program (not part  of  xgospel).  It implements a
connection between 2 unix machines. It is built to run over a modem to connect
a non-internet machine  with an internet machine. It  is run at both ends, and
does multiplexing,  error correction, and compression  across  the serial link
between them.

   Term can be found on all major ftp sites, such as
sunsite.unc.edu:/pub/Linux/apps/comm/term/term/term112.tar.gz.  [To decompress:
gzip -cd term112.tar.gz | tar xvf - gzip is available in /pub/gnu/gzip* ] Read
the term  documentation to know how  to establish the connection  between your
local machine and your gateway. Once the connection is established, do:

  tredir 6969 igs.nuri.net:6969
  xgospel -site localhost &

   on your local machine.  You can avoid typing  "-site localhost" if you  put
the line

xgospel*site: localhost

in your X resources file.

   The main advantage of  term over relay is that  term multiplexes your modem
line. So you  can still  have shells on  your  remote machine or perform  file
transfers  in parallel  with xgospel.  Term  can  also do compression if  your
modem does not support it.

The rport program:
------------------

   This one is mainly used to circumvent fire walls. It often happens that you
can't directly connect from machine A to Z, but you  can reach Z  from B and B
from A (with B being a UNIX machine and all connections using TCP/IP). In that
case you compile the `rport' program on host  B (use the --without-x option to
`configure'  if you  are  not interesting in  having  xgospel on  site B. Then
finish the  compile with  `make rport' if  you don't  want relay  and  relog).
Every time you want a session you do:

  on host B:
    rport
    connect <Z> 6969 <Nr>

  [The second line ("connect ...") is something you type as input for rport;
   there is no program called "connect" -- Jean-loup]
  Replace Z by the  name of the IGS  server (probably igs.nuri.net),  and use
some arbitrary number for Nr (this is a UNIX port, so it should be bigger than
1000, smaller  than 65536, and not  already  be in use  (but there  are enough
numbers to try) If you don't give any number, rport will choose a free one for
you). Nothing stops you from using 6969 for <Nr>, and this is in fact the most
reasonable choice,  setting B up  as a kind  of relay  IGS server.

   On host A (or from any other place that can reach B) you can now connect to rport using:

   xgospel -site <B> -port <Nr>

   If you chose 6969  for <Nr>, you don't have  to  give the -port option.  In
case  you plan using  this method very often, you  can of course add site (and
port) in your X defaults file.

   `rport' will quit  if it loses stdin (so  if you logout,  or type ^D). This
makes sure that rport will only be running  only when you want  it (if you get
somehow disconnected  it will in general  loose stdin and quit). But sometimes
you might not want  this behavior.  In  most cases you  also have a number  of
fixed commands you want to  give to rport.  You can  collect the commands in a
file (let's  call  it  <file>), and use  rport   in the background   like this
(solving both previously mentioned problems):

   (tail -f <file> | rport) &
     
or if you really want to be sure that it keeps running when you log out:

   nohup (tail -f <file> | rport) &

   Notice  that rport can  pass  any number of  connections  to any number  of
machines, while    these connections are  used  arbitrarily  often  (of course
limited by your system resources), so you can use it  for all kinds of things.
If you set up  port 6969 on site B,  everybody can use  B as an  alternate IGS
server. Or maybe you cannot  read `news' from site  A, but can  from site B  ?
Give `rport' the command:

   connect <newsserver_machine> 119 <Nr>

and anybody can read news on site B using port <Nr> (if you are root you
should probably choose <Nr> to be 119, since for most news readers you
can not set an alternate port (119 is the default `news' port). Or you can 
have both newsserver and IGS redirection by just giving the two corresponding
connect commands.

   You can also `chain' rports. Start them up in any order:
     on B
       rport
       connect C <Nr1> <Nr0>
     on C
       rport
       connect D <Nr2> <Nr1>
     on D
       rport
       connect E <Nr3> <Nr2>
     on E
       rport
       connect Z 6969 <Nr3>

  If anybody now connects to B on port <Nr0>, B  will contact C on port <Nr1>,
C will contact  D on port  <Nr2>, D will  contact E on port  <Nr3>  and E will
contact Z on port 6969. So B will serve as an  IGS server on  port Nr0. If you
want to have   Nr0 as a  number  below 1000  (all lower  numbers are the  UNIX
protected ports), you should of course be super user  on site B, but you don't
have to be on any of the intervening sites. Or if you are only super user on A,
you can add an extra level at A itself:

   on A
     rport
     connect B <Nr0> <protected_port>

   So if all  intervening machines  are  UNIX machines (so  you  can get rport
compiled), and all communication is over  TCP/IP (rport assumes the BSD socket
interface), the `rport' program should be all  you need to pass on connections
that would otherwise demand a long trail of `rlogin' or `telnet' commands. With
sufficient motivation, you can even combine all three methods (rport, relay and
term).

The current widget tree:
------------------------

        xgospel(ApplicationShell)
            quit(Command)
            main(TopLevelShell)
                collect(Paned)
                    buttons(Box)
                        quit(Command)
                        options(Command)
                            optionMenu(SimpleMenu)
                                outputLabel(SmeLabel)
                                line(SmeLine)
                                beep(SmeBell)
                                raise(SmeToggle)
                                line(SmeLine)
                                errorLabel(SmeLabel)
                                line(SmeLine)
                                errorBeep(SmeBell)
                                errorRaise(SmeToggle)
                                line(SmeLine)
                                fileLabel(SmeLabel)
                                line(SmeLine)
                                file(SmeBSB)
                                overwrite(SmeToggle)
                                line(SmeLine)
                                connect(SmeLabel)
                                line(SmeLine)
                                hasConnect(SmeToggle)
                                wantConnect(SmeToggle)
                                line(SmeLine)
                                othersLabel(SmeLabel)
                                line(SmeLine)
                                analyzeSize(SmeBSB)
                                stdout(SmeToggle)
                        commands(Command)
                            commandMenu(SimpleMenu)
                                analyzeButton(SmeBSB)
                                save(SmeBSB)
                                statsMe(SmeBSB)
                        gamesButton(Toggle)
                        playersButton(Toggle)
                        messageButton(Toggle)
                        broadcastButton(Toggle)
                        yellButton(Toggle)
                        eventsButton(Toggle)
                        reviewsButton(Toggle)
                    times(Box)
                        localTime(Text)[string,localTime]
                        universalTime(Text)[string,universalTime]
                        serverTime(Text)[string,serverTime]
                    shortHelp(Label)
                    info(Text)
                    input(Text)
            quitConfirm(TopLevelShell)
                buttons(Paned)
                    confirm(Label)
                        confirmContainer(Paned)
                            ok(Command)
                            filler(Core)
                            cancel(Command)
            askString(TopLevelShell)[iconName,title][title,title]
                collect(Box)
                        user(Paned)
                            userLabel(Label)
                            textForm(Form)
                                text(Text)
                        password(Paned)
                            passwordLabel(Label)
                            textForm(Form)
                                text(Text)
                        sgfFilename(Paned)
                            sgfFilenameLabel(Label)
                            textForm(Form)
                                text(Text)[string,filename]
                        kibitzFilename(Paned)
                            kibitzFilenameLabel(Label)
                            textForm(Form)
                                text(Text)[string,filename]
                        broadcastFilename(Paned)
                            broadcastFilenameLabel(Label)
                            textForm(Form)
                                text(Text)[string,filename]
                        yellFilename(Paned)
                            yellFilenameLabel(Label)
                            textForm(Form)
                                text(Text)[string,filename]
                        tellFilename(Paned)
                            tellFilenameLabel(Label)
                            textForm(Form)
                                text(Text)[string,filename]
                        serverFilename(Paned)
                            serverFilenameLabel(Label)
                            textForm(Form)
                                text(Text)[string,filename]
                        eventsFilename(Paned)
                            serverFilenameLabel(Label)
                            textForm(Form)
                                text(Text)[string,filename]
                        mainFilename(Paned)
                            mainFilenameLabel(Label)
                            textForm(Form)
                                text(Text)[string,filename]
                        analyzeSize(Paned)
                            analyzeSizeLabel(Label)
                            textForm(Form)
                                text(Text)[string,size]
            popMessage(TopLevelShell)[iconName,title][title,title]
                collect(Paned)
                    message(Label)[label,text]
                    ok(Command)
            players(TopLevelShell)
                collect(Paned)
                    set(Box)
                    stats(Label)
                    stripform(Form)
                        strip(StripChart)
                    info(Text)
            games(TopLevelShell)
                collect(Paned)
                    set(Box)
                    stripform(Form)
                        strip(StripChart)
                    info(Text)
            igsMessages(TopLevelShell)
                collect(Paned)
                    buttons(Box)
                        quit(Command)
                        options(Command)
                            optionMenu(SimpleMenu)
                                messageLabel(SmeLabel)
                                line(SmeLine)
                                beep(SmeBell)
                                raise(SmeToggle)
                                line(SmeLine)
                                errorLabel(SmeLabel)
                                line(SmeLine)
                                errorBeep(SmeBell)
                                errorRaise(SmeToggle)
                                line(SmeLine)
                                fileLabel(SmeLabel)
                                line(SmeLine)
                                file(SmeBSB)
                                overwrite(SmeToggle)
                        save(Command)
                    info(Text)
            events(TopLevelShell)
                collect(Paned)
                    buttons(Box)
                        quit(Command)
                        options(Command)
                            optionMenu(SimpleMenu)
                                messageLabel(SmeLabel)
                                line(SmeLine)
                                beep(SmeBell)
                                raise(SmeToggle)
                                line(SmeLine)
                                errorLabel(SmeLabel)
                                line(SmeLine)
                                errorBeep(SmeBell)
                                errorRaise(SmeToggle)
                                line(SmeLine)
                                fileLabel(SmeLabel)
                                line(SmeLine)
                                file(SmeBSB)
                                overwrite(SmeToggle)
                        save(Command)
                    info(Text)
            broadcasts(TopLevelShell)
                collect(Paned)
                    buttons(Box)
                        quit(Command)
                        options(Command)
                            optionMenu(SimpleMenu)
                                messageLabel(SmeLabel)
                                line(SmeLine)
                                beep(SmeBell)
                                raise(SmeToggle)
                                line(SmeLine)
                                errorLabel(SmeLabel)
                                line(SmeLine)
                                errorBeep(SmeBell)
                                errorRaise(SmeToggle)
                                line(SmeLine)
                                fileLabel(SmeLabel)
                                line(SmeLine)
                                file(SmeBSB)
                                overwrite(SmeToggle)
                        save(Command)
                    info(Text)
                    input(Text)
            yells(TopLevelShell)
                collect(Paned)
                    buttons(Box)
                        quit(Command)
                        options(Command)
                            optionMenu(SimpleMenu)
                                messageLabel(SmeLabel)
                                line(SmeLine)
                                beep(SmeBell)
                                raise(SmeToggle)
                                line(SmeLine)
                                errorLabel(SmeLabel)
                                line(SmeLine)
                                errorBeep(SmeBell)
                                errorRaise(SmeToggle)
                                line(SmeLine)
                                fileLabel(SmeLabel)
                                line(SmeLine)
                                file(SmeBSB)
                                overwrite(SmeToggle)
                        save(Command)
                        channels(Command)
                        channel(Text)
                    info(Text)
                    input(Text)
            tell(TopLevelShell)
                collect(Paned)
                    buttons(Box)
                        quit(Command)
                        options(Command)
                            optionMenu(SimpleMenu)
                                messageLabel(SmeLabel)
                                line(SmeLine)
                                beep(SmeBell)
                                raise(SmeToggle)
                                line(SmeLine)
                                bugLabel(SmeLabel)
                                line(SmeLine)
                                bugBeep(SmeBell)
                                bugRaise(SmeToggle)
                                line(SmeLine)
                                errorLabel(SmeLabel)
                                line(SmeLine)
                                errorBeep(SmeBell)
                                errorRaise(SmeToggle)
                                line(SmeLine)
                                fileLabel(SmeLabel)
                                line(SmeLine)
                                file(SmeBSB)
                                overwrite(SmeToggle)
                        save(Command)
                        bug(Command)
                        getStats(Command)
                    info(Text)
                    input(Text)
            stats(TopLevelShell)
                collect(Box)
                    quit(Command)
                        info(Label)
                        text(Label)[label,text]
            observe(TopLevelShell)
                collect(Paned)
                    buttons(Box)
                        quit(Command)
                        options(Command)
                            optionMenu(SimpleMenu)
                                observeMoveLabel(SmeLabel)
                                line(SmeLine)
                                moveBeep(SmeBell)
                                moveRaise(SmeToggle)
                                line(SmeLine)
                                kibitzActionLabel(SmeLabel)
                                line(SmeLine)
                                kibitzBeep(SmeBell)
                                kibitzRaise(SmeToggle)
                                line(SmeLine)
                                errorLabel(SmeLabel)
                                line(SmeLine)
                                errorBeep(SmeBell)
                                errorRaise(SmeToggle)
                                line(SmeLine)
                                sgfLabel(SmeLabel)
                                line(SmeLine)
                                sgfFile(SmeBSB)
                                sgfFromStart(SmeToggle)
                                sgfOverwrite(SmeToggle)
                                line(SmeLine)
                                kibitzLabel(SmeLabel)
                                line(SmeLine)
                                kibitzFile(SmeBSB)
                                kibitzOverwrite(SmeToggle)
                                line(SmeLine)
                                othersLabel(SmeLabel)
                                line(SmeLine)
                                blink(SmeToggle)
                                replay(SmeToggle)
                        commands(Command)
                            commandMenu(SimpleMenu)
                                refresh(SmeBSB)
                                observers(SmeBSB)
                                dup(SmeBSB)
                                analyze(SmeBSB)
                                sgfSave(SmeBSB)
                                kibitzSave(SmeBSB)
                                talkBlack(SmeBSB)
                                talkWhite(SmeBSB)
                                statsBlack(SmeBSB)
                                statsWhite(SmeBSB)
                        captures(Label)[label,captures]
                        move(Label)[label,move]
                        time(Text)[string,time]
                    scrollboard(Paned)
                        scroll(Scrollbar)
                        board(Board)[boardSize,boardSize]
                    info(Text)
                    input(Text)
            observe(TopLevelShell)
                collect(Paned)
                    buttons(Box)
                        quit(Command)
                        options(Command)
                            optionMenu(SimpleMenu)
                                observeMoveLabel(SmeLabel)
                                line(SmeLine)
                                moveBeep(SmeBell)
                                moveRaise(SmeToggle)
                                line(SmeLine)
                                kibitzActionLabel(SmeLabel)
                                line(SmeLine)
                                kibitzBeep(SmeBell)
                                kibitzRaise(SmeToggle)
                                line(SmeLine)
                                errorLabel(SmeLabel)
                                line(SmeLine)
                                errorBeep(SmeBell)
                                errorRaise(SmeToggle)
                                line(SmeLine)
                                sgfLabel(SmeLabel)
                                line(SmeLine)
                                sgfFile(SmeBSB)
                                sgfFromStart(SmeToggle)
                                sgfOverwrite(SmeToggle)
                                line(SmeLine)
                                kibitzLabel(SmeLabel)
                                line(SmeLine)
                                kibitzFile(SmeBSB)
                                kibitzOverwrite(SmeToggle)
                                line(SmeLine)
                                othersLabel(SmeLabel)
                                line(SmeLine)
                                blink(SmeToggle)
                                replay(SmeToggle)
                        commands(Command)
                            commandMenu(SimpleMenu)
                                resume(SmeBSB)
                                refresh(SmeBSB)
                                observers(SmeBSB)
                                dup(SmeBSB)
                                analyze(SmeBSB)
                                sgfSave(SmeBSB)
                                kibitzSave(SmeBSB)
                                talkBlack(SmeBSB)
                                talkWhite(SmeBSB)
                                statsBlack(SmeBSB)
                                statsWhite(SmeBSB)
                        undo(Command)
                        pass(Command)
                        done(Command)
                        captures(Label)[label,captures]
                        move(Label)[label,move]
                        time(Text)[string,time]
                    scrollboard(Paned)
                        scroll(Scrollbar)
                        board(Board)[boardSize,boardSize]
                    info(Text)
                    input(Text)
            analyzer(TopLevelShell)
                collect(Paned)
                    buttons(Box)
                        quit(Command)
                        options(Command)
                            optionMenu(SimpleMenu)
                                errorLabel(SmeLabel)
                                line(SmeLine)
                                errorBeep(SmeBell)
                                errorRaise(SmeToggle)
                                line(SmeLine)
                                sgfLabel(SmeLabel)
                                line(SmeLine)
                                sgfFile(SmeBSB)
                                sgfFromStart(SmeToggle)
                                sgfOverwrite(SmeToggle)
                                line(SmeLine)
                                othersLabel(SmeLabel)
                                line(SmeLine)
                                blink(SmeToggle)
                        commands(Command)
                            commandMenu(SimpleMenu)
                                reset(SmeBSB)
                                analyze(SmeBSB)
                                sgfSave(SmeBSB)
                        undo(Command)
                    scrollboard(Paned)
                        scroll(Scrollbar)
                        board(Board)[boardSize,boardSize]
    resourceTree(TopLevelShell)
            widgetInfo(TopLevelShell)
                collect(Paned)
                    buttons(Box)
                        restreeQuit(Command)
                            parent(Command)
                            children(Command)
                            popups(Command)
                        change(Command)
                    info(Text)
            widgetTree(TopLevelShell)
                collect(Paned)
                    buttons(Box)
                        restreeQuit(Command)
                    viewport(Viewport)
                        tree(Tree)
            widgetChange(TopLevelShell)
                box(Box)
                    cancel(Command)
                    ok(Command)
                    name(Text)[editType,editType]
                    value(Text)[editType,editType]
    widgetHelp(OverrideShell)
        text(Label)

Todo:
-----
- Recognize more commands and messages

Copyright:
----------
   Gnu copyleft. You can essentially do anything you want with the program, as
long as you  make sure  the source is  available.   Take a  look  in the  file
my/COPYRIGHTS  for  info about the   parts that are not  written  by me and of
course retain their original copyrights.

P.S.
----
   Not only the program,  but also  this file are  still under  development. I
would appreciate comments about both the program and the documentation.

P.P.S.
------
   The  latest development  version of the   program should  be  available for
anonymous ftp on linux3.cc.kuleuven.ac.be in the directory pub/games/Go as the
file xgospel-???.tar.gz  . Don't forget  to  set the  transfer mode to binary.
For the latest released version,  ftp to bsdserver.ucsf.edu,  and look in  the
directory Go/clients for xgospel-???.tar.Z

   When sending bug/problem,  please report the system  type type (see the end
of the INSTALL file on how to find out). Also give a  log of the make process,
instead of just saying `it didn't work'.

                                              Ton Hospel
                                              ton@linux.cc.kuleuven.ac.be
                                              (AshaiRey on IGS)

The rest of this file exists to make emacs (ispell) happy.

 LocalWords:  xgospel applicationdefaults skipAdjust bsdserver ucsf edu wharton
 LocalWords:  AshaiRey hellspark upenn debugFun debugFile iconName SmeBell NR
 LocalWords:  errorBeep moveRaise kibitzRaise sortName sortNumber set cs nuri
 LocalWords:  playerEntryk playerEntryd playerEntryp backgroundPixmap lineWidth
 LocalWords:  playerEntry playerEntryNR playerEntryIGS playerToWidget audioFile
 LocalWords:  MyStrength playerEntryMe playerEntryMyStrength Nici Schraudolph
 LocalWords:  schraudo ucsd nic ApplicationShell TopLevelShell Paned optionMenu
 LocalWords:  SimpleMenu outputLabel SmeLabel SmeLine SmeToggle errorLabel ok
 LocalWords:  errorRaise fileLabel SmeBSB hasConnect wantConnect othersLabel gz
 LocalWords:  analyzeSize commandMenu analyzeButton statsMe gamesButton rport
 LocalWords:  playersButton messageButton broadcastButton yellButton localTime
 LocalWords:  eventsButton reviewsButton localTime universalTime universalTime
 LocalWords:  serverTime serverTime shortHelp quitConfirm confirmContainer dup
 LocalWords:  askString userLabel textForm passwordLabel sgfFilename popMessage
 LocalWords:  sgfFilenameLabel kibitzFilename kibitzFilenameLabel yellFilename
 LocalWords:  broadcastFilename broadcastFilenameLabel yellFilenameLabel popups
 LocalWords:  tellFilename tellFilenameLabel serverFilename serverFilenameLabel
 LocalWords:  eventsFilename mainFilename mainFilenameLabel analyzeSizeLabel ac
 LocalWords:  stripform StripChart igsMessages messageLabel bugLabel bugBeep
 LocalWords:  bugRaise getStats observeMoveLabel moveBeep kibitzActionLabel mgt
 LocalWords:  kibitzBeep sgfLabel sgfFile sgfFromStart sgfOverwrite kibitzLabel
 LocalWords:  kibitzFile kibitzOverwrite sgfSave kibitzSave talkBlack talkWhite
 LocalWords:  statsBlack statsWhite scrollboard boardSize boardSize widgetInfo
 LocalWords:  resourceTree restreeQuit widgetTree viewport Viewport editType
 LocalWords:  widgetChange editType widgetHelp OverrideShell allowShellResize
 LocalWords:  helptext relnet dorelay unices loup Gailly sunsite unc apps comm
 LocalWords:  xvf tredir Todo copyleft LocalWords newsserver rports nohup relog
