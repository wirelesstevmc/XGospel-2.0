#include <algorithm>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFrame>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTextBrowser>
#include <QtGui/QIcon>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QFileDialog>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QStyledItemDelegate>
#include <QtCore/QStack>
#include <QtCore/QQueue>
#include <QtCore/QSet>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtGui/QCursor>
#include <QtGui/QCloseEvent>
#include <QtCore/QSortFilterProxyModel>
#include <QtCore/QTimer>
#include <QtCore/QRandomGenerator>
#include <QtCore/QCoreApplication>
#include <QtCore/QEventLoop>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>
#include "xgospel2_parser.h"
#include "xgospel2_games_parser.h"
#include "board_window.h"
#include "igs_move_parser.h"
#include "sgf_parser.h"
#include "settings.h"
#include "preferences_dialog.h"
#include "engine_manager.h"
#include "katago_engine.h"
#include "local_game_dialog.h"
#include "score_engine.h"

// Version information - update these with each release
const QString XGOSPEL_VERSION = "v286";
const QString XGOSPEL_BUILD_DATE = "2026-08-09";

class FixedRankSortProxyModel : public QSortFilterProxyModel {
public:
 FixedRankSortProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent) {}

protected:
 bool lessThan(const QModelIndex &left, const QModelIndex &right) const override {
 // Use UserRole sort key for any column that stores one (rank columns in both
 // players window [col 2] and games window [col 2=WR, col 4=BR]).
 QString leftKey  = sourceModel()->data(left,  Qt::UserRole).toString();
 QString rightKey = sourceModel()->data(right, Qt::UserRole).toString();
 if (!leftKey.isEmpty() && !rightKey.isEmpty())
     return leftKey < rightKey;
 return QSortFilterProxyModel::lessThan(left, right);
 }
};

// Player Stats and Match Dialog
class PlayerStatsDialog : public QDialog {
 Q_OBJECT

private:
 QString player_name;
 QString player_rank;
 QString country;
 QString stat;
 QString playing;
 QString observing;
 QString idle_time;
 QString info;
 QString rated_record;
 QString match_prefs;
 QString last_log;
 int wins, losses;
 QString chat_history;  // Persistent chat history storage (Bug #2 fix)
 bool is_local_player;  // Feature 33: Is this the local player's dialog?

 // Feature 33c: Toggle button state tracking
 bool state_looking;
 bool state_open;
 bool state_quiet;
 bool state_shout;

 QLabel *player_name_label;
 QLabel *stats_label;
 QLabel *rated_label;
 QLabel *details_label;
 QLabel *match_prefs_label;
 // Dynamic data labels updated in-place on stats refresh (avoids Close button duplication)
 QLabel *dyn_wins_label;
 QLabel *dyn_losses_label;
 QLabel *dyn_rated_label;
 QLabel *dyn_country_label;
 QLabel *dyn_info_label;
 QLabel *dyn_idle_label;
 QLabel *dyn_playing_label;
 QLabel *dyn_observing_label;
 QLabel *dyn_lastlog_label;
 QLabel *dyn_matchprefs_label;
 QTextEdit *message_display;
 QLineEdit *message_input;
 QPushButton *send_button;
 QPushButton *match_button;
 QPushButton *nmatch_button;
 QPushButton *close_button;

 // Feature 33c: Toggle button pointers (local player only)
 QPushButton *toggle_looking;
 QPushButton *toggle_open;
 QPushButton *toggle_quiet;
 QPushButton *toggle_shout;

public:
 PlayerStatsDialog(QWidget *parent, const QString &name, const QString &rank,
 const QString &country_str, int w, int l,
 const QString &stat_str = "", const QString &pl = "", const QString &ob = "",
 const QString &idle = "", const QString &info_str = "",
 const QString &rated_str = "", const QString &match_prefs_str = "",
 bool is_local = false)
 : QDialog(parent), player_name(name), player_rank(rank),
 country(country_str), stat(stat_str), playing(pl), observing(ob),
 idle_time(idle), info(info_str), rated_record(rated_str),
 match_prefs(match_prefs_str), wins(w), losses(l), is_local_player(is_local),
 state_looking(false), state_open(false), state_quiet(false), state_shout(false),
 toggle_looking(nullptr), toggle_open(nullptr), toggle_quiet(nullptr), toggle_shout(nullptr) {

 setWindowTitle(QString("Player: %1 [%2]").arg(name, rank));
 parseToggleStatesFromStat();  // Parse toggle states before setupUI
 setupUI();
 }

 // Parse toggle states from stat field (IGS flags)
 void parseToggleStatesFromStat() {
 // IGS stat field flags:
 // '!' = Looking for game / Not accepting matches
 // 'X' = Not open for matches
 // 'Q' = Quiet mode
 // 'S' = Shout mode
 state_looking = stat.contains('!');
 state_open = !stat.contains('X');  // Open is the inverse of X flag
 state_quiet = stat.contains('Q');
 state_shout = stat.contains('S');

 qDebug() << "[TOGGLE-PARSE] Parsed stat field:" << stat
          << "-> Looking:" << state_looking
          << "Open:" << state_open
          << "Quiet:" << state_quiet
          << "Shout:" << state_shout;
 }

 void setupUI() {
 // Set xgospel icon for parentless dialogs (important for taskbar identification)
 setWindowIcon(QIcon("XgospelIcon.xpm"));

 // Initialise dynamic label pointers so updateDynamicLabels() is safe before first stats arrive
 player_name_label = nullptr; // name shown in title bar; widget not created
 dyn_wins_label = dyn_losses_label = dyn_rated_label = dyn_country_label = nullptr;
 dyn_info_label = dyn_idle_label = dyn_playing_label = dyn_observing_label = nullptr;
 dyn_lastlog_label = dyn_matchprefs_label = nullptr;

 QVBoxLayout *main_layout = new QVBoxLayout(this);
 main_layout->setSpacing(6);
 main_layout->setMargin(10);

 // Player info header - compact grid layout
 QFrame *header_frame = new QFrame;
 header_frame->setFrameStyle(QFrame::Sunken | QFrame::Panel);
 header_frame->setLineWidth(2);
 header_frame->setStyleSheet(
 "QFrame {" " background-" " border: 2px inset #888;" " padding: 5px;" "}"
 );

 QVBoxLayout *header_container = new QVBoxLayout(header_frame);
 header_container->setSpacing(1);
 header_container->setMargin(3);

 // Compact 2-column grid for player stats (name already shown in title bar)
 QGridLayout *grid = new QGridLayout();
 grid->setSpacing(1);
 grid->setVerticalSpacing(2);
 grid->setHorizontalSpacing(12);
 grid->setMargin(0);

 int row = 0;

 // Row 0: Wins | Losses
 dyn_wins_label = new QLabel(QString("<b>Wins:</b> %1").arg(wins == 0 && losses == 0 ? "--" : QString::number(wins)));
 dyn_wins_label->setStyleSheet("font-size: 11px;");
 grid->addWidget(dyn_wins_label, row, 0, Qt::AlignLeft);

 dyn_losses_label = new QLabel(QString("<b>Losses:</b> %1").arg(wins == 0 && losses == 0 ? "--" : QString::number(losses)));
 dyn_losses_label->setStyleSheet("font-size: 11px;");
 grid->addWidget(dyn_losses_label, row, 1, Qt::AlignLeft);
 row++;

 // Row 1: Rated Games | Country
 dyn_rated_label = new QLabel();
 dyn_rated_label->setStyleSheet("font-size: 11px;");
 grid->addWidget(dyn_rated_label, row, 0, Qt::AlignLeft);

 dyn_country_label = new QLabel();
 dyn_country_label->setStyleSheet("font-size: 11px;");
 grid->addWidget(dyn_country_label, row, 1, Qt::AlignLeft);
 row++;

 // Row 2: Info | Idle
 dyn_info_label = new QLabel();
 dyn_info_label->setStyleSheet("font-size: 11px;");
 dyn_info_label->setWordWrap(true);
 grid->addWidget(dyn_info_label, row, 0, Qt::AlignLeft);

 dyn_idle_label = new QLabel();
 dyn_idle_label->setStyleSheet("font-size: 11px;");
 grid->addWidget(dyn_idle_label, row, 1, Qt::AlignLeft);
 row++;

 // Row 3: Playing | Observing
 dyn_playing_label = new QLabel();
 dyn_playing_label->setStyleSheet("font-size: 11px;");
 grid->addWidget(dyn_playing_label, row, 0, Qt::AlignLeft);

 dyn_observing_label = new QLabel();
 dyn_observing_label->setStyleSheet("font-size: 11px;");
 grid->addWidget(dyn_observing_label, row, 1, Qt::AlignLeft);
 row++;

 // Row 4: Last log — spans both columns
 dyn_lastlog_label = new QLabel();
 dyn_lastlog_label->setStyleSheet("font-size: 11px;");
 dyn_lastlog_label->setMinimumWidth(400);
 grid->addWidget(dyn_lastlog_label, row, 0, 1, 2, Qt::AlignLeft);
 row++;

 header_container->addLayout(grid);

 // Match Preferences — always created, shown/hidden by updateDynamicLabels
 dyn_matchprefs_label = new QLabel();
 dyn_matchprefs_label->setStyleSheet("font-size: 11px; margin-top: 2px;");
 dyn_matchprefs_label->setWordWrap(true);
 header_container->addWidget(dyn_matchprefs_label);

 // Populate all dynamic labels with current data
 updateDynamicLabels();

 main_layout->addWidget(header_frame);

 // Message/Tell section
 QGroupBox *tell_group = new QGroupBox("Send Message");
 QVBoxLayout *tell_layout = new QVBoxLayout(tell_group);

 message_display = new QTextEdit;
 message_display->setReadOnly(true);
 message_display->setMaximumHeight(120);
 message_display->setStyleSheet(
 "QTextEdit {" " " " border: 1px solid #ccc;" " font-family: monospace;" " font-size: 11px;" "}"
 );
 tell_layout->addWidget(message_display);

 QHBoxLayout *input_layout = new QHBoxLayout;
 message_input = new QLineEdit;
 message_input->setPlaceholderText("Type message to send...");
 message_input->setMinimumHeight(30);
 message_input->setStyleSheet("font-size: 11px;");
 send_button = new QPushButton("Send");
 send_button->setMaximumWidth(80);
 send_button->setStyleSheet(
 "QPushButton {" " background-" " " " border: none;" " padding: 5px 15px;" " font-weight: bold;" "}" "QPushButton:pressed {" " background-" "}"
 );
 input_layout->addWidget(message_input);
 input_layout->addWidget(send_button);
 tell_layout->addLayout(input_layout);

 main_layout->addWidget(tell_group);

 // Feature 33: Toggle buttons for local player
 if (is_local_player) {
 QGroupBox *toggle_group = new QGroupBox("Account Settings");
 QGridLayout *toggle_layout = new QGridLayout(toggle_group);
 toggle_layout->setSpacing(4);

 // Feature 33c: Create toggle buttons with state tracking
 toggle_looking = new QPushButton("Looking");
 toggle_open = new QPushButton("Open");
 toggle_quiet = new QPushButton("Quiet");
 toggle_shout = new QPushButton("Shout");

 // Connect to send toggle commands
 connect(toggle_looking, &QPushButton::clicked, this, [this]() {
 qDebug() << "[TOGGLE] Looking clicked";
 emit toggleRequested("looking");
 });
 connect(toggle_open, &QPushButton::clicked, this, [this]() {
 qDebug() << "[TOGGLE] Open clicked";
 emit toggleRequested("open");
 });
 connect(toggle_quiet, &QPushButton::clicked, this, [this]() {
 qDebug() << "[TOGGLE] Quiet clicked";
 emit toggleRequested("quiet");
 });
 connect(toggle_shout, &QPushButton::clicked, this, [this]() {
 qDebug() << "[TOGGLE] Shout clicked";
 emit toggleRequested("shout");
 });

 // Layout in 2x2 grid
 toggle_layout->addWidget(toggle_looking, 0, 0);
 toggle_layout->addWidget(toggle_open, 0, 1);
 toggle_layout->addWidget(toggle_quiet, 1, 0);
 toggle_layout->addWidget(toggle_shout, 1, 1);

 main_layout->addWidget(toggle_group);

 // Feature 33c: Apply initial button styling (all OFF)
 updateToggleButtonStyle();
 }

 // Match buttons
 QGroupBox *match_group = new QGroupBox("Request Game");
 QVBoxLayout *match_layout = new QVBoxLayout(match_group);

 QHBoxLayout *button_layout = new QHBoxLayout;

 match_button = new QPushButton("Match\n(Standard)");
 match_button->setMinimumHeight(50);
 match_button->setStyleSheet(
 "QPushButton {" " background-" " " " border: 2px outset #1565C0;" " padding: 10px;" " font-weight: bold;" " font-size: 12px;" "}" "QPushButton:pressed {" " border: 2px inset #1565C0;" " background-" "}"
 );

 nmatch_button = new QPushButton("Nmatch\n(Modern)");
 nmatch_button->setMinimumHeight(50);
 nmatch_button->setStyleSheet(
 "QPushButton {" " background-" " " " border: 2px outset #F57C00;" " padding: 10px;" " font-weight: bold;" " font-size: 12px;" "}" "QPushButton:pressed {" " border: 2px inset #F57C00;" " background-" "}"
 );

 button_layout->addWidget(match_button, 1);
 button_layout->addWidget(nmatch_button, 1);
 match_layout->addLayout(button_layout);

 QLabel *info_label = new QLabel(
 "<small><b>Match:</b> Standard IGS protocol<br>" "<b>Nmatch:</b> Modern protocol with handicap/komi options</small>"
 );
 info_label->setStyleSheet(" margin-top: 5px;");
 match_layout->addWidget(info_label);

 main_layout->addWidget(match_group);

 // Close button
 QHBoxLayout *close_layout = new QHBoxLayout;
 close_layout->addStretch();
 close_button = new QPushButton("Close");
 close_button->setMaximumWidth(100);
 close_button->setStyleSheet(
 "QPushButton {" " background-" " " " border: none;" " padding: 8px 16px;" " font-weight: bold;" "}" "QPushButton:pressed {" " background-" "}"
 );
 close_layout->addWidget(close_button);
 main_layout->addLayout(close_layout);

 // Connections
 connect(send_button, &QPushButton::clicked, this, &PlayerStatsDialog::sendMessage);
 connect(message_input, &QLineEdit::returnPressed, this, &PlayerStatsDialog::sendMessage);
 connect(match_button, &QPushButton::clicked, this, &PlayerStatsDialog::requestMatch);
 connect(nmatch_button, &QPushButton::clicked, this, &PlayerStatsDialog::requestNmatch);
 connect(close_button, &QPushButton::clicked, this, &QDialog::accept);

 // Restore chat history after widget recreation (Bug #2 fix)
 if (!chat_history.isEmpty() && message_display) {
 message_display->setHtml(chat_history);
 qDebug() << "[CHAT-RESTORE] Restored chat history with" << chat_history.count('\n') << "messages";
 }

 // Auto-resize dialog to fit content
 adjustSize();
 // No minimum width constraint - let adjustSize() calculate optimal width
 }

 QString getPlayerName() const { return player_name; }
 QString getPlayerRank() const { return player_rank; }

 // Feature 33c: Update toggle button styling based on current state
 void updateToggleButtonStyle() {
 if (!is_local_player) return;  // Only for local player

 // Gold color for ON state, gray for OFF state
 QString style_on =
 "QPushButton {"
 "  background-color: #edd20d;"
 "  color: black;"
 "  border: 3px inset #d4b909;"  // Inset = pressed/ON
 "  padding: 6px;"
 "  font-weight: bold;"
 "  font-size: 10px;"
 "  min-height: 28px;"
 "}"
 "QPushButton:pressed {"
 "  border: 3px inset #c4a808;"
 "  background-color: #c4a808;"
 "}";

 QString style_off =
 "QPushButton {"
 "  background-color: #d0d0d0;"
 "  color: #606060;"
 "  border: 3px outset #e0e0e0;"  // Outset = unpressed/OFF
 "  padding: 6px;"
 "  font-weight: bold;"
 "  font-size: 10px;"
 "  min-height: 28px;"
 "}"
 "QPushButton:pressed {"
 "  border: 3px inset #b0b0b0;"
 "  background-color: #b0b0b0;"
 "}";

 if (toggle_looking) toggle_looking->setStyleSheet(state_looking ? style_on : style_off);
 if (toggle_open) toggle_open->setStyleSheet(state_open ? style_on : style_off);
 if (toggle_quiet) toggle_quiet->setStyleSheet(state_quiet ? style_on : style_off);
 if (toggle_shout) toggle_shout->setStyleSheet(state_shout ? style_on : style_off);

 qDebug() << "[TOGGLE-STATE] Updated button styles: Looking=" << state_looking
          << "Open=" << state_open << "Quiet=" << state_quiet << "Shout=" << state_shout;
 }

 // Feature 33c: Update toggle state from server response
 void updateToggleState(const QString &toggle_name, bool is_on) {
 if (!is_local_player) return;  // Only for local player

 qDebug() << "[TOGGLE-STATE] Updating" << toggle_name << "to" << (is_on ? "ON" : "OFF");

 if (toggle_name == "looking") {
 state_looking = is_on;
 } else if (toggle_name == "open") {
 state_open = is_on;
 } else if (toggle_name == "quiet") {
 state_quiet = is_on;
 } else if (toggle_name == "shout") {
 state_shout = is_on;
 }

 updateToggleButtonStyle();
 }

protected:
 void showEvent(QShowEvent *event) override {
 QDialog::showEvent(event);
 // Request player stats from server when dialog is first shown
 emit statsRequested(player_name);
 qDebug() << "[STATS] Requesting stats for player:" << player_name;
 }

public:
 // Update only the data labels in place — no layout teardown, no button duplication.
 void updateDynamicLabels() {
     if (dyn_wins_label)
         dyn_wins_label->setText(QString("<b>Wins:</b> %1").arg(wins == 0 && losses == 0 ? "--" : QString::number(wins)));
     if (dyn_losses_label)
         dyn_losses_label->setText(QString("<b>Losses:</b> %1").arg(wins == 0 && losses == 0 ? "--" : QString::number(losses)));
     if (dyn_rated_label) {
         bool show = !rated_record.isEmpty() && rated_record != "0";
         dyn_rated_label->setText(show ? QString("<b>Rated:</b> %1").arg(rated_record) : QString());
         dyn_rated_label->setVisible(show);
     }
     if (dyn_country_label) {
         dyn_country_label->setText(!country.isEmpty() ? QString("<b>Country:</b> %1").arg(country) : QString());
         dyn_country_label->setVisible(!country.isEmpty());
     }
     if (dyn_info_label) {
         bool show = !info.isEmpty() && info != "<None>";
         if (show) {
             QString esc = QString(info).replace("&","&amp;").replace("<","&lt;").replace(">","&gt;").replace("\"","&quot;");
             dyn_info_label->setText(QString("<b>Info:</b> %1").arg(esc));
         } else {
             dyn_info_label->setText(QString());
         }
         dyn_info_label->setVisible(show);
     }
     if (dyn_idle_label) {
         dyn_idle_label->setText(!idle_time.isEmpty() ? QString("<b>Idle:</b> %1").arg(idle_time) : QString());
         dyn_idle_label->setVisible(!idle_time.isEmpty());
     }
     if (dyn_playing_label) {
         bool show = !playing.isEmpty() && playing != "0" && playing != "--";
         dyn_playing_label->setText(show ? QString("<b>Playing:</b> %1").arg(playing) : QString());
         dyn_playing_label->setVisible(show);
     }
     if (dyn_observing_label) {
         bool show = !observing.isEmpty() && observing != "0" && observing != "--";
         dyn_observing_label->setText(show ? QString("<b>Observing:</b> %1").arg(observing) : QString());
         dyn_observing_label->setVisible(show);
     }
     if (dyn_lastlog_label) {
         dyn_lastlog_label->setText(!last_log.isEmpty() ? QString("<b>Last log:</b> %1").arg(last_log) : QString());
         dyn_lastlog_label->setVisible(!last_log.isEmpty());
     }
     if (dyn_matchprefs_label) {
         dyn_matchprefs_label->setText(!match_prefs.isEmpty() ? QString("<b>Match Prefs:</b> %1").arg(match_prefs) : QString());
         dyn_matchprefs_label->setVisible(!match_prefs.isEmpty());
     }
     if (player_name_label)
         player_name_label->setText(QString("<b>%1</b> [<b>%2</b>]").arg(player_name, player_rank));
 }

 // Update dialog with stats data from server
 void updateStatsData(int w, int l, const QString &rated_str, const QString &obs_str = QString(), const QString &play_str = QString(), const QString &match_prefs_str = QString(), const QString &info_str = QString(), const QString &rank_str = QString(), const QString &country_str = QString(), const QString &last_log_str = QString()) {
 wins = w;
 losses = l;
 rated_record = rated_str;
 if (!obs_str.isEmpty()) {
 observing = obs_str;
 qDebug() << "[STATS-UPDATE] Setting observing to:" << obs_str;
 } else {
 qDebug() << "[STATS-UPDATE] obs_str is empty, keeping observing as:" << observing;
 }
 if (!play_str.isEmpty()) {
 playing = play_str;
 qDebug() << "[STATS-UPDATE] Setting playing to:" << play_str;
 } else {
 qDebug() << "[STATS-UPDATE] play_str is empty, keeping playing as:" << playing;
 }
 if (!match_prefs_str.isEmpty()) {
 match_prefs = match_prefs_str;
 qDebug() << "[STATS-UPDATE] Setting match_prefs to:" << match_prefs_str;
 }
 if (!info_str.isEmpty()) {
 info = info_str;
 qDebug() << "[STATS-UPDATE] Setting info to:" << info_str;
 }
 if (!rank_str.isEmpty()) {
 player_rank = rank_str;
 setWindowTitle(QString("Player: %1 [%2]").arg(player_name, player_rank));
 qDebug() << "[STATS-UPDATE] Setting rank to:" << rank_str;
 }
 if (!country_str.isEmpty()) {
 country = country_str;
 qDebug() << "[STATS-UPDATE] Setting country to:" << country_str;
 }
 if (!last_log_str.isEmpty()) {
 last_log = last_log_str;
 qDebug() << "[STATS-UPDATE] Setting last_log to:" << last_log_str;
 }

 parseToggleStatesFromStat();
 updateDynamicLabels();

 qDebug() << "[STATS-UPDATE] Updated labels - Rank:" << player_rank << "Country:" << country << "Last log:" << last_log
 << "Wins:" << wins << "Losses:" << losses
 << "Rated:" << rated_record << "Playing:" << playing << "Observing:" << observing
 << "Match Prefs:" << match_prefs;
 }

public slots:
 void receiveIncomingTell(const QString &sender, const QString &message) {
 // Only display if this tell is from the player we're chatting with
 if (!message_display) {
 return;
 }
 if (sender.compare(player_name, Qt::CaseInsensitive) == 0) {
 QString formatted_msg = QString("<b>%1:</b> %2").arg(sender, message);
 chat_history += formatted_msg + "\n";  // Store in persistent history (Bug #2 fix)
 message_display->append(formatted_msg);
 }
 }

private slots:
 void sendMessage() {
 QString msg = message_input->text().trimmed();
 if (!msg.isEmpty()) {
 QString formatted_msg = QString("<b>You:</b> %1").arg(msg);
 chat_history += formatted_msg + "\n";  // Store in persistent history (Bug #2 fix)
 message_display->append(formatted_msg);
 emit tellRequested(player_name, msg);
 message_input->clear();
 }
 }

 void requestMatch() {
 emit matchRequested(player_name, false); // false = old match protocol
 // Bug 32b: Don't close dialog, keep it open for continued chatting
 // accept();
 }

 void requestNmatch() {
 emit matchRequested(player_name, true); // true = nmatch protocol
 // Bug 32b: Don't close dialog, keep it open for continued chatting
 // accept();
 }

signals:
 void tellRequested(const QString &player, const QString &message);
 void matchRequested(const QString &player, bool is_nmatch);
 void statsRequested(const QString &player); // Request player stats from server
 void toggleRequested(const QString &parameter); // Feature 33: Toggle account setting
};

// Custom delegate for 3D button-style player rows (v50_R2)
// Also highlights players with open stats dialogs
class PlayerDelegate : public QStyledItemDelegate {
 Q_OBJECT
private:
 QSortFilterProxyModel* proxy_model;
 const QSet<QString>* open_player_names;  // Track which players have open dialogs
 bool use_focus_colors;

 // State tracking for button rendering
 mutable QPersistentModelIndex hover_index;
 mutable QPersistentModelIndex pressed_index;

public:
 PlayerDelegate(QSortFilterProxyModel* proxy, const QSet<QString>* open_names, bool focus_colors, QObject* parent = nullptr)
 : QStyledItemDelegate(parent), proxy_model(proxy), open_player_names(open_names), use_focus_colors(focus_colors) {}

 void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
 // Get the player name from column 1 of this row
 QModelIndex name_index = index.sibling(index.row(), 1);
 QString player_name = name_index.data().toString();

 bool is_open = (open_player_names && open_player_names->contains(player_name));
 bool is_hover = (hover_index.isValid() && hover_index.row() == index.row());
 bool is_pressed = (pressed_index.isValid() && pressed_index.row() == index.row());

 if (is_open) {
 // Highlight players with open dialogs (system theme colors, like observed games)
 QStyleOptionViewItem opt = option;

 // Mark as selected to get 3D rendering effect from Qt style system
 opt.state |= QStyle::State_Selected;

 // Control focus-based color changes (q5Go always uses inactive color)
 if (!use_focus_colors) {
 // Remove State_Active to always use the inactive/unfocused color
 opt.state &= ~QStyle::State_Active;
 }

 QStyledItemDelegate::paint(painter, opt, index);
 } else {
 // v50_R2: Render players without open dialogs as 3D buttons

 // Only draw the button frame once per row (when painting first column)
 if (index.column() == 0) {
 // Get the view to calculate full row rect
 const QAbstractItemView* view = qobject_cast<const QAbstractItemView*>(option.widget);
 if (view) {
 // Calculate the full row rectangle spanning all columns
 QRect row_rect = option.rect;
 row_rect.setLeft(0);
 row_rect.setWidth(view->viewport()->width());

 // Draw 3D button frame for the entire row
 QStyleOptionButton button_opt;
 button_opt.rect = row_rect;
 button_opt.state = QStyle::State_Enabled;

 // Add raised/sunken state based on interaction
 if (is_pressed) {
 button_opt.state |= QStyle::State_Sunken;
 } else {
 button_opt.state |= QStyle::State_Raised;
 }

 // Add hover state for visual feedback
 if (is_hover) {
 button_opt.state |= QStyle::State_MouseOver;
 }

 // Draw button background (3D frame) for entire row
 QApplication::style()->drawControl(QStyle::CE_PushButton, &button_opt, painter);
 }
 }

 // Draw the text content normally (for all columns)
 QStyledItemDelegate::paint(painter, option, index);
 }
 }

 bool editorEvent(QEvent* event, QAbstractItemModel* model,
 const QStyleOptionViewItem& option, const QModelIndex& index) override {
 // Handle mouse events for single-click actuation
 if (event->type() == QEvent::MouseButtonPress) {
 QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
 if (mouseEvent->button() == Qt::LeftButton) {
 pressed_index = index;
 if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(const_cast<QWidget*>(option.widget))) {
 view->viewport()->update();
 }
 return false;  // Allow event to propagate
 }
 }
 else if (event->type() == QEvent::MouseButtonRelease) {
 QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
 if (mouseEvent->button() == Qt::LeftButton && pressed_index.isValid()) {
 pressed_index = QPersistentModelIndex();
 if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(const_cast<QWidget*>(option.widget))) {
 view->viewport()->update();

 // Emit single-click signal to trigger player info dialog
 if (index.isValid()) {
 emit clicked(index);
 }
 }
 return true;  // Event handled
 }
 }
 else if (event->type() == QEvent::MouseMove) {
 QPersistentModelIndex old_hover = hover_index;
 hover_index = index;
 if (old_hover != hover_index) {
 if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(const_cast<QWidget*>(option.widget))) {
 view->viewport()->update();
 }
 }
 return false;
 }
 else if (event->type() == QEvent::Leave) {
 hover_index = QPersistentModelIndex();
 pressed_index = QPersistentModelIndex();
 if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(const_cast<QWidget*>(option.widget))) {
 view->viewport()->update();
 }
 return false;
 }

 return QStyledItemDelegate::editorEvent(event, model, option, index);
 }

signals:
 void clicked(const QModelIndex& index);
};

class FixedPlayersWindow : public QMainWindow {
 Q_OBJECT

private:
 QTreeView *players_table;
 QStandardItemModel *players_model;
 FixedRankSortProxyModel *proxy_model;
 PlayerDelegate* player_delegate;  // v50_R2: Custom delegate for button-style rows
 QCheckBox *open_filter_checkbox;  // Filter to show only "open" players
 QComboBox *from_rank_combo;  // Bug 35: From rank filter
 QComboBox *to_rank_combo;    // Bug 35: To rank filter
	QCheckBox *hide_guests_checkbox;  // Filter to hide guest accounts (guestXXXX)
 bool is_guest_mode;
 bool is_fallback_mode;
 bool newrating_enabled;
 QTextEdit *output_console = nullptr; // Reference to main console for debug output
 QList<PlayerStatsDialog*> open_dialogs; // Track open player stats dialogs
 QSet<QString> open_player_names; // Track player names with open dialogs (for delegate highlighting)
 QTimer *refresh_timer; // Auto-refresh timer for players list
 QString local_username; // Feature 33: Username of logged-in player

public:
 FixedPlayersWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
 setWindowTitle("Players Online");
 setMinimumSize(900, 600);
 is_guest_mode = false;
 is_fallback_mode = false;
 newrating_enabled = false;
 setupUI();

 // Restore filter preferences from settings
 bool saved_open_filter = settings->readBoolEntry("players_filter_open", false);
 bool saved_hide_guests = settings->readBoolEntry("players_filter_hide_guests", false);
 QString saved_rank_from = settings->readEntry("players_filter_rank_from", "BC");
 QString saved_rank_to = settings->readEntry("players_filter_rank_to", "9p");

 qDebug() << "[PLAYERS] Loading filter preferences - open:" << saved_open_filter
          << "hide_guests:" << saved_hide_guests
          << "rank:" << saved_rank_from << "to" << saved_rank_to;

 // Apply saved filter states (will trigger preference saving again, but that's harmless)
 open_filter_checkbox->setChecked(saved_open_filter);
 hide_guests_checkbox->setChecked(saved_hide_guests);
 from_rank_combo->setCurrentText(saved_rank_from);
 to_rank_combo->setCurrentText(saved_rank_to);

 // Restore window geometry from settings (xgospel1 .Xdefaults style)
 QRect savedGeometry = settings->loadWindowGeometry("players", QRect(200, 100, 900, 600));
 qDebug() << "[PLAYERS] Loading geometry:" << savedGeometry;
 setGeometry(savedGeometry);

 // Setup auto-refresh timer for players list
 refresh_timer = new QTimer(this);
 connect(refresh_timer, &QTimer::timeout, this, &FixedPlayersWindow::refreshRequested);
 int refresh_interval = settings->getPlayersWindowRefreshInterval();
 if (refresh_interval > 0) {
 refresh_timer->start(refresh_interval * 1000);  // Convert seconds to milliseconds
 qDebug() << "[PLAYERS] Auto-refresh enabled, interval:" << refresh_interval << "seconds";
 } else {
 qDebug() << "[PLAYERS] Auto-refresh disabled (interval = 0)";
 }
 }

 void closeEvent(QCloseEvent *event) override {
 // Save players window geometry before closing (xgospel1 style)
 QRect currentGeometry = geometry();
 qDebug() << "[PLAYERS] Saving geometry:" << currentGeometry;
 settings->saveWindowGeometry("players", currentGeometry);

 // Save column widths
 QList<int> columnWidths;
 for (int i = 0; i < players_model->columnCount(); i++) {
 columnWidths.append(players_table->columnWidth(i));
 }
 settings->saveSplitterSizes("players_columns", columnWidths);
 qDebug() << "[PLAYERS] Saving column widths:" << columnWidths;

 settings->save();
 QMainWindow::closeEvent(event);
 }

 void setupUI() {
 QWidget *central = new QWidget;
 setCentralWidget(central);
 // Removed hardcoded colors - inherit from KDE theme

 QVBoxLayout *layout = new QVBoxLayout(central);
 layout->setSpacing(10);
 layout->setMargin(10);
 
 // Refresh button with xgospel1-style colors (black on darker gold #edd20d)
 QPushButton *refresh_btn = new QPushButton("Refresh Players");
 refresh_btn->setMinimumWidth(180);
 refresh_btn->setStyleSheet(
 "QPushButton {"
 " background-color: #edd20d;"
 " color: black;"
 " border: 4px outset #f5e030;"
 " border-top-color: #fffacd;"
 " border-left-color: #fffacd;"
 " border-right-color: #b8960a;"
 " border-bottom-color: #b8960a;"
 " padding: 10px 15px;"
 " font-weight: bold;"
 " font-size: 13px;"
 "}"
 "QPushButton:pressed {"
 " border: 4px inset #d4b909;"
 " border-top-color: #b8960a;"
 " border-left-color: #b8960a;"
 " border-right-color: #fffacd;"
 " border-bottom-color: #fffacd;"
 " background-color: #d4b909;"
 "}"
 "QPushButton:hover {"
 " background-color: #f5e030;"
 "}"
 );
 connect(refresh_btn, &QPushButton::clicked, this, &FixedPlayersWindow::refreshRequested);
 layout->addWidget(refresh_btn);

 // Players table with high contrast
 QFrame *players_frame = new QFrame;
 players_frame->setFrameStyle(QFrame::Sunken | QFrame::Panel);
 players_frame->setLineWidth(4);
 players_frame->setStyleSheet(
 "QFrame {" " border: 4px inset #808080;" "}"
 );
 
 // Create the table view and models
 players_table = new QTreeView;
 players_model = new QStandardItemModel(0, 12, this);
 proxy_model = new FixedRankSortProxyModel(this);
 proxy_model->setSourceModel(players_model);
 players_table->setModel(proxy_model);
 
 // Configure table with high contrast
 players_table->setSortingEnabled(true);
 players_table->header()->setSectionsMovable(false);
 players_table->header()->setVisible(true);
 players_table->setItemsExpandable(false);
 
 // Set initial sort by rank column (column 2) in ascending order of sort keys (strongest first)
 // Note: Sort keys are pre-reversed, so ascending sort order gives descending rank order
 players_table->sortByColumn(2, Qt::AscendingOrder);
 players_table->setRootIsDecorated(false);
 players_table->setUniformRowHeights(true);
 players_table->setAlternatingRowColors(false);  // v50_R2: Disabled - 3D button frames provide visual separation
 players_table->setSelectionBehavior(QAbstractItemView::SelectRows);
 players_table->setStyleSheet(
 "QTreeView {" " " " alternate-background-" " gridline-" " selection-background-" " selection-" " font-size: 11px;" "}" "QHeaderView::section {" " background-" " " " border: 1px solid #999;" " padding: 5px;" " font-weight: bold;" "}"
 );

 // Set up custom delegate for 3D button-style rendering and player highlighting (v50_R2)
 player_delegate = new PlayerDelegate(proxy_model, &open_player_names, settings->getUseFocusColors(), this);
 players_table->setItemDelegate(player_delegate);

 // Enable mouse tracking for hover effects
 players_table->setMouseTracking(true);
 players_table->viewport()->setMouseTracking(true);

 // v50_R2: Connect single-click for player info dialog
 connect(player_delegate, &PlayerDelegate::clicked, this, &FixedPlayersWindow::onPlayerDoubleClicked);

 // Enable context menu for right-click match requests
 players_table->setContextMenuPolicy(Qt::CustomContextMenu);
 connect(players_table, &QTreeView::customContextMenuRequested,
 this, &FixedPlayersWindow::onContextMenuRequested);

 // Keep double-click as fallback for compatibility
 connect(players_table, &QTreeView::doubleClicked,
 this, &FixedPlayersWindow::onPlayerDoubleClicked);

 // Bug 35: Reapply filters when user sorts by clicking column headers
 connect(players_table->header(), &QHeaderView::sortIndicatorChanged,
 this, [this](int logicalIndex, Qt::SortOrder order) {
 Q_UNUSED(logicalIndex);
 Q_UNUSED(order);
 // Reapply all filters after sorting (unified to avoid conflicts)
 this->applyAllFilters();
 });

 setupHeaders();
 
 QVBoxLayout *players_layout = new QVBoxLayout(players_frame);
 players_layout->setMargin(6);
 players_layout->addWidget(players_table);
 layout->addWidget(players_frame);

 // "Open" filter checkbox (q5Go style)
 open_filter_checkbox = new QCheckBox("open");
 open_filter_checkbox->setChecked(false);  // Will be overridden by saved preference
 connect(open_filter_checkbox, &QCheckBox::toggled, this, [this](bool checked) {
 settings->writeBoolEntry("players_filter_open", checked);
 this->applyAllFilters();
 });

		// Hide Guests filter checkbox
		hide_guests_checkbox = new QCheckBox("Hide Guests");
		hide_guests_checkbox->setChecked(false);  // Will be overridden by saved preference
		connect(hide_guests_checkbox, &QCheckBox::toggled, this, [this](bool checked) {
			settings->writeBoolEntry("players_filter_hide_guests", checked);
			this->applyAllFilters();
		});

 // Bug 35: Rank range filter (q5Go style)
 from_rank_combo = new QComboBox();
 to_rank_combo = new QComboBox();

 // Populate rank lists (strongest to weakest)
 QStringList ranks;
 ranks << "9p" << "8p" << "7p" << "6p" << "5p" << "4p" << "3p" << "2p" << "1p"  // Pro
       << "10d" << "9d" << "8d" << "7d" << "6d" << "5d" << "4d" << "3d" << "2d" << "1d"  // Dan
       << "1k" << "2k" << "3k" << "4k" << "5k" << "6k" << "7k" << "8k" << "9k" << "10k"  // Kyu
       << "11k" << "12k" << "13k" << "14k" << "15k" << "16k" << "17k" << "18k" << "19k" << "20k"
       << "21k" << "22k" << "23k" << "24k" << "25k" << "26k" << "27k" << "28k" << "29k" << "30k"
       << "BC";  // Beginner Class

 from_rank_combo->addItems(ranks);
 to_rank_combo->addItems(ranks);

 // Default: BC to 9p (shows all players from beginners to professionals)
 from_rank_combo->setCurrentText("BC");
 to_rank_combo->setCurrentText("9p");

 connect(from_rank_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
         this, [this]() {
         settings->writeEntry("players_filter_rank_from", from_rank_combo->currentText());
         this->applyAllFilters();
         });
 connect(to_rank_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
         this, [this]() {
         settings->writeEntry("players_filter_rank_to", to_rank_combo->currentText());
         this->applyAllFilters();
         });

 QHBoxLayout *filter_layout = new QHBoxLayout();
 filter_layout->addWidget(new QLabel("From:"));
 filter_layout->addWidget(from_rank_combo);
 filter_layout->addWidget(new QLabel("To:"));
 filter_layout->addWidget(to_rank_combo);
 filter_layout->addStretch();
 filter_layout->addWidget(open_filter_checkbox);
		filter_layout->addWidget(hide_guests_checkbox);
 layout->addLayout(filter_layout);
 }

 void setupHeaders() {
 QStringList headers;
 headers << "Stat" << "Name" << "Rk" << "pl" << "ob" << "Idle"
 << "X" << "Info" << "Won" << "Lost" << "Rated" << "Country" << "Match prefs";
 players_model->setHorizontalHeaderLabels(headers);
 }
 
 void addPlayerFromRawLine(const QString& line) {
 // Handle both formats based on what command was sent
 Q5GoParser parser;

 // Check if this is "who" format with multiple players per line
 if (line.contains("|")) {
 // Parse multiple players from "who" format
 QStringList sections = line.split("|");
 for (const QString& section : sections) {
 QString trimmedSection = section.trimmed();
 if (trimmedSection.isEmpty()) continue;

 Q5GoPlayer player;
 if (parseWhoSection(trimmedSection, player)) {
 addPlayerToTable(player);
 } else {
 qDebug() << "[PLAYER PARSE FAIL - WHO] Failed to parse:" << trimmedSection;
 }
 }
 } else {
 // For single-line entries, parse based on account type
 Q5GoPlayer player;
 bool parsed = false;

 if (is_guest_mode || is_fallback_mode) {
 // Guest accounts or fallback mode: try WHO format first
 if (parseWhoSection(line, player)) {
 addPlayerToTable(player);
 parsed = true;
 } else {
 // Fallback to userlist format parsing
 if (parser.parsePlayerLine(line, player)) {
 addPlayerToTable(player);
 parsed = true;
 }
 }
 } else {
 // Registered accounts: try userlist format first
 if (parser.parsePlayerLine(line, player)) {
 addPlayerToTable(player);
 parsed = true;
 } else {
 // Fallback to WHO format parsing
 if (parseWhoSection(line, player)) {
 addPlayerToTable(player);
 parsed = true;
 }
 }
 }

 if (!parsed) {
 qDebug() << "[PLAYER PARSE FAIL] Failed to parse line:" << line;
 }
 }
 }
 
 bool parseWhoSection(const QString& section, Q5GoPlayer& player) {
 // Use standard q5Go WHO format parser
 // IGS WHO format: [flags] [name] [idle] [rank]
 // Example: "X -- -- thirdstone 0s BC" or "-- 27 Prima 0s 13k*"
 // Sections from pipe-split don't have "< 27" prefix

 Q5GoParser parser;
 return parser.parseWhoFormatLine(section, player);
 }
 
 void setNewratingEnabled(bool enabled) {
 qDebug() << "NEWRATING STATE CHANGE: Players window newrating_enabled =" << enabled;
 newrating_enabled = enabled;
 }
 
 void addPlayerToTable(const Q5GoPlayer& player) {
 if (player.name.isEmpty() || player.rank.isEmpty()) return;

 // Dedup guard: scan for existing row with same name (case-insensitive).
 // The userlist_player_names set in the main window is the primary dedup
 // mechanism, but it relies on the parser succeeding for every row. If a
 // line fails the q5Go regex (unusual fields, wide chars, etc.) the name
 // is never inserted into the set and the WHO supplement adds a duplicate.
 // This catch-all at the model level ensures duplicates never reach the UI.
 int rows = players_model->rowCount();
 for (int r = 0; r < rows; ++r) {
     QStandardItem *ni = players_model->item(r, 1); // Name column
     if (ni && ni->text().compare(player.name, Qt::CaseInsensitive) == 0) {
         qDebug() << "[DEDUP] Skipping duplicate player in model:" << player.name;
         return;
     }
 }
 
 QList<QStandardItem*> row;
 
 // Create rank item with sort key
 // Handle special rank display rules:
 // - Pro ranks (1p-9p): Never show +, no half-ranks exist
 // - 10d: Special highest amateur rank, never show +
 // - Other dan/kyu: Convert * to + only for actual half-ranks
 QString display_rank = player.rank;
 QString lower_rank = display_rank.toLower();
 
 if (lower_rank.contains('p')) {
 // Pro ranks: remove any * or + (pros don't have half-ranks)
 display_rank.remove('*').remove('+');
 } else if (lower_rank.startsWith("10d")) {
 // 10d is special: highest amateur rank, no 10d+ exists
 display_rank.remove('*').remove('+');
 display_rank = "10d";
 } else if (lower_rank.contains('d') || lower_rank.contains('k')) {
 // Regular dan/kyu ranks: handle * and + based on newrating state
 if (newrating_enabled) {
 // Only convert * to + for rated players (who have *)
 if (display_rank.contains('*')) {
 display_rank.replace('*', '+');
 }
 // Leave unrated players (plain "4d") unchanged
 } else {
 // Convert + back to * when newrating is disabled
 if (display_rank.contains('+')) {
 display_rank.replace('+', '*');
 }
 // Leave unrated players (plain "4d") unchanged
 }
 }
 
 QStandardItem *rank_item = new QStandardItem(display_rank);
 QString sort_key = createRankSortKey(player.rank);
 rank_item->setData(sort_key, Qt::UserRole);
 
 // Debug output for rank sorting and newrating state
 // Debug removed to reduce console flooding
 
 row << new QStandardItem(player.info) // Stat (status flags like Q, X)
 << new QStandardItem(player.name) // Name
 << rank_item // Rk (with sort key)
 << new QStandardItem(player.play_str) // pl
 << new QStandardItem(player.obs_str) // ob
 << new QStandardItem(player.idle) // Idle
 << new QStandardItem(player.mark) // X (mark - shows "X" for declining matches)
 << new QStandardItem(player.extInfo) // Info (descriptive text like <None>)
 << new QStandardItem(player.won) // Won
 << new QStandardItem(player.lost) // Lost
 << new QStandardItem(player.rated) // Rated (rated games W-L)
 << new QStandardItem(player.country) // Country
 << new QStandardItem(player.nmatch_settings); // Match prefs

 players_model->appendRow(row);
 }
 
 QString createRankSortKey(const QString& rank) {
 // Create sort key: pro="a"+reverse, dan="b"+reverse, kyu="c"+normal
 // Special cases:
 // - Pro ranks (1p-9p): No half-ranks exist
 // - 10d: Highest amateur rank, no 10d+ exists 
 // - Other dan/kyu: + indicates half-rank, * indicates evaluated rank
 QString trimmed = rank.trimmed().toLower();
 
 if (trimmed.contains('p')) {
 int num = trimmed.left(trimmed.indexOf('p')).toInt();
 // Pro ranks: higher number = stronger, so reverse for ascending sort
 // Pro ranks don't have half-ranks (ignore + or *)
 double rankValue = num;
 return QString("a%1").arg((int)((100.0 - rankValue) * 10), 4, 10, QChar('0'));
 } else if (trimmed.contains('d')) {
 int num = trimmed.left(trimmed.indexOf('d')).toInt();
 // Dan ranks: higher number = stronger, so reverse for ascending sort
 bool hasHalf = false;
 if (num < 10) {
 // Only ranks below 10d can have half-ranks
 hasHalf = trimmed.contains('+');
 }
 // 10d is special: highest amateur rank, no 10d+ exists
 double rankValue = num + (hasHalf ? 0.5 : 0.0);
 return QString("b%1").arg((int)((100.0 - rankValue) * 10), 4, 10, QChar('0'));
 } else if (trimmed.contains('k')) {
 int num = trimmed.left(trimmed.indexOf('k')).toInt();
 // Kyu ranks: lower number = stronger, so normal order for ascending sort
 bool hasHalf = trimmed.contains('+'); // + indicates half-rank
 double rankValue = num + (hasHalf ? -0.5 : 0.0); // + means stronger (lower effective kyu)
 return QString("c%1").arg((int)(rankValue * 10), 4, 10, QChar('0'));
 }
 return "d9999"; // Unranked at bottom
 }
 
 void clearPlayers() {
 // Bug 35: Disable sorting during population for better performance
 players_table->setSortingEnabled(false);

 players_model->clear();
 setupHeaders();

 // Restore column widths after clearing (otherwise they reset to defaults)
 QList<int> savedWidths = settings->loadSplitterSizes("players_columns");
 if (!savedWidths.isEmpty() && savedWidths.size() == 13) {  // 13 columns in players table
 for (int i = 0; i < savedWidths.size(); i++) {
 players_table->setColumnWidth(i, savedWidths[i]);
 }
 qDebug() << "[PLAYERS] Restored column widths after clear:" << savedWidths;
 }
 }
 
 void resizeColumns() {
 // Try to load saved column widths first
 QList<int> savedWidths = settings->loadSplitterSizes("players_columns");

 if (!savedWidths.isEmpty() && savedWidths.size() == players_model->columnCount()) {
 // Restore saved column widths
 for (int i = 0; i < savedWidths.size(); i++) {
 players_table->setColumnWidth(i, savedWidths[i]);
 }
 qDebug() << "[PLAYERS] Restored column widths:" << savedWidths;
 } else {
 // No saved widths, auto-resize to contents
 for (int i = 0; i < players_model->columnCount(); i++) {
 players_table->resizeColumnToContents(i);
 }
 }
 }

 void sortByRank() {
 // Bug 35: Re-enable sorting and apply default rank sort
 players_table->setSortingEnabled(true);
 // Apply default rank sort (column 2) - ascending sort of pre-reversed keys = descending rank
 players_table->sortByColumn(2, Qt::AscendingOrder);
 // Force the proxy model to re-sort immediately
 proxy_model->sort(2, Qt::AscendingOrder);
 // Process events to ensure proxy model completes sorting before filters are applied
 QCoreApplication::processEvents();
 qDebug() << "[PLAYERS] Sorted by rank column 2 in ascending order (strongest first)";
 }

 void reapplyOpenFilter() {
 // Reapply the open filter after refresh to restore formatting
 // This is needed because clearPlayers() wipes all formatting
 if (open_filter_checkbox) {
 bool filter_state = open_filter_checkbox->isChecked();
 qDebug() << "[REAPPLY-FILTER] reapplyOpenFilter() called - checkbox state:" << filter_state;
 qDebug() << "[REAPPLY-FILTER] Players model row count:" << (players_model ? players_model->rowCount() : -1);
 qDebug() << "[REAPPLY-FILTER] Sorting enabled:" << (players_table ? players_table->isSortingEnabled() : false);
 applyOpenFilter(filter_state);
 qDebug() << "[REAPPLY-FILTER] applyOpenFilter() completed";
 }
 }

 void reapplyRankRangeFilter() {
 // Bug 35: Reapply the rank range filter after refresh
 // This is needed because clearPlayers() wipes all row visibility
 if (from_rank_combo && to_rank_combo) {
 applyRankRangeFilter();
 }
 }

 void setGuestMode(bool is_guest) {
 is_guest_mode = is_guest;
 }

 void setFallbackMode(bool is_fallback) {
 is_fallback_mode = is_fallback;
 }


	QString getRankForPlayer(const QString &player_name) const {
		if (!players_model) return "?";
		for (int row = 0; row < players_model->rowCount(); ++row) {
			QStandardItem *name_item = players_model->item(row, 1);
			if (name_item && name_item->text().compare(player_name, Qt::CaseInsensitive) == 0) {
				QStandardItem *rank_item = players_model->item(row, 2);
				if (rank_item && !rank_item->text().isEmpty())
					return rank_item->text();
			}
		}
		return "?";
	}

	void upsertPlayerRank(const QString &player_name, const QString &rank) {
		if (!players_model || player_name.isEmpty() || rank.isEmpty()) return;
		for (int row = 0; row < players_model->rowCount(); ++row) {
			QStandardItem *name_item = players_model->item(row, 1);
			if (name_item && name_item->text().compare(player_name, Qt::CaseInsensitive) == 0) {
				QStandardItem *rank_item = players_model->item(row, 2);
				if (rank_item) rank_item->setText(rank);
				return;
			}
		}
		// Player not in model — insert a stub row (col 1=name, col 2=rank, rest empty)
		QList<QStandardItem*> row;
		for (int i = 0; i < players_model->columnCount(); ++i)
			row << new QStandardItem();
		row[1]->setText(player_name);
		row[2]->setText(rank);
		players_model->appendRow(row);
	}

	void updatePlayerGameStatus(const QString& player_name, const QString& game_nr, const QString& observing = QString()) {
		if (!players_model) return;
		qDebug() << "[PLAYER-STATUS] Updating" << player_name << "game:" << game_nr;
		for (int row = 0; row < players_model->rowCount(); ++row) {
			QStandardItem *name_item = players_model->item(row, 1);
			if (!name_item) continue;
			if (name_item->text() == player_name) {
				QStandardItem *pl_item = players_model->item(row, 3);
				if (pl_item) {
					pl_item->setText(game_nr.isEmpty() ? "--" : game_nr);
					qDebug() << "[PLAYER-STATUS] Updated" << player_name << "pl to:" << (game_nr.isEmpty() ? "--" : game_nr);
				}
				if (!observing.isEmpty()) {
					QStandardItem *ob_item = players_model->item(row, 4);
					if (ob_item) ob_item->setText(observing);
				}
				return;
			}
		}
		qDebug() << "[PLAYER-STATUS] Player" << player_name << "not found";
	}

	void resetAllPlayerGameStatuses() {
		if (!players_model) return;
		qDebug() << "[PLAYER-STATUS] Resetting PLAYING statuses to '--' (preserving observing data from WHO/userlist)";
		for (int row = 0; row < players_model->rowCount(); ++row) {
			QStandardItem *pl_item = players_model->item(row, 3);
			if (pl_item) pl_item->setText("--");
			// DON'T reset observing column (column 4) - it comes from WHO/userlist command
			// and already contains correct observing data for all players
		}
	}

	void applyHideGuestsFilter() {
		if (!players_model || !proxy_model) return;
		bool hide_guests = hide_guests_checkbox->isChecked();
		if (!players_table->isSortingEnabled()) {
			qDebug() << "[GUEST-FILTER] Skipping - sorting disabled";
			return;
		}
		qDebug() << "[GUEST-FILTER] Hide guests:" << hide_guests;
		int hidden_count = 0, visible_count = 0;
		for (int row = 0; row < players_model->rowCount(); ++row) {
			QStandardItem *name_item = players_model->item(row, 1);
			if (!name_item) continue;
			QString name = name_item->text();
			bool is_guest = name.startsWith("guest", Qt::CaseInsensitive);
			bool should_hide = hide_guests && is_guest;
			QModelIndex source_index = players_model->index(row, 0);
			QModelIndex proxy_index = proxy_model->mapFromSource(source_index);
			if (proxy_index.isValid()) {
				players_table->setRowHidden(proxy_index.row(), proxy_index.parent(), should_hide);
				if (should_hide) hidden_count++; else visible_count++;
			}
		}
		qDebug() << "[GUEST-FILTER] Visible:" << visible_count << "Hidden:" << hidden_count;
	}

	void reapplyHideGuestsFilter() {
		if (hide_guests_checkbox) applyHideGuestsFilter();
	}

	void applyAllFilters() {
		// Apply ALL three filters in a single pass to avoid conflicts
		if (!players_model || !proxy_model) return;
		if (!players_table->isSortingEnabled()) return;

		bool open_checked = open_filter_checkbox ? open_filter_checkbox->isChecked() : false;
		bool hide_guests = hide_guests_checkbox ? hide_guests_checkbox->isChecked() : false;
		QString from_rank = from_rank_combo ? from_rank_combo->currentText() : "BC";
		QString to_rank = to_rank_combo ? to_rank_combo->currentText() : "9p";

		int from_value = getRankNumericValue(from_rank);
		int to_value = getRankNumericValue(to_rank);
		if (from_value < to_value) qSwap(from_value, to_value);

		qDebug() << "[ALL-FILTERS] Applying unified filter - open:" << open_checked << "hide_guests:" << hide_guests << "rank:" << from_rank << "to" << to_rank;

		int visible = 0, hidden = 0;

		for (int row = 0; row < players_model->rowCount(); ++row) {
			QStandardItem *stat_item = players_model->item(row, 0);
			QStandardItem *name_item = players_model->item(row, 1);
			QStandardItem *rank_item = players_model->item(row, 2);
			QStandardItem *pl_item = players_model->item(row, 3);

			if (!stat_item || !name_item || !rank_item || !pl_item) continue;

			QString name = name_item->text();
			QString rank = rank_item->text();
			QString pl_value = pl_item->text();
			QString stat_value = stat_item->text();

			bool is_playing = !pl_value.contains('-');
			bool has_x_mark = stat_value.contains('X');
			bool is_guest = name.startsWith("guest", Qt::CaseInsensitive);
			int rank_value = getRankNumericValue(rank);
			bool in_rank_range = (rank_value <= from_value && rank_value >= to_value);

			// Combine all filter conditions
			bool should_hide = false;

			// Open filter: hide if playing OR has X mark
			if (open_checked && (is_playing || has_x_mark)) {
				should_hide = true;
			}

			// Guest filter: hide if guest
			if (hide_guests && is_guest) {
				should_hide = true;
			}

			// Rank filter: hide if outside range
			if (!in_rank_range) {
				should_hide = true;
			}

			QModelIndex source_index = players_model->index(row, 0);
			QModelIndex proxy_index = proxy_model->mapFromSource(source_index);

			if (proxy_index.isValid()) {
				players_table->setRowHidden(proxy_index.row(), proxy_index.parent(), should_hide);
				if (should_hide) hidden++; else visible++;

				// Apply grey formatting to X-marked players that are visible
				if (has_x_mark && !should_hide) {
					for (int col = 0; col < players_model->columnCount(); ++col) {
						QStandardItem *item = players_model->item(row, col);
						if (item) item->setForeground(QBrush(QColor(170, 170, 170)));
					}
				} else if (!has_x_mark && !should_hide) {
					// Restore normal color for non-X players
					for (int col = 0; col < players_model->columnCount(); ++col) {
						QStandardItem *item = players_model->item(row, col);
						if (item) item->setForeground(QBrush(QColor(255, 255, 255)));
					}
				}
			}
		}

		qDebug() << "[ALL-FILTERS] Complete - Visible:" << visible << "Hidden:" << hidden << "Total:" << players_model->rowCount();
	}

protected:
 void mouseDoubleClickEvent(QMouseEvent *event) override {
 qDebug() << "[MOUSE] MOUSE DOUBLE CLICK: Event detected on players window";
 Q_UNUSED(event)
 QModelIndex index = players_table->indexAt(players_table->mapFromGlobal(QCursor::pos()));
 if (index.isValid()) {
 QString name = players_model->item(index.row(), 1)->text();
 QString rank = players_model->item(index.row(), 2)->text();
 qDebug() << "[INFO] PLAYER SELECTED:" << name << "rank:" << rank;
 emit matchRequested(name, rank);
 } else {
 qDebug() << "[ERROR] INVALID INDEX: Double click not on valid table item";
 }
 QMainWindow::mouseDoubleClickEvent(event);
 }

private slots:
 void onContextMenuRequested(const QPoint &pos) {
 QModelIndex index = players_table->indexAt(pos);
 if (!index.isValid()) return;

 // Get the source index (proxy model mapping)
 QModelIndex sourceIndex = proxy_model->mapToSource(index);
 if (!sourceIndex.isValid()) return;

 // Extract all player data from table
 QString playerName = players_model->item(sourceIndex.row(), 1)->text();
 QString playerRank = players_model->item(sourceIndex.row(), 2)->text();
 QString stat = players_model->item(sourceIndex.row(), 0)->text();
 QString playing = players_model->item(sourceIndex.row(), 3)->text();
 QString observing = players_model->item(sourceIndex.row(), 4)->text();
 QString idle = players_model->item(sourceIndex.row(), 5)->text();
 QString x_flag = players_model->item(sourceIndex.row(), 6)->text();
 QString info = players_model->item(sourceIndex.row(), 7)->text();
 QString wonText = players_model->item(sourceIndex.row(), 8)->text();
 QString lostText = players_model->item(sourceIndex.row(), 9)->text();
 QString rated = players_model->item(sourceIndex.row(), 10)->text();
 QString country = players_model->item(sourceIndex.row(), 11)->text();
 QString match_prefs = players_model->item(sourceIndex.row(), 12)->text();

 int wins = wonText.toInt();
 int losses = lostText.toInt();

 qDebug() << "[MOUSE] RIGHT CLICK: Context menu requested for player:" << playerName << "rank:" << playerRank;

 // Create context menu
 QMenu *contextMenu = new QMenu(this);
 contextMenu->setStyleSheet(
 "QMenu {" " " " border: 2px solid #999;" "}" "QMenu::item {" " padding: 8px 16px;" " background-color: transparent;" "}" "QMenu::item:selected {" " background-" " " "}"
 );

 QAction *statsAction = contextMenu->addAction(QString("Show Stats/Tell Dialog for %1").arg(playerName));
 QAction *matchAction = contextMenu->addAction(QString("Request Match with %1").arg(playerName));

 // Execute context menu at the clicked position
 QAction *selectedAction = contextMenu->exec(players_table->mapToGlobal(pos));

 if (selectedAction == statsAction) {
 qDebug() << "[STATS] CONTEXT MENU: Opening stats dialog for" << playerName;

 // Open the same dialog as double-click with all player data
 // Note: Don't pass extInfo (player list info like "<None>") as the info parameter
 // The real Info field comes from the stats command and will be set via updateStatsData
 // Feature 33: Check if this is the local player
 bool is_local = (playerName.compare(local_username, Qt::CaseInsensitive) == 0);
 PlayerStatsDialog *dialog = new PlayerStatsDialog(this, playerName, playerRank, country, wins, losses,
 stat, playing, observing, idle, "", rated, match_prefs, is_local);

 // Connect the dialog signals
 connect(dialog, &PlayerStatsDialog::tellRequested, this, [this, dialog](const QString &player, const QString &msg) {
 qDebug() << "[TELL] TELL REQUESTED: player=" << player << "msg=" << msg;
 if (output_console) {
 }
 emit tellRequested(player, msg);
 });

 connect(dialog, &PlayerStatsDialog::matchRequested, this, [this, dialog](const QString &player, bool is_nmatch) {
 qDebug() << "[GAME] MATCH REQUESTED: player=" << player << "is_nmatch=" << is_nmatch;
 QString player_rank = dialog->getPlayerRank();
 if (output_console) {
 if (is_nmatch) {
 } else {
 }
 }
 // Forward to main window's match request handler
 emit matchRequested(player, player_rank);
 });

 // Feature 33: Connect toggle signal
 connect(dialog, &PlayerStatsDialog::toggleRequested, this, &FixedPlayersWindow::toggleCommandRequested);

 dialog->setAttribute(Qt::WA_DeleteOnClose);
 open_dialogs.append(dialog);

 // Track player name for highlighting (v50_R2)
 open_player_names.insert(playerName);
 players_table->viewport()->repaint();  // Trigger immediate highlight update

 connect(dialog, &QDialog::destroyed, this, [this, dialog]() {
 open_dialogs.removeAll(dialog);
 // Remove player from highlighting set (v50_R2)
 QString player_name = dialog->getPlayerName();
 open_player_names.remove(player_name);
 players_table->viewport()->repaint();  // Trigger immediate unhighlight
 });

 dialog->show();

 } else if (selectedAction == matchAction) {
 qDebug() << "[MENU] CONTEXT MENU: Match request selected for" << playerName;
 emit matchRequested(playerName, playerRank);
 }

 contextMenu->deleteLater();
 }

 void onPlayerDoubleClicked(const QModelIndex &index) {
 if (!index.isValid()) return;

 // Get the source index (proxy model mapping)
 QModelIndex sourceIndex = proxy_model->mapToSource(index);
 if (!sourceIndex.isValid()) return;

 // Extract all player info from the table
 QString playerName = players_model->item(sourceIndex.row(), 1)->text();
 QString playerRank = players_model->item(sourceIndex.row(), 2)->text();
 QString stat = players_model->item(sourceIndex.row(), 0)->text();
 QString playing = players_model->item(sourceIndex.row(), 3)->text();
 QString observing = players_model->item(sourceIndex.row(), 4)->text();
 QString idle = players_model->item(sourceIndex.row(), 5)->text();
 QString x_flag = players_model->item(sourceIndex.row(), 6)->text();
 QString info = players_model->item(sourceIndex.row(), 7)->text();
 QString wonText = players_model->item(sourceIndex.row(), 8)->text();
 QString lostText = players_model->item(sourceIndex.row(), 9)->text();
 QString rated = players_model->item(sourceIndex.row(), 10)->text();
 QString country = players_model->item(sourceIndex.row(), 11)->text();
 QString match_prefs = players_model->item(sourceIndex.row(), 12)->text();

 int wins = wonText.toInt();
 int losses = lostText.toInt();

 qDebug() << "[USER] DOUBLE CLICK: Opening stats dialog for" << playerName << playerRank;

 // Create and show the player stats dialog with all data
 // Note: Don't pass extInfo (player list info like "<None>") as the info parameter
 // The real Info field comes from the stats command and will be set via updateStatsData
 // Use nullptr as parent to make dialog a top-level window (better cross-desktop behavior)
        // Feature 33: Check if this is the local player
        bool is_local = (playerName.compare(local_username, Qt::CaseInsensitive) == 0);

 PlayerStatsDialog *dialog = new PlayerStatsDialog(nullptr, playerName, playerRank, country, wins, losses,
 stat, playing, observing, idle, "", rated, match_prefs, is_local);

 // Connect the dialog signals to main window slots
 connect(dialog, &PlayerStatsDialog::tellRequested, this, [this, dialog](const QString &player, const QString &msg) {
 qDebug() << "[TELL] TELL REQUESTED: player=" << player << "msg=" << msg;
 if (output_console) {
 }
 emit tellRequested(player, msg);
 });

 connect(dialog, &PlayerStatsDialog::matchRequested, this, [this, dialog](const QString &player, bool is_nmatch) {
 qDebug() << "[GAME] MATCH REQUESTED: player=" << player << "is_nmatch=" << is_nmatch;
 QString player_rank = dialog->getPlayerRank();
 if (output_console) {
 if (is_nmatch) {
 } else {
 }
 }
 // Forward to main window's match request handler
 emit matchRequested(player, player_rank);
 });

 connect(dialog, &PlayerStatsDialog::statsRequested, this, &FixedPlayersWindow::statsRequested);

        // Feature 33: Connect toggle signal
        connect(dialog, &PlayerStatsDialog::toggleRequested, this, &FixedPlayersWindow::toggleCommandRequested);

 // Make dialog modeless (non-blocking) and auto-delete when closed
 dialog->setAttribute(Qt::WA_DeleteOnClose);

 // Track this dialog so we can broadcast tells to it
 open_dialogs.append(dialog);

 // Track player name for highlighting (v50_R2)
 open_player_names.insert(playerName);
 players_table->viewport()->repaint();  // Trigger immediate highlight update

 // Clean up the dialog from the list when it closes
 connect(dialog, &QDialog::destroyed, this, [this, dialog]() {
 open_dialogs.removeAll(dialog);
 // Remove player from highlighting set (v50_R2)
 QString player_name = dialog->getPlayerName();
 open_player_names.remove(player_name);
 players_table->viewport()->repaint();  // Trigger immediate unhighlight
 });

 dialog->show();
 }

public:
 void setOutputConsole(QTextEdit *console) {
 output_console = console;
 }

 // Feature 33: Set local username for toggle buttons
 void setLocalUsername(const QString &username) {
 local_username = username;
 qDebug() << "[PLAYERS] Local username set to:" << username;
 }

 // Get the list of open player dialogs
 const QList<PlayerStatsDialog*>& getOpenDialogs() const {
 return open_dialogs;
 }

 // Get the local player's username
 QString getLocalUsername() const {
 return local_username;
 }

 // Open a player stats dialog for a given player name
 // Called when a tell is received and dialog doesn't already exist
 void openStatsDialogForPlayer(const QString &playerName) {
 // First check if dialog already exists for this player
 for (PlayerStatsDialog* dialog : open_dialogs) {
 if (dialog && dialog->getPlayerName().compare(playerName, Qt::CaseInsensitive) == 0) {
 // Dialog already exists, just bring it to front
 dialog->raise();
 dialog->activateWindow();
 qDebug() << "[INFO] Stats dialog already open for" << playerName;
 return;
 }
 }

 // Find the player in the players list to get all their stats
 QString playerRank = "NR";
 QString stat = "";
 QString playing = "";
 QString observing = "";
 QString idle = "";
 QString x_flag = "";
 QString info = "";
 QString rated = "";
 QString country = "";
 QString match_prefs = "";
 int wins = 0;
 int losses = 0;

 for (int row = 0; row < players_model->rowCount(); row++) {
 QString name = players_model->item(row, 1)->text();
 if (name.compare(playerName, Qt::CaseInsensitive) == 0) {
 // Found the player, extract all their data
 stat = players_model->item(row, 0)->text();
 playerRank = players_model->item(row, 2)->text();
 playing = players_model->item(row, 3)->text();
 observing = players_model->item(row, 4)->text();
 idle = players_model->item(row, 5)->text();
 x_flag = players_model->item(row, 6)->text();
 info = players_model->item(row, 7)->text();
 QString wonText = players_model->item(row, 8)->text();
 QString lostText = players_model->item(row, 9)->text();
 rated = players_model->item(row, 10)->text();
 country = players_model->item(row, 11)->text();
 match_prefs = players_model->item(row, 12)->text();
 wins = wonText.toInt();
 losses = lostText.toInt();
 break;
 }
 }

 qDebug() << "[NOTIFY] Auto-opening stats dialog for incoming tell from" << playerName << "(" << playerRank << ")";

 // Create and show the player stats dialog with all data
 // Note: Don't pass extInfo (player list info like "<None>") as the info parameter
 // The real Info field comes from the stats command and will be set via updateStatsData
 // Use nullptr as parent to make dialog a top-level window (better cross-desktop behavior)
        // Feature 33: Check if this is the local player
        bool is_local = (playerName.compare(local_username, Qt::CaseInsensitive) == 0);

 PlayerStatsDialog *dialog = new PlayerStatsDialog(nullptr, playerName, playerRank, country, wins, losses,
 stat, playing, observing, idle, "", rated, match_prefs, is_local);

 // Connect the dialog signals to main window slots
 connect(dialog, &PlayerStatsDialog::tellRequested, this, [this, dialog](const QString &player, const QString &msg) {
 qDebug() << "[TELL] TELL REQUESTED: player=" << player << "msg=" << msg;
 if (output_console) {
 }
 emit tellRequested(player, msg);
 });

 connect(dialog, &PlayerStatsDialog::matchRequested, this, [this, dialog](const QString &player, bool is_nmatch) {
 qDebug() << "[GAME] MATCH REQUESTED: player=" << player << "is_nmatch=" << is_nmatch;
 QString player_rank = dialog->getPlayerRank();
 if (output_console) {
 if (is_nmatch) {
 } else {
 }
 }
 // Forward to main window's match request handler
 emit matchRequested(player, player_rank);
 });

 connect(dialog, &PlayerStatsDialog::statsRequested, this, &FixedPlayersWindow::statsRequested);

        // Feature 33: Connect toggle signal
        connect(dialog, &PlayerStatsDialog::toggleRequested, this, &FixedPlayersWindow::toggleCommandRequested);

 // Make dialog modeless (non-blocking) and auto-delete when closed
 dialog->setAttribute(Qt::WA_DeleteOnClose);

 // Track this dialog so we can broadcast tells to it
 open_dialogs.append(dialog);

 // Track player name for highlighting (v50_R2)
 open_player_names.insert(playerName);
 players_table->viewport()->repaint();  // Trigger immediate highlight update

 // Clean up the dialog from the list when it closes
 connect(dialog, &QDialog::destroyed, this, [this, dialog]() {
 open_dialogs.removeAll(dialog);
 // Remove player from highlighting set (v50_R2)
 QString player_name = dialog->getPlayerName();
 open_player_names.remove(player_name);
 players_table->viewport()->repaint();  // Trigger immediate unhighlight
 });

 // Show dialog with window activation hints for multi-desktop support
 dialog->show();
 dialog->raise();
 dialog->activateWindow();

 // Request attention from window manager (flashes taskbar/demands attention)
 // Use delayed alert to ensure window is fully created before requesting attention
 QTimer::singleShot(100, [dialog]() {
 if (dialog) {
 QApplication::alert(dialog, 0); // 0 = alert until user responds
 // Also set X11 urgency hint for better KDE/X11 notification
 dialog->activateWindow();
 }
 });
 }

 void broadcastIncomingTell(const QString &sender, const QString &message) {
 // Check if a dialog already exists for this sender
 bool dialogExists = false;
 PlayerStatsDialog* targetDialog = nullptr;
 for (PlayerStatsDialog* dialog : open_dialogs) {
 if (dialog && dialog->getPlayerName().compare(sender, Qt::CaseInsensitive) == 0) {
 dialogExists = true;
 targetDialog = dialog;
 break;
 }
 }

 // If no dialog exists, auto-open one for this incoming tell
 // Open regardless of whether players_model is populated — openStatsDialogForPlayer handles missing data with defaults
 if (!dialogExists) {
 qDebug() << "[NOTIFY] Dialog doesn't exist for sender:" << sender << "- auto-opening";
 openStatsDialogForPlayer(sender);
 // After opening, find the newly created dialog
 for (PlayerStatsDialog* dialog : open_dialogs) {
 if (dialog && dialog->getPlayerName().compare(sender, Qt::CaseInsensitive) == 0) {
 targetDialog = dialog;
 break;
 }
 }
 }

 // Send incoming tell to the target dialog (or all dialogs if sender matches)
 qDebug() << "[BROADCAST] broadcastIncomingTell called - sender:" << sender << "open_dialogs count:" << open_dialogs.size();
 for (PlayerStatsDialog* dialog : open_dialogs) {
 if (dialog) {
 qDebug() << " [SEND] Sending to dialog for player:" << dialog->getPlayerName();
 dialog->receiveIncomingTell(sender, message);
 }
 }
 }

 void applyOpenFilter(bool checked) {
 // q5Go logic: When "open" is checked, HIDE players who:
 // 1. Have 'X' in their info/stat field (declining matches)
 // 2. Are currently playing (play_str does NOT contain '-')

 qDebug() << "[APPLY-FILTER] applyOpenFilter() START - checked:" << checked;

 if (!players_model || !proxy_model) {
 qDebug() << "[APPLY-FILTER] ERROR: players_model or proxy_model is NULL!";
 return;
 }

 qDebug() << "[APPLY-FILTER] Model row count:" << players_model->rowCount();

 int visible_count = 0;
 int hidden_count = 0;
 int invalid_proxy_count = 0;

 for (int row = 0; row < players_model->rowCount(); ++row) {
 // Column indices: 0=Stat, 1=Name, 2=Rk, 3=pl, 4=ob, 5=Idle, 6=X
 QStandardItem *stat_item = players_model->item(row, 0); // "Stat" column (info flags)
 QStandardItem *pl_item = players_model->item(row, 3); // "pl" column (playing)
 QStandardItem *name_item = players_model->item(row, 1); // Name

 if (!pl_item || !stat_item || !name_item) continue;

 QString pl_value = pl_item->text();
 QString stat_value = stat_item->text();

 // q5Go logic: is_playing = !play_str.contains('-')
 // When NOT playing: play_str is "--" (contains '-')
 // When playing: play_str is a game number like "123" (does NOT contain '-')
 bool is_playing = !pl_value.contains('-');
 bool has_x_mark = stat_value.contains('X');

 // Get the source model index and map to proxy
 QModelIndex sourceIndex = players_model->index(row, 0);
 QModelIndex proxyIndex = proxy_model->mapFromSource(sourceIndex);

 // Check if proxy index is valid (may be invalid during/after sorting)
 if (!proxyIndex.isValid()) {
 qDebug() << "[APPLY-FILTER] WARNING: Invalid proxy index for row" << row << "player:" << (name_item ? name_item->text() : "?") << "- skipping";
 invalid_proxy_count++;
 continue;
 }

 if (checked) {
 // "open" is checked: hide players who are playing or have X mark (q5Go behavior)
 bool should_hide = is_playing || has_x_mark;

 if (should_hide) {
 hidden_count++;
 } else {
 visible_count++;
 }

 players_table->setRowHidden(proxyIndex.row(), proxyIndex.parent(), should_hide);

 // Grey out players with X mark (q5Go style - lighter grey for better readability)
 if (has_x_mark && !should_hide) {
 for (int col = 0; col < players_model->columnCount(); ++col) {
 QStandardItem *item = players_model->item(row, col);
 if (item) {
 item->setForeground(QBrush(QColor(170, 170, 170))); // Lighter grey text (q5Go style)
 }
 }
 }
 } else {
 // "open" is unchecked: show all players
 players_table->setRowHidden(proxyIndex.row(), proxyIndex.parent(), false);
 visible_count++;

 // Grey out players with X mark (q5Go style - lighter grey for better readability)
 if (has_x_mark) {
 for (int col = 0; col < players_model->columnCount(); ++col) {
 QStandardItem *item = players_model->item(row, col);
 if (item) {
 item->setForeground(QBrush(QColor(170, 170, 170))); // Lighter grey text (q5Go style)
 }
 }
 } else {
 // Restore normal color
 for (int col = 0; col < players_model->columnCount(); ++col) {
 QStandardItem *item = players_model->item(row, col);
 if (item) {
 item->setForeground(QBrush(QColor(255, 255, 255))); // White text
 }
 }
 }
 }
 }

 qDebug() << "[APPLY-FILTER] COMPLETE - checked=" << checked << "Visible:" << visible_count << "Hidden:" << hidden_count << "Invalid proxy:" << invalid_proxy_count << "Total:" << players_model->rowCount();
 }

 void applyRankRangeFilter() {
     // Bug 35: Filter players by rank range
     if (!players_model || !proxy_model) return;

     // Don't apply filter while sorting is disabled (during population)
     if (!players_table->isSortingEnabled()) {
         qDebug() << "[RANK-FILTER] Skipping filter - sorting disabled (populating data)";
         return;
     }

     QString from_rank = from_rank_combo->currentText();
     QString to_rank = to_rank_combo->currentText();

     qDebug() << "[RANK-FILTER] Filtering from" << from_rank << "to" << to_rank;

     // Get numeric rank values for comparison
     int from_value = getRankNumericValue(from_rank);
     int to_value = getRankNumericValue(to_rank);

     // Ensure from_value >= to_value (stronger to weaker)
     if (from_value < to_value) {
         qSwap(from_value, to_value);
     }

     // Hide rows outside the rank range
     for (int row = 0; row < players_model->rowCount(); ++row) {
         QStandardItem *rank_item = players_model->item(row, 2);  // Column 2 = Rank
         if (!rank_item) continue;

         QString rank = rank_item->text();
         int rank_value = getRankNumericValue(rank);

         // Show if rank is within range (from_value >= rank_value >= to_value)
         bool in_range = (rank_value <= from_value && rank_value >= to_value);

         // Get corresponding proxy row
         QModelIndex source_index = players_model->index(row, 0);
         QModelIndex proxy_index = proxy_model->mapFromSource(source_index);

         if (proxy_index.isValid()) {
             players_table->setRowHidden(proxy_index.row(), proxy_index.parent(), !in_range);
         }
     }

     qDebug() << "[RANK-FILTER] Filter applied: from_value=" << from_value << "to_value=" << to_value;
 }

 int getRankNumericValue(const QString &rank) {
     // Convert rank to numeric value (higher = stronger)
     // Pro: 9p=900, 8p=800, ..., 1p=100
     // Dan: 10d=100, 9d=90, ..., 1d=10
     // Kyu: 1k=9, 2k=8, ..., 30k=-20
     // BC/NR: -30

     QString clean_rank = rank.trimmed().toLower();
     clean_rank.remove('*').remove('+').remove('?');

     if (clean_rank == "bc" || clean_rank == "nr") {
         return -30;
     }

     QRegExp rank_regex("([0-9]+)([dkp])");
     if (rank_regex.indexIn(clean_rank) != -1) {
         int num = rank_regex.cap(1).toInt();
         QString type = rank_regex.cap(2);

         if (type == "p") {
             return num * 100;  // 1p=100, 9p=900
         } else if (type == "d") {
             return num * 10;  // 1d=10, 10d=100
         } else if (type == "k") {
             return 10 - num;  // 1k=9, 2k=8, ..., 30k=-20
         }
     }

     return -30;  // Default to BC level
 }

signals:
 void refreshRequested();
 void matchRequested(const QString &opponent, const QString &opponent_rank);
 void statsRequested(const QString &player);
 void tellRequested(const QString &player, const QString &message);
    void toggleCommandRequested(const QString &parameter);  // Feature 33: Toggle account setting
};

// Custom delegate to render observed games in bold (xgospel1 pattern)
// v50: Now with 3D button-style rendering and single-click actuation
class ObservedGameDelegate : public QStyledItemDelegate {
 Q_OBJECT
private:
 const QSet<int>* observed_game_ids;
 QSortFilterProxyModel* proxy_model;
 bool use_focus_colors;

 // State tracking for button rendering
 mutable QPersistentModelIndex hover_index;
 mutable QPersistentModelIndex pressed_index;

public:
 ObservedGameDelegate(const QSet<int>* observed_ids, QSortFilterProxyModel* proxy, bool focus_colors, QObject* parent = nullptr)
 : QStyledItemDelegate(parent), observed_game_ids(observed_ids), proxy_model(proxy), use_focus_colors(focus_colors) {}

 void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
 // Get the game ID from column 0 of this row
 QModelIndex id_index = index.sibling(index.row(), 0);
 int game_id = id_index.data().toInt();

 bool is_observed = (observed_game_ids && observed_game_ids->contains(game_id));
 bool is_hover = (hover_index.isValid() && hover_index.row() == index.row());
 bool is_pressed = (pressed_index.isValid() && pressed_index.row() == index.row());

 if (is_observed) {
 // Highlight observed games with 3D selected appearance (system theme colors)
 QStyleOptionViewItem opt = option;

 // Mark as selected to get 3D rendering effect from Qt style system
 opt.state |= QStyle::State_Selected;

 // Control focus-based color changes (q5Go always uses inactive color)
 if (!use_focus_colors) {
 // Remove State_Active to always use the inactive/unfocused color
 opt.state &= ~QStyle::State_Active;
 }

 // Use default window manager theme colors (no custom palette override)
 // Future: Make this configurable via preferences (e.g., custom color #82548a)

 // Custom color code (commented out - will be used when preferences added):
 // QColor lavender(130, 84, 138);  // #82548a
 // opt.palette.setColor(QPalette::Active, QPalette::Highlight, lavender);
 // opt.palette.setColor(QPalette::Inactive, QPalette::Highlight, lavender);
 // opt.palette.setColor(QPalette::Active, QPalette::HighlightedText, QColor(255, 255, 255));
 // opt.palette.setColor(QPalette::Inactive, QPalette::HighlightedText, QColor(255, 255, 255));

 QStyledItemDelegate::paint(painter, opt, index);
 } else {
 // v50: Render unobserved games as 3D buttons (row-level rendering)

 // Only draw the button frame once per row (when painting first column)
 if (index.column() == 0) {
 // Get the view to calculate full row rect
 const QAbstractItemView* view = qobject_cast<const QAbstractItemView*>(option.widget);
 if (view) {
 // Calculate the full row rectangle spanning all columns
 QRect row_rect = option.rect;
 row_rect.setLeft(0);
 row_rect.setWidth(view->viewport()->width());

 // Draw 3D button frame for the entire row
 QStyleOptionButton button_opt;
 button_opt.rect = row_rect;
 button_opt.state = QStyle::State_Enabled;

 // Add raised/sunken state based on interaction
 if (is_pressed) {
 button_opt.state |= QStyle::State_Sunken;
 } else {
 button_opt.state |= QStyle::State_Raised;
 }

 // Add hover state for visual feedback
 if (is_hover) {
 button_opt.state |= QStyle::State_MouseOver;
 }

 // Draw button background (3D frame) for entire row
 QApplication::style()->drawControl(QStyle::CE_PushButton, &button_opt, painter);
 }
 }

 // Draw the text content normally (for all columns)
 QStyledItemDelegate::paint(painter, option, index);
 }
 }

 bool editorEvent(QEvent* event, QAbstractItemModel* model,
 const QStyleOptionViewItem& option, const QModelIndex& index) override {
 // Handle mouse events for single-click actuation
 if (event->type() == QEvent::MouseButtonPress) {
 QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
 if (mouseEvent->button() == Qt::LeftButton) {
 pressed_index = index;
 if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(const_cast<QWidget*>(option.widget))) {
 view->viewport()->update();
 }
 return false;  // Allow event to propagate
 }
 }
 else if (event->type() == QEvent::MouseButtonRelease) {
 QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
 if (mouseEvent->button() == Qt::LeftButton && pressed_index.isValid()) {
 pressed_index = QPersistentModelIndex();
 if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(const_cast<QWidget*>(option.widget))) {
 view->viewport()->update();

 // Emit single-click signal to trigger observation
 if (index.isValid()) {
 emit clicked(index);
 }
 }
 return true;  // Event handled
 }
 }
 else if (event->type() == QEvent::MouseMove) {
 QPersistentModelIndex old_hover = hover_index;
 hover_index = index;
 if (old_hover != hover_index) {
 if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(const_cast<QWidget*>(option.widget))) {
 view->viewport()->update();
 }
 }
 return false;
 }
 else if (event->type() == QEvent::Leave) {
 hover_index = QPersistentModelIndex();
 pressed_index = QPersistentModelIndex();
 if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(const_cast<QWidget*>(option.widget))) {
 view->viewport()->update();
 }
 return false;
 }

 return QStyledItemDelegate::editorEvent(event, model, option, index);
 }

signals:
 void clicked(const QModelIndex& index);
};

// ============================================================
// Shout Window — displays IGS CMD21 broadcast messages
// ============================================================
class FixedShoutWindow : public QMainWindow {
    Q_OBJECT

public:
    FixedShoutWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Shouts");
        setMinimumSize(400, 250);
        resize(500, 300);

        QWidget *central = new QWidget(this);
        setCentralWidget(central);
        QVBoxLayout *layout = new QVBoxLayout(central);
        layout->setSpacing(4);
        layout->setContentsMargins(4, 4, 4, 4);

        shout_display = new QTextBrowser(this);
        shout_display->setOpenLinks(false);
        shout_display->setStyleSheet("font-size: 11px;");
        connect(shout_display, &QTextBrowser::anchorClicked, this, [this](const QUrl &url) {
            if (url.scheme() == "player")
                emit playerClicked(url.path());
        });
        layout->addWidget(shout_display);

        QHBoxLayout *input_layout = new QHBoxLayout;
        shout_input = new QLineEdit(this);
        shout_input->setPlaceholderText("Type shout message...");
        shout_input->setStyleSheet("font-size: 11px;");
        send_button = new QPushButton("Shout", this);
        send_button->setStyleSheet("font-size: 11px;");
        input_layout->addWidget(shout_input);
        input_layout->addWidget(send_button);
        layout->addLayout(input_layout);

        connect(send_button, &QPushButton::clicked, this, &FixedShoutWindow::onSendClicked);
        connect(shout_input, &QLineEdit::returnPressed, this, &FixedShoutWindow::onSendClicked);
    }

    void addShout(const QString &sender, const QString &message) {
        QString escaped_msg = message.toHtmlEscaped();
        QString html = QString("<b><a href=\"player:%1\" style=\"color:#4488ff;\">%2</a></b>: %3")
            .arg(sender.toHtmlEscaped(), sender.toHtmlEscaped(), escaped_msg);
        shout_display->append(html);
        QTextCursor cursor = shout_display->textCursor();
        cursor.movePosition(QTextCursor::End);
        shout_display->setTextCursor(cursor);
    }

    // Called to bring the window to front and ensure cached shouts are visible
    void showAndRaise() {
        show();
        raise();
        activateWindow();
    }

signals:
    void shoutRequested(const QString &message);
    void playerClicked(const QString &name);

private slots:
    void onSendClicked() {
        QString text = shout_input->text().trimmed();
        if (text.isEmpty()) return;
        emit shoutRequested(text);
        shout_input->clear();
    }

private:
    QTextBrowser *shout_display;
    QLineEdit    *shout_input;
    QPushButton  *send_button;
};

class FixedGamesWindow : public QMainWindow {
 Q_OBJECT

private:
 QTreeView *games_table;
 QStandardItemModel *games_model;
 FixedRankSortProxyModel *proxy_model;
 bool newrating_enabled;
 QSet<int> observed_game_ids;  // Track observed games for delegate rendering
 ObservedGameDelegate* game_delegate;
 QTimer *refresh_timer;  // Auto-refresh timer for games list
	FixedPlayersWindow *players_window_ref;  // Reference to players window for status updates

public:
 FixedGamesWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
 setWindowTitle("Games Online");
 setMinimumSize(750, 700);
 newrating_enabled = false;
		players_window_ref = nullptr;
 setupUI();

 // Setup auto-refresh timer for games list
 refresh_timer = new QTimer(this);
 connect(refresh_timer, &QTimer::timeout, this, &FixedGamesWindow::refreshRequested);
 int refresh_interval = settings->getGamesWindowRefreshInterval();
 if (refresh_interval > 0) {
 refresh_timer->start(refresh_interval * 1000);  // Convert seconds to milliseconds
 qDebug() << "[GAMES] Auto-refresh enabled, interval:" << refresh_interval << "seconds";
 } else {
 qDebug() << "[GAMES] Auto-refresh disabled (interval = 0)";
 }

 // Restore window geometry from settings (xgospel1 .Xdefaults style)
 QRect savedGeometry = settings->loadWindowGeometry("games", QRect(250, 150, 1100, 700));
 qDebug() << "[GAMES] Loading geometry:" << savedGeometry;
 setGeometry(savedGeometry);
 }

 void closeEvent(QCloseEvent *event) override {
 // Save games window geometry before closing (xgospel1 style)
 QRect currentGeometry = geometry();
 qDebug() << "[GAMES] Saving geometry:" << currentGeometry;
	FixedPlayersWindow *players_window_ref;
 settings->saveWindowGeometry("games", currentGeometry);

 // Save column widths
 QList<int> columnWidths;
 for (int i = 0; i < games_model->columnCount(); i++) {
 columnWidths.append(games_table->columnWidth(i));
 }
 settings->saveSplitterSizes("games_columns", columnWidths);
 qDebug() << "[GAMES] Saving column widths:" << columnWidths;

 settings->save();
 QMainWindow::closeEvent(event);
 }

 void setNewratingEnabled(bool enabled) {
 newrating_enabled = enabled;
 }

 void setupUI() {
 QWidget *central = new QWidget;
 setCentralWidget(central);
 // Removed hardcoded colors - inherit from KDE theme

 QVBoxLayout *layout = new QVBoxLayout(central);
 layout->setSpacing(10);
 layout->setMargin(10);
 
 // Refresh button with xgospel1-style colors (black on darker gold #edd20d)
 QPushButton *refresh_btn = new QPushButton("Refresh Games");
 refresh_btn->setMinimumWidth(180);
 refresh_btn->setStyleSheet(
 "QPushButton {"
 " background-color: #edd20d;"
 " color: black;"
 " border: 4px outset #f5e030;"
 " border-top-color: #fffacd;"
 " border-left-color: #fffacd;"
 " border-right-color: #b8960a;"
 " border-bottom-color: #b8960a;"
 " padding: 10px 15px;"
 " font-weight: bold;"
 " font-size: 13px;"
 "}"
 "QPushButton:pressed {"
 " border: 4px inset #d4b909;"
 " border-top-color: #b8960a;"
 " border-left-color: #b8960a;"
 " border-right-color: #fffacd;"
 " border-bottom-color: #fffacd;"
 " background-color: #d4b909;"
 "}"
 "QPushButton:hover {"
 " background-color: #f5e030;"
 "}"
 );
 connect(refresh_btn, &QPushButton::clicked, this, &FixedGamesWindow::refreshRequested);
 layout->addWidget(refresh_btn);
 
 // Games table with high contrast
 QFrame *games_frame = new QFrame;
 games_frame->setFrameStyle(QFrame::Sunken | QFrame::Panel);
 games_frame->setLineWidth(4);
 games_frame->setStyleSheet(
 "QFrame {" " border: 4px inset #808080;" "}"
 );
 
 // Create the table view and models
 games_table = new QTreeView;
 games_model = new QStandardItemModel(0, 12, this);
 proxy_model = new FixedRankSortProxyModel(this);
 proxy_model->setSourceModel(games_model);
 games_table->setModel(proxy_model);
 
 // Configure table with high contrast
 games_table->setSortingEnabled(true);
 games_table->header()->setSectionsMovable(false);
 games_table->header()->setVisible(true);
 games_table->setItemsExpandable(false);
 
 // Set initial sort by white rank column (column 2) in ascending order (strongest first)
 games_table->sortByColumn(2, Qt::AscendingOrder);
 games_table->setRootIsDecorated(false);
 games_table->setUniformRowHeights(true);
 games_table->setAlternatingRowColors(false);  // v50: Disabled - 3D button frames provide visual separation
 games_table->setSelectionBehavior(QAbstractItemView::SelectRows);
 games_table->setStyleSheet(
 "QTreeView {" " " " alternate-background-" " gridline-" " selection-background-" " selection-" " font-size: 11px;" "}" "QHeaderView::section {" " background-" " " " border: 1px solid #999;" " padding: 5px;" " font-weight: bold;" "}"
 );
 
 setupHeaders();

 // Set up custom delegate for rendering observed games in bold (xgospel1 pattern)
 // v50: Now with 3D button-style rendering and single-click actuation
 game_delegate = new ObservedGameDelegate(&observed_game_ids, proxy_model, settings->getUseFocusColors(), this);
 games_table->setItemDelegate(game_delegate);

 // Enable mouse tracking for hover effects
 games_table->setMouseTracking(true);
 games_table->viewport()->setMouseTracking(true);

 // v50: Connect single-click for observe functionality (replaces double-click)
 connect(game_delegate, &ObservedGameDelegate::clicked, this, &FixedGamesWindow::observeGame);

 // Keep double-click as fallback for compatibility
 connect(games_table, &QTreeView::doubleClicked, this, &FixedGamesWindow::observeGame);

 QVBoxLayout *games_layout = new QVBoxLayout(games_frame);
 games_layout->setMargin(6);
 games_layout->addWidget(games_table);
 layout->addWidget(games_frame);
 }
 
 void setupHeaders() {
 QStringList headers;
 headers << "Id" << "White" << "WR" << "Black" << "BR" << "Mv" 
 << "Sz" << "H" << "K" << "By" << "FR" << "Ob";
 games_model->setHorizontalHeaderLabels(headers);
 }
 
 void addGameFromRawLine(const QString& line) {
 Q5GoGamesParser parser;
 Q5GoGameInfo game;

 if (parser.parseGameLine(line, game)) {
 addGameToTable(game);
 }
 }
 
 void addGameToTable(const Q5GoGameInfo& game) {
 if (game.nr.isEmpty() || game.wname.isEmpty() || game.bname.isEmpty()) return;
 
 QList<QStandardItem*> row;
 
 // Create rank items with sort keys
 // Handle special rank display rules for both white and black ranks
 QString display_wrank = game.wrank;
 QString display_brank = game.brank;
 
 // Apply rank display rules to white rank
 QString lower_wrank = display_wrank.toLower();
 if (lower_wrank.contains('p')) {
 display_wrank.remove('*').remove('+');
 } else if (lower_wrank.startsWith("10d")) {
 display_wrank.remove('*').remove('+');
 display_wrank = "10d";
 } else if (lower_wrank.contains('d') || lower_wrank.contains('k')) {
 // Handle * and + based on newrating state
 if (newrating_enabled) {
 if (display_wrank.contains('*')) {
 display_wrank.replace('*', '+');
 }
 } else {
 if (display_wrank.contains('+')) {
 display_wrank.replace('+', '*');
 }
 }
 }
 
 // Apply rank display rules to black rank 
 QString lower_brank = display_brank.toLower();
 if (lower_brank.contains('p')) {
 display_brank.remove('*').remove('+');
 } else if (lower_brank.startsWith("10d")) {
 display_brank.remove('*').remove('+');
 display_brank = "10d";
 } else if (lower_brank.contains('d') || lower_brank.contains('k')) {
 // Handle * and + based on newrating state
 if (newrating_enabled) {
 if (display_brank.contains('*')) {
 display_brank.replace('*', '+');
 }
 } else {
 if (display_brank.contains('+')) {
 display_brank.replace('+', '*');
 }
 }
 }
 
 QStandardItem *wrank_item = new QStandardItem(display_wrank);
 wrank_item->setData(game.sort_rk_w, Qt::UserRole);
 
 QStandardItem *brank_item = new QStandardItem(display_brank);
 brank_item->setData(game.sort_rk_b, Qt::UserRole);
 
 row << new QStandardItem(game.nr) // Id
 << new QStandardItem(game.wname) // White
 << wrank_item // WR (with sort key)
 << new QStandardItem(game.bname) // Black
 << brank_item // BR (with sort key)
 << new QStandardItem(game.mv) // Mv
 << new QStandardItem(game.Sz) // Sz
 << new QStandardItem(game.H) // H
 << new QStandardItem(game.K) // K
 << new QStandardItem(game.By) // By
 << new QStandardItem(game.FR) // FR
 << new QStandardItem(game.ob); // Ob
 
 games_model->appendRow(row);
		if (players_window_ref) {
			players_window_ref->updatePlayerGameStatus(game.wname, game.nr);
			players_window_ref->updatePlayerGameStatus(game.bname, game.nr);
		}
		updateGamesCount();
 }
 
 void clearGames() {
 games_model->clear();
 setupHeaders();

 // Restore column widths after clearing (otherwise they reset to defaults)
 QList<int> savedWidths = settings->loadSplitterSizes("games_columns");
 if (!savedWidths.isEmpty() && savedWidths.size() == 12) {  // 12 columns in games table
 for (int i = 0; i < savedWidths.size(); i++) {
 games_table->setColumnWidth(i, savedWidths[i]);
 }
 qDebug() << "[GAMES] Restored column widths after clear:" << savedWidths;
 }
 }
 
 void resizeColumns() {
 // Try to load saved column widths first
 QList<int> savedWidths = settings->loadSplitterSizes("games_columns");

 if (!savedWidths.isEmpty() && savedWidths.size() == games_model->columnCount()) {
 // Restore saved column widths
 for (int i = 0; i < savedWidths.size(); i++) {
 games_table->setColumnWidth(i, savedWidths[i]);
 }
 qDebug() << "[GAMES] Restored column widths:" << savedWidths;
 } else {
 // No saved widths, auto-resize to contents
 for (int i = 0; i < games_model->columnCount(); i++) {
 games_table->resizeColumnToContents(i);

 }
 }

 // Force viewport repaint to ensure rendering (fixes occasional blank window issue)
 if (games_table && games_table->viewport()) {
 games_table->viewport()->repaint();
 }
 }

	void setPlayersWindowRef(FixedPlayersWindow *pw) {
		players_window_ref = pw;
		qDebug() << "[GAMES] Set players window reference";
	}

	void syncAllPlayerStatuses() {
		if (!players_window_ref || !games_model) return;
		qDebug() << "[GAMES] Syncing all player PLAYING statuses...";
		qDebug() << "[GAMES] NOTE: Observing statuses come from WHO/userlist and are preserved";
		players_window_ref->resetAllPlayerGameStatuses();
		for (int row = 0; row < games_model->rowCount(); ++row) {
			QStandardItem *game_nr_item = games_model->item(row, 0);
			QStandardItem *wname_item = games_model->item(row, 1);
			QStandardItem *bname_item = games_model->item(row, 3);
			if (!game_nr_item || !wname_item || !bname_item) continue;
			QString game_nr = game_nr_item->text();
			QString white_name = wname_item->text();
			QString black_name = bname_item->text();
			players_window_ref->updatePlayerGameStatus(white_name, game_nr);
			players_window_ref->updatePlayerGameStatus(black_name, game_nr);
			// Observing data comes from WHO/userlist command, not from games list
		}
		qDebug() << "[GAMES] Player status sync complete for" << games_model->rowCount() << "games";
		players_window_ref->applyAllFilters();
	}

	void updateGamesCount() {
		int count = games_model->rowCount();
		setWindowTitle(QString("Games Online (%1)").arg(count));
	}


 void updateObservedGames(const QSet<int>& observed_ids) {
 // Update the observed games set (used by custom delegate for bold rendering)
 // xgospel1 pattern: track observed games and render them bold via delegate
 qDebug() << "[updateObservedGames] Updating observed games, old size:" << observed_game_ids.size()
          << "new size:" << observed_ids.size();
 observed_game_ids = observed_ids;

 if (games_table && games_table->viewport()) {
 games_table->viewport()->update();
 } else {
 qDebug() << "[updateObservedGames] WARNING: games_table or viewport is NULL!";
 }
 }

 void clearSelectionForGame(int game_id) {
 // Clear Qt selection state for the row containing this game_id
 // This prevents the default selection color from showing after we stop observing
 if (!games_table || !proxy_model || !games_model) return;

 qDebug() << "[clearSelectionForGame] Searching for game_id" << game_id << "to clear selection";

 // Search through the model to find the row with this game_id
 for (int row = 0; row < games_model->rowCount(); ++row) {
 QStandardItem* id_item = games_model->item(row, 0);
 if (id_item && id_item->text().toInt() == game_id) {
 // Found the row - now find it in the proxy model
 QModelIndex source_index = games_model->index(row, 0);
 QModelIndex proxy_index = proxy_model->mapFromSource(source_index);

 if (proxy_index.isValid()) {
 qDebug() << "[clearSelectionForGame] Found game" << game_id << "at proxy row" << proxy_index.row()
                      << "- clearing selection";
 // Clear the selection for this row
 games_table->selectionModel()->select(proxy_index,
                                                   QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
 } else {
 qDebug() << "[clearSelectionForGame] Found game in model but proxy_index is invalid (filtered out?)";
 }
 return;
 }
 }
 qDebug() << "[clearSelectionForGame] Game" << game_id << "not found in games table";
 }

private slots:
 void observeGame(const QModelIndex& index) {
 if (!index.isValid()) return;
 
 // Get game information from the selected row
 int game_id = proxy_model->data(proxy_model->index(index.row(), 0)).toString().toInt();
 QString white = proxy_model->data(proxy_model->index(index.row(), 1)).toString();
 QString white_rank = proxy_model->data(proxy_model->index(index.row(), 2)).toString();
 QString black = proxy_model->data(proxy_model->index(index.row(), 3)).toString();
 QString black_rank = proxy_model->data(proxy_model->index(index.row(), 4)).toString();
 
 emit observeRequested(game_id, white, black, white_rank, black_rank);
 }

signals:
 void refreshRequested();
 void observeRequested(int game_id, const QString& white, const QString& black, 
 const QString& white_rank, const QString& black_rank);
};

class MatchDialog : public QDialog {
 Q_OBJECT

private:
 QLineEdit *opponent_edit;
 QRadioButton *black_button;
 QRadioButton *white_button;
 QRadioButton *nigiri_button;
 QSpinBox *board_size_spin;
 QDoubleSpinBox *komi_spin;
 QSpinBox *handicap_spin;
 QSpinBox *time_spin;
 QSpinBox *byo_time_spin;
 QComboBox *free_combo;
 QPushButton *offer_button;
 QPushButton *cancel_button;
 QPushButton *decline_button;
 QPushButton *suggest_button;
 QPushButton *stats_button;
 
 QString my_name;
 QString my_rank;
 QString opponent_rank;
 bool is_nmatch;
 bool is_incoming_request;  // Bug 34: Track if this is accepting an incoming request
    QString server_suggested_command;  // Bug 34: Store server's suggested match command

public:
 MatchDialog(QWidget *parent, const QString &my_name, const QString &my_rank,
 const QString &opponent, const QString &opponent_rank)
 : QDialog(parent), my_name(my_name), my_rank(my_rank), opponent_rank(opponent_rank), is_nmatch(false), is_incoming_request(false) {

 setWindowTitle("New Game");
 setModal(false);  // Bug 32a: Non-modal so it doesn't block or tie to parent
 setMinimumSize(350, 500);
 setWindowIcon(QIcon("XgospelIcon.xpm"));  // Bug 32a: Set icon for independent window
 setupUI(opponent);
 }

 void setupUI(const QString &opponent) {
 QVBoxLayout *main_layout = new QVBoxLayout(this);
 main_layout->setSpacing(10);
 main_layout->setMargin(15);

 // Title
 QLabel *title = new QLabel("New Game");
 title->setAlignment(Qt::AlignCenter);
 title->setStyleSheet("font-weight: bold; font-size: 16px; margin-bottom: 10px;");
 main_layout->addWidget(title);

 // Opponent section
 QGroupBox *opponent_group = new QGroupBox("Game against");
 QHBoxLayout *opponent_layout = new QHBoxLayout(opponent_group);
 
 opponent_edit = new QLineEdit(opponent);
 opponent_edit->setMinimumHeight(25);
 QLabel *opponent_rank_label = new QLabel(opponent_rank);
 opponent_rank_label->setStyleSheet("font-weight: bold; min-width: 50px;");
 
 stats_button = new QPushButton("Stats");
 stats_button->setMaximumWidth(60);
 suggest_button = new QPushButton("Suggest");
 suggest_button->setMaximumWidth(70);
 
 opponent_layout->addWidget(opponent_edit, 3);
 opponent_layout->addWidget(opponent_rank_label, 1);
 opponent_layout->addWidget(stats_button);
 opponent_layout->addWidget(suggest_button);
 main_layout->addWidget(opponent_group);

 // Color selection
 QGroupBox *color_group = new QGroupBox("You play");
 QHBoxLayout *color_layout = new QHBoxLayout(color_group);
 
 black_button = new QRadioButton("Black");
 white_button = new QRadioButton("White");
 nigiri_button = new QRadioButton("Nigiri");
 black_button->setChecked(true); // Default to Black
 
 color_layout->addWidget(black_button);
 color_layout->addWidget(white_button);
 color_layout->addWidget(nigiri_button);
 main_layout->addWidget(color_group);

 // Game settings
 QGroupBox *settings_group = new QGroupBox("Game Settings");
 QGridLayout *settings_layout = new QGridLayout(settings_group);
 
 // Board size
 settings_layout->addWidget(new QLabel("Board size:"), 0, 0);
 board_size_spin = new QSpinBox();
 board_size_spin->setRange(9, 19);
 board_size_spin->setValue(19);
 settings_layout->addWidget(board_size_spin, 0, 1);

 // Komi
 settings_layout->addWidget(new QLabel("Komi:"), 1, 0);
 komi_spin = new QDoubleSpinBox();
 komi_spin->setRange(-9.5, 9.5);
 komi_spin->setSingleStep(0.5);
 komi_spin->setValue(0.5); // Default to 0.5 for IGS
 komi_spin->setDecimals(1);
 settings_layout->addWidget(komi_spin, 1, 1);

 // Handicap
 settings_layout->addWidget(new QLabel("Handicap:"), 2, 0);
 handicap_spin = new QSpinBox();
 handicap_spin->setRange(0, 9);
 handicap_spin->setValue(0);
 settings_layout->addWidget(handicap_spin, 2, 1);

 // Time
 settings_layout->addWidget(new QLabel("Time:"), 3, 0);
 time_spin = new QSpinBox();
 time_spin->setRange(1, 300);
 time_spin->setValue(60);
 time_spin->setSuffix(" min");
 settings_layout->addWidget(time_spin, 3, 1);

 // Byoyomi time
 settings_layout->addWidget(new QLabel("Byoyomi Time:"), 4, 0);
 byo_time_spin = new QSpinBox();
 byo_time_spin->setRange(1, 60);
 byo_time_spin->setValue(10);
 byo_time_spin->setSuffix(" min");
 settings_layout->addWidget(byo_time_spin, 4, 1);

 // Free game (IGS auto-determines based on rank difference)
 settings_layout->addWidget(new QLabel("Game Type:"), 5, 0);
 free_combo = new QComboBox();
 free_combo->addItem("Auto (IGS decides)");
 free_combo->addItem("Override to Free");
 free_combo->setToolTip("IGS automatically makes games free if rank difference >=4 stones, rated if <=3 stones.\nUse 'Override to Free' to request free game regardless of ranks.\nActual override happens during game with 'free' command.");
 settings_layout->addWidget(free_combo, 5, 1);

 main_layout->addWidget(settings_group);

 // Button layout
 QWidget *button_widget = new QWidget();
 QHBoxLayout *button_layout = new QHBoxLayout(button_widget);
 
 offer_button = new QPushButton("Offer");
 offer_button->setStyleSheet("QPushButton { background- font-weight: bold; padding: 8px 16px; }");
 
 decline_button = new QPushButton("Decline");
 decline_button->setStyleSheet("QPushButton { background- font-weight: bold; padding: 8px 16px; }");
 decline_button->setEnabled(false);
 
 cancel_button = new QPushButton("Cancel");
 cancel_button->setStyleSheet("QPushButton { background- font-weight: bold; padding: 8px 16px; }");
 
 button_layout->addWidget(decline_button);
 button_layout->addStretch();
 button_layout->addWidget(offer_button);
 button_layout->addWidget(cancel_button);
 main_layout->addWidget(button_widget);

 // Connect signals
 connect(offer_button, &QPushButton::clicked, this, &MatchDialog::onOfferClicked);
 connect(cancel_button, &QPushButton::clicked, this, &QDialog::reject);
 connect(decline_button, &QPushButton::clicked, this, &MatchDialog::onDeclineClicked);
 connect(stats_button, &QPushButton::clicked, this, &MatchDialog::onStatsClicked);
 connect(suggest_button, &QPushButton::clicked, this, &MatchDialog::onSuggestClicked);

 // Connect settings change handler
 connect(opponent_edit, &QLineEdit::textChanged, this, &MatchDialog::onSettingsChanged);
 connect(handicap_spin, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MatchDialog::onSettingsChanged);
 connect(komi_spin, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &MatchDialog::onSettingsChanged);
 connect(board_size_spin, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MatchDialog::onSettingsChanged);
 connect(time_spin, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MatchDialog::onSettingsChanged);
 connect(byo_time_spin, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MatchDialog::onSettingsChanged);

 onSettingsChanged(); // Initial call to set button text
 }

 void setIsNmatch(bool nmatch) {
 is_nmatch = nmatch;
 // Enable/disable certain options based on nmatch
 handicap_spin->setEnabled(nmatch);
 nigiri_button->setEnabled(nmatch);
 }

    // Bug 34: Set whether this is an incoming request (accept) vs outgoing (offer)
    void setIsIncomingRequest(bool incoming) {
        is_incoming_request = incoming;
    }

    // Bug 34: Set server's suggested match command for accepting incoming requests
    void setServerSuggestedCommand(const QString &cmd) {
        server_suggested_command = cmd;
    }

 QString getOpponent() const { return opponent_edit->text(); }
 QString getColor() const {
 if (black_button->isChecked()) return "B";
 if (white_button->isChecked()) return "W";
 if (nigiri_button->isChecked()) return "N";
 return "B"; // Default
 }
 int getBoardSize() const { return board_size_spin->value(); }
 double getKomi() const { return komi_spin->value(); }
 int getHandicap() const { return handicap_spin->value(); }
 int getTime() const { return time_spin->value(); }
 int getByoTime() const { return byo_time_spin->value(); }
 bool isFreeGame() const { return free_combo->currentText() == "Override to Free"; }
 bool isNmatch() const { return is_nmatch; }
 
 // Getter methods for UI controls (for incoming requests)
 QSpinBox* getBoardSizeSpin() { return board_size_spin; }
 QSpinBox* getTimeSpin() { return time_spin; }
 QSpinBox* getByoTimeSpin() { return byo_time_spin; }
 QSpinBox* getHandicapSpin() { return handicap_spin; }
 QRadioButton* getBlackButton() { return black_button; }
 QRadioButton* getWhiteButton() { return white_button; }
 QRadioButton* getNigiriButton() { return nigiri_button; }
 QPushButton* getOfferButton() { return offer_button; }
 QPushButton* getDeclineButton() { return decline_button; }
 QPushButton* getCancelButton() { return cancel_button; }

signals:
 void matchOffered(const QString &command);
 void statsRequested(const QString &player);
 void suggestRequested(const QString &player);

public slots:
 void onOfferClicked() {
 QString command;
 QString opponent = getOpponent();
 QString color_char = getColor();

        // Bug 34: If this is an incoming request, use server's suggested command
        if (is_incoming_request) {
            if (!server_suggested_command.isEmpty()) {
                command = server_suggested_command;
            } else {
                // Fallback: construct command from dialog parameters
                if (is_nmatch) {
                    command = "nmatch";
                } else {
                    command = QString("match %1 %2 %3 %4 %5")
                        .arg(opponent)
                        .arg(color_char)
                        .arg(getBoardSize())
                        .arg(getTime())
                        .arg(getByoTime());
                }
            }
        } else if (is_nmatch) {
 // nmatch command format: "nmatch opponent color handicap komi size time_seconds byo_seconds byo_stones rated reserved1 reserved2"
 // Parameter 9: 0=rated, 1=free
 // NOTE: BOTH players must send free_param=1 for game to be free
 // NOTE: IGS may override to rated if players are on same network (anti-cheating)
 int free_param = isFreeGame() ? 1 : 0;
 command = QString("nmatch %1 %2 %3 %4 %5 %6 %7 25 %8 0 0")
 .arg(opponent)
 .arg(color_char)
 .arg(getHandicap())
 .arg(getKomi(), 0, 'f', 1) // Komi with 1 decimal place
 .arg(getBoardSize())
 .arg(getTime() * 60) // Convert minutes to seconds
 .arg(getByoTime() * 60) // Convert minutes to seconds
 .arg(free_param); // 0=rated, 1=free

 if (free_param == 1) {
 qDebug() << ">>> NMATCH: Requesting FREE game (param 8 = 1)";
 qDebug() << ">>> NOTE: Opponent must also select FREE, otherwise server makes it RATED";
 qDebug() << ">>> NOTE: Server may force RATED if same network/IP detected";
 }
 } else {
 // regular match command format: "match opponent color size time_minutes byo_minutes"
 // Note: old match protocol doesn't have free/rated parameter in the command
 // For old match, we need to send "free" command after game starts
 command = QString("match %1 %2 %3 %4 %5")
 .arg(opponent)
 .arg(color_char)
 .arg(getBoardSize())
 .arg(getTime())
 .arg(getByoTime());
 }

 emit matchOffered(command);

        // Bug 34: Only send komi/free requests for outgoing matches, not when accepting incoming
        if (!is_incoming_request) {
 // Send komi command after game starts if komi is not the default
 // Default komi: 6.5 for even games, 0.5 for handicap games
 double komi = getKomi();
 int handicap = getHandicap();
 double default_komi = (handicap > 0) ? 0.5 : 6.5;

 if (qAbs(komi - default_komi) > 0.01) { // Not using default komi
 QString komi_marker = QString("#KOMI_REQUEST:%1#").arg(komi, 0, 'f', 1);
 emit matchOffered(komi_marker);
 qDebug() << ">>> Will send komi command after game starts: komi" << komi;
 }

 // For old match protocol, we need to send "free" command after game starts
 if (!is_nmatch && isFreeGame()) {
 // Emit a special marker so the main window knows to request free status after game starts
 emit matchOffered("#FREE_GAME_REQUEST#");
 qDebug() << ">>> MATCH: Requesting FREE game with old match protocol - will send 'free' after game starts";
 }

        }

 accept();
 }
 
 void onDeclineClicked() {
 QString command = QString("decline %1").arg(getOpponent());
 emit matchOffered(command);
 reject();
 }
 
 void onStatsClicked() {
 emit statsRequested(getOpponent());
 }
 
 void onSuggestClicked() {
 emit suggestRequested(getOpponent());
 }
 
 void onSettingsChanged() {
 // Check if this is a teaching game (same player)
 if (opponent_edit->text() == my_name) {
 offer_button->setText("Teaching");
 free_combo->setEnabled(false);
 byo_time_spin->setEnabled(false);
 time_spin->setEnabled(false);
 } else {
 offer_button->setText("Offer");
 free_combo->setEnabled(true);
 byo_time_spin->setEnabled(true);
 time_spin->setEnabled(true);
 }
 
 // Auto-set free game for unranked players
 if (opponent_rank == "NR" || my_rank == "NR" || opponent_rank == my_rank) {
 free_combo->setCurrentText("yes");
 }
 }
};

// ---------------------------------------------------------------------------
// EngineVsEngineDialog — pick Black/White engine profiles, komi, handicap
// ---------------------------------------------------------------------------
class EngineVsEngineDialog : public QDialog {
public:
    explicit EngineVsEngineDialog(const QList<EngineProfile> &profiles,
                                  QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("Engine vs Engine");
        setFixedWidth(340);

        QVBoxLayout *main_layout = new QVBoxLayout(this);
        QFormLayout *form = new QFormLayout();
        m_black_combo = new QComboBox();
        m_white_combo = new QComboBox();
        for (const EngineProfile &p : profiles) {
            m_black_combo->addItem(p.name, p.id);
            m_white_combo->addItem(p.name, p.id);
        }
        if (profiles.size() >= 2) m_white_combo->setCurrentIndex(1);
        form->addRow("Black engine:", m_black_combo);
        form->addRow("White engine:", m_white_combo);

        m_handicap_spin = new QSpinBox();
        m_handicap_spin->setRange(0, 9);
        m_handicap_spin->setValue(0);
        m_komi_spin = new QDoubleSpinBox();
        m_komi_spin->setRange(0.0, 9.5);
        m_komi_spin->setSingleStep(0.5);
        m_komi_spin->setValue(6.5);
        m_komi_spin->setDecimals(1);
        form->addRow("Handicap:", m_handicap_spin);
        form->addRow("Komi:", m_komi_spin);
        connect(m_handicap_spin,
                static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                [this](int v){ m_komi_spin->setValue(v > 0 ? 0.5 : 6.5); });
        main_layout->addLayout(form);

        QHBoxLayout *btns = new QHBoxLayout();
        btns->addStretch();
        QPushButton *ok  = new QPushButton("Start");
        QPushButton *can = new QPushButton("Cancel");
        ok->setDefault(true);
        connect(ok,  &QPushButton::clicked, this, &QDialog::accept);
        connect(can, &QPushButton::clicked, this, &QDialog::reject);
        btns->addWidget(ok);
        btns->addWidget(can);
        main_layout->addLayout(btns);
    }

    QString blackProfileId() const { return m_black_combo->currentData().toString(); }
    QString whiteProfileId() const { return m_white_combo->currentData().toString(); }
    double  komi()           const { return m_komi_spin->value(); }
    int     handicap()       const { return m_handicap_spin->value(); }

private:
    QComboBox      *m_black_combo;
    QComboBox      *m_white_combo;
    QDoubleSpinBox *m_komi_spin;
    QSpinBox       *m_handicap_spin;
};

class FixedXGospelWindow : public QMainWindow {
 Q_OBJECT

private:
 QTextEdit *output_console;
 QLineEdit *command_text;
 QTcpSocket *socket;
 FixedPlayersWindow *players_window;
 FixedGamesWindow *games_window;
 FixedShoutWindow *shout_window;
    MatchDialog *pending_match_dialog;  // Bug 34: Track pending match dialog for server command
 QList<BoardWindow*> board_windows;
 BoardWindow *most_recently_observed_board; // Track most recent observation for teaching title assignment
 int most_recently_observed_game_id = -1;  // Fallback for title lines that arrive before first Command 15

 // Docked-pane mode members (populated only when use_docked_game_pane = true)
 bool               docked_pane_mode       = false;
 QList<GameSlot*>   game_slots;
 int                active_slot_game_id    = -1;
 BoardWindow       *shared_board_window    = nullptr;
 IGSMoveParser *move_parser;
 int current_game_context; // Track which game context we're in
 bool waiting_for_players;
 bool waiting_for_games;
 bool connected_to_igs;
 QDateTime last_games_console_log;  // Throttle console output for games list refresh
 bool suppress_games_console;  // User option to suppress games output to console
 bool suppress_moves_console;  // User option to suppress moves output to console
 bool suppress_server_console;  // User option to suppress debug messages (>>>) to console (raw server output always shown)
 bool auto_launched_windows; // Track if we've auto-launched Players/Games windows on login
 bool auto_launch_populate_pending; // Track if we're waiting to populate from buffer during auto-launch
 int player_count;
 int game_count;
 QString login_username;
 QString login_password;
 bool tried_fallback_who;
 bool waiting_for_who_supplement;  // Bug 35: Track if we're waiting for WHO to supplement userlist with BC/NR/guest players
 QSet<QString> userlist_player_names;  // Bug 35: Track players from userlist to avoid duplicates when merging WHO data
 bool newrating_enabled;
 QString pending_kibitz_user;
 QString current_kibitzer;
 int current_kibitz_game;
 int pending_kibitz_game;
 
 // Store game details from Command 7 for later use when board windows are created
 QMap<int, double> game_komi_map;
 QMap<int, int> game_handicap_map;
 QMap<int, QString> game_type_map;
 QMap<int, QString> game_white_player_map; // Store white player names from Command 7
 QMap<int, QString> game_black_player_map; // Store black player names from Command 7
 QSet<int> games_with_moves_requested; // q5Go pattern: track which games have had "moves N" sent
 QSet<int> games_pending_moves_request; // games waiting for IGS observation confirmation before sending "moves N"
 QList<int> moves_dispatch_queue;       // serialized "moves N" queue: only one in-flight at a time
 QSet<int> observed_game_ids; // Track currently observed games for Games window highlighting (xgospel1 pattern)

 // Buffer Command 7 game lines during login to populate games window on auto-launch
 QList<QString> buffered_game_lines;
 
 // Track next game type from match setup messages
 bool next_game_is_free;
 double next_game_komi;

 // Track games that need the "free" command sent (for old match protocol)
 QSet<int> free_game_requests;

 // Track games that need komi set (game_id -> komi_value)
 QMap<int, double> komi_requests;

 // User account information for match dialogs
 QString my_account_name;
 QString my_account_rank;
 
 // Heartbeat system for IGS connection maintenance
 QTimer *heartbeat_timer;
 int heartbeat_counter;
 bool hold_the_line;

 // Auto-reconnect state
 QTimer *reconnect_timer;
 int  reconnect_attempts;
 bool manual_disconnect;    // true when user explicitly disconnected — suppresses auto-reconnect
 QString reconnect_opponent; // opponent name saved at disconnect for post-reconnect load command
 QString reconnect_game_file; // full IGS game filename (e.g. "woodnstone-weakkyu") from stored response
 int cross_session_resume_game_id = -1; // set when CMD67 creates a fresh slot needing moves fetch + engine restart
 int bot_restart_after_replay_game_id = -1; // when set, restart engine + genmove once this game's replay completes
 int newly_confirmed_game_id = -1; // set by "9 Creating match [N]" so stale-CMD15 guard skips genuine new games

 // Observer list parsing state
 bool waiting_for_observer_response;
 int observer_request_game_id;
 bool parsing_observers;
 int observer_game_id;
 
 // Manual observe command state
 bool pending_manual_observe;
 int pending_observe_game_id;

 // Set when IGS confirms "X has restored/restarted your game" — next "9 Observing game N"
 // for matching players should reassign the adjourned slot to the new game ID.
 QString pending_resume_player;
 bool    stored_header_seen = false;
 QStringList stored_game_lines;
 QSet<QString> stored_adjourned_opponents; // opponents with server-saved games from prior sessions

 // History replay: when "moves N" is sent, pin current_game_context to N until
 // the replay finishes so interleaved CMD15 headers from other games don't hijack
 // move routing and cause all history moves to be dropped as SKIPPED.
 // Per-slot replay state machines replace the old global history_replay_game_id pin.
 // No global pin, no deferred queue — each slot tracks its own replay state.
 
 // IGS territory data state (following q5Go protocol)
 bool receiving_territory_data;
 int territory_data_column;
 BoardWindow* territory_board;
 GameSlot*    territory_slot;   // non-null when territory data is for an inactive docked slot
 QString territory_player_name; // Track first player name from Command 22 to match board
 int cmd22_first_captures;     // Prisoner count from CMD22 first header line (first player)
 
 // TCP buffering variables (from q5Go)
 char *saved_data;
 int len_saved_data;

 // Connection menu for dynamic account management
 QMenu *connection_menu;

 // Engines menu and attach submenu (rebuilt dynamically)
 QMenu *engines_menu   = nullptr;
 QMenu *attach_submenu = nullptr;

 // AI engine integration
 EngineManager *engine_manager   = nullptr;  // human-vs-engine / EvE black engine
 EngineManager *engine_manager2  = nullptr;  // EvE white engine
 BoardWindow   *engine_board     = nullptr;
 StoneColor     engine_color     = BLACK_STONE;  // which color the engine plays (human-vs-engine)
 int            engine_game_id   = -2;  // sentinel — negative avoids IGS game ID collision
 QString        m_pending_engine_profile_id;
 bool           eve_mode         = false;  // true = engine-vs-engine game in progress
 bool           m_eve_black_ready = false;
 bool           m_eve_white_ready = false;
 // EvE deferred white engine start (stored until black is ready)
 QString        m_eve_white_id;
 double         m_eve_komi       = 6.5;
 int            m_eve_handicap   = 0;

 // --- Bot mode (Phase 2) ---
 bool           bot_mode_active  = false;  // true = auto-accept matches, engine plays on IGS
 bool           bot_engine_ready = false;  // true after onBotEngineReady completes (set_free_handicap sent)
 QAction       *bot_mode_action  = nullptr; // menu toggle (kept for checkmark updates)
 StoneColor     bot_color        = BLACK_STONE; // color bot plays in current IGS game
 int            bot_game_id      = -1;    // IGS game ID of current bot game (-1 = none)
 int            bot_main_time    = 0;     // main time in seconds from match params
 int            bot_byoyomi_time = 300;   // byoyomi period in seconds (default 5 min)
 int            bot_byoyomi_stones = 25;  // stones per byoyomi period (default 25)
 double         bot_komi         = 6.5;
 int            bot_handicap     = 0;
 int            bot_boardsize    = 19;
 int            bot_time_remaining = 0;   // bot's remaining time (seconds), updated per move
 int            bot_stones_remaining = 0; // stones remaining in current byoyomi period
 QString        bot_opponent;             // opponent's IGS handle
 bool           bot_scoring_pending  = false; // true after scoring trigger, waiting for ownership
 bool           bot_done_sent        = false; // true after done written to socket during scoring phase
 bool           bot_cmd20_received   = false; // true once CMD20 score line received (suppress "9 game completed" fallback)
 bool           bot_pass2_rendered   = false; // true after Pass 2 territory overlay fired (once only)
 bool           bot_overlay_active   = false; // true while KataGo territory overlay is displayed (pass 1 or 2)
 bool           bot_farewell_sent    = false; // true after farewell tell sent (avoid double-send on resign)
 QString        bot_last_sent_vertex;         // GTP vertex of last move sent to IGS (cleared on rejection/end)
 QString        bot_time_forfeit_loser;       // player name from "9 X has run out of time." — used to determine result in Removed game file path
 QStringList               bot_pending_removes;          // IGS coord strings for remove commands (one per group)
 QList<QPair<int,int>>     bot_pending_remove_positions; // board (x,y) seed for markStoneAsDead
 // Full group membership for each pending remove — index matches bot_pending_removes.
 // Used to retry with an alternate stone if the seed coord is rejected by IGS
 // (e.g. opponent already removed the group, leaving the seed as an empty liberty).
 QList<QList<QPair<int,int>>> bot_pending_remove_groups;
 int  bot_remove_index        = 0;    // index into bot_pending_removes currently being sent
 int  bot_remove_retry_offset = 0;    // how many stones in current group we've tried as seed
 QVector<float> bot_cached_ownership;        // ownership prefetched during pass sequence; consumed at scoring
 QVector<float> bot_ownership_snapshot;     // ownership saved when bot sends done; used for progressive territory renders
 int  bot_local_white_score  = -1;    // local territory score at Pass 2 (for CMD20 discrepancy check)
 int  bot_local_black_score  = -1;
 // Canned responses sent at random when a tell arrives during bot mode
 const QStringList bot_tell_responses = {
     "Hello! I am a KataGo bot. Good luck and have fun!",
     "I am an automated bot powered by KataGo. I cannot read tells but wish you a great game!",
     "Hi there! I am a Go bot — I cannot chat, but I enjoy playing. GL HF!",
     "Thanks for the message! I am a bot so I cannot respond personally. Enjoy the game!",
     "Good luck! I am powered by KataGo and will do my best. Feel free to review the game afterwards."
 };

 // Three-clock display (local, GMT, server)
 QLabel *clock_local  = nullptr;
 QLabel *clock_gmt    = nullptr;
 QLabel *clock_server = nullptr;
 QDateTime server_time_value;          // server's wall-clock time at moment of receipt
 QDateTime server_time_received;       // our local UTC time at moment of receipt
 bool     server_time_set = false;     // true once IGS has responded to "time"

public:
 FixedXGospelWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
 setWindowTitle("XGospel Console");
 setMinimumSize(700, 500);
 players_window = nullptr;
 games_window = nullptr;
 shout_window = nullptr;
        pending_match_dialog = nullptr;  // Bug 34: Initialize pending match dialog tracker
 move_parser = new IGSMoveParser();
 current_game_context = -1; // No game context initially
 next_game_is_free = false; // Default to rated until we see match setup
 next_game_komi = 0.0; // Default to server default komi
 my_account_name = ""; // Will be set when we connect
 my_account_rank = "NR"; // Default to unranked
 most_recently_observed_board = nullptr; // Initialize teaching title tracking
 docked_pane_mode = settings->getUseDockedGamePane();
 m_pending_engine_profile_id = settings->getSelectedEngineProfile();
 engine_manager  = new EngineManager(this);
 engine_manager2 = new EngineManager(this);
 waiting_for_players = false;
 waiting_for_games = false;
 auto_launched_windows = false;
 connected_to_igs = false;
 newrating_enabled = false;
 player_count = 0;
 current_kibitz_game = -1;
 game_count = 0;
 tried_fallback_who = false;
 waiting_for_who_supplement = false;  // Bug 35: Initialize WHO supplement tracking
 userlist_player_names.clear();  // Bug 35: Initialize player name tracking set
 pending_kibitz_game = -1;
 suppress_games_console = false;  // Don't suppress games output by default
 suppress_moves_console = false;  // Don't suppress moves output by default
 suppress_server_console = false;  // Don't suppress debug messages by default

 // Initialize heartbeat system
 heartbeat_timer = new QTimer(this);
 heartbeat_counter = 899; // 15 minutes like q5Go
 hold_the_line = false;

 // Initialize auto-reconnect system
 reconnect_timer = new QTimer(this);
 reconnect_timer->setSingleShot(true);
 reconnect_attempts = 0;
 manual_disconnect  = false;
 connect(reconnect_timer, &QTimer::timeout, this, &FixedXGospelWindow::attemptReconnect);

 reconnect_game_file.clear();
 
 // Initialize observer parsing state
 parsing_observers = false;
 observer_game_id = -1;
 waiting_for_observer_response = false;
 observer_request_game_id = -1;
 
 // Initialize manual observe state
 pending_manual_observe = false;
 pending_observe_game_id = -1;
 pending_resume_player.clear();
 
 // Initialize IGS territory data state
 receiving_territory_data = false;
 territory_data_column = 0;
 territory_board = nullptr;
 territory_slot  = nullptr;
 territory_player_name = "";
 cmd22_first_captures = 0;

 // Initialize TCP buffering
 saved_data = nullptr;
 len_saved_data = 0;
 
 socket = new QTcpSocket(this);
 
 connect(socket, &QTcpSocket::connected, this, &FixedXGospelWindow::onConnected);
 connect(socket, &QTcpSocket::disconnected, this, &FixedXGospelWindow::onDisconnected);
 connect(socket, &QTcpSocket::readyRead, this, &FixedXGospelWindow::onDataReceived);
 connect(socket, &QTcpSocket::errorOccurred,
         this, [this](QAbstractSocket::SocketError err) {
             if (!manual_disconnect && reconnect_attempts > 0) {
                 output_console->append(QString(">>> AUTO-RECONNECT: Connection error (%1) — retrying...").arg(socket->errorString()));
                 scheduleReconnect();
             }
         });
 
 // Connect heartbeat timer - 1 second intervals like q5Go
 connect(heartbeat_timer, &QTimer::timeout, this, &FixedXGospelWindow::handleHeartbeat);
 heartbeat_timer->setInterval(1000); // 1 second
 heartbeat_timer->start(); // Start immediately so clocks tick from startup

 setupUI();
 setupMenu();

 // Restore window geometry from settings (xgospel1 .Xdefaults style)
 QRect savedGeometry = settings->loadWindowGeometry("console", QRect(100, 100, 800, 600));
 setGeometry(savedGeometry);

 output_console->append("Welcome to XGospel 2.0 - Fixed Version!");
 output_console->append("*** BUILD: 2026-02-27 v20 Console Suppression ***");
 output_console->append("*** FIX: TIME: messages no longer parsed as pass moves ***");
 output_console->append("Use Connection > Connect to IGS to get started");
 }
 
 ~FixedXGospelWindow() {
 // Clean up TCP buffering
 if (saved_data) {
 delete[] saved_data;
 saved_data = nullptr;
 }
 }

 void closeEvent(QCloseEvent *event) override {
 // Save console window geometry before closing (xgospel1 style)
 settings->saveWindowGeometry("console", geometry());
 settings->save();
 QMainWindow::closeEvent(event);
 }

 void setupUI() {
 // Set xgospel icon (XgospelIcon.xpm from source directory)
 setWindowIcon(QIcon("XgospelIcon.xpm"));

 QWidget *central = new QWidget;
 setCentralWidget(central);

 // HIGH CONTRAST STYLING
 central->setStyleSheet(
 "QWidget { " " " "}"
 );
 
 QVBoxLayout *layout = new QVBoxLayout(central);
 layout->setSpacing(8);
 layout->setMargin(10);
 
 // Status with high contrast
 QLabel *status_label = new QLabel("Status: Not connected");
 status_label->setObjectName("status_label");
 status_label->setStyleSheet(
 "QLabel { " " font-weight: bold; " " " " background- " " padding: 5px; " " border: 1px solid #ccc; " "}"
 );
 layout->addWidget(status_label);

 // Three-clock bar: Local | GMT | Server (xgospel1 style)
 {
     const QString clock_style =
         "QLabel {"
         "  background-color: #000000;"
         "  color: #00FF00;"
         "  font-family: monospace;"
         "  font-size: 11px;"
         "  padding: 3px 8px;"
         "  border: 2px solid #007700;"
         "}";
     QFrame *clock_bar = new QFrame;
     clock_bar->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
     QHBoxLayout *clock_layout = new QHBoxLayout(clock_bar);
     clock_layout->setSpacing(6);
     clock_layout->setMargin(2);

     clock_local  = new QLabel("Local:  --");
     clock_gmt    = new QLabel("GMT:    --");
     clock_server = new QLabel("Server: -- (waiting)");
     clock_local ->setStyleSheet(clock_style);
     clock_gmt   ->setStyleSheet(clock_style);
     clock_server->setStyleSheet(clock_style);
     clock_local ->setAlignment(Qt::AlignCenter);
     clock_gmt   ->setAlignment(Qt::AlignCenter);
     clock_server->setAlignment(Qt::AlignCenter);

     clock_layout->addWidget(clock_local);
     clock_layout->addWidget(clock_gmt);
     clock_layout->addWidget(clock_server);
     layout->addWidget(clock_bar);
 }

 // Output console with HIGH CONTRAST
 QFrame *console_frame = new QFrame;
 console_frame->setFrameStyle(QFrame::Sunken | QFrame::Panel);
 console_frame->setLineWidth(2);
 
 output_console = new QTextEdit;
 output_console->setReadOnly(true);
 output_console->setFont(QFont("monospace", 10));

 // Set console buffer size limit (prevents memory bloat from unlimited growth)
 int buffer_size = settings->getConsoleBufferSize();
 if (buffer_size > 0) {
     output_console->document()->setMaximumBlockCount(buffer_size);
     qDebug() << "[CONSOLE] Buffer size limit set to" << buffer_size << "lines";
 } else {
     qDebug() << "[CONSOLE] Buffer size unlimited (warning: may consume excessive memory)";
 }

 // Green-on-black console styling (xgospel1 style)
 output_console->setStyleSheet(
     "QTextEdit {"
     "    background-color: #000000;"  // Black background
     "    color: #00FF00;"              // Bright green text
     "}"
 );
 
 QVBoxLayout *console_layout = new QVBoxLayout(console_frame);
 console_layout->setMargin(4);
 console_layout->addWidget(output_console);
 layout->addWidget(console_frame);
 
 // Command input with high contrast
 QFrame *command_frame = new QFrame;
 command_frame->setFrameStyle(QFrame::Raised | QFrame::Panel);
 command_frame->setLineWidth(2);
 command_frame->setStyleSheet(
 "QFrame { " " background- " "}"
 );
 
 QHBoxLayout *command_layout = new QHBoxLayout(command_frame);
 
 QLabel *cmd_label = new QLabel("Command:");
 cmd_label->setStyleSheet("QLabel { font-weight: bold; }");
 command_layout->addWidget(cmd_label);
 
 command_text = new QLineEdit;
 command_text->setPlaceholderText("Enter IGS command (e.g., 'who', 'games')");
 command_text->setStyleSheet(
 "QLineEdit { " " " " border: 2px solid #666; " " padding: 5px; " " font-size: 12px; " "}"
 );
 connect(command_text, &QLineEdit::returnPressed, this, &FixedXGospelWindow::sendCommand);
 command_layout->addWidget(command_text);
 
 QPushButton *send_btn = new QPushButton("Send");
 send_btn->setStyleSheet(
 "QPushButton { " " background- " " " " border: none; " " padding: 8px 16px; " " font-weight: bold; " "}" "QPushButton:pressed { " " background- " "}"
 );
 connect(send_btn, &QPushButton::clicked, this, &FixedXGospelWindow::sendCommand);
 command_layout->addWidget(send_btn);

 // Feature 33b: Add "Stats" button to open local player's stats dialog
 QPushButton *stats_btn = new QPushButton("Stats");
 stats_btn->setStyleSheet(
 "QPushButton { " " background-color: #edd20d; " " color: black; " " border: 2px outset #f5e030; " " padding: 8px 16px; " " font-weight: bold; " "}" "QPushButton:pressed { " " background-color: #d4b909; " " border: 2px inset #d4b909; " "}"
 );
 stats_btn->setToolTip("Open your player stats dialog");
 connect(stats_btn, &QPushButton::clicked, this, [this]() {
 if (!login_username.isEmpty()) {
 qDebug() << "[STATS-BTN] Opening stats for local player:" << login_username;
 sendCommandString(QString("stats %1").arg(login_username));
 } else {
 output_console->append(">>> ERROR: Not logged in yet");
 }
 });
 command_layout->addWidget(stats_btn);

 layout->addWidget(command_frame);
 }
 
 void setupMenu() {
 QMenuBar *menu = menuBar();
 menu->setStyleSheet(
 "QMenuBar { " " background- " " " " border-bottom: 1px solid #ccc; " "}" "QMenuBar::item { " " background-color: transparent; " " padding: 5px 10px; " "}" "QMenuBar::item:selected { " " background- " " " "}" "QMenu { " " " " border: 1px solid #ccc; " "}" "QMenu::item:selected { " " background- " " " "}"
 );

 // File menu
 QMenu *file_menu = menu->addMenu("File");
 QAction *open_sgf_action = file_menu->addAction("Open SGF...");
 connect(open_sgf_action, &QAction::triggered, this, &FixedXGospelWindow::openSGF);
 file_menu->addSeparator();
 QAction *exit_action = file_menu->addAction("Exit");
 connect(exit_action, &QAction::triggered, this, []() { QApplication::quit(); });

 connection_menu = menu->addMenu("Connection");
 populateConnectionMenu();

 QMenu *windows_menu = menu->addMenu("Windows");
 QAction *players_action = windows_menu->addAction("Show Players");
 connect(players_action, &QAction::triggered, this, &FixedXGospelWindow::showPlayersWindow);

 QAction *games_action = windows_menu->addAction("Show Games");
 connect(games_action, &QAction::triggered, this, &FixedXGospelWindow::showGamesWindow);

 QAction *shouts_action = windows_menu->addAction("Show Shouts");
 connect(shouts_action, &QAction::triggered, this, [this]() {
     if (!shout_window) return;
     shout_window->showAndRaise();
 });

 // Console menu with suppression options
 QMenu *console_menu = menu->addMenu("Console");

 QAction *save_console_action = console_menu->addAction("Save Console to File...");
 connect(save_console_action, &QAction::triggered, this, [this]() {
     QString dumpDir = settings->getConsoleDumpDirectory();
     QString defaultName = dumpDir + "/xgospel2_console_dump_" +
         XGOSPEL_VERSION + "_" +
         QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".txt";
     QString path = QFileDialog::getSaveFileName(this, "Save Console",
         defaultName, "Text files (*.txt);;All files (*)");
     if (path.isEmpty()) return;
     QFile f(path);
     if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
         output_console->append(QString("[Console] ERROR: could not open %1 for writing").arg(path));
         return;
     }
     QTextStream out(&f);
     out << output_console->toPlainText();
     f.close();
     output_console->append(QString("[Console] Saved to %1").arg(path));
 });

 console_menu->addSeparator();

 // Add "Suppress All Output" option first
 QAction *suppress_all_action = console_menu->addAction("Suppress All Output");
 suppress_all_action->setCheckable(true);
 suppress_all_action->setChecked(false);

 console_menu->addSeparator();

 QAction *suppress_games_action = console_menu->addAction("Suppress Games Output");
 suppress_games_action->setCheckable(true);
 suppress_games_action->setChecked(false);
 connect(suppress_games_action, &QAction::toggled, this, [this](bool checked) {
  suppress_games_console = checked;
  output_console->append(QString("[Console] Games output %1").arg(checked ? "suppressed" : "enabled"));
 });

 QAction *suppress_moves_action = console_menu->addAction("Suppress Moves Output");
 suppress_moves_action->setCheckable(true);
 suppress_moves_action->setChecked(false);
 connect(suppress_moves_action, &QAction::toggled, this, [this](bool checked) {
  suppress_moves_console = checked;
  output_console->append(QString("[Console] Moves output %1").arg(checked ? "suppressed" : "enabled"));
 });

 QAction *suppress_server_action = console_menu->addAction("Suppress Debug Output");
 suppress_server_action->setCheckable(true);
 suppress_server_action->setChecked(false);
 connect(suppress_server_action, &QAction::toggled, this, [this](bool checked) {
  suppress_server_console = checked;
  output_console->append(QString("[Console] Debug output %1").arg(checked ? "suppressed" : "enabled"));
 });

 // Connect "Suppress All" to control all three checkboxes
 connect(suppress_all_action, &QAction::toggled, this, [this, suppress_games_action, suppress_moves_action, suppress_server_action](bool checked) {
  suppress_games_action->setChecked(checked);
  suppress_moves_action->setChecked(checked);
  suppress_server_action->setChecked(checked);
  output_console->append(QString("[Console] All output %1").arg(checked ? "suppressed" : "enabled"));
 });

 QMenu *settings_menu = menu->addMenu("Settings");
 QAction *preferences_action = settings_menu->addAction("Preferences...");
 connect(preferences_action, &QAction::triggered, this, &FixedXGospelWindow::showPreferences);

 // Engines menu
 engines_menu = menu->addMenu("Engines");
 engines_menu->addAction("Manage Engines...", this, &FixedXGospelWindow::openEnginesPrefs);
 engines_menu->addSeparator();
 attach_submenu = engines_menu->addMenu("Attach Engine");
 engines_menu->addAction("Detach Engine", this, &FixedXGospelWindow::detachEngine);
 engines_menu->addSeparator();
 engines_menu->addAction("Play vs Engine...", this, &FixedXGospelWindow::launchLocalEngineGame);
 engines_menu->addAction("Engine vs Engine...", this, &FixedXGospelWindow::launchEngineVsEngine);
 engines_menu->addSeparator();
 bot_mode_action = engines_menu->addAction("Bot Mode (IGS)");
 bot_mode_action->setCheckable(true);
 bot_mode_action->setChecked(settings->getBotModeEnabled());
 connect(bot_mode_action, &QAction::toggled, this, &FixedXGospelWindow::toggleBotMode);
 connect(engines_menu, &QMenu::aboutToShow, this, &FixedXGospelWindow::populateAttachSubmenu);

 // Help menu with About dialog
 QMenu *help_menu = menu->addMenu("Help");
 QAction *about_action = help_menu->addAction("About XGospel...");
 connect(about_action, &QAction::triggered, this, &FixedXGospelWindow::showAboutDialog);
 }

 void populateConnectionMenu() {
 if (!connection_menu) return;

 // Clear existing actions
 connection_menu->clear();

 // Load saved accounts from settings and create menu items
 std::vector<Host> hosts = settings->getHosts();

 if (!hosts.empty()) {
 for (const Host &host : hosts) {
 QAction *action = connection_menu->addAction(QString("Connect to %1 (%2)")
 .arg(host.title, host.host));
 connect(action, &QAction::triggered, this, [this, host]() {
 login_username = host.login_name;
 login_password = host.password;
 connectToIGS();
 });
 }
 connection_menu->addSeparator();
 }

 // Add guest login option
 QAction *connect_guest_action = connection_menu->addAction("Connect as Guest");
 connect(connect_guest_action, &QAction::triggered, this, &FixedXGospelWindow::connectAsGuest);

 connection_menu->addSeparator();

 // Add custom credentials option
 QAction *connect_custom_action = connection_menu->addAction("Connect with Custom Credentials...");
 connect(connect_custom_action, &QAction::triggered, this, &FixedXGospelWindow::connectCustom);

 connection_menu->addSeparator();

 // Add disconnect option
 QAction *disconnect_action = connection_menu->addAction("Disconnect");
 connect(disconnect_action, &QAction::triggered, this, &FixedXGospelWindow::disconnectFromIGS);
 }

 void openSGF() {
 QString filename = QFileDialog::getOpenFileName(this,
 "Open SGF File",
 QDir::homePath(),
 "SGF Files (*.sgf);;All Files (*)");

 if (filename.isEmpty()) {
 return; // User cancelled
 }

 // Parse the SGF file
 SGFParser parser;
 QString error;
 GameNode* root = parser.parseFile(filename, error);

 if (!root) {
 QMessageBox::warning(this, "SGF Parse Error",
 QString("Failed to parse SGF file:\n%1").arg(error));
 return;
 }

 // Create a new board window for the SGF
 BoardWindow* board = new BoardWindow(this, login_username);
 connect(board, &BoardWindow::boardClosed, this, &FixedXGospelWindow::closeBoardWindow);
 board_windows.append(board);

 // Set up the board with SGF data
 board->loadSGF(root, parser.getPlayerWhite(), parser.getPlayerBlack(),
 parser.getWhiteRank(), parser.getBlackRank(),
 parser.getKomi(), parser.getHandicap(),
 parser.getGameResult(), filename,
 parser.getGameName());

 board->show();
 board->raise();
 board->activateWindow();

 if (!this->suppress_server_console) output_console->append(QString(">>> Opened SGF: %1 - %2 vs %3")
 .arg(QFileInfo(filename).fileName())
 .arg(parser.getPlayerWhite())
 .arg(parser.getPlayerBlack()));
 }

private slots:
 void connectAsWireless() {
 login_username = "wireless";
 login_password = "andrew1";
 connectToIGS();
 }
 
 void connectAsWeakkyu() {
 login_username = "weakkyu";
 login_password = "andrew1";
 connectToIGS();
 }
 
 void connectAsGuest() {
 login_username = "guest";
 login_password = "";
 connectToIGS();
 }
 
 void connectCustom() {
 bool ok;
 QString username = QInputDialog::getText(this, "Custom Login", 
 "Enter IGS username:", QLineEdit::Normal, 
 "guest", &ok);
 if (!ok || username.isEmpty()) return;
 
 QString password = "";
 if (username != "guest") {
 password = QInputDialog::getText(this, "Custom Login", 
 "Enter password:", QLineEdit::Password, 
 "", &ok);
 if (!ok) return;
 }
 
 login_username = username;
 login_password = password;
 connectToIGS();
 }
 
 void connectToIGS() {
 if (socket->state() != QTcpSocket::UnconnectedState) {
 output_console->append("Already connected or connecting...");
 return;
 }
 
 if (!this->suppress_server_console) output_console->append(QString(">>> Connecting to IGS server as '%1'...").arg(login_username));
 updateStatus("Connecting...");
 socket->connectToHost("igs.joyjoy.net", 6969);
 }
 
 void disconnectFromIGS() {
     manual_disconnect = true;
     reconnect_timer->stop();
     reconnect_attempts = 0;
     if (socket->state() == QTcpSocket::ConnectedState)
         socket->disconnectFromHost();
 }
 
 void onConnected() {
 if (!this->suppress_server_console) output_console->append(">>> CONNECTED to IGS successfully!");
 updateStatus(QString("Connected - logging in as %1").arg(login_username));
 connected_to_igs = true;
 reconnect_timer->stop();
 reconnect_attempts = 0;
 manual_disconnect = false;
 // Disable Nagle algorithm so small move packets are sent immediately
 socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

 // Set account name for match dialogs
 my_account_name = login_username;

 // Activate keep-alive logic
 hold_the_line = true;
 heartbeat_counter = 899; // Reset to 15 minutes
 if (!this->suppress_server_console) output_console->append(">>> Heartbeat system started");
 
 // Auto-login with selected credentials
 QTimer::singleShot(1000, this, [this]() {
 socket->write((login_username + "\n").toUtf8());
 if (!this->suppress_server_console) output_console->append(QString(">>> SENT: %1 (username)").arg(login_username));
 });
 
 // Send password after username (only if not guest)
 if (!login_password.isEmpty()) {
 QTimer::singleShot(2000, this, [this]() {
 socket->write((login_password + "\n").toUtf8());
 if (!this->suppress_server_console) output_console->append(">>> SENT: password (authentication)");
 });
 }
 }
 
 void onDisconnected() {
     output_console->append(">>> DISCONNECTED from IGS");
     updateStatus("Not connected");
     connected_to_igs = false;
     auto_launched_windows = false;
     waiting_for_players = false;
     waiting_for_games = false;

     // Stop heartbeat system
     hold_the_line = false;
     heartbeat_timer->stop();
     output_console->append(">>> Heartbeat system stopped");

     if (manual_disconnect) {
         // User-initiated disconnect — do not auto-reconnect
         manual_disconnect = false;
         reconnect_attempts = 0;
         reconnect_opponent.clear();
         return;
     }

     // Unexpected disconnect — save bot game opponent for post-reconnect load command
     if (bot_mode_active && bot_game_id != -1 && !bot_opponent.isEmpty())
         reconnect_opponent = bot_opponent;
     else
         reconnect_opponent.clear();

     // Schedule first reconnect attempt after 5 seconds
     reconnect_attempts = 0;
     scheduleReconnect();
 }

 void scheduleReconnect() {
     // Back-off: 5s, 10s, 20s, 30s cap. Give up after 10 attempts (~3.5 min total).
     const int MAX_ATTEMPTS = 10;
     if (reconnect_attempts >= MAX_ATTEMPTS) {
         output_console->append(QString(">>> AUTO-RECONNECT: Giving up after %1 attempts — please reconnect manually").arg(MAX_ATTEMPTS));
         reconnect_attempts = 0;
         reconnect_opponent.clear();
         return;
     }
     int delay_s = qMin(5 * (1 << qMin(reconnect_attempts, 3)), 30); // 5,10,20,30,30,...
     output_console->append(QString(">>> AUTO-RECONNECT: Attempt %1/%2 in %3s...")
         .arg(reconnect_attempts + 1).arg(MAX_ATTEMPTS).arg(delay_s));
     reconnect_timer->start(delay_s * 1000);
 }

 void attemptReconnect() {
     if (connected_to_igs) return; // already reconnected (e.g. user did it manually)
     if (socket->state() != QTcpSocket::UnconnectedState) {
         socket->abort(); // force immediate close — don't wait for graceful shutdown
     }
     reconnect_attempts++;
     output_console->append(QString(">>> AUTO-RECONNECT: Connecting (attempt %1)...").arg(reconnect_attempts));
     socket->connectToHost("igs.joyjoy.net", 6969);
     // onConnected fires on success → login sequence runs automatically.
     // If connection fails, QTcpSocket emits errorOccurred → we schedule next attempt.
 }
 
 void onDataReceived() {
 // Read all complete lines from the socket.
 // Use QTcpSocket::readLine() returning QByteArray (no size limit) to avoid
 // truncation bugs that dropped moves when bytesAvailable() was computed before
 // the full line arrived in the kernel buffer.
 while (socket->canReadLine()) {
 QByteArray raw = socket->readLine();
 QString line = QString::fromUtf8(raw).trimmed();
 if (line.isEmpty()) continue;

 // Parse stats command response (Command 9)
 static QString stats_player_name;
 static int stats_wins = 0;
 static int stats_losses = 0;
 static QString stats_rated;
 static QString stats_rank;
 static QString stats_country;
 static QString stats_last_log;
 static QString stats_observing;
 static QString stats_playing;
 static QString stats_match_prefs;
 static QString stats_info;

 // Debug: catch ALL Command 9 lines
 // Feature 33a: Always show Command 9 in console (never suppress)
 if (line.startsWith("9 ")) {
 qDebug() << "[STATS-DEBUG] *** RAW Command 9 line:" << line;
 output_console->append(QString("< %1").arg(line));
 }

 // Feature 33c: Parse toggle state responses from server
 // IGS format: "9 Set looking to be True." or "9 Set looking to be False."
 if (line.startsWith("9 Set looking to be ")) {
 bool is_on = line.contains("True");
 qDebug() << "[TOGGLE-PARSE] Looking state:" << (is_on ? "ON" : "OFF");
 if (players_window) {
 QString logged_in_user = login_username;
 for (PlayerStatsDialog *dialog : players_window->getOpenDialogs()) {
 if (dialog->getPlayerName().compare(logged_in_user, Qt::CaseInsensitive) == 0) {
 dialog->updateToggleState("looking", is_on);
 break;
 }
 }
 }
 }
 if (line.startsWith("9 Set open to be ")) {
 bool is_on = line.contains("True");
 qDebug() << "[TOGGLE-PARSE] Open state:" << (is_on ? "ON" : "OFF");
 if (players_window) {
 QString logged_in_user = login_username;
 for (PlayerStatsDialog *dialog : players_window->getOpenDialogs()) {
 if (dialog->getPlayerName().compare(logged_in_user, Qt::CaseInsensitive) == 0) {
 dialog->updateToggleState("open", is_on);
 break;
 }
 }
 }
 }
 if (line.startsWith("9 Set quiet to be ")) {
 bool is_on = line.contains("True");
 qDebug() << "[TOGGLE-PARSE] Quiet state:" << (is_on ? "ON" : "OFF");
 if (players_window) {
 QString logged_in_user = login_username;
 for (PlayerStatsDialog *dialog : players_window->getOpenDialogs()) {
 if (dialog->getPlayerName().compare(logged_in_user, Qt::CaseInsensitive) == 0) {
 dialog->updateToggleState("quiet", is_on);
 break;
 }
 }
 }
 }
 if (line.startsWith("9 Set shout to be ")) {
 bool is_on = line.contains("True");
 qDebug() << "[TOGGLE-PARSE] Shout state:" << (is_on ? "ON" : "OFF");
 if (players_window) {
 QString logged_in_user = login_username;
 for (PlayerStatsDialog *dialog : players_window->getOpenDialogs()) {
 if (dialog->getPlayerName().compare(logged_in_user, Qt::CaseInsensitive) == 0) {
 dialog->updateToggleState("shout", is_on);
 break;
 }
 }
 }
 }

 // OLD Command 22 handler REMOVED - was causing multi-game contamination
 // by assigning territory data to first observing board instead of matching by player names.
 // The NEW Command 22 handler with player name matching is below (around line 3563).

 // IGS Command 9 territory data parsing (after 3 passes in scoring mode)
 // Format: "9 <19 digits>" where each digit is 0-5 representing board state
 // 0=black stone, 1=white stone, 2=free, 3=neutral, 4=white territory, 5=black territory
 // DISABLED: Command 9 territory handler - REDUNDANT and causes multi-game contamination
 // This handler assigned territory to first observing board without player name matching.
 // Territory data is correctly handled by Command 22 handler (lines 3526-3640) which
 // uses player name matching like q5Go does. q5Go does NOT handle territory in Command 9.
 // Disabled in v15 based on q5Go source code analysis.
 /*
 if (line.startsWith("9 ") && line.length() == 21) { // "9 " + 19 digits
 QString data_part = line.mid(2); // Strip "9 " prefix
 bool is_territory_line = true;

 // Verify all 19 characters are digits 0-5
 for (int i = 0; i < data_part.length() && i < 19; i++) {
 QChar c = data_part[i];
 if (!c.isDigit() || c.digitValue() < 0 || c.digitValue() > 5) {
 is_territory_line = false;
 break;
 }
 }

 if (is_territory_line) {
 qDebug() << "*** IGS TERRITORY LINE detected:" << data_part;

 // Forward to the appropriate board window
 if (territory_board != nullptr) {
 if (!receiving_territory_data) {
 // First territory line - initialize
 qDebug() << "*** IGS TERRITORY: Starting territory data reception";
 territory_board->receiveScoreBegin();
 receiving_territory_data = true;
 territory_data_column = 0;
 }

 // Send this row of territory data
 territory_board->receiveScoreLine(territory_data_column, data_part);
 territory_data_column++;

 // If we've received all 19 rows, finalize
 if (territory_data_column >= 19) {
 qDebug() << "*** IGS TERRITORY: Received all 19 rows, finalizing";
 territory_board->receiveScoreEnd();
 receiving_territory_data = false;
 territory_data_column = 0;
 territory_board = nullptr;
 }
 } else {
 // Try to find the active observed board
 for (BoardWindow* board : board_windows) {
 if (board->isObserving()) {
 territory_board = board;
 qDebug() << "*** IGS TERRITORY: Auto-detected board for game" << board->getObservedGameId();
 territory_board->receiveScoreBegin();
 receiving_territory_data = true;
 territory_data_column = 0;
 territory_board->receiveScoreLine(territory_data_column, data_part);
 territory_data_column++;
 break;
 }
 }

 if (territory_board == nullptr) {
 qDebug() << "*** IGS TERRITORY: WARNING - No active board found for territory data";
 }
 }

 continue; // Skip further processing of this line
 }
 }
 */

 if (line.startsWith("9 Player:")) {
 qDebug() << "[STATS-DEBUG] Found Player line:" << line;
 // Start of stats response - extract player name
 QRegExp stats_player_re("9 Player:\\s+(\\S+)");
 if (stats_player_re.indexIn(line) != -1) {
 stats_player_name = stats_player_re.cap(1);
 stats_wins = 0;
 stats_losses = 0;
 stats_rated.clear();
 stats_rank.clear();
 stats_country.clear();
 stats_last_log.clear();
 stats_observing.clear();
 stats_playing.clear();
 stats_match_prefs.clear();
 stats_info.clear();
 qDebug() << "[STATS] Parsing stats for player:" << stats_player_name;
 } else {
 qDebug() << "[STATS-DEBUG] Player regex FAILED to match:" << line;
 }
 } else if (line.startsWith("9 Rank:")) {
 qDebug() << "[STATS-DEBUG] Found Rank line:" << line;
 // Format: "9 Rank: 8d+ 60" - extract just the rank part (first token after "Rank:")
 QRegExp rank_re("9 Rank:\\s+(\\S+)");
 if (rank_re.indexIn(line) != -1) {
 stats_rank = rank_re.cap(1).trimmed();
 qDebug() << "[STATS-DEBUG] Parsed rank:" << stats_rank;
 } else {
 qDebug() << "[STATS-DEBUG] Rank regex FAILED to match:" << line;
 }
 } else if (line.startsWith("9 Country:")) {
 qDebug() << "[STATS-DEBUG] Found Country line:" << line;
 // Format: "9 Country: USA" - extract country name
 QRegExp country_re("9 Country:\\s+(.+)$");
 if (country_re.indexIn(line) != -1) {
 stats_country = country_re.cap(1).trimmed();
 qDebug() << "[STATS-DEBUG] Parsed country:" << stats_country;
 } else {
 qDebug() << "[STATS-DEBUG] Country regex FAILED to match:" << line;
 }
 } else if (line.startsWith("9 Last Access(local):")) {
 qDebug() << "[STATS-DEBUG] Found Last Access line:" << line;
 // Format: "9 Last Access(local): (Not on) Sun Dec 21 14:22:49 2025" - extract entire status/timestamp
 QRegExp lastlog_re("9 Last Access\\(local\\):\\s+(.+)$");
 if (lastlog_re.indexIn(line) != -1) {
 stats_last_log = lastlog_re.cap(1).trimmed();
 qDebug() << "[STATS-DEBUG] Parsed last access:" << stats_last_log;
 } else {
 qDebug() << "[STATS-DEBUG] Last Access regex FAILED to match:" << line;
 }
 } else if (line.startsWith("9 Wins:")) {
 qDebug() << "[STATS-DEBUG] Found Wins line:" << line;
 QRegExp wins_re("9 Wins:\\s+(\\d+)");
 if (wins_re.indexIn(line) != -1) {
 stats_wins = wins_re.cap(1).toInt();
 qDebug() << "[STATS-DEBUG] Parsed wins:" << stats_wins;
 } else {
 qDebug() << "[STATS-DEBUG] Wins regex FAILED to match:" << line;
 }
 } else if (line.startsWith("9 Rated Games:")) {
 qDebug() << "[STATS-DEBUG] Found Rated Games line:" << line;
 QRegExp rated_re("9 Rated Games:\\s+(\\d+)");
 if (rated_re.indexIn(line) != -1) {
 stats_rated = rated_re.cap(1);
 qDebug() << "[STATS-DEBUG] Parsed rated:" << stats_rated;
 } else {
 qDebug() << "[STATS-DEBUG] Rated games regex FAILED to match:" << line;
 }
 } else if (line.startsWith("9 Losses:")) {
 qDebug() << "[STATS-DEBUG] Found Losses line:" << line;
 QRegExp losses_re("9 Losses:\\s+(\\d+)");
 if (losses_re.indexIn(line) != -1) {
 stats_losses = losses_re.cap(1).toInt();
 qDebug() << "[STATS-DEBUG] Parsed losses:" << stats_losses;
 } else {
 qDebug() << "[STATS-DEBUG] Losses regex FAILED to match:" << line;
 }
 } else if (line.startsWith("9 Playing in game:")) {
 qDebug() << "[STATS-DEBUG] Found Playing in game line:" << line;
 // Format: "9 Playing in game: 168 (I)" - extract game number
 QRegExp play_re("9 Playing in game:\\s+(\\d+)");
 if (play_re.indexIn(line) != -1) {
 stats_playing = play_re.cap(1);
 qDebug() << "[STATS-DEBUG] Parsed playing game:" << stats_playing;
 } else {
 qDebug() << "[STATS-DEBUG] Playing in game regex FAILED to match:" << line;
 }
 } else if (line.startsWith("9 Observing game:")) {
 qDebug() << "[STATS-DEBUG] Found Observing game line:" << line;
 // Format: "9 Observing game: 672 (1)" - extract game number
 QRegExp obs_re("9 Observing game:\\s+(\\d+)");
 if (obs_re.indexIn(line) != -1) {
 stats_observing = obs_re.cap(1);
 qDebug() << "[STATS-DEBUG] Parsed observing game:" << stats_observing;
 } else {
 qDebug() << "[STATS-DEBUG] Observing game regex FAILED to match:" << line;
 }
 } else if (line.startsWith("9 Info:") || line.startsWith("< 9 Info:")) {
 qDebug() << "[STATS-DEBUG] Found Info line:" << line;
 // Format: "< 9 Info: <world "IGO" player more than 1d" or "9 Info: <world..."
 // Extract everything after "9 Info:" (with or without "< " prefix)
 // First strip "< " prefix if present
 QString info_line = line;
 if (info_line.startsWith("< ")) {
 info_line = info_line.mid(2);
 }
 // Now extract the info text after "9 Info:"
 QRegExp info_re("9 Info:\\s+(.+)$");
 if (info_re.indexIn(info_line) != -1) {
 stats_info = info_re.cap(1).trimmed();
 qDebug() << "[STATS-DEBUG] Parsed info:" << stats_info;
 } else {
 qDebug() << "[STATS-DEBUG] Info regex FAILED to match:" << info_line;
 }
 } else if (line.contains("Verbose") && line.contains("Bell") && line.contains("Quiet")) {
 // Feature 33c: Parse verbose settings header line (for toggle state detection)
 // Format: "9 Verbose   Bell  Quiet  Shout  Automail  Open  Looking  Client  Kibitz  Chatter"
 // This is just the header - actual values come in next line starting with "9    "
 qDebug() << "[STATS-DEBUG] Found Verbose settings header:" << line;
 } else if (line.startsWith("9    ") || line.startsWith("< 9    ")) {
 // Feature 33c: Parse verbose settings values line
 // Format: "< 9      Off   Off    On     On       Off    Off       On      On      On     On"
 // Columns: Verbose Bell Quiet Shout Automail Open Looking Client Kibitz Chatter
 QString settings_line = line;
 if (settings_line.startsWith("< ")) {
 settings_line = settings_line.mid(2);  // Remove "< " prefix
 }
 settings_line = settings_line.mid(2).trimmed();  // Remove "9 " and trim

 QStringList values = settings_line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
 if (values.size() >= 7) {  // Need at least 7 values to get Looking
 // Indices: 0=Verbose 1=Bell 2=Quiet 3=Shout 4=Automail 5=Open 6=Looking
 bool quiet_on = (values[2].compare("On", Qt::CaseInsensitive) == 0);
 bool shout_on = (values[3].compare("On", Qt::CaseInsensitive) == 0);
 bool open_on = (values[5].compare("On", Qt::CaseInsensitive) == 0);
 bool looking_on = (values[6].compare("On", Qt::CaseInsensitive) == 0);

 qDebug() << "[TOGGLE-INIT] Parsed initial states - Looking:" << looking_on
          << "Open:" << open_on << "Quiet:" << quiet_on << "Shout:" << shout_on;

 // Update dialog if it exists for this player
 if (players_window && !stats_player_name.isEmpty()) {
 for (PlayerStatsDialog *dialog : players_window->getOpenDialogs()) {
 if (dialog->getPlayerName().compare(stats_player_name, Qt::CaseInsensitive) == 0) {
 dialog->updateToggleState("looking", looking_on);
 dialog->updateToggleState("open", open_on);
 dialog->updateToggleState("quiet", quiet_on);
 dialog->updateToggleState("shout", shout_on);
 qDebug() << "[TOGGLE-INIT] Updated dialog toggle states for" << stats_player_name;
 break;
 }
 }
 }
 }
 } else if (line.startsWith("9 Defaults")) {
 // "Defaults" is the last line - parse match preferences and update dialogs
 qDebug() << "[STATS-DEBUG] Found Defaults line (end of stats):" << line;

 // Parse match preferences from Defaults line
 // Format: "9 Defaults (help defs): time 90, size 19, byo-yomi time 10, byo-yomi stones 25"
 QRegExp defaults_re("9 Defaults \\(help defs\\):\\s+(.+)$");
 if (defaults_re.indexIn(line) != -1) {
 stats_match_prefs = defaults_re.cap(1).trimmed();
 qDebug() << "[STATS-DEBUG] Parsed match prefs:" << stats_match_prefs;
 }

 if (players_window && !stats_player_name.isEmpty() && !stats_rank.isEmpty()) {
     players_window->upsertPlayerRank(stats_player_name, stats_rank);
     // Refresh the bot board if this rank arrived for the current bot opponent
     if (bot_mode_active && bot_game_id != -1 &&
         stats_player_name.compare(bot_opponent, Qt::CaseInsensitive) == 0 &&
         engine_board) {
         bool is_white_opp = (bot_color == BLACK_STONE);
         if (docked_pane_mode) {
             if (GameSlot *bslot = findSlot(bot_game_id)) {
                 if (is_white_opp) bslot->white_rank = stats_rank;
                 else              bslot->black_rank = stats_rank;
                 // Refresh the dock button now that we have the real rank
                 if (shared_board_window) {
                     if (GameSelectionDock *dock2 = shared_board_window->getGameSelectionDock())
                         dock2->updateGameRanks(bot_game_id, bslot->black_rank, bslot->white_rank);
                 }
             }
         }
         if (is_white_opp) engine_board->setWhiteRank(stats_rank);
         else              engine_board->setBlackRank(stats_rank);
     }
 }

 if (players_window && !stats_player_name.isEmpty() && !stats_rated.isEmpty()) {
 qDebug() << "[STATS] Got complete stats:" << stats_player_name
 << "Wins:" << stats_wins << "Losses:" << stats_losses
 << "Rated:" << stats_rated << "Playing:" << stats_playing << "Observing:" << stats_observing
 << "Match Prefs:" << stats_match_prefs;

 qDebug() << "[STATS-DEBUG] Number of open dialogs:" << players_window->getOpenDialogs().size();

 // Check if dialog already exists for this player
 bool dialog_found = false;
 for (PlayerStatsDialog *dialog : players_window->getOpenDialogs()) {
 qDebug() << "[STATS-DEBUG] Checking dialog for player:" << dialog->getPlayerName()
 << "against stats player:" << stats_player_name;
 if (dialog->getPlayerName().compare(stats_player_name, Qt::CaseInsensitive) == 0) {
 qDebug() << "[STATS] Calling updateStatsData for" << stats_player_name;
 dialog->updateStatsData(stats_wins, stats_losses, stats_rated, stats_observing, stats_playing, stats_match_prefs, stats_info, stats_rank, stats_country, stats_last_log);
 qDebug() << "[STATS] Updated dialog for" << stats_player_name;
 dialog_found = true;
 break; // Only update first matching dialog
 }
 }

 // If no dialog exists, create a new one (for console stats commands)
 // This handles the case where user types "stats <playername>" in console
 if (!dialog_found) {
 qDebug() << "[STATS] No existing dialog found, creating new one for" << stats_player_name;
 // Since we're creating a dialog from a console stats command,
 // we just call openStatsDialogForPlayer which will:
 // 1. Search for player in the players list to get rank/country
 // 2. Create the dialog
 // 3. The dialog will then be updated with stats data we just parsed
 players_window->openStatsDialogForPlayer(stats_player_name);
 qDebug() << "[STATS] Triggered openStatsDialogForPlayer for" << stats_player_name;
 }
 } else {
 qDebug() << "[STATS-DEBUG] Can't update - players_window:" << (players_window ? "exists" : "NULL")
 << "stats_player_name empty:" << stats_player_name.isEmpty()
 << "stats_rated empty:" << stats_rated.isEmpty();
 }
 }

 // Auto-launch Players and Games windows on first command after login
 bool is_command_line = (line.startsWith("1 ") || line.startsWith("2 ") ||
 line.startsWith("3 ") || line.startsWith("4 ") ||
 line.startsWith("5 ") || line.startsWith("7 ") ||
 line.startsWith("11 ") || line.startsWith("19 ") ||
 line.startsWith("24 "));

 if (!auto_launched_windows && is_command_line) {
 // First command received after login - auto-launch windows minimized
 auto_launched_windows = true;
 auto_launch_populate_pending = true; // Flag to populate games window from Command 7 buffer
 QTimer::singleShot(500, this, &FixedXGospelWindow::autoLaunchWindowsMinimized);
 }

 // Capture ALL responses when waiting for observer data
 if (waiting_for_observer_response) {
 // if (!this->suppress_server_console) output_console->append(QString(">>> OBS_DEBUG: %1").arg(line.trimmed()));
 
 // Look for any lines that might contain observer information
 if (line.contains("observ", Qt::CaseInsensitive) || 
 line.contains("watch", Qt::CaseInsensitive) ||
 line.contains("kibitz", Qt::CaseInsensitive) ||
 line.contains("spectator", Qt::CaseInsensitive)) {
 if (!this->suppress_server_console) output_console->append(QString(">>> POTENTIAL OBSERVER INFO: %1").arg(line.trimmed()));
 }
 
 // Look for any Command responses that might be related
 if (line.startsWith("7 ") || line.startsWith("8 ") || line.startsWith("9 ") || 
 line.startsWith("16 ") || line.startsWith("20 ")) {
 if (!this->suppress_server_console) output_console->append(QString(">>> COMMAND RESPONSE: %1").arg(line.trimmed()));
 }
 }
 
 // DEBUG: Log any line containing "observ" to understand IGS responses
 if (line.toLower().contains("observ")) {
 // if (!this->suppress_server_console) output_console->append(QString(">>> DEBUG ALL OBSERVE: %1").arg(line));
 }
 
 // Handle "9 Adding game to observation list" confirmation
 if (line.contains("Adding game to observation list")) {
     if (!this->suppress_server_console) output_console->append(QString(">>> DEBUG: IGS confirmed observation, waiting for game info..."));
     // Enqueue all pending games — we will dispatch them one at a time to avoid
     // interleaved "moves N" history replies corrupting each other's board state.
     for (int pending_id : games_pending_moves_request) {
         if (findSlot(pending_id) && !moves_dispatch_queue.contains(pending_id))
             moves_dispatch_queue.append(pending_id);
     }
     games_pending_moves_request.clear();
     qDebug() << "[MOVES-QUEUE] enqueued" << moves_dispatch_queue.size() << "games for serialized moves N dispatch";
     // Dispatch the first one immediately if no "moves N" reply is currently
     // being received. REPLAYING means a reply is in-flight; WAITING_FOR_MOVES0
     // just means the request hasn't been sent yet — not a blocking condition.
     bool any_inflight = false;
     for (GameSlot *s : game_slots) {
         if (s->replay_state == GameSlot::REPLAYING) {
             any_inflight = true;
             break;
         }
     }
     if (!any_inflight && !moves_dispatch_queue.isEmpty()) {
         int next_id = moves_dispatch_queue.takeFirst();
         if (GameSlot *slot = findSlot(next_id)) {
             games_with_moves_requested.insert(next_id);
             if (next_id == active_slot_game_id && shared_board_window) {
                 shared_board_window->clearMoveHistoryBeforeMovesCommand();
                 slot->game_root    = shared_board_window->getGameRoot();
                 slot->current_node = shared_board_window->getGameRoot();
                 slot->move_history.clear();
             }
             QString moves_cmd = QString("moves %1").arg(next_id);
             socket->write((moves_cmd + "\n").toUtf8());
             if (!suppress_server_console)
                 output_console->append(QString(">>> SENT: %1 (serialized moves N dispatch)").arg(moves_cmd));
             qDebug() << "[MOVES-QUEUE] dispatched moves" << next_id << "-> WAITING_FOR_MOVES0";
         }
     }
 }
 
 // IGS Command 9 Observer Parsing (based on q5Go protocol)
 // Pattern: "9 Observing game 89 (player1 vs. player2) :"
 // Skip observer-list refreshes for the bot's own game during scoring — the auto-refresh
 // timer can fire mid-scoring and the resulting OBSERVE-MATCH update corrupts the board display.
 if (line.startsWith("9 Observing game ") && line.contains(" (") && line.contains(") :") &&
     !(bot_mode_active && bot_done_sent && line.contains(QString("game %1 ").arg(bot_game_id)))) {
 if (!this->suppress_server_console) output_console->append(QString(">>> DEBUG: Found potential observe response: %1").arg(line));
 
 // Try to extract player names using parseObserveStart
 int game_id;
 QString white_player, black_player;
 
 if (move_parser->parseObserveStart(line, game_id, white_player, black_player)) {
 if (!this->suppress_server_console) output_console->append(QString(">>> EXTRACTED PLAYER NAMES: Game %1 - White: %2, Black: %3")
 .arg(game_id).arg(white_player).arg(black_player));
 
 // If this is a response to manual observe command, create board window now
 if (pending_manual_observe && game_id == pending_observe_game_id) {
 if (!this->suppress_server_console) output_console->append(QString(">>> MANUAL OBSERVE: Creating board window for game %1").arg(game_id));
 if (!this->suppress_server_console) output_console->append(QString(">>> DEBUG: About to call observeGame(%1, %2, %3, ?, ?)")
 .arg(game_id).arg(white_player).arg(black_player));
 observeGame(game_id, white_player, black_player, "?", "?");
 pending_manual_observe = false;
 pending_observe_game_id = -1;
 if (!this->suppress_server_console) output_console->append(QString(">>> DEBUG: observeGame call completed, cleared pending flags"));
 } else {
 // Update existing board window with correct player names.
 // Guard against IGS reusing a game ID: if findSlot returns a finished slot,
 // the ID has been recycled and we must not overwrite the old game's players.
 if (docked_pane_mode) {
     // Check if this is a resumed adjourned game arriving under a new game ID.
     // Two match paths:
     // (a) Normal: slot still has original player names + adjourned_player set
     // (b) Fallback: slot ID was recycled and names overwritten, but adjourned_player
     //     still names the reconnecting player (pending_resume_player confirms it)
     output_console->append(QString(">>> OBSERVE-MATCH: game=%1 w=%2 b=%3 findSlot=%4 pending_resume=%5")
         .arg(game_id).arg(white_player).arg(black_player)
         .arg(findSlot(game_id) ? "FOUND" : "null").arg(pending_resume_player.isEmpty() ? "(none)" : pending_resume_player));
     GameSlot *adjourned_slot = nullptr;
     if (!findSlot(game_id)) {
         for (GameSlot *s : game_slots) {
             if (s->game_finished || s->adjourned_player.isEmpty()) continue;
             // Path (a): player names still intact
             bool names_match = (s->white_player.compare(white_player, Qt::CaseInsensitive) == 0 &&
                                 s->black_player.compare(black_player, Qt::CaseInsensitive) == 0);
             // Path (b): names overwritten by ID recycle but restore signal names the player
             bool resume_signal = (!pending_resume_player.isEmpty() &&
                                   s->adjourned_player.compare(pending_resume_player, Qt::CaseInsensitive) == 0);
             if (names_match || resume_signal) {
                 adjourned_slot = s;
                 break;
             }
         }
     }
     if (adjourned_slot) {
         // Reassign slot to new game ID — game ID is the only thing that changes.
         // Board position, move history, player names, comments all stay intact.
         // If path (b) was used (names were overwritten by ID recycle), restore from slot's
         // adjourned_player + the IGS "Observing game" line which has the correct names.
         int old_id = adjourned_slot->game_id;
         // Restore player names: trust the "Observing game" line since adjourned_player
         // may have been the only survivor. white_player/black_player come from IGS.
         adjourned_slot->white_player   = white_player;
         adjourned_slot->black_player   = black_player;
         adjourned_slot->game_id        = game_id;
         adjourned_slot->adjourned_player.clear();
         pending_resume_player.clear();
         adjourned_slot->clock_timer->start();
         GameSelectionDock *dock_s = shared_board_window ? shared_board_window->getGameSelectionDock() : nullptr;
         if (dock_s) { dock_s->removeGame(old_id); dock_s->addGame(game_id, black_player, adjourned_slot->black_rank, white_player, adjourned_slot->white_rank); }
         if (active_slot_game_id == old_id) active_slot_game_id = game_id;
         if (observed_game_ids.remove(old_id)) {
             observed_game_ids.insert(game_id);
             if (games_window) games_window->updateObservedGames(observed_game_ids);
         }
         if (shared_board_window) {
             shared_board_window->updatePlayerNames(white_player, black_player);
             shared_board_window->processComment("*SYSTEM*", "Opponent reconnected — game resumed.", false);
         }
         // Update bot_game_id so bot end-of-game logic targets the correct ID
         if (bot_mode_active && bot_game_id == old_id)
             bot_game_id = game_id;
         output_console->append(QString(">>> ADJOURN RESUME: Game %1 resumed as game %2 (%3 vs %4)").arg(old_id).arg(game_id).arg(white_player).arg(black_player));
         // Game successfully resumed — clear reconnect state
         reconnect_opponent.clear();
         reconnect_game_file.clear();
         // Request move history to restore board position (deferred moves N fires after confirm)
         games_pending_moves_request.insert(game_id);
         socket->write(QString("observe %1\n").arg(game_id).toUtf8());
     } else if (GameSlot *slot = findSlot(game_id)) {
         // Finished-slot ID recycle: IGS reused the game ID for a different game.
         // This OBSERVE-MATCH line is from a server game-list refresh (not a command
         // we sent), so we must NOT call observeGame() — that would send "observe N"
         // and subscribe us to a random game.  Instead, just evict the stale finished
         // slot and clear the board so no phantom stones remain.  The new game will be
         // observed properly only if the user or bot explicitly clicks/requests it.
         if (slot->game_finished) {
             // IGS recycled this game ID for a new game.  Remap the finished slot
             // to a synthetic negative ID so the game history stays in the dock for
             // review.  The new game will be observed via the normal path later.
             int synthetic_id = -game_id;
             output_console->append(QString(
                 "[EVICT] Game ID %1 recycled: finished slot (%2 vs %3) remapped to ID %4 to preserve history")
                 .arg(game_id).arg(slot->white_player).arg(slot->black_player).arg(synthetic_id));
             // If this slot is currently visible, note the original game ID in Comments.
             if (active_slot_game_id == game_id && shared_board_window)
                 shared_board_window->processComment("*SYSTEM*",
                     QString("IGS game ID %1 recycled for new game (%2 vs %3). This finished game history retained as ID %4.")
                         .arg(game_id).arg(white_player).arg(black_player).arg(synthetic_id),
                     false);
             slot->game_id = synthetic_id;
             GameSelectionDock *dock_s = shared_board_window ? shared_board_window->getGameSelectionDock() : nullptr;
             if (dock_s) dock_s->updateGameId(game_id, synthetic_id);
             if (active_slot_game_id == game_id) {
                 active_slot_game_id = synthetic_id;
                 // Board already shows this finished game; no need to clear it.
             }
             // Slot is retained with synthetic ID; nothing more to do here.
             return;
         }
         // Observed game (game_finished=false): detect ID recycle by player name mismatch.
         // untrackFinishedGame() is only called for bot playing games, so purely observed
         // games stay with game_finished=false when they end. When IGS silently recycles
         // the same ID for a new game, the OBSERVE-MATCH line will carry different player
         // names. Detect this and remap the stale slot to a synthetic negative ID.
         if (slot->is_observing && !slot->is_playing && slot->adjourned_player.isEmpty() &&
                 !slot->white_player.isEmpty() && !slot->black_player.isEmpty()) {
             bool white_differs = slot->white_player.compare(white_player, Qt::CaseInsensitive) != 0;
             bool black_differs = slot->black_player.compare(black_player, Qt::CaseInsensitive) != 0;
             if (white_differs && black_differs) {
                 int synthetic_id = -game_id;
                 output_console->append(QString(
                     "[EVICT] Observed game ID %1 recycled (%2 vs %3 → %4 vs %5): remapping to %6")
                     .arg(game_id).arg(slot->white_player).arg(slot->black_player)
                     .arg(white_player).arg(black_player).arg(synthetic_id));
                 slot->game_id       = synthetic_id;
                 slot->game_finished = true;
                 GameSelectionDock *dock_s = shared_board_window ? shared_board_window->getGameSelectionDock() : nullptr;
                 if (dock_s) dock_s->updateGameId(game_id, synthetic_id);
                 if (active_slot_game_id == game_id) active_slot_game_id = synthetic_id;
                 return;
             }
         }
         // Don't overwrite player names on an adjourned slot — it still belongs to
         // the original players; IGS may recycle the same ID for a different game
         // before the disconnected player reconnects.
         if (!slot->game_finished && slot->adjourned_player.isEmpty()) {
             // Guard: detect recycled game ID — IGS names differ from what we clicked on
             bool id_recycled = false;
             if (!slot->expected_white_player.isEmpty() && !slot->expected_black_player.isEmpty()) {
                 bool white_ok = slot->expected_white_player.compare(white_player, Qt::CaseInsensitive) == 0 ||
                                 slot->expected_white_player == "?" || slot->expected_white_player == "Unknown";
                 bool black_ok = slot->expected_black_player.compare(black_player, Qt::CaseInsensitive) == 0 ||
                                 slot->expected_black_player == "?" || slot->expected_black_player == "Unknown";
                 if (!white_ok || !black_ok) {
                     id_recycled = true;
                     output_console->append(QString(
                         ">>> WARNING: Game ID %1 was recycled! Clicked on %2 vs %3 but IGS reports %4 vs %5")
                         .arg(game_id)
                         .arg(slot->expected_white_player).arg(slot->expected_black_player)
                         .arg(white_player).arg(black_player));
                 }
             }
             slot->white_player = white_player;
             slot->black_player = black_player;
             if (game_id == active_slot_game_id) {
                 shared_board_window->updatePlayerNames(white_player, black_player);
                 if (id_recycled) {
                     shared_board_window->processComment("*SYSTEM*",
                         QString("WARNING: Game ID recycled! You clicked on %1 vs %2 but this game is now %3 vs %4. "
                                 "The game list has been refreshed.")
                             .arg(slot->expected_white_player).arg(slot->expected_black_player)
                             .arg(white_player).arg(black_player),
                         false);
                 }
             }
         }
     }
 } else {
 for (BoardWindow* board : board_windows) {
 if (board->isObserving() && board->getObservedGameId() == game_id && !board->isFinished()) {
 board->updatePlayerNames(white_player, black_player);
 if (!this->suppress_server_console) output_console->append(QString(">>> UPDATED PLAYER NAMES for game %1").arg(game_id));
 }
 }
 }
 }
 } else {
 if (!this->suppress_server_console) output_console->append(QString(">>> DEBUG: parseObserveStart FAILED for line: %1").arg(line));
 
 // If we're waiting for a manual observe response but parsing failed, create board anyway
 if (pending_manual_observe) {
 // Extract game ID from the line manually
 QString game_part = line.section(' ', 3, 3);
 bool ok;
 int fallback_game_id = game_part.toInt(&ok);
 if (ok && fallback_game_id == pending_observe_game_id) {
 if (!this->suppress_server_console) output_console->append(QString(">>> FALLBACK: Creating board window for game %1 with unknown players").arg(fallback_game_id));
 observeGame(fallback_game_id, "Unknown", "Unknown", "?", "?");
 pending_manual_observe = false;
 pending_observe_game_id = -1;
 }
 }
 }
 
 // Extract game ID from the line for observer parsing
 QString game_part = line.section(' ', 3, 3); // Get the 4th word (game ID)
 bool ok;
 int extracted_game_id = game_part.toInt(&ok);
 
 if (ok && extracted_game_id == observer_request_game_id) {
 if (!this->suppress_server_console) output_console->append(QString(">>> OBSERVER LIST START for game %1").arg(extracted_game_id));
 parsing_observers = true;
 observer_game_id = extracted_game_id;

 // Clear existing observers
 if (docked_pane_mode) {
     if (GameSlot *slot = findSlot(extracted_game_id)) {
         slot->observers.clear();
         if (extracted_game_id == active_slot_game_id)
             shared_board_window->clearObservers();
     }
 } else {
 for (BoardWindow* board : board_windows) {
 if (board->isObserving() && board->getObservedGameId() == extracted_game_id) {
 board->clearObservers();
 }
 }
 }
 }
 }
 // Parse observer entries when we're in observer parsing mode.
 // IGS format: "9   name1 rank1       name2 rank2       name3 rank3"
 // Guard: lines with ':' are stats responses (e.g. "Idle Time:  3s") — never observer rows.
 else if (parsing_observers && line.startsWith("9 ") && !line.contains(':')
          && !line.contains("Found") && !line.contains("Observing")) {
 QString observer_line = line.mid(2).trimmed();
 QStringList words = observer_line.split(QRegExp("\\s+"), QString::SkipEmptyParts);

 // Valid rank tokens end in k/d/p (optionally followed by +/*/?), or equal "BC".
 // Reject the whole line if no valid rank token is present.
 static QRegExp rank_re("^\\d+[kdp][+*?]?$");
 bool has_valid_rank = false;
 for (const QString &w : words)
     if (w == "BC" || rank_re.exactMatch(w)) { has_valid_rank = true; break; }
 if (!has_valid_rank) { /* skip — not a real observer line */ }
 else {
 for (int i = 0; i + 1 < words.size(); i += 2) {
     QString observer_name = words[i];
     QString observer_rank = words[i + 1];
     // Skip pair if rank token doesn't look like a rank (extra safety).
     if (observer_rank != "BC" && !rank_re.exactMatch(observer_rank)) continue;

     if (docked_pane_mode) {
         if (GameSlot *slot = findSlot(observer_game_id)) {
             GameSlot::ObserverEntry e; e.name = observer_name; e.rank = observer_rank;
             slot->observers.append(e);
             if (observer_game_id == active_slot_game_id)
                 shared_board_window->addObserver(observer_name, observer_rank);
         }
     } else {
         for (BoardWindow* board : board_windows) {
             if (board->isObserving() && board->getObservedGameId() == observer_game_id)
                 board->addObserver(observer_name, observer_rank);
         }
     }
 }
 }
 }
 // End observer parsing when we see "Found X observers."
 else if (parsing_observers && line.startsWith("9 Found ") && line.contains("observers")) {
 QString count_str = line.section(' ', 2, 2);
 if (!this->suppress_server_console) output_console->append(QString(">>> OBSERVER LIST END - Found %1 observers").arg(count_str));
 parsing_observers = false;
 observer_game_id = -1;
 }

 // Parse IGS Command 24 - Say messages (private player communication)
 // Possible formats:
 // "24 Ulysses says: hello"
 // "24 Ulysses*: hello"
 // "24 Ulysses: hello"
 if (line.startsWith("24 ")) {
 if (!this->suppress_server_console) output_console->append(QString(">>> DEBUG SAY: Raw line = [%1]").arg(line));

 QString sender, message;
 bool parsed = false;

 // Try multiple patterns
 // Pattern 1: "24 *username*: message" (IGS format with asterisks)
 QRegExp say_re1("^24\\s+\\*([^*]+)\\*:\\s*(.+)$");
 if (say_re1.indexIn(line) != -1) {
 sender = say_re1.cap(1).trimmed();
 message = say_re1.cap(2).trimmed();
 parsed = true;
 if (!this->suppress_server_console) output_console->append(QString(">>> DEBUG SAY: Matched pattern 1 (*username*:)"));
 }

 // Pattern 2: "24 username says: message"
 if (!parsed) {
 QRegExp say_re2("^24\\s+([^\\s:]+)\\s+says:\\s*(.+)$");
 if (say_re2.indexIn(line) != -1) {
 sender = say_re2.cap(1).trimmed();
 message = say_re2.cap(2).trimmed();
 parsed = true;
 if (!this->suppress_server_console) output_console->append(QString(">>> DEBUG SAY: Matched pattern 2 (says:)"));
 }
 }

 // Pattern 3: "24 username: message" (simple format)
 if (!parsed) {
 QRegExp say_re3("^24\\s+([^\\s:*]+):\\s*(.+)$");
 if (say_re3.indexIn(line) != -1) {
 sender = say_re3.cap(1).trimmed();
 message = say_re3.cap(2).trimmed();
 parsed = true;
 if (!this->suppress_server_console) output_console->append(QString(">>> DEBUG SAY: Matched pattern 3 (simple :)"));
 }
 }

 if (parsed) {
 if (!this->suppress_server_console) output_console->append(QString(">>> MESSAGE from %1: %2").arg(sender, message));

 int boards_delivered = 0;

 // Dock mode: deliver to shared_board_window if sender is a player in the active (non-finished) game
 if (docked_pane_mode && shared_board_window && active_slot_game_id != -1) {
     if (GameSlot *slot = findSlot(active_slot_game_id); slot && !slot->game_finished) {
         QString white = shared_board_window->getWhitePlayer();
         QString black = shared_board_window->getBlackPlayer();
         if (sender.compare(white, Qt::CaseInsensitive) == 0 ||
             sender.compare(black, Qt::CaseInsensitive) == 0) {
             GameSlot::CommentEntry ce; ce.user = sender; ce.text = message; ce.is_kibitz = false;
             slot->comments.append(ce);
             shared_board_window->processComment(sender, message, false);
             boards_delivered++;
         }
     }
 }

 // Non-dock mode: search board_windows for a matching playing board
 if (!docked_pane_mode) {
     for (BoardWindow* board : board_windows) {
         if (board->isPlaying() && !board->isFinished()) {
             QString white = board->getWhitePlayer();
             QString black = board->getBlackPlayer();
             if (sender.compare(white, Qt::CaseInsensitive) == 0 ||
                 sender.compare(black, Qt::CaseInsensitive) == 0) {
                 board->processComment(sender, message, false);
                 if (!this->suppress_server_console) output_console->append(QString(">>> [OK] MESSAGE DELIVERED to game board: %1 vs %2")
                     .arg(white, black));
                 boards_delivered++;
                 break;
             }
         }
     }
 }

 // If not delivered to a game board, treat as tell and broadcast to Player Stats Dialogs
 if (boards_delivered == 0) {
     if (!this->suppress_server_console) output_console->append(QString(">>> [TELL] TELL MESSAGE from %1 (no matching game board)").arg(sender));
     if (players_window)
         players_window->broadcastIncomingTell(sender, message);
 }
 } else {
 if (!this->suppress_server_console) output_console->append(QString(">>> [WARN] SAY PARSE FAILED: Could not match any pattern for line: %1").arg(line));
 }
 }

 // Parse IGS tell messages - Format: "<Player> tells you: message"
 if (line.contains("tells you:")) {
 QRegExp tell_re("^<([^>]+)>\\s+tells you:\\s*(.+)$");
 qDebug() << "[DEBUG] Checking tell pattern for line:" << line;
 if (tell_re.indexIn(line) != -1) {
 QString sender = tell_re.cap(1).trimmed();
 QString message = tell_re.cap(2).trimmed();

 if (!this->suppress_server_console) output_console->append(QString(">>> [TELL] TELL RECEIVED from %1: %2").arg(sender, message));

 // Route to the board comment panel if sender is a player in the active game (dock or non-dock)
 bool delivered_to_board = false;
 GameSlot *active_slot = (active_slot_game_id != -1) ? findSlot(active_slot_game_id) : nullptr;
 if (docked_pane_mode && shared_board_window && active_slot && !active_slot->game_finished) {
     QString white = shared_board_window->getWhitePlayer();
     QString black = shared_board_window->getBlackPlayer();
     if (sender.compare(white, Qt::CaseInsensitive) == 0 ||
         sender.compare(black, Qt::CaseInsensitive) == 0) {
         GameSlot::CommentEntry ce; ce.user = sender; ce.text = message; ce.is_kibitz = false;
         active_slot->comments.append(ce);
         shared_board_window->processComment(sender, message, false);
         delivered_to_board = true;
     }
 } else {
     for (BoardWindow* board : board_windows) {
         if (board->isPlaying() && !board->isFinished()) {
             QString white = board->getWhitePlayer();
             QString black = board->getBlackPlayer();
             if (sender.compare(white, Qt::CaseInsensitive) == 0 ||
                 sender.compare(black, Qt::CaseInsensitive) == 0) {
                 board->processComment(sender, message, false);
                 delivered_to_board = true;
                 break;
             }
         }
     }
 }

 // Always also broadcast to open player stats dialogs
 if (players_window)
     players_window->broadcastIncomingTell(sender, message);
 // Bot mode: send a canned auto-reply
 if (bot_mode_active)
     botReplyToTell(sender);
 Q_UNUSED(delivered_to_board);
 }
 }

 // Show all data in high contrast console
 // Determine if this line contains games data (Command 7), moves data (Command 15), players data (Commands 27/42), or stats data (Command 9)
	bool is_games_data = line.startsWith("7 ");
	bool is_moves_data = line.startsWith("15 ");
	bool is_players_data = line.startsWith("27 ") || line.startsWith("42 ");
	bool is_stats_data = line.startsWith("9 ");
	bool is_prompt_data = line.startsWith("1 ") || line == "2" || line.startsWith("2 ");  // IGS command 1/2: server prompts/status

	// Check if this is an important message that should never be suppressed
	bool is_important = line.startsWith("ayt") ||  // Keep-alive heartbeat
	                    line.contains("!!*Pandanet*!!") ||  // Server announcements
	                    line.contains("Handicap and komi");  // Important game status messages

	bool suppress_this_line = !is_important &&
	                          ((is_games_data && suppress_games_console) ||
	                           (is_moves_data && suppress_moves_console) ||
	                           (is_players_data && suppress_server_console) ||
	                           (is_stats_data && suppress_server_console) ||
	                           (is_prompt_data && suppress_server_console));

	if (!suppress_this_line) {
		output_console->append("< " + line);
	}
 
 // Debug: Look for ANY line that might contain kibitz or be from woodnstone
 if (line.contains("kibitz", Qt::CaseInsensitive) || 
 line.contains("woodnstone", Qt::CaseInsensitive) ||
 line.startsWith("11 ") || line.startsWith("19 ")) {
 if (!this->suppress_server_console) output_console->append(QString(">>> [STAR] KIBITZ DEBUG: %1").arg(line));
 }
 
 // Handle login completion
 if (line.contains("1 1") && line.length() < 10) {
 updateStatus(QString("Connected as %1 - ready").arg(login_username));
 if (!this->suppress_server_console) output_console->append(">>> LOGIN SUCCESSFUL! Enabling enhanced rating protocol...");

 // Restore bot_mode_active immediately so incoming match requests are handled
 // correctly. We only set the flag + update the UI here — socket commands
 // (toggle open) are deferred into the 1s timer below so they don't inject
 // server responses into the middle of the userlist/who data stream.
 if (settings->getBotModeEnabled()) {
     bot_mode_active = true;
     if (bot_mode_action) bot_mode_action->setChecked(true);
     output_console->append("[BOT] Bot mode restored from previous session.");
 }

 // Initialize IGS enhanced protocol for numerical ratings (like q5Go)
 QTimer::singleShot(1000, this, [this]() {
 // Identify client to IGS (required for full protocol support)
 socket->write("id xgospel2 1.0\n");
 if (!this->suppress_server_console) output_console->append(">>> SENT: id xgospel2 1.0 (client identification)");

 socket->write("toggle newrating\n");
 newrating_enabled = true; // Track newrating state
 qDebug() << "MAIN WINDOW: Setting newrating_enabled = true";

 // Send nmatch range parameters (required for rated games)
 // Format: nmatchrange BWN handicap boardsize maintime byotime byostones
 // Use broader ranges to be compatible with more clients
 socket->write("nmatchrange BWN 0-9 19-19 60-18000 60-18000 25-25 0 0 0-0\n");
 if (!this->suppress_server_console) output_console->append(">>> SENT: nmatchrange BWN 0-9 19-19 60-18000 60-18000 25-25 (broad compatibility)");

 // Enable nmatch protocol (must be after nmatchrange)
 socket->write("toggle nmatch true\n");
 if (!this->suppress_server_console) output_console->append(">>> SENT: toggle nmatch true (enable nmatch protocol)");

 // Advertise open status if bot mode was restored
 if (bot_mode_active) {
     socket->write("toggle open true\n");
     socket->flush();
     output_console->append("[BOT] >>> toggle open true (bot accepting matches)");
 }

 // Check for adjourned games saved on the server from previous sessions
 socket->write("stored\n");

 // If this login follows an unexpected disconnect during a bot game,
 // wait for the "stored" response (CMD18) to get the exact game filename,
 // then send "load <filename>" from the CMD18 handler below.
 if (!reconnect_opponent.isEmpty()) {
     reconnect_game_file.clear();
     reconnect_attempts = 0;
     output_console->append(QString("[BOT] Auto-reconnect: waiting for 'stored' response to resume game vs %1...").arg(reconnect_opponent));
 }

 // Update existing windows
 if (players_window) {
 players_window->setNewratingEnabled(newrating_enabled);
 }
 if (games_window) {
 games_window->setNewratingEnabled(newrating_enabled);
 }
 if (!this->suppress_server_console) output_console->append(">>> SENT: toggle newrating (enable numerical rating data)");
 if (!this->suppress_server_console) output_console->append(">>> READY! You can now use Windows > Show Players / Show Games");
 });
 }
 
 // Handle AYT response from IGS
 if (line.startsWith("9 yes") || (line.startsWith("9 ") && line.contains("yes") && line.length() < 10)) {
 if (!this->suppress_server_console) output_console->append(">>> AYT response received - connection maintained");
 resetHeartbeatCounter(); // Reset heartbeat timer
 }
 
 // Detect newrating state changes from server responses
 if (line.contains("Set newrating to be True")) {
 newrating_enabled = true;
 qDebug() << "DETECTED: Server set newrating to TRUE";
 // Update existing windows
 if (players_window) {
 players_window->setNewratingEnabled(newrating_enabled);
 }
 if (games_window) {
 games_window->setNewratingEnabled(newrating_enabled);
 }
 } else if (line.contains("Set newrating to be False")) {
 newrating_enabled = false;
 qDebug() << "DETECTED: Server set newrating to FALSE";
 // Update existing windows
 if (players_window) {
 players_window->setNewratingEnabled(newrating_enabled);
 }
 if (games_window) {
 games_window->setNewratingEnabled(newrating_enabled);
 }
 }
 
 // Detect IGS observation status changes
 if (line.contains("Observing game") || line.contains("observing game") || 
 line.contains("Now observing") || line.contains("now observing")) {
 if (!this->suppress_server_console) output_console->append(">>> IGS CONFIRMED: Game observation started - comments should now work");
 qDebug() << "IGS confirmed game observation:" << line;
 }
 
 if (line.contains("You are not observing") || line.contains("not observing")) {
 if (!this->suppress_server_console) output_console->append(">>> IGS WARNING: Not observing any game - comments will not work");
 qDebug() << "IGS reports not observing:" << line;
 }
 
 // Parse player data with improved detection
 if (waiting_for_players && players_window) {
 // Look for any line that might contain player data
 if (line.contains("27 ") ||
		// Bug 35 FIX: Allow BC rank (no numbers) and NR players
 (line.length() > 20 && (line.contains(QRegExp("[0-9]+[dkp*]")) || line.contains("BC") || line.contains("NR")) &&
 line.contains(QRegExp("[a-zA-Z][a-zA-Z0-9]{2,10}")))) {

 // Bug 35: When processing WHO supplement, check for duplicates
 bool should_add = true;
 if (waiting_for_who_supplement) {
 // Extract player name to check against userlist
 Q5GoPlayer temp_player;
 Q5GoParser parser;
 if (parser.parseWhoFormatLine(line, temp_player) ||
 parser.parsePlayerLine(line, temp_player)) {
 // Skip if player was already added from userlist
 if (userlist_player_names.contains(temp_player.name)) {
 should_add = false;
 qDebug() << "[WHO-SUPPLEMENT] Skipping duplicate player:" << temp_player.name;
 } else {
 qDebug() << "[WHO-SUPPLEMENT] Adding player:" << temp_player.name << "rank:" << temp_player.rank;
 }
 }
 }

 if (should_add) {
 players_window->addPlayerFromRawLine(line);

 // Bug 35: Track names from userlist (but not from WHO supplement)
 if (!waiting_for_who_supplement) {
 Q5GoPlayer temp_player;
 Q5GoParser parser;
 if (parser.parsePlayerLine(line, temp_player) ||
 parser.parseWhoFormatLine(line, temp_player)) {
 userlist_player_names.insert(temp_player.name);
 qDebug() << "[USERLIST-TRACK] Tracking player:" << temp_player.name;
 }
 }

 player_count++;

 if (player_count % 10 == 0) {
 if (!this->suppress_server_console) output_console->append(QString(">>> Parsed %1 players so far...").arg(player_count));
 }
 }
 }
 
 // Check for userlist command failure and fallback to "who"
 if (waiting_for_players && !tried_fallback_who && login_username != "guest" && 
 (line.contains("Unknown command") || line.contains("userlist") && line.contains("not recognized"))) {
 if (!this->suppress_server_console) output_console->append(">>> USERLIST failed - trying WHO command as fallback...");
 tried_fallback_who = true;
 players_window->setFallbackMode(true); // Switch to WHO parsing mode
 players_window->clearPlayers();
 player_count = 0;
 socket->write("who\n");
 if (!this->suppress_server_console) output_console->append(">>> SENT: who (fallback from userlist)");
 return; // Don't process this line further
 }
 
 // Check for end of player list (be more careful about end detection)
 // Bug 35: WHO terminator looks like: "27  ******** 542 Players 116 Total Games ********"
 if (line.contains("**") ||
 line.contains("players listed") ||
 (line.contains("Players") && line.contains("Games") && line.contains("**")) ||
 (line.contains("1 1") && line.length() < 10) ||
 line.startsWith("Type 'who") ||
 line.contains("end of list") ||
 line.startsWith("##") ||
 (line.trimmed() == "1 1")) {
 
 // If userlist failed and we haven't tried fallback yet, try "who"
 if (player_count < 5 && !tried_fallback_who && login_username != "guest") {
 if (!this->suppress_server_console) output_console->append(QString(">>> Only %1 players found - trying WHO command as fallback...").arg(player_count));
 tried_fallback_who = true;
 players_window->setFallbackMode(true); // Switch to WHO parsing mode
 players_window->clearPlayers();
 player_count = 0;
 userlist_player_names.clear();  // Bug 35: Clear tracking set for fallback
 socket->write("who\n");
 if (!this->suppress_server_console) output_console->append(">>> SENT: who (fallback from userlist)");
 } else {
 // Bug 35: After userlist completes, supplement with WHO for BC/NR/guest players
 if (!waiting_for_who_supplement && login_username != "guest" && player_count > 5) {
 // userlist succeeded, now get BC/NR/guest players from WHO
 if (!this->suppress_server_console) output_console->append(QString(">>> USERLIST complete (%1 players), now fetching BC/NR/guests with WHO...").arg(player_count));
 if (!this->suppress_server_console) output_console->append(QString(">>> Tracked %1 unique player names from userlist").arg(userlist_player_names.size()));
 waiting_for_who_supplement = true;
 players_window->setFallbackMode(true);  // Switch to WHO parsing mode
 socket->write("who\n");
 if (!this->suppress_server_console) output_console->append(">>> SENT: who (supplement for BC/NR/guest players)");
 } else {
 // Either WHO supplement is done, or we're in guest mode
 waiting_for_players = false;
 waiting_for_who_supplement = false;
 userlist_player_names.clear();  // Bug 35: Clear tracking set
 players_window->resizeColumns();
 players_window->sortByRank();  // Apply default descending rank sort after data loaded

 // Sync player game statuses from current games list
 // This ensures player statuses are correct even if games list loaded first
 if (games_window) {
 games_window->syncAllPlayerStatuses();
 }

 players_window->applyAllFilters();  // Apply all filters together to avoid conflicts

 // If local player dialog is open, refresh their stats to update toggle flags
 if (players_window && !players_window->getLocalUsername().isEmpty()) {
 QString local_user = players_window->getLocalUsername();
 for (PlayerStatsDialog *dialog : players_window->getOpenDialogs()) {
 if (dialog->getPlayerName().compare(local_user, Qt::CaseInsensitive) == 0) {
 qDebug() << "[REFRESH] Local player dialog is open, requesting fresh stats for" << local_user;
 sendCommandString(QString("stats %1").arg(local_user));
 break;
 }
 }
 }

 if (!this->suppress_server_console) output_console->append(QString(">>> COMPLETED: Parsed %1 total players from IGS").arg(player_count));

 // Refresh dock button ranks now that the player list is fully populated.
 // addGame() was called at observe/play time when ranks were still "?".
 if (docked_pane_mode && shared_board_window) {
     if (GameSelectionDock *rank_dock = shared_board_window->getGameSelectionDock()) {
         for (GameSlot *s : game_slots) {
             if (s->game_finished) continue;
             QString br = findPlayerRank(s->black_player);
             QString wr = findPlayerRank(s->white_player);
             if (br.isEmpty()) br = "?";
             if (wr.isEmpty()) wr = "?";
             if (br != "?" || wr != "?") {
                 s->black_rank = br;
                 s->white_rank = wr;
                 rank_dock->updateGameRanks(s->game_id, br, wr);
             }
         }
     }
 }
 }
 }
 }
 }

 // Parse games data with improved detection
 if (waiting_for_games) {
 // DEBUG: Check window status
 if (!games_window) {
 if (!this->suppress_server_console) output_console->append(">>> DEBUG: Games data arriving but games_window is NULL!");
 }

 if (waiting_for_games && games_window) {
 // Look for game lines with format: [number] name [rank] vs. name [rank] (details) (obs)
 if (line.contains("[") && line.contains("]") && line.contains("vs.") &&
 line.contains("(") && line.contains(")")) {

 // Clear old list on the first game line received — not at request time.
 // This keeps the previous list visible during the network round-trip so
 // a delayed server response never leaves the window blank.
 if (game_count == 0)
     games_window->clearGames();

 games_window->addGameFromRawLine(line);
 game_count++;

 if (game_count % 5 == 0) {
 if (!this->suppress_server_console) output_console->append(QString(">>> Parsed %1 games so far...").arg(game_count));
 }
 }

 // Check for end of games list.
 // Guard "**" against IGS periodic broadcasts like "*** 1246 Players 366 Total Games ***"
 // which also contain "**" but no "[" — previously they falsely tripped end-of-list,
 // setting waiting_for_games=false and discarding all subsequent game lines.
 if ((line.contains("**") && !line.contains("[") && !line.contains("Players")) ||
 line.contains("games listed") ||
 (line.contains("1 1") && line.length() < 10) ||
 line.startsWith("Type 'games'") ||
 line.contains("end of list") ||
 line.startsWith("##") ||
 (line.trimmed() == "1 1")) {

 waiting_for_games = false;
 games_window->resizeColumns();
 games_window->updateObservedGames(observed_game_ids);
			games_window->syncAllPlayerStatuses();
			games_window->updateGamesCount();
 if (!this->suppress_server_console) output_console->append(QString(">>> COMPLETED: Parsed %1 total games from IGS").arg(game_count));
 }
 }
 }
 
 // Parse move data for board windows (cmd15 format)
 // Only try to parse lines that look like actual moves: "15 123(B): Q16"
 if (line.startsWith("15 ")) {
 if (settings->getDebugCmd15()) if (!this->suppress_server_console) output_console->append(QString(">>> [CMD15-TEST] Line starts with '15': %1").arg(line));
 // Check if this is game info with capture counts (both rated "I:" and free "FI:" games)
 // IGS format: "15 Game 373 I: player1..." (space after colon)
 bool cond1 = line.contains(" Game ");
 bool cond2 = (line.contains(" I: ") || line.contains(" FI: ") || line.contains(" TI: "));
 bool cond3 = line.contains(" vs ");
 if (settings->getDebugCmd15()) if (!this->suppress_server_console) output_console->append(QString(">>> [CMD15-TEST] CONDITIONS: Game=%1 Type=%2 vs=%3").arg(cond1).arg(cond2).arg(cond3));
 if (cond1 && cond2 && cond3) {
 if (settings->getDebugCmd15()) if (!this->suppress_server_console) output_console->append(QString(">>> [CMD15-TEST] CONDITION PASSED - calling parseGameInfo"));
 if (settings->getDebugCmd15()) if (!this->suppress_server_console) output_console->append(QString(">>> [CMD15-TEST] About to call move_parser->parseGameInfo"));
 int game_id;
 QString white_name, black_name, game_type;
 int white_captures, black_captures, white_time, black_time, white_byo_moves, black_byo_moves;

 bool parse_result = move_parser->parseGameInfo(line, game_id, white_name, black_name,
 white_captures, black_captures, white_time, black_time,
 white_byo_moves, black_byo_moves, game_type);
 if (settings->getDebugCmd15()) if (!this->suppress_server_console) output_console->append(QString(">>> [CMD15-TEST] parseGameInfo returned: %1").arg(parse_result));

 if (parse_result) {
 if (settings->getDebugCmd15()) if (!this->suppress_server_console) output_console->append(QString(">>> [CMD15-TEST] PARSE SUCCESS - GameID: %1 Type: %2").arg(game_id).arg(game_type));

 // Store game type from Command 15 if not already set by Command 7
 // Command 7 is authoritative, but Command 15 provides backup
 if (!game_type_map.contains(game_id)) {
 game_type_map[game_id] = game_type;
 qDebug() << "[MENU] GAME TYPE STORED from Command 15: Game" << game_id << "-> Type:" << game_type;
 }

 // q5Go pattern: Do NOT create board in Command 9!
 // Board will be created in Command 15 handler on first move
 if (pending_manual_observe && game_id == pending_observe_game_id) {
 if (!this->suppress_server_console) output_console->append(QString(">>> MANUAL OBSERVE SUCCESS: Game %1 - %2 vs %3")
 .arg(game_id).arg(white_name).arg(black_name));
 if (!this->suppress_server_console) output_console->append(QString(">>> Waiting for first Command 15 move to create board..."));

 pending_manual_observe = false;
 pending_observe_game_id = -1;
 }
 
 // Always update current game context from CMD15 header — no suppression.
 // Each slot tracks its own replay state independently, so we never need
 // to pin the global context to protect a replay in progress.
 current_game_context = game_id;
 if (settings->getDebugCmd15()) if (!this->suppress_server_console) output_console->append(QString(">>> [CMD15-TEST] About to check if playing - white: %1 black: %2 us: %3").arg(white_name).arg(black_name).arg(login_username));

 // Check if we are playing in this game (not just observing)
 // CRITICAL: Command 15 player names may be in wrong order, so check against BOTH positions
 // Also check against Command 7 names if available (authoritative)
 bool we_are_playing = false;
 if (game_white_player_map.contains(game_id) && game_black_player_map.contains(game_id)) {
 // Check if Command 7 names match Command 15 names (same game)
 QString cmd7_white = game_white_player_map[game_id];
 QString cmd7_black = game_black_player_map[game_id];

 // Verify Command 7 data matches this game (same players)
 bool cmd7_matches = ((cmd7_white == white_name || cmd7_white == black_name) &&
 (cmd7_black == white_name || cmd7_black == black_name));

 if (cmd7_matches) {
 // Use authoritative Command 7 names
 we_are_playing = (cmd7_white == login_username || cmd7_black == login_username);
 if (settings->getDebugCmd15()) if (!this->suppress_server_console) output_console->append(QString(">>> [CMD15-TEST] Using CMD7 (matched) - white: %1 black: %2 playing: %3").arg(cmd7_white).arg(cmd7_black).arg(we_are_playing));
 } else {
 // Command 7 data is stale/wrong - use Command 15 names
 we_are_playing = (white_name == login_username || black_name == login_username);
 if (settings->getDebugCmd15()) if (!this->suppress_server_console) output_console->append(QString(">>> [CMD15-TEST] CMD7 stale (w:%1 b:%2), using CMD15 - playing: %3").arg(cmd7_white).arg(cmd7_black).arg(we_are_playing));
 }
 } else {
 // No Command 7 yet - fallback to Command 15 names (check both positions since order may be wrong)
 we_are_playing = (white_name == login_username || black_name == login_username);
 if (settings->getDebugCmd15()) if (!this->suppress_server_console) output_console->append(QString(">>> [CMD15-TEST] No CMD7, using CMD15 - playing: %1").arg(we_are_playing));
 }

 if (settings->getDebugCmd15()) if (!this->suppress_server_console) output_console->append(QString(">>> [CMD15-TEST] Final we_are_playing: %1").arg(we_are_playing));

 if (we_are_playing) {
 if (settings->getDebugCmd15()) if (!this->suppress_server_console) output_console->append(QString(">>> [CMD15-TEST] YES WE ARE PLAYING - creating board!"));
 // Check if we already have a board window for this game
 bool board_exists = false;
 for (BoardWindow* board : board_windows) {
 if (board->getObservedGameId() == game_id) {
 // Update existing board to playing mode
 board->setPlayingMode(true);
 board_exists = true;
 qDebug() << "[GAME] PLAYING MODE: Set existing board for game" << game_id << "to playing mode";
 break;
 }
 }
 
 if (!board_exists) {
 // -----------------------------------------------------------------------
 // Dock mode: create a GameSlot (playing) and route through shared_board_window
 // -----------------------------------------------------------------------
 if (docked_pane_mode) {
     // Guard: distinguish stale post-game CMD15 from a genuine IGS game-ID recycle.
     // A CMD67 "67 N ..." always signals a real new game — evict the finished slot so
     // a fresh one can be created.  A stale CMD15 never carries a CMD67 header, so it
     // won't reach this path; the cmd15_is_cmd67 flag (set by the CMD67 parser before
     // calling into this we_are_playing block) is used as the discriminator.
     // For now: if a finished slot exists AND we arrived via the stale CMD15 path
     // (detected by byo_moves == -1 as IGS only sends -1 in post-game cleanup lines),
     // skip creation.  Otherwise evict and proceed normally.
     if (GameSlot *finished_slot = findSlot(game_id)) {
         if (finished_slot->game_finished) {
             if ((white_byo_moves == -1 || black_byo_moves == -1) &&
                 game_id != newly_confirmed_game_id) {
                 // Stale post-game CMD15 (byo=-1) and no "Creating match [N]" for this ID.
                 // Skip — this is a server cleanup echo, not a real new game.
                 goto skip_playing_board_creation;
             }
             // Genuine IGS game-ID recycle — evict stale slot first.
             GameSelectionDock *dock_s = shared_board_window ? shared_board_window->getGameSelectionDock() : nullptr;
             if (dock_s) dock_s->removeGame(game_id);
             game_slots.removeOne(finished_slot);
             if (active_slot_game_id == game_id) {
                 active_slot_game_id = -1;
                 // Clear the board widget immediately so no stale stones from the
                 // finished game remain visible while the new slot is being set up.
                 if (shared_board_window) shared_board_window->clearBoard();
             }
             if (shared_board_window) shared_board_window->detachSlotClockTimer();
             delete finished_slot;
         }
     }
     // If this CMD67 is the resumed game arriving under a new ID (pending_resume_player set),
     // reassign the adjourned slot instead of creating a fresh one.
     // Fallback: if pending_resume_player is empty but the opponent is a known stored-game
     // opponent (from the "stored" query at login), treat this CMD67 as a cross-session resume
     // so a fresh board is created without needing the "restored your old game" signal.
     if (pending_resume_player.isEmpty()) {
         QString opp = (white_name.compare(login_username, Qt::CaseInsensitive) == 0) ? black_name : white_name;
         if (stored_adjourned_opponents.contains(opp.toLower())) {
             output_console->append(QString(">>> ADJOURN RESUME (CMD67): %1 is a stored-game opponent — cross-session resume").arg(opp));
             pending_resume_player = opp;
         }
     }
     if (!pending_resume_player.isEmpty()) {
         GameSlot *adjourned = nullptr;
         for (GameSlot *s : game_slots) {
             if (!s->game_finished && !s->adjourned_player.isEmpty() &&
                 s->adjourned_player.compare(pending_resume_player, Qt::CaseInsensitive) == 0) {
                 adjourned = s;
                 break;
             }
         }
         if (!adjourned) {
             // Cross-session adjournment: no slot exists (adjourned in a prior session).
             // Clear the flag so the normal slot-creation path below runs and builds a
             // fresh board; move history will be fetched from the server via moves N.
             output_console->append(QString(">>> ADJOURN RESUME (CMD67): No slot for %1 — cross-session resume, creating fresh board").arg(pending_resume_player));
             stored_adjourned_opponents.remove(pending_resume_player.toLower());
             pending_resume_player.clear();
             cross_session_resume_game_id = game_id; // signal post-slot-creation to fetch moves
             if (bot_mode_active) {
                 bot_restart_after_replay_game_id = game_id; // restart engine after replay
                 bot_game_id = game_id; // update bot's active game ID to the new one
             }
         }
         if (adjourned) {
             int old_id = adjourned->game_id;
             adjourned->game_id       = game_id;
             adjourned->white_player  = white_name;
             adjourned->black_player  = black_name;
             adjourned->adjourned_player.clear();
             stored_adjourned_opponents.remove(pending_resume_player.toLower());
             pending_resume_player.clear();
             adjourned->clock_timer->start();
             adjourned->is_playing    = true;
             if (bot_mode_active && bot_game_id == old_id) bot_game_id = game_id;
             // Board state is already intact in slot->board_cells and move_history.
             // Set LIVE immediately — no server replay needed.
             adjourned->replay_state = GameSlot::LIVE;
             // Register new dock button BEFORE switchActiveGame so setActiveGame can find it.
             GameSelectionDock *dock_s = shared_board_window ? shared_board_window->getGameSelectionDock() : nullptr;
             if (dock_s) { dock_s->removeGame(old_id); dock_s->addGame(game_id, black_name, adjourned->black_rank, white_name, adjourned->white_rank); }
             observed_game_ids.remove(old_id);
             observed_game_ids.insert(game_id);
             if (games_window) games_window->updateObservedGames(observed_game_ids);
             output_console->append(QString(">>> ADJOURN RESUME (CMD67): Game %1 resumed as game %2 (%3 vs %4)").arg(old_id).arg(game_id).arg(white_name).arg(black_name));
             // Game successfully resumed — clear reconnect state
             reconnect_opponent.clear();
             reconnect_game_file.clear();
             // switchActiveGame checks (game_id == active_slot_game_id) — keep active_slot_game_id
             // as the old ID so the switch executes fully (loadSlot + setActiveGame + highlight).
             // active_slot_game_id is updated inside switchActiveGame.
             switchActiveGame(game_id);
             shared_board_window->setPlayingMode(true);
             if (shared_board_window)
                 shared_board_window->processComment("*SYSTEM*", "Opponent reconnected — game resumed.", false);
             // Restart the bot: replay all moves into KataGo then request genmove if our turn.
             if (bot_mode_active && bot_game_id == game_id) {
                 KataGoEngine *engine = engine_manager ? engine_manager->currentEngine() : nullptr;
                 if (engine && bot_engine_ready) {
                     engine->enqueueRaw("clear_board");
                     engine->enqueueRaw(QString("komi %1").arg(bot_komi));
                     if (bot_handicap > 1) {
                         QList<QPair<int,int>> hc_pos = IGSMoveParser::getHandicapPositions(bot_handicap);
                         QStringList verts;
                         for (const auto &p : hc_pos) verts << coordsToGtp(p.first, p.second);
                         engine->enqueueRaw(QString("set_free_handicap %1").arg(verts.join(' ')));
                     }
                     // Replay all history moves into the engine
                     for (const GameMove &m : adjourned->move_history) {
                         if (m.x < 0) continue; // skip pass/handicap markers
                         QString col_str = (m.color == BLACK_STONE) ? "black" : "white";
                         engine->enqueueRaw(QString("play %1 %2").arg(col_str).arg(coordsToGtp(m.x, m.y)));
                     }
                     // If it is now our turn, request genmove
                     StoneColor our_color = bot_color;
                     StoneColor next_to_play = static_cast<StoneColor>(adjourned->current_player);
                     if (next_to_play == our_color) {
                         output_console->append(QString("[BOT] Resume: replayed %1 moves into engine — requesting genmove").arg(adjourned->move_history.size()));
                         engine->requestGenmove(our_color);
                     } else {
                         output_console->append(QString("[BOT] Resume: replayed %1 moves into engine — waiting for opponent").arg(adjourned->move_history.size()));
                     }
                 }
             }
         }
     }
     if (pending_resume_player.isEmpty()) {
     // Evict any pre-existing slot for this game ID that is NOT already our live playing slot.
     // An observation slot means IGS recycled the number — evict and create fresh.
     // A finished slot is stale — evict and create fresh.
     // A live playing slot (is_playing=true, !game_finished) means IGS sent a duplicate CMD15
     // confirmation (it sends two: one immediately, one after "Creating match [N]") — keep it.
     if (GameSlot *stale = findSlot(game_id)) {
         if (stale->is_playing && !stale->game_finished) {
             // Duplicate CMD15 for a game we already set up — skip eviction entirely.
             qDebug() << "[CMD15-PLAYING] Duplicate CMD15 for live playing slot game" << game_id << "— skipping";
             goto skip_playing_board_creation;
         }
         qDebug() << "[CMD15-PLAYING] Evicting pre-existing slot for game" << game_id
                  << "(finished=" << stale->game_finished << "is_playing=" << stale->is_playing
                  << "is_observing=" << stale->is_observing << ")";
         GameSelectionDock *dock_s = shared_board_window ? shared_board_window->getGameSelectionDock() : nullptr;
         if (dock_s) dock_s->removeGame(game_id);
         game_slots.removeOne(stale);
         if (active_slot_game_id == game_id) active_slot_game_id = -1;
         observed_game_ids.remove(game_id);
         if (shared_board_window) shared_board_window->detachSlotClockTimer();
         delete stale;
     }
     if (!findSlot(game_id)) {
         // Resolve correct player name order from CMD7 if available
         QString actual_white_name = white_name;
         QString actual_black_name = black_name;
         if (game_white_player_map.contains(game_id) && game_black_player_map.contains(game_id)) {
             QString w7 = game_white_player_map[game_id];
             QString b7 = game_black_player_map[game_id];
             if ((w7 == white_name || w7 == black_name) && (b7 == white_name || b7 == black_name)) {
                 actual_white_name = w7;
                 actual_black_name = b7;
             }
         }

         GameSlot *slot = new GameSlot(this);
         slot->game_id      = game_id;
         slot->white_player = actual_white_name;
         slot->black_player = actual_black_name;
         slot->white_rank         = findPlayerRank(actual_white_name);
         slot->black_rank         = findPlayerRank(actual_black_name);
         slot->white_rank_at_start = slot->white_rank;
         slot->black_rank_at_start = slot->black_rank;
         slot->my_username  = login_username;
         slot->is_playing   = true;
         slot->is_observing = false;
         slot->observation_state = GameSlot::JOINING_GAME;
         slot->observation_start_time = QDateTime::currentDateTime();
         slot->game_start_time        = QDateTime::currentDateTime();
         slot->game_mode   = MODE_NORMAL;
         slot->board_size  = 19;
         slot->current_player = BLACK_STONE;
         slot->game_root   = new GameNode();
         slot->current_node = slot->game_root;
         slot->auto_follow_mode = true;
         slot->komi        = game_komi_map.value(game_id, 6.5);
         slot->handicap    = game_handicap_map.value(game_id, 0);
         slot->game_type   = game_type_map.value(game_id, "Rated");
         // Cross-session resume needs history replay via moves N; normal new games are LIVE.
         slot->replay_state = (bot_restart_after_replay_game_id == game_id)
                              ? GameSlot::WAITING_FOR_MOVES0 : GameSlot::LIVE;

         connect(slot, &GameSlot::clockTick, this, [this, slot]() {
             if (slot->game_finished) return;
             if (slot->last_time_update.isNull()) return;
             int elapsed = static_cast<int>(slot->last_time_update.secsTo(
                               QDateTime::currentDateTime()));
             if (slot->current_player == BLACK_STONE)
                 slot->black_time_seconds = std::max(0, slot->black_time_seconds - elapsed);
             else
                 slot->white_time_seconds = std::max(0, slot->white_time_seconds - elapsed);
             slot->last_time_update = QDateTime::currentDateTime();
         });

         game_slots.append(slot);
         if (newly_confirmed_game_id == game_id)
             newly_confirmed_game_id = -1;  // consumed — clear so stale guard works again

         // Create shared_board_window if this is the first game in dock
         if (!shared_board_window) {
             shared_board_window = new BoardWindow(this, login_username);
             connect(shared_board_window, &BoardWindow::boardClosed,
                     this, &FixedXGospelWindow::closeBoardWindow);
             connect(shared_board_window, &BoardWindow::saveRequested,
                     this, &FixedXGospelWindow::saveBoardGame);
             connect(shared_board_window, &BoardWindow::gameSaved,
                     this, [this](int game_id, const QString &filename) {
                         if (GameSlot *slot = findSlot(game_id))
                             slot->system_messages.append(QString("✓ Game saved to: %1").arg(filename));
                     });
             connect(shared_board_window, &BoardWindow::resignRequested,
                     this, &FixedXGospelWindow::resignGame);
             connect(shared_board_window, &BoardWindow::passRequested,
                     this, [this](int) { socket->write("pass\n"); socket->flush();
                         output_console->append("[IGS] >>> pass (manual Pass button)"); });
             connect(shared_board_window, &BoardWindow::doneRequested,
                     this, [this](int) { socket->write("done\n"); socket->flush();
                         output_console->append("[BOT] >>> done (manual Done button)"); });
             connect(shared_board_window, &BoardWindow::refreshRequested,
                     this, [this](int gid) {
                         socket->write(QString("moves %1\n").arg(gid).toUtf8());
                         output_console->append(QString("[IGS] >>> moves %1 (Refresh Board)").arg(gid)); });
             connect(shared_board_window, &BoardWindow::commentRequested,
                     this, &FixedXGospelWindow::sendComment);
             connect(shared_board_window, &BoardWindow::sayRequested,
                     this, &FixedXGospelWindow::sendSay);
             connect(shared_board_window, &BoardWindow::tellRequested,
                     this, &FixedXGospelWindow::sendTell);
             connect(shared_board_window, &BoardWindow::observersRequested,
                     this, &FixedXGospelWindow::requestObservers);
             connect(shared_board_window, &BoardWindow::observerClicked,
                     this, [this](const QString &name) { if (players_window) players_window->openStatsDialogForPlayer(name); });
             connect(shared_board_window, &BoardWindow::whitePlayerClicked,
                     this, [this](const QString &name) { if (players_window) players_window->openStatsDialogForPlayer(name); });
             connect(shared_board_window, &BoardWindow::blackPlayerClicked,
                     this, [this](const QString &name) { if (players_window) players_window->openStatsDialogForPlayer(name); });
             connect(shared_board_window, &BoardWindow::moveRequested,
                     this, &FixedXGospelWindow::sendMove);
             GameSelectionDock *dock = shared_board_window->getGameSelectionDock();
             if (dock) {
                 dock->show();
                 connect(dock, &GameSelectionDock::gameSelected,
                         this, &FixedXGospelWindow::switchActiveGame);
                 connect(dock, &GameSelectionDock::gameCloseRequested,
                         this, &FixedXGospelWindow::closeBoardWindow);
             }
             shared_board_window->setSharedWindow(true);
             shared_board_window->show();
         }

         GameSelectionDock *dock2 = shared_board_window->getGameSelectionDock();
         if (dock2) {
             dock2->addGame(game_id, actual_black_name, slot->black_rank, actual_white_name, slot->white_rank);
             dock2->setGameCloseable(game_id, false);  // playing game — close disabled until finished
         }

         // Always switch to the playing game — we need to be able to make moves
         // Use switchActiveGame so the dock button highlight updates correctly
         if (active_slot_game_id != -1) {
             switchActiveGame(game_id);
         } else {
             shared_board_window->loadSlot(slot);
             active_slot_game_id = game_id;
             if (GameSelectionDock *dock3 = shared_board_window->getGameSelectionDock())
                 dock3->setActiveGame(game_id);
         }

         // Apply komi/handicap and set playing mode on the now-active slot
         shared_board_window->updateGameSetup(slot->handicap, slot->komi, slot->game_type);
         shared_board_window->updateGameDetails(slot->game_type, 0);
         shared_board_window->setPlayingMode(true);
         shared_board_window->updateCaptures(white_captures, black_captures);
         shared_board_window->updateByoyomi(white_time, black_time, white_byo_moves, black_byo_moves);
         updateHoverPixmapForSlot(slot);

         // Print the IGS game ID and ranks at game start — visible in Comments & Kibitz
         // even after the ID is later recycled by the server.
         shared_board_window->processComment("*SYSTEM*",
             QString("IGS Game #%1 started. %2 [%3] vs %4 [%5]")
                 .arg(game_id)
                 .arg(actual_white_name).arg(slot->white_rank_at_start.isEmpty() ? QString("?") : slot->white_rank_at_start)
                 .arg(actual_black_name).arg(slot->black_rank_at_start.isEmpty() ? QString("?") : slot->black_rank_at_start),
             false);

         most_recently_observed_board = shared_board_window;
         most_recently_observed_game_id = game_id;
         observed_game_ids.insert(game_id);
         if (games_window) games_window->updateObservedGames(observed_game_ids);

         if (cross_session_resume_game_id == game_id) {
             // Cross-session resume: fetch move history to restore board position,
             // then engine restart happens in the moves-N completion handler.
             cross_session_resume_game_id = -1;
             reconnect_opponent.clear();
             reconnect_game_file.clear();
             output_console->append(QString("[BOT] Cross-session resume: fetching move history for game %1").arg(game_id));
             socket->write(QString("moves %1\n").arg(game_id).toUtf8());
             // Do NOT add to games_with_moves_requested — we want replay to run.
         } else {
             games_with_moves_requested.insert(game_id);  // prevent spurious "moves N" request
         }

         qDebug() << "[GAME] DOCK PLAYING BOARD: Game" << game_id
                  << "White:" << actual_white_name << "Black:" << actual_black_name;
     }
     } // end if (pending_resume_player.isEmpty()) — skip new-slot creation on resume
 } else {

 // -----------------------------------------------------------------------
 // Non-dock mode: create a standalone BoardWindow for playing
 // -----------------------------------------------------------------------
 // Create new board window for playing
 // CRITICAL: Use player names from Command 7 if available (authoritative white/black order)
 // Command 15 may list players in wrong order (initiator first, not white first)
 QString actual_white_name = white_name;
 QString actual_black_name = black_name;

 if (game_white_player_map.contains(game_id) && game_black_player_map.contains(game_id)) {
 QString cmd7_white = game_white_player_map[game_id];
 QString cmd7_black = game_black_player_map[game_id];

 // Verify Command 7 data matches this game (same players)
 bool cmd7_matches = ((cmd7_white == white_name || cmd7_white == black_name) &&
 (cmd7_black == white_name || cmd7_black == black_name));

 if (cmd7_matches) {
 // Use authoritative Command 7 names (correct white/black order)
 actual_white_name = cmd7_white;
 actual_black_name = cmd7_black;
 if (!this->suppress_server_console) output_console->append(QString(">>> [BOARD] Using CMD7 names: W:%1 B:%2").arg(actual_white_name).arg(actual_black_name));
 } else {
 // Command 7 data is stale - use Command 15 names
 if (!this->suppress_server_console) output_console->append(QString(">>> [BOARD] CMD7 stale (w:%1 b:%2), using CMD15: W:%3 B:%4").arg(cmd7_white).arg(cmd7_black).arg(actual_white_name).arg(actual_black_name));
 }
 } else {
 if (!this->suppress_server_console) output_console->append(QString(">>> [BOARD] No CMD7, using CMD15 names: W:%1 B:%2").arg(actual_white_name).arg(actual_black_name));
 }

 qDebug() << "[GAME] CREATING PLAYING BOARD: Game" << game_id << "White:" << actual_white_name << "Black:" << actual_black_name;
 BoardWindow* board = new BoardWindow(this, login_username);
 connect(board, &BoardWindow::boardClosed, this, &FixedXGospelWindow::closeBoardWindow);
 connect(board, &BoardWindow::saveRequested, this, &FixedXGospelWindow::saveBoardGame);
 connect(board, &BoardWindow::resignRequested, this, &FixedXGospelWindow::resignGame);
 connect(board, &BoardWindow::doneRequested, this, [this](int) {
     socket->write("done\n"); socket->flush();
     output_console->append("[IGS] >>> done (manual Done button)"); });
 connect(board, &BoardWindow::refreshRequested, this, [this](int gid) {
     socket->write(QString("moves %1\n").arg(gid).toUtf8());
     output_console->append(QString("[IGS] >>> moves %1 (Refresh Board)").arg(gid)); });
 connect(board, &BoardWindow::commentRequested, this, &FixedXGospelWindow::sendComment);
 connect(board, &BoardWindow::sayRequested, this, &FixedXGospelWindow::sendSay);
 connect(board, &BoardWindow::tellRequested, this, &FixedXGospelWindow::sendTell);
 connect(board, &BoardWindow::observersRequested, this, &FixedXGospelWindow::requestObservers);
 connect(board, &BoardWindow::observerClicked, this, [this](const QString &name) { if (players_window) players_window->openStatsDialogForPlayer(name); });
 connect(board, &BoardWindow::whitePlayerClicked, this, [this](const QString &name) { if (players_window) players_window->openStatsDialogForPlayer(name); });
 connect(board, &BoardWindow::blackPlayerClicked, this, [this](const QString &name) { if (players_window) players_window->openStatsDialogForPlayer(name); });
 connect(board, &BoardWindow::moveRequested, this, &FixedXGospelWindow::sendMove);

 board_windows.append(board);
 most_recently_observed_board = board; // Track for teaching title assignment

 // Look up actual ranks from player list
 QString white_rank = "?";
 QString black_rank = "?";
 if (players_window) {
 white_rank = findPlayerRank(actual_white_name);
 black_rank = findPlayerRank(actual_black_name);
 qDebug() << "[MENU] RANK LOOKUP: Game" << game_id << "White:" << actual_white_name << "=" << white_rank << "Black:" << actual_black_name << "=" << black_rank;
 }
 board->startObserving(game_id, actual_white_name, actual_black_name, white_rank, black_rank);

 // Initialize with captures and byoyomi info
 board->updateCaptures(white_captures, black_captures);
 board->updateByoyomi(white_time, black_time, white_byo_moves, black_byo_moves);

 // Determine correct game type and apply komi/handicap
 // Priority: 1) Command 7 (authoritative), 2) next_game_is_free flag, 3) Command 15
 {
 double stored_komi    = game_komi_map.value(game_id, -999.0);
 int stored_handicap   = game_handicap_map.value(game_id, 0);
 QString stored_type   = game_type_map.value(game_id, QString());
 QString detected_type = next_game_is_free ? "Free" : (stored_type.isEmpty() ? game_type : stored_type);

 if (stored_komi < -998.0) {
     // Command 7 not yet received — pick a sensible default:
     // handicap games use 0.5, even games use 6.5
     stored_komi = (stored_handicap > 0) ? 0.5 : 6.5;
     qDebug() << "[GAME] DEFAULT SETUP: Game" << game_id << "komi:" << stored_komi
              << "handicap:" << stored_handicap << "type:" << detected_type << "(will update from Command 7)";
 } else {
     qDebug() << "[GAME] KOMI APPLIED: Game" << game_id << "komi:" << stored_komi
              << "handicap:" << stored_handicap << "type:" << detected_type << "(from Command 7)";
 }
 board->updateGameSetup(stored_handicap, stored_komi, detected_type);
 board->updateGameDetails(detected_type, 0);
 next_game_is_free = false;
 }
 
 board->setPlayingMode(true); // Set to playing mode
 board->show();
 board->raise();
 board->activateWindow();

 // Request game details from server to get authoritative type
 QTimer::singleShot(500, this, [this, game_id]() {
 QString games_cmd = QString("games %1").arg(game_id);
 socket->write((games_cmd + "\n").toUtf8());
 if (!this->suppress_server_console) output_console->append(QString(">>> [AUTO] Requesting game details for accurate type: %1").arg(games_cmd));
 });

 // Send "free" command if this game was requested as free with old match protocol
 // Bug 34: DISABLED - IGS server rejects with "That command is currently disabled"
 /*
 if (next_game_is_free) {
 qDebug() << ">>> FREE COMMAND: Sending 'free' for game" << game_id << "(old match protocol)";
 sendCommandString("free");
 next_game_is_free = false; // Reset for next game
 }
 */

 // Send "komi" command if this game has a non-default komi
 if (next_game_komi > 0.0) {
 qDebug() << ">>> KOMI COMMAND: Sending 'komi" << next_game_komi << "' for game" << game_id;
 sendCommandString(QString("komi %1").arg(next_game_komi, 0, 'f', 1));
 next_game_komi = 0.0; // Reset for next game
 }

 qDebug() << "[GAME] BOARD WINDOW: Explicitly shown and raised for game" << game_id;
 qDebug() << "[GAME] GAME INFO INITIALIZED: Type:" << game_type << "W:" << white_time << "B:" << black_time;
 if (!this->suppress_server_console) output_console->append(QString(">>> [GAME] PLAYING BOARD CREATED: Game %1 - %2 vs %3 (We are %4)")
 .arg(game_id).arg(white_name).arg(black_name).arg(login_username));
 } // end non-dock mode board creation
 } // end if (!board_exists)
 skip_playing_board_creation:;

 // Bot mode: start engine for this IGS game
 if (bot_mode_active && bot_game_id == -1 && !bot_opponent.isEmpty()) {
     // Determine authoritative color from CMD 67's own white_name/black_name.
     // This reliably handles nigiri ("N") because CMD 67 always carries the real assignment.
     // CMD 7 maps may not yet contain the new game ID (fresh nmatch games arrive in CMD 67
     // before the next CMD 7 broadcast), so we prefer CMD 67 data here.
     if (white_name.compare(login_username, Qt::CaseInsensitive) == 0)
         bot_color = WHITE_STONE;
     else if (black_name.compare(login_username, Qt::CaseInsensitive) == 0)
         bot_color = BLACK_STONE;
     // Fall back to CMD 7 maps only when CMD 67 names don't match (shouldn't happen,
     // but guards against parser edge cases)
     else if (game_white_player_map.contains(game_id) && game_black_player_map.contains(game_id)) {
         QString auth_white = game_white_player_map.value(game_id);
         QString auth_black = game_black_player_map.value(game_id);
         if (auth_white.compare(login_username, Qt::CaseInsensitive) == 0)
             bot_color = WHITE_STONE;
         else if (auth_black.compare(login_username, Qt::CaseInsensitive) == 0)
             bot_color = BLACK_STONE;
     }
     botStartGame(game_id);
 }
 }  // end if (we_are_playing)

 // Update capture counts, game type, and byoyomi for this game
 QString correct_game_type = game_type_map.value(game_id, game_type);
 if (docked_pane_mode) {
     if (GameSlot *slot = findSlot(game_id)) {
         slot->white_captures = white_captures;
         slot->black_captures = black_captures;
         if (!slot->game_type_locked) slot->game_type = correct_game_type;
         slot->white_time_seconds = white_time;
         slot->black_time_seconds = black_time;
         slot->white_byo_moves = white_byo_moves;
         slot->black_byo_moves = black_byo_moves;
         slot->last_time_update = QDateTime::currentDateTime();
         if (game_id == active_slot_game_id) {
             shared_board_window->updateCaptures(white_captures, black_captures);
             shared_board_window->updateGameDetails(correct_game_type, 0);
             shared_board_window->updateByoyomi(white_time, black_time, white_byo_moves, black_byo_moves);
         }
     }
 } else {
 for (BoardWindow* board : board_windows) {
 if (board->getObservedGameId() == game_id) {
 board->updateCaptures(white_captures, black_captures);
 board->updateGameDetails(correct_game_type, 0);
 board->updateByoyomi(white_time, black_time, white_byo_moves, black_byo_moves);
 }
 }
 }
 }
 }
 // Check for regular moves (INSIDE the "15 " block!)
 // CRITICAL FIX: Exclude TIME updates - they're not moves!
 // TIME format: "15 TIME:9:j987654321(W): 1 0/60 510/600 17/25 0/0 0/0 0/0"
 else if (line.contains("(") && line.contains("):") && !line.contains("TIME:") && !line.contains("GAMERPROPS:")) {
 // qDebug() << "[DEBUG] MOVE DEBUG: Processing move line:" << line;
 // Removed debug output to reduce clutter

 // Skip the "15 " prefix and try to parse the move
 QString move_part = line.mid(3).trimmed();
 GameMove move;
 if (move_parser->parseMoveLine(move_part, move)) {
 // Removed move debug output to reduce clutter
 
 // CRITICAL FIX: Use game context to route moves correctly
 BoardWindow* target_board = nullptr;
 bool dock_move_handled = false;

 if (current_game_context != -1) {
 if (docked_pane_mode) {
     // In docked mode the slot was created in observeGame — find it
     GameSlot *slot = findSlot(current_game_context);
     if (slot) {
         {
             move.game_id = current_game_context;
             if (current_game_context == active_slot_game_id &&
                 slot->replay_state == GameSlot::LIVE) {
                 // Detect spurious "moves N" history replay arriving while the game is
                 // already live (e.g. user typed "moves N" in console, or ID-recycle
                 // board was never cleared).  move_number == 0 while we already have
                 // history means a full replay is starting — reset and replay cleanly.
                 if (move.move_number == 0 && !slot->move_history.isEmpty()) {
                     // server_move_count == number of events processed; last move_number == count-1
                     int last_move_number = slot->server_move_count - 1;
                     memset(slot->board_state, 0, sizeof(slot->board_state));
                     slot->move_history.clear();
                     slot->server_move_count = 0;
                     slot->white_captures = 0;
                     slot->black_captures = 0;
                     slot->consecutive_passes = 0;
                     slot->catchup_high = last_move_number; // replay transitions to LIVE at this move
                     slot->pending_catchup_moves.clear();
                     if (slot->game_root) {
                         delete slot->game_root;
                         slot->game_root = new GameNode();
                         slot->current_node = slot->game_root;
                     }
                     slot->replay_state = GameSlot::REPLAYING;
                     if (shared_board_window) shared_board_window->clearBoard();
                     output_console->append(QString("[BOARD] Spurious moves-N reply for live game %1 — resetting board for clean replay").arg(current_game_context));
                     goto replaying_case;
                 }
                 // Active (viewed) slot, fully live — route to the shared board window,
                 // but also keep slot->board_state in sync for CMD22 dead-stone detection.
                 slot->move_history.append(move);
                 slot->server_move_count++;
                 applyMoveToSlotBoard(slot, move);
                 target_board = shared_board_window;
             } else if (current_game_context == active_slot_game_id &&
                        (slot->replay_state == GameSlot::WAITING_FOR_MOVES0 ||
                         slot->replay_state == GameSlot::REPLAYING)) {
                 // Active slot but still replaying history — run through the slot state
                 // machine so board_state and move_history are built correctly.
                 // The board window will be reloaded when replay completes (-> LIVE).
                 // Fall through to the inactive-slot switch below.
                 goto active_slot_replaying;
             } else if (slot->game_finished) {
                 // Game scored — drop all subsequent CMD15 lines (stone removals etc).
             } else if (slot->consecutive_passes >= 3 && move.x >= 0) {
                 // Triple-pass = game over by IGS protocol.  Any non-pass line after
                 // this is a scoring-phase stone removal arriving before CMD9 sets
                 // game_finished.  Drop it.
                 qDebug() << "[SCORING] Dropping post-triple-pass removal"
                          << move.x << move.y << "for game" << current_game_context;
             } else {
                 active_slot_replaying:
                 // Inactive slot, OR active slot still in WAITING_FOR_MOVES0/REPLAYING.
                 // Run through the per-slot state machine so board_state stays current.
                 switch (slot->replay_state) {

                 case GameSlot::WAITING_FOR_MOVES0:
                     // Catch-up flood phase: IGS streams live moves before the moves N
                     // history reply arrives.  Buffer them so they can fill any gap after
                     // history ends; track the highest move_number as the high-water mark.
                     if (move.move_number == 0) {
                         // moves N reply has started — transition to REPLAYING
                         slot->replay_state = GameSlot::REPLAYING;
                         qDebug() << "[SLOT] WAITING_FOR_MOVES0: game" << current_game_context
                                  << "move 0 received, catchup_high=" << slot->catchup_high
                                  << "buffered=" << slot->pending_catchup_moves.size()
                                  << "-> REPLAYING";
                         // Fall through into REPLAYING to process this move 0
                         goto replaying_case;
                     }
                     if (move.move_number > slot->catchup_high)
                         slot->catchup_high = move.move_number;
                     // Buffer for gap-fill after history replay ends.
                     slot->pending_catchup_moves.append(move);
                     qDebug() << "[SLOT] WAITING_FOR_MOVES0: game" << current_game_context
                              << "buffered move" << move.move_number
                              << "catchup_high now" << slot->catchup_high;
                     break;

                 case GameSlot::REPLAYING:
                 replaying_case:
                     // moves N history reply — apply every move cleanly from move 0.
                     // move_number == 0 is the first replay move; reset state first.
                     if (move.move_number == 0 && !slot->move_history.isEmpty()) {
                         memset(slot->board_state, 0, sizeof(slot->board_state));
                         slot->move_history.clear();
                         slot->server_move_count = 0;
                         slot->white_captures = 0;
                         slot->black_captures = 0;
                         slot->consecutive_passes = 0;
                         if (slot->game_root) {
                             delete slot->game_root;
                             slot->game_root = new GameNode();
                             slot->current_node = slot->game_root;
                         }
                         qDebug() << "[SLOT] REPLAYING: game" << current_game_context
                                  << "move 0 received - board reset for clean replay";
                     }
                     slot->move_history.append(move);
                     slot->server_move_count++;
                     applyMoveToSlotBoard(slot, move);
                     updateHoverPixmapForSlot(slot);
                     // Transition to LIVE once replay has reached the catch-up high-water mark.
                     if (move.move_number >= slot->catchup_high) {
                         slot->replay_state = GameSlot::LIVE;
                         qDebug() << "[SLOT] game" << current_game_context
                                  << "replay complete at move" << move.move_number
                                  << "(catchup_high=" << slot->catchup_high << ") -> LIVE";
                         // Drain any buffered catch-up moves with move_number > what history gave us.
                         // Sort by move_number so they apply in order.
                         std::sort(slot->pending_catchup_moves.begin(),
                                   slot->pending_catchup_moves.end(),
                                   [](const GameMove &a, const GameMove &b){
                                       return a.move_number < b.move_number; });
                         for (const GameMove &pm : slot->pending_catchup_moves) {
                             if (pm.move_number > move.move_number) {
                                 slot->move_history.append(pm);
                                 slot->server_move_count++;
                                 applyMoveToSlotBoard(slot, pm);
                                 updateHoverPixmapForSlot(slot);
                                 qDebug() << "[SLOT] drained buffered catchup move"
                                          << pm.move_number << "for game" << current_game_context;
                             }
                         }
                         slot->pending_catchup_moves.clear();
                         // If the user switched to this slot while it was still replaying,
                         // the board widget shows a partial or empty position.  Reload now
                         // that the slot's board_state is complete.
                         if (current_game_context == active_slot_game_id && shared_board_window)
                             shared_board_window->loadSlot(slot);
                         // Slot is now LIVE — dispatch the next queued "moves N" if any.
                         dispatchNextMovesRequest();
                         // Cross-session reconnect: replay is done — restart engine and genmove if our turn.
                         if (bot_restart_after_replay_game_id == current_game_context) {
                             bot_restart_after_replay_game_id = -1;
                             KataGoEngine *engine = engine_manager ? engine_manager->currentEngine() : nullptr;
                             if (engine && bot_engine_ready) {
                                 engine->enqueueRaw("clear_board");
                                 engine->enqueueRaw(QString("komi %1").arg(bot_komi));
                                 if (bot_handicap > 1) {
                                     QList<QPair<int,int>> hc_pos = IGSMoveParser::getHandicapPositions(bot_handicap);
                                     QStringList verts;
                                     for (const auto &p : hc_pos) verts << coordsToGtp(p.first, p.second);
                                     engine->enqueueRaw(QString("set_free_handicap %1").arg(verts.join(' ')));
                                 }
                                 for (const GameMove &m : slot->move_history) {
                                     if (m.x < 0) continue;
                                     QString col_str = (m.color == BLACK_STONE) ? "black" : "white";
                                     engine->enqueueRaw(QString("play %1 %2").arg(col_str).arg(coordsToGtp(m.x, m.y)));
                                 }
                                 StoneColor next_to_play = static_cast<StoneColor>(slot->current_player);
                                 if (next_to_play == bot_color) {
                                     output_console->append(QString("[BOT] Cross-session resume: replayed %1 moves — requesting genmove").arg(slot->move_history.size()));
                                     engine->requestGenmove(bot_color);
                                 } else {
                                     output_console->append(QString("[BOT] Cross-session resume: replayed %1 moves — waiting for opponent").arg(slot->move_history.size()));
                                 }
                             }
                         }
                     }
                     break;

                 case GameSlot::LIVE:
                     // Normal live move — drop re-sent catch-up duplicates, apply the rest.
                     if (move.move_number > 0 && move.move_number <= slot->catchup_high) {
                         qDebug() << "[SLOT] LIVE: dropping re-sent catch-up move"
                                  << move.move_number << "<= catchup_high"
                                  << slot->catchup_high << "for game" << current_game_context;
                     } else {
                         slot->move_history.append(move);
                         slot->server_move_count++;
                         applyMoveToSlotBoard(slot, move);
                         updateHoverPixmapForSlot(slot);
                         dock_move_handled = true;
                     }
                     break;
                 }
             }
         }
     } else {
         dock_move_handled = true; // slot null but dock mode — don't warn
     }
 } else {
 qDebug() << "[BOARD-LOOKUP] Looking for board for game" << current_game_context << "- have" << board_windows.size() << "boards";
 // q5Go pattern: Find the board for the current game context
 for (BoardWindow* board : board_windows) {
 int board_game_id = board->getObservedGameId();
 qDebug() << "[BOARD-LOOKUP] Checking board with game_id" << board_game_id;
 if (board_game_id == current_game_context && !board->isFinished()) {
 target_board = board;
 qDebug() << "[BOARD-LOOKUP] FOUND matching board!";
 break;
 }
 }

 // q5Go pattern: If board doesn't exist, this is the FIRST Command 15 move!
 // Create the board AND send "moves N" command
 if (!target_board && !games_with_moves_requested.contains(current_game_context)) {
 qDebug() << "[q5Go] First Command 15 move for game" << current_game_context << "- creating board now!";

 // Create the board (matching what we removed from Command 9)
 target_board = new BoardWindow(this, login_username);
 connect(target_board, &BoardWindow::boardClosed, this, &FixedXGospelWindow::closeBoardWindow);
 connect(target_board, &BoardWindow::saveRequested, this, &FixedXGospelWindow::saveBoardGame);
 connect(target_board, &BoardWindow::resignRequested, this, &FixedXGospelWindow::resignGame);
 connect(target_board, &BoardWindow::commentRequested, this, &FixedXGospelWindow::sendComment);
 connect(target_board, &BoardWindow::sayRequested, this, &FixedXGospelWindow::sendSay);
 connect(target_board, &BoardWindow::tellRequested, this, &FixedXGospelWindow::sendTell);
 connect(target_board, &BoardWindow::observersRequested, this, &FixedXGospelWindow::requestObservers);
 connect(target_board, &BoardWindow::observerClicked, this, [this](const QString &name) { if (players_window) players_window->openStatsDialogForPlayer(name); });
 connect(target_board, &BoardWindow::whitePlayerClicked, this, [this](const QString &name) { if (players_window) players_window->openStatsDialogForPlayer(name); });
 connect(target_board, &BoardWindow::blackPlayerClicked, this, [this](const QString &name) { if (players_window) players_window->openStatsDialogForPlayer(name); });
 connect(target_board, &BoardWindow::moveRequested, this, &FixedXGospelWindow::sendMove);
 board_windows.append(target_board);
 most_recently_observed_board = target_board; // Track for teaching title assignment

 // Get player info from Command 7 maps
 QString white_name = game_white_player_map.value(current_game_context, "Unknown");
 QString black_name = game_black_player_map.value(current_game_context, "Unknown");
 QString white_rank = "?";
 QString black_rank = "?";
 if (players_window) {
 white_rank = findPlayerRank(white_name);
 black_rank = findPlayerRank(black_name);
 }

 target_board->startObserving(current_game_context, white_name, black_name, white_rank, black_rank);

 // Apply stored game details from Command 7
 if (game_komi_map.contains(current_game_context)) {
 double stored_komi = game_komi_map[current_game_context];
 int stored_handicap = game_handicap_map.value(current_game_context, 0);
 QString stored_type = game_type_map.value(current_game_context, "Free");
 target_board->updateGameSetup(stored_handicap, stored_komi, QString("Type: %1").arg(stored_type));
 target_board->updateGameDetails(stored_type, 0);
 }

 target_board->show();
 if (!this->suppress_server_console) output_console->append(QString(">>> [q5Go] BOARD CREATED for game %1: %2 vs %3")
 .arg(current_game_context).arg(white_name).arg(black_name));
 }

 // q5Go pattern: Send "moves N" on first live move for this non-docked game
 if (target_board && !games_with_moves_requested.contains(current_game_context)) {
     games_with_moves_requested.insert(current_game_context);
     target_board->clearMoveHistoryBeforeMovesCommand();
     QString moves_cmd = QString("moves %1").arg(current_game_context);
     socket->write((moves_cmd + "\n").toUtf8());
     if (!suppress_server_console)
         output_console->append(QString(">>> [q5Go] SENT: %1").arg(moves_cmd));
     target_board = nullptr; // skip this catch-up move; history replay follows
 }
 } // end non-docked move routing
 }

 if (target_board) {
 move.game_id = current_game_context;
 target_board->processMove(move);
 qDebug() << "[MENU] MOVE ROUTED: Game" << current_game_context << "move" << move_part;
 // Refresh dock thumbnail for the active slot after the move is applied
 if (docked_pane_mode) {
     GameSlot *active_slot = findSlot(current_game_context);
     if (active_slot) {
         shared_board_window->snapshotBoardStateToSlot(active_slot);
         updateHoverPixmapForSlot(active_slot);
     }
 }
 } else if (!dock_move_handled) {
 qDebug() << "[WARN] MOVE SKIPPED:" << move_part << "- No target board for game context" << current_game_context;
 }

 // Bot mode: handle incoming moves — decoupled from UI routing.
 // The bot must respond to opponent moves even if target_board is temporarily null
 // (e.g. active_slot_game_id mismatch due to a concurrent UI update).
 // Skip during history replay — WAITING_FOR_MOVES0/REPLAYING moves are not live opponent moves.
 bool bot_slot_replaying = false;
 if (bot_mode_active && bot_game_id != -1) {
     if (GameSlot *bslot = findSlot(bot_game_id))
         bot_slot_replaying = (bslot->replay_state == GameSlot::WAITING_FOR_MOVES0 ||
                               bslot->replay_state == GameSlot::REPLAYING);
 }
 if (bot_mode_active && bot_game_id != -1 && current_game_context == bot_game_id && !bot_slot_replaying) {
     // Special case: "0(B): Handicap N" — parser sets x=-2, y=N.
     if (move.x == -2 && move.y > 0) {
         bot_handicap = move.y;
         output_console->append(QString("[BOT] Handicap placement detected: %1 stones — stored for engine setup").arg(bot_handicap));
     } else if (bot_engine_ready) {
         // Skip move_number==0 in handicap games — IGS replays hc stones already placed
         // by set_free_handicap; sending play black Q16 again causes "? illegal move".
         if (move.move_number == 0 && bot_handicap >= 2) {
             output_console->append(QString("[BOT] Skipping hc-stone replay: move 0 (%1) — engine already placed via set_free_handicap").arg(coordsToGtp(move.x, move.y)));
         } else {
             StoneColor opponent_color = (bot_color == BLACK_STONE) ? WHITE_STONE : BLACK_STONE;
             if (static_cast<StoneColor>(move.color) == opponent_color) {
                 // Update our remaining time from the slot (works docked and non-docked)
                 if (GameSlot *bslot = findSlot(bot_game_id)) {
                     bot_time_remaining   = (bot_color == BLACK_STONE) ? bslot->black_time_seconds : bslot->white_time_seconds;
                     bot_stones_remaining = (bot_color == BLACK_STONE) ? bslot->black_byo_moves    : bslot->white_byo_moves;
                 }
                 QString gtp_vertex = coordsToGtp(move.x, move.y);
                 onBotOpponentMove(gtp_vertex, bot_time_remaining, bot_stones_remaining);
             }
         }
     }
 }
 } else {
 // Failed to parse move - no output to reduce clutter
 }
 }
 }
 
 // Handle moves history response (contains multiple moves) - debug output removed for clarity
 
 // Parse incoming kibitz messages - check for username(rank): format
 if (line.toLower().contains("kibitz") || QRegExp("^[a-zA-Z0-9_]+\\([^)]+\\):").indexIn(line.trimmed()) != -1) {
 if (!this->suppress_server_console) output_console->append(QString(">>> RAW KIBITZ LINE DETECTED: %1").arg(line));
 
 // Try to extract username and message from common IGS formats
 QString kibitz_user;
 QString kibitz_message;
 
 // Format 1: IGS command 11 kibitz format variations
 if (line.startsWith("11 kibitz ") || line.startsWith("11 Kibitz ")) {
 // Try different patterns for IGS kibitz header
 // Pattern: "11 kibitz player1 vs player2 [game_number]"
 QRegExp kibitz_simple("^11 [Kk]ibitz ([^\\[]+) \\[([^\\]]+)\\]$");
 if (kibitz_simple.indexIn(line) != -1) {
 QString players = kibitz_simple.cap(1).trimmed();
 pending_kibitz_game = kibitz_simple.cap(2).toInt();
 // Extract first player name as kibitzer (this is a simplification)
 QStringList player_parts = players.split(" vs ");
 if (player_parts.size() > 0) {
 pending_kibitz_user = player_parts[0].trimmed();
 }
 if (!this->suppress_server_console) output_console->append(QString(">>> KIBITZ HEADER: User %1 in game %2").arg(pending_kibitz_user).arg(pending_kibitz_game));
 }
 // More complex pattern: "11 Kibitz username [rank]: Game player1 vs player2 [game_number]"
 else {
 QRegExp kibitz_header("^11 [Kk]ibitz ([^\\[]+) \\[([^\\]]+)\\]: Game (.+) \\[(\\d+)\\]$");
 if (kibitz_header.indexIn(line) != -1) {
 pending_kibitz_user = kibitz_header.cap(1).trimmed();
 pending_kibitz_game = kibitz_header.cap(4).toInt();
 if (!this->suppress_server_console) output_console->append(QString(">>> KIBITZ HEADER: User %1 in game %2").arg(pending_kibitz_user).arg(pending_kibitz_game));
 }
 }
 }
 // Format 2: "11 actual kibitz message text" (following header)
 else if (line.startsWith("11 ")) {
 kibitz_message = line.mid(5).trimmed(); // Remove "11 " prefix and trim
 // Use pending user if we have one, otherwise use "Unknown"
 kibitz_user = pending_kibitz_user.isEmpty() ? "Unknown" : pending_kibitz_user;
 if (!this->suppress_server_console) output_console->append(QString(">>> KIBITZ MESSAGE: %1: %2").arg(kibitz_user, kibitz_message));
 
 // Clear pending context after use
 if (!pending_kibitz_user.isEmpty()) {
 pending_kibitz_user.clear();
 pending_kibitz_game = -1;
 }
 }
 // Format 3: Try simple pattern like "quietone(6d): message"
 else {
 // Make sure to trim the line and handle the exact format
 QString trimmed_line = line.trimmed();
 QRegExp simple_kibitz("^([a-zA-Z0-9_]+)\\(([^)]+)\\):\\s*(.+)$");
 if (simple_kibitz.indexIn(trimmed_line) != -1) {
 kibitz_user = simple_kibitz.cap(1);
 kibitz_message = simple_kibitz.cap(3);
 if (!this->suppress_server_console) output_console->append(QString(">>> SIMPLE KIBITZ: %1: %2").arg(kibitz_user, kibitz_message));
 } else {
 // Try even simpler pattern - just look for username(rank): message anywhere in line
 QRegExp fallback_kibitz("([a-zA-Z0-9_]+)\\([^)]+\\):\\s*(.+)");
 if (fallback_kibitz.indexIn(trimmed_line) != -1) {
 kibitz_user = fallback_kibitz.cap(1);
 kibitz_message = fallback_kibitz.cap(2);
 if (!this->suppress_server_console) output_console->append(QString(">>> FALLBACK KIBITZ: %1: %2").arg(kibitz_user, kibitz_message));
 }
 }
 }
 
 // If we have a user and message, send to board
 if (!kibitz_user.isEmpty() && !kibitz_message.isEmpty()) {
 if (docked_pane_mode) {
     // Route to the game the kibitz belongs to (pending_kibitz_game),
     // falling back to active slot only if we have no game context.
     int target_id = (pending_kibitz_game > 0) ? pending_kibitz_game : active_slot_game_id;
     if (target_id != -1 && shared_board_window) {
         if (GameSlot *slot = findSlot(target_id)) {
             GameSlot::CommentEntry ce; ce.user = kibitz_user; ce.text = kibitz_message; ce.is_kibitz = true;
             slot->comments.append(ce);
             // Only display immediately if this slot is currently visible
             if (target_id == active_slot_game_id)
                 shared_board_window->processComment(kibitz_user, kibitz_message, true);
         }
     }
 } else {
 for (BoardWindow* board : board_windows) {
 if (board->isObserving()) {
 board->processComment(kibitz_user, kibitz_message, true);
 if (!this->suppress_server_console) output_console->append(QString(">>> Sent kibitz to board window"));
 }
 }
 }
 }
 }
 
 // Parse IGS Command 11 kibitz messages (main format used by IGS)
 if (line.startsWith("11 ")) {
 if (!this->suppress_server_console) output_console->append(QString(">>> DEBUG: Found Command 11 line: %1").arg(line));
 
 // Format: "11 Kibitz [username] [rank]: Game [player1] vs [player2] [game_number]"
 if (line.contains("Kibitz ") && line.contains(": Game ") && line.contains(" vs ")) {
 QRegExp kibitz_header("^11 Kibitz ([^\\[]+) \\[[^\\]]+\\]: Game .+ vs .+ \\[(\\d+)\\]");
 if (kibitz_header.indexIn(line) != -1) {
 QString kibitzer = kibitz_header.cap(1).trimmed();
 int game_id = kibitz_header.cap(2).toInt();
 if (!this->suppress_server_console) output_console->append(QString(">>> IGS COMMAND 11 KIBITZ HEADER: %1 for game %2").arg(kibitzer).arg(game_id));
 
 // Store for multi-line kibitz continuation
 current_kibitzer = kibitzer;
 current_kibitz_game = game_id;
 }
 }
 // Continuation lines for multi-line kibitz
 else if (!current_kibitzer.isEmpty() && line.startsWith("11 ")) {
 QString kibitz_message = line.mid(6); // Remove "11 " prefix
 if (!this->suppress_server_console) output_console->append(QString(">>> IGS COMMAND 11 KIBITZ MESSAGE: %1: %2").arg(current_kibitzer, kibitz_message));

 // Send to slot/board window
 if (docked_pane_mode) {
     if (GameSlot *slot = findSlot(current_kibitz_game)) {
         GameSlot::CommentEntry ce; ce.user = current_kibitzer; ce.text = kibitz_message; ce.is_kibitz = true;
         slot->comments.append(ce);
         if (current_kibitz_game == active_slot_game_id)
             shared_board_window->processComment(current_kibitzer, kibitz_message, true);
     }
 } else {
 for (BoardWindow* board : board_windows) {
 if (board->isObserving() && board->getObservedGameId() == current_kibitz_game) {
 board->processComment(current_kibitzer, kibitz_message, true);
 }
 }
 }
 }
 }
 
 // Parse NNGS-style kibitz messages (IGS command 19 format)
 if (line.startsWith("19 ")) {
 if (!this->suppress_server_console) output_console->append(QString(">>> DEBUG: Found Command 19 line: %1").arg(line));
 QString nngs_part = line.mid(3); // Remove "19 " prefix
 QString kibitz_user;
 QString kibitz_message;
 
 // Format: "19 (username): message" - this is the actual IGS kibitz format!
 if (nngs_part.startsWith("(")) {
 QRegExp paren_format("^\\(([^)]+)\\):\\s*(.+)$");
 if (paren_format.indexIn(nngs_part) != -1) {
 kibitz_user = paren_format.cap(1);
 kibitz_message = paren_format.cap(2);
 if (!this->suppress_server_console) output_console->append(QString(">>> IGS COMMAND 19 KIBITZ: %1: %2").arg(kibitz_user, kibitz_message));
 }
 }
 // Original NNGS formats
 else if (nngs_part.startsWith("--> ")) {
 // Format: "19 --> username message"
 QRegExp nngs_arrow("^--> (\\w+) (.+)$");
 if (nngs_arrow.indexIn(nngs_part) != -1) {
 kibitz_user = nngs_arrow.cap(1);
 kibitz_message = nngs_arrow.cap(2);
 if (!this->suppress_server_console) output_console->append(QString(">>> NNGS ARROW KIBITZ: %1: %2").arg(kibitz_user, kibitz_message));
 }
 } else if (nngs_part.contains("*:")) {
 // Format: "19 *username*: message" - This is SAY (private player communication), not kibitz!
 QRegExp nngs_say("^\\*([^*]+)\\*: (.+)$");
 if (nngs_say.indexIn(nngs_part) != -1) {
 kibitz_user = nngs_say.cap(1);
 kibitz_message = nngs_say.cap(2);
 if (!this->suppress_server_console) output_console->append(QString(">>> SAY MESSAGE (Command 19): %1: %2").arg(kibitz_user, kibitz_message));
 }
 }

 if (!kibitz_user.isEmpty() && !kibitz_message.isEmpty()) {
 // Determine if this is SAY (private message) or KIBITZ (public comment)
 bool is_say = nngs_part.contains("*:"); // Say messages have *username*: format

 int boards_updated = 0;
 if (docked_pane_mode) {
     if (active_slot_game_id != -1 && shared_board_window) {
         if (GameSlot *slot = findSlot(active_slot_game_id); slot && !slot->game_finished) {
             GameSlot::CommentEntry ce; ce.user = kibitz_user; ce.text = kibitz_message; ce.is_kibitz = !is_say;
             slot->comments.append(ce);
             shared_board_window->processComment(kibitz_user, kibitz_message, !is_say);
             boards_updated++;
         }
     }
 } else {
 for (BoardWindow* board : board_windows) {
 if (is_say) {
 // SAY messages go to PLAYING boards only
 if (board->isPlaying()) {
 // Check if sender is one of the players in this game
 QString white = board->getWhitePlayer();
 QString black = board->getBlackPlayer();

 if (kibitz_user.compare(white, Qt::CaseInsensitive) == 0 ||
 kibitz_user.compare(black, Qt::CaseInsensitive) == 0) {
 board->processComment(kibitz_user, kibitz_message, false); // false = not kibitz
 boards_updated++;
 if (!this->suppress_server_console) output_console->append(QString(">>> [OK] SAY DELIVERED to playing board: %1 vs %2")
 .arg(white, black));
 }
 }
 } else {
 // Regular kibitz goes to OBSERVING boards
 if (board->isObserving()) {
 board->processComment(kibitz_user, kibitz_message, true); // true = kibitz
 boards_updated++;
 }
 }
 }
 }

 if (is_say) {
 output_console->append(QString(">>> [OK] SAY SUCCESS: Delivered \"%1: %2\" to %3 playing board(s)")
 .arg(kibitz_user, kibitz_message).arg(boards_updated));
 } else if (!this->suppress_server_console) {
 output_console->append(QString(">>> [OK] KIBITZ SUCCESS: Sent \"%1: %2\" to %3 observing board(s)")
 .arg(kibitz_user, kibitz_message).arg(boards_updated));
 }
 }
 }
 
 // Parse IGS Command 7 - Game listings with handicap, komi, byoyomi, and game type
 if (line.startsWith("7 ")) {
 // Format: 7 [id] white [rank] vs. black [rank] (move size handicap komi byoyomi flags) (observers)
 // Actual format: 7 [255] YK [7d*] vs. SOJ [7d*] ( 56 19 0 5.5 4 I) ( 18)
 QRegExp gamesre("\\[\\s*(\\d+)\\s*\\]\\s+" "([^\\s]+)\\s+\\[\\s*([^\\]\\s]*)\\s*\\]\\s+vs\\.\\s+" "([^\\s]+)\\s+\\[\\s*([^\\]\\s]*)\\s*\\]\\s+" "\\(\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+([\\d\\-.]+)\\s+(\\d+)\\s+([^\\s\\)]+)\\)\\s+" "\\(\\s*(\\d+)\\).*");
 
 if (gamesre.indexIn(line) != -1) {
 int game_id = gamesre.cap(1).toInt();
 QString white = gamesre.cap(2);
 QString white_rank = gamesre.cap(3);
 QString black = gamesre.cap(4);
 QString black_rank = gamesre.cap(5);
 int move_count = gamesre.cap(6).toInt(); Q_UNUSED(move_count)
 int board_size = gamesre.cap(7).toInt(); Q_UNUSED(board_size)
 int handicap_val = gamesre.cap(8).toInt();
 double komi_val = gamesre.cap(9).toDouble();
 int byoyomi_val = gamesre.cap(10).toInt();
 
 qDebug() << "*** DEBUG KOMI: Command 7 parsed komi=" << komi_val << "for game" << game_id;
 QString game_flags = gamesre.cap(11);

 // Store game details for later use when board windows are created
 game_komi_map[game_id] = komi_val;
 game_handicap_map[game_id] = handicap_val;
 game_white_player_map[game_id] = white;
 game_black_player_map[game_id] = black;
 
 // Determine game type from flags
 // FI=Free, TI=Teaching (also free), I=Rated
 // Check specific flags first (FI, TI) before checking for "I" alone
 QString game_type = "Free"; // Default
 QString trimmed_flags = game_flags.trimmed();
 if (trimmed_flags == "FI" || trimmed_flags.startsWith("FI ")) {
 game_type = "Free";
 } else if (trimmed_flags == "TI" || trimmed_flags.startsWith("TI ")) {
 game_type = "Teaching"; // Teaching games are free
 } else if (trimmed_flags == "I" || trimmed_flags.startsWith("I ")) {
 game_type = "Rated";
 }
 game_type_map[game_id] = game_type;
 qDebug() << "[GAME] Game" << game_id << "- Type from flags [" << trimmed_flags << "] -> " << game_type;

 // Bug 32f: Auto-send "free" command if game is Rated but has handicap >= 4
 // IGS rule: Games with 4+ stone handicap should be free (unrated)
 // Both players must send "free" to make it work
 // Bug 34: DISABLED - IGS server rejects with "That command is currently disabled"
 // for games created via match protocol. Keeping code for reference.
 /*
 bool is_my_game = (white == my_account_name || black == my_account_name);
 if (is_my_game && game_type == "Rated" && handicap_val >= 4) {
 if (!this->suppress_server_console) output_console->append(QString(">>> AUTO-FREE: Game %1 has %2 stone handicap - sending 'free' command")
 .arg(game_id).arg(handicap_val));
 sendCommandString("free");
 // Update our local game type to reflect the request
 game_type = "Free (requested)";
 game_type_map[game_id] = game_type;
 }
 */

	if (!suppress_games_console) {
 output_console->append(QString("[INFO] Game %1 Details: %2[%3] vs %4[%5] - H:%6 K:%7 By:%8 Type:%9")
 .arg(game_id).arg(white, white_rank, black, black_rank)
 .arg(handicap_val).arg(komi_val, 0, 'f', 1).arg(byoyomi_val).arg(game_type));

	}
 // Update game setup for this game (komi, handicap, type, byoyomi)
 if (docked_pane_mode) {
     if (GameSlot *slot = findSlot(game_id)) {
         if (!slot->game_finished) {
             slot->komi = komi_val;
             slot->handicap = handicap_val;
             slot->byoyomi_time = byoyomi_val;
             if (!slot->game_type_locked)
                 slot->game_type = game_type;
             if (game_id == active_slot_game_id) {
                 shared_board_window->updateGameSetup(handicap_val, komi_val, QString("Byoyomi: %1s").arg(byoyomi_val));
                 shared_board_window->updateGameDetails(game_type, byoyomi_val);
             }
         }
     }
 } else {
 // Update any board window (both observing AND playing) for this game with complete details.
 // Guard: skip finished boards and boards whose players don't match (IGS reuses game IDs).
 for (BoardWindow* board : board_windows) {
 if (board->getObservedGameId() == game_id && !board->isFinished()) {
 // Verify player names match so a reused game ID doesn't corrupt a stale open board.
 bool players_match = (board->getWhitePlayer() == white || board->getWhitePlayer() == "?" ||
                       board->getBlackPlayer() == black || board->getBlackPlayer() == "?");
 if (!players_match) {
 qDebug() << "*** KOMI GUARD: Skipping game" << game_id << "board update - players mismatch ("
          << board->getWhitePlayer() << "vs" << white << ") - likely reused game ID";
 continue;
 }
 qDebug() << "*** DEBUG KOMI: Calling updateGameSetup with komi=" << komi_val << "for game" << game_id;
 board->updateGameSetup(handicap_val, komi_val, QString("Byoyomi: %1s").arg(byoyomi_val));
 qDebug() << ">>> [CMD7] UPDATING BOARD GAME TYPE: Game" << game_id << "setting type to" << game_type;
 board->updateGameDetails(game_type, byoyomi_val);
 if (!this->suppress_server_console) output_console->append(QString(">>> [CMD7] Board for game %1 updated to type: %2").arg(game_id).arg(game_type));
 }
 }
 }

 // Buffer this Command 7 line for auto-populating games window on login
 // Convert Command 7 format to games list format for consistent parsing
 // Command 7: "7 [id] white [rank] vs. black [rank] (moves size handicap komi byoyomi flags) (observers)"
 // Games format: "[id] white [rank] vs. black [rank] (moves size handicap komi byoyomi flags) (observers)"
 QString game_line = line.mid(2).trimmed(); // Remove "7 " prefix
 buffered_game_lines.append(game_line);

 // If we're in the auto-launch pending state, check if games window exists yet
 // If it does, trigger population. If not, we'll trigger it when the window is created.
 if (auto_launch_populate_pending && games_window) {
 if (buffered_game_lines.count() == 1) {
 // First game buffered - start population from buffer after small delay
 QTimer::singleShot(50, this, &FixedXGospelWindow::populateGamesFromBufferIfPending);
 }
 }
 }
 }
 
 // Parse IGS server time responses (arrive passively with game time events)
 // "9 The current time (GMT) is: Fri May 28 07:09:31 2026"  — use this as authoritative server time
 // "9 The current IGS local time is: Fri May 28 16:09:31 2026"  — JST (UTC+9), secondary
 if (line.startsWith("9 The current time (GMT) is:")) {
     QString ts = line.mid(QString("9 The current time (GMT) is:").length()).trimmed();
     QDateTime parsed = QDateTime::fromString(ts, "ddd MMM d hh:mm:ss yyyy");
     if (!parsed.isValid())
         parsed = QDateTime::fromString(ts, "ddd MMM  d hh:mm:ss yyyy");
     if (parsed.isValid()) {
         // Server is joyjoy.net (Tokyo, JST = UTC+9) — offset GMT to get server local time
         server_time_value    = parsed.addSecs(9 * 3600);
         server_time_received = QDateTime::currentDateTimeUtc();
         server_time_set      = true;
         qDebug() << "[CLOCK] Server time set from GMT:" << server_time_value.toString();
     }
 }
 if (line.startsWith("9 The current IGS local time is:") && !server_time_set) {
     QString ts = line.mid(QString("9 The current IGS local time is:").length()).trimmed();
     QDateTime parsed = QDateTime::fromString(ts, "ddd MMM d hh:mm:ss yyyy");
     if (!parsed.isValid())
         parsed = QDateTime::fromString(ts, "ddd MMM  d hh:mm:ss yyyy");
     if (parsed.isValid()) {
         server_time_value    = parsed;
         server_time_received = QDateTime::currentDateTimeUtc();
         server_time_set      = true;
         qDebug() << "[CLOCK] Server time set from local line:" << parsed.toString();
     }
 }

 // Parse IGS Command 9 - Komi messages
 if (line.startsWith("9 ")) {
 QString msg_part = line.mid(2); // Remove "9 " prefix
 if (msg_part.contains("komi", Qt::CaseInsensitive)) {
 // Format: "9 Komi is now set to 5.5" or "9 player wants the komi to be 1.5"
 QRegExp komi_set("Komi is now set to ([\\d\\-.]+)");
 QRegExp komi_want("wants the komi to be ([\\d\\-.]+)");
 
 double komi_val = 0.0;
 if (komi_set.indexIn(msg_part) != -1) {
 komi_val = komi_set.cap(1).toDouble();
 } else if (komi_want.indexIn(msg_part) != -1) {
 komi_val = komi_want.cap(1).toDouble();
 }
 
 if (komi_val != 0.0) {
 if (docked_pane_mode) {
     // Apply to active slot only (komi messages from server are per-current-game context)
     if (active_slot_game_id != -1) {
         if (GameSlot *slot = findSlot(active_slot_game_id)) {
             if (!slot->game_finished) {
                 slot->komi = komi_val;
                 shared_board_window->updateGameSetup(slot->handicap, komi_val);
             }
         }
     }
 } else {
 // Update all observing board windows
 for (BoardWindow* board : board_windows) {
 if (board->isObserving()) {
 board->updateGameSetup(board->getHandicap(), komi_val);
 }
 }
 }
 }
 }
 }
 
 // Command 22: IGS territory data (multi-line format) - MUST be BEFORE Command 9 check
 // This handler MUST be outside the "if (line.startsWith("9 "))" block!
 // Format: 22 PlayerName Rank Captures Time... (header lines x2)
 // 22 0: 0550501444414110505 (territory row 0-18)
 // ...
 // 22 18: 5001411000050055555 (territory row 18)
 // Territory encoding: 0=black stone, 1=white stone, 4=white territory, 5=black territory

 // Match Command 22 header: "22 PlayerName Rank Captures Time..."
 // Format: 22 <name> <rank> <captures> <rating> <wins> T <komi> <handicap>
 QRegExp cmd22_header_re("^22\\s+(\\w+)\\s+\\S+\\s+(\\d+)\\s+"); // name + captures
 QRegExp cmd22_row_re("^22\\s+(\\d+):\\s+([012345]+)$"); // Match "22 0: 0550501..."

 if (cmd22_header_re.indexIn(line) != -1) {
 // This is one of the two header lines
 QString player_name = cmd22_header_re.cap(1);
 int player_captures = cmd22_header_re.cap(2).toInt();

 if (!receiving_territory_data) {
 // First header line — begin territory data reception, save capture count
 receiving_territory_data = true;
 territory_data_column = 0;
 territory_player_name = player_name;
 cmd22_first_captures  = player_captures;
 if (!this->suppress_server_console) output_console->append(QString(">>> CMD22: Territory data beginning for player: %1 (captures=%2)").arg(player_name).arg(player_captures));

 // Find the board window matching this player name
 territory_board = nullptr;
 territory_slot  = nullptr;
 if (docked_pane_mode) {
     for (GameSlot *slot : game_slots) {
         if (slot->game_finished) continue; // skip completed games — same players appear in rematches
         if (slot->is_playing && !slot->is_scoring_mode) continue; // bot playing slot must be in scoring mode before CMD22
         if (slot->white_player == player_name || slot->black_player == player_name) {
             if (slot->game_id == active_slot_game_id && shared_board_window) {
                 // Active game — route directly to the shared board window
                 territory_board = shared_board_window;
                 territory_board->receiveScoreBegin();
             } else {
                 // Inactive slot — buffer territory rows in the slot itself
                 territory_slot = slot;
                 slot->territory_ownership.clear();
                 slot->receiving_territory_data = true;
                 slot->territory_data_row = 0;
                 slot->is_scoring_mode = true;
             }
             break;
         }
     }
 } else {
 for (BoardWindow* board : board_windows) {
 if (board->isObserving() && !board->isFinished()) {
 // Match by player name (could be white or black); skip finished games (rematches share names)
 if (board->getWhitePlayer() == player_name || board->getBlackPlayer() == player_name) {
 territory_board = board;
 territory_board->receiveScoreBegin();
 if (!this->suppress_server_console) output_console->append(QString(">>> CMD22: Matched territory data to game %1 (W:%2 vs B:%3)")
 .arg(board->getObservedGameId())
 .arg(board->getWhitePlayer())
 .arg(board->getBlackPlayer()));
 break;
 }
 }
 }
 } // end else (non-docked)

 if (!territory_board && !territory_slot) {
 if (!this->suppress_server_console) output_console->append(QString(">>> CMD22 WARNING: Could not find board matching player %1").arg(player_name));
 }
 } else {
     // Second header line — both capture counts now known; update the board.
     // The CMD22 header prisoner counts are authoritative (server-tracked).
     // They replace any replay-counted white_captures/black_captures which
     // can be inflated by scoring-phase removals replayed as regular moves.
     if (territory_board) {
         // Active slot — update board window directly.
         int white_caps, black_caps;
         if (territory_board->getWhitePlayer() == territory_player_name) {
             white_caps = cmd22_first_captures;
             black_caps = player_captures;
         } else {
             white_caps = player_captures;
             black_caps = cmd22_first_captures;
         }
         territory_board->updateCaptures(white_caps, black_caps);
         if (!this->suppress_server_console) output_console->append(
             QString(">>> CMD22: Set captures (active) W=%1 B=%2 from server header").arg(white_caps).arg(black_caps));
     } else if (territory_slot) {
         // Inactive slot — write CMD22 header captures into the slot directly.
         // The counting result handler reads slot->white_captures for its prisoner
         // calculation, so setting them here ensures the authoritative server values
         // are used instead of the replay-counted values.
         int white_caps, black_caps;
         if (territory_slot->white_player == territory_player_name) {
             white_caps = cmd22_first_captures;
             black_caps = player_captures;
         } else {
             white_caps = player_captures;
             black_caps = cmd22_first_captures;
         }
         territory_slot->white_captures = white_caps;
         territory_slot->black_captures = black_caps;
         if (!this->suppress_server_console) output_console->append(
             QString(">>> CMD22: Set captures (inactive) W=%1 B=%2 from server header").arg(white_caps).arg(black_caps));
     }
 }
 } else if (cmd22_row_re.indexIn(line) != -1 && receiving_territory_data) {
 // This is a territory row - process it
 int row_number = cmd22_row_re.cap(1).toInt();
 QString row_data = cmd22_row_re.cap(2);

 if (!this->suppress_server_console) output_console->append(QString(">>> CMD22: Row %1 = %2").arg(row_number).arg(row_data));

 if (territory_board) {
 territory_board->receiveScoreLine(row_number, row_data);
 } else if (territory_slot) {
 // Buffer row into slot's territory_ownership map for later decode
 qDebug() << "[CMD22-INACTIVE] game" << territory_slot->game_id
          << "row" << row_number << "data:" << row_data;
 for (int col = 0; col < row_data.length() && col < 19; col++)
     territory_slot->territory_ownership[QPair<int,int>(row_number, col)] = row_data[col].digitValue();
 }

 // If this is the last row (18), end territory reception
 if (row_number == 18) {
 if (territory_board) {
 territory_board->receiveScoreEnd();
 bot_overlay_active = false; // CMD22 authoritative data now in control; stop protecting KataGo overlay
 if (!this->suppress_server_console) output_console->append(QString(">>> CMD22: Territory data complete for game %1").arg(territory_board->getObservedGameId()));
 } else if (territory_slot) {
 // Decode territory_ownership into slot->territory_map and extract dead stones.
 // Same logic as receiveScoreEnd() but reads from slot->board_state instead of board widget.
 territory_slot->territory_map.clear();
 territory_slot->dead_stone_positions.clear();
 int dbg_d4=0, dbg_d5=0, dbg_dame=0, dbg_dead_on_d4=0, dbg_dead_on_d5=0;
 for (auto it = territory_slot->territory_ownership.constBegin();
      it != territory_slot->territory_ownership.constEnd(); ++it) {
     int digit = it.value();
     QPair<int,int> pos = it.key();
     if (digit == 4) {
         dbg_d4++;
         territory_slot->territory_map[pos] = static_cast<int>(WHITE_STONE);
         // Black stone in white territory = dead
         if (territory_slot->board_state[pos.first][pos.second] == static_cast<int>(BLACK_STONE)) {
             territory_slot->dead_stone_positions.insert(pos);
             dbg_dead_on_d4++;
         }
     } else if (digit == 5) {
         dbg_d5++;
         territory_slot->territory_map[pos] = static_cast<int>(BLACK_STONE);
         // White stone in black territory = dead
         if (territory_slot->board_state[pos.first][pos.second] == static_cast<int>(WHITE_STONE)) {
             territory_slot->dead_stone_positions.insert(pos);
             dbg_dead_on_d5++;
         }
     } else if (digit == 2 || digit == 3) {
         // Dame / neutral — store as EMPTY so loadSlot() restores the green square
         // overlay immediately on slot switch without requiring a slider round-trip.
         dbg_dame++;
         territory_slot->territory_map[pos] = static_cast<int>(EMPTY);
     }
 }
 qDebug() << ">>> CMD22 inactive slot game" << territory_slot->game_id
          << "d4(W-terr):" << dbg_d4 << "d5(B-terr):" << dbg_d5 << "dame:" << dbg_dame
          << "dead-on-d4:" << dbg_dead_on_d4 << "dead-on-d5:" << dbg_dead_on_d5
          << "territory points:" << territory_slot->territory_map.size()
          << "dead stones detected:" << territory_slot->dead_stone_positions.size();
 if (!this->suppress_server_console)
     output_console->append(QString(">>> CMD22: Territory data buffered for inactive game %1 (%2 points, %3 dead)")
         .arg(territory_slot->game_id).arg(territory_slot->territory_map.size())
         .arg(territory_slot->dead_stone_positions.size()));
 territory_slot = nullptr;
 }
 receiving_territory_data = false;
 territory_board = nullptr;
 }
 }

 // Parse IGS Command 9 - Multiple purposes: scoring mode trigger AND counting results
 if (line.startsWith("9 ")) {
 // Parse custom game titles for teaching games
 // Format: "9 Game is titled: The 70th Kansai Ki-In Best Player Tournament, 1st Round, Nishi Takemoto"
 if (line.contains("Game is titled:")) {
 QString title_line = line.mid(2).trimmed(); // Remove "9 " prefix
 if (title_line.startsWith("Game is titled:")) {
 QString game_title = title_line.mid(15).trimmed(); // Remove "Game is titled:" prefix
 if (!this->suppress_server_console) output_console->append(QString(">>> CUSTOM TITLE DETECTED: %1").arg(game_title));

 // Apply teaching title to the board for the current game context.
 // "Game is titled:" arrives via Command 9 right after observation begins,
 // before any Command 15 move sets current_game_context — fall back to
 // most_recently_observed_game_id in that case.
 int title_game_id = (current_game_context != -1)
                     ? current_game_context
                     : most_recently_observed_game_id;
 if (title_game_id != -1) {
 // Clear the fallback immediately — title consumed, don't let it bleed to other games
 if (current_game_context == -1)
     most_recently_observed_game_id = -1;
 if (docked_pane_mode) {
     if (GameSlot *slot = findSlot(title_game_id)) {
         slot->custom_game_title = game_title;
         if (title_game_id == active_slot_game_id)
             shared_board_window->setCustomGameTitle(game_title);
     }
 } else {
 BoardWindow *target_board = nullptr;
 for (BoardWindow* board : board_windows) {
 if (board->getObservedGameId() == title_game_id) {
 target_board = board;
 break;
 }
 }
 if (target_board != nullptr) {
 target_board->setCustomGameTitle(game_title);
 } else {
 if (!this->suppress_server_console) output_console->append(QString(">>> WARNING: CUSTOM TITLE for game %1 but no board found: %2").arg(title_game_id).arg(game_title));
 }
 }
 } else {
 if (!this->suppress_server_console) output_console->append(QString(">>> WARNING: CUSTOM TITLE DETECTED but no game context: %1").arg(game_title));
 }
 }
 }

 // Check for scoring mode trigger
 if (line.contains("check your score with the score command")) {
 if (!this->suppress_server_console) output_console->append(">>> IGS REMOVESTONES: Game entered scoring phase - following q5Go protocol");

 // Bot mode scoring phase entry.
 // Protocol: 3 consecutive passes trigger CMD9. Each player independently sends
 // remove commands for dead groups in their territory, then done. IGS scores
 // when both dones are received. Bot waits 1500ms after ownership analysis
 // before sending removes+done to ensure IGS scoring window is fully open.
 if (bot_mode_active && bot_game_id != -1 && !bot_scoring_pending) {
     // "Board is restored" re-sends this line — reset done/pass2 flags so the
     // 1500ms timer lambda (guarded by !bot_done_sent) can fire again.
     if (bot_done_sent) {
         bot_done_sent      = false;
         bot_pass2_rendered = false;
         output_console->append("[BOT] Scoring phase re-entered (Board is restored) — resetting done flags");
     }
     bot_scoring_pending = true;
     bot_cached_ownership.clear();
     KataGoEngine *engine = engine_manager->currentEngine();
     if (engine) {
         if (engine->isOwnershipInFlight()) {
             // Prefetch already running — it will deliver to onBotOwnershipReady.
             // bot_scoring_pending=true ensures it's treated as the scoring result.
             output_console->append("[BOT] Scoring phase — ownership already in flight, will use when ready");
         } else {
             output_console->append("[BOT] Scoring phase — requesting kata-raw-nn ownership");
             engine->requestOwnership();
         }
     }
 }

 // Implement q5Go's slot_removestones functionality
 if (docked_pane_mode) {
     // Enter scoring mode for the active game
     if (active_slot_game_id != -1 && shared_board_window) {
         if (GameSlot *slot = findSlot(active_slot_game_id)) {
             slot->is_scoring_mode = true;
             shared_board_window->enterScoringMode();
         }
     }
 } else {
 for (BoardWindow* board : board_windows) {
 if (board->isObserving()) {
 int game_id = board->getObservedGameId();
 if (!this->suppress_server_console) output_console->append(QString(">>> Game %1 entering scoring mode (modeScoreRemote equivalent)").arg(game_id));

 // Enter scoring mode exactly like q5Go's enter_scoring_mode(true)
 board->enterScoringMode();

 // Enable dead stone clicking and territory marking
 if (!this->suppress_server_console) output_console->append(QString(">>> SCORE MODE: click on a stone to mark as dead... (Game %1)").arg(game_id));
 }
 }
 }
 }

 // "Board is restored" — IGS reset scoring because a player continued marking after typing done.
 // Both players must re-submit all removals from scratch before typing done again.
 if (bot_mode_active && bot_game_id != -1 &&
         line.contains("Board is restored to what it was when you started scoring")) {
     output_console->append("[BOT] Board restored by IGS — re-running ownership analysis and re-sending removals");
     bot_done_sent      = false;
     bot_scoring_pending = true;
     bot_pass2_rendered = false;
     bot_pending_removes.clear();
     bot_pending_remove_positions.clear();
     bot_pending_remove_groups.clear();
     bot_remove_index        = 0;
     bot_remove_retry_offset = 0;
     bot_cached_ownership.clear();
     KataGoEngine *engine = engine_manager->currentEngine();
     if (engine) {
         if (engine->isOwnershipInFlight()) {
             output_console->append("[BOT] Board restored — ownership already in flight, will use when ready");
         } else {
             output_console->append("[BOT] Board restored — requesting kata-raw-nn ownership");
             engine->requestOwnership();
         }
     }
 }

 // Opponent typed done during scoring phase — line format: "9 <player> has typed done."
 if (bot_mode_active && line.contains("has typed done.")) {
     QString msg = line.startsWith("9 ") ? line.mid(2) : line;
     QString who = msg.section(' ', 0, 0);
     if (who != login_username) {
         output_console->append(QString("[BOT] Opponent %1 has typed done — waiting for score").arg(who));
         // Render pass 2 once: opponent has finished removing dead stones (CMD49s already
         // applied to board); re-render with both sides' removals reflected.
         // IGS can send "has typed done" multiple times — guard with bot_pass2_rendered.
         if (bot_done_sent && !bot_pass2_rendered) {
             bot_pass2_rendered = true;
             applyBotTerritoryOverlay("Pass 2 (opponent done)");
         }
         // If opponent types done again after bot already sent done (re-negotiation after
         // additional stone removals), re-send done to keep scoring in sync.
         if (bot_done_sent && bot_pass2_rendered) {
             socket->write("done\n"); socket->flush();
             output_console->append("[BOT] >>> done (re-send after opponent re-done)");
         }
     }
 }

 // Check for counting results: Format: 9 {Game 8: jlclub33 vs b53092967 : W 108.5 B 113.0}
 QRegExp count_result_re("9\\s+\\{Game\\s+(\\d+):\\s+[^:]+:\\s+([WB])\\s+([\\d\\.]+)\\s+([WB])\\s+([\\d\\.]+)\\}");
 if (count_result_re.indexIn(line) != -1) {
 int game_id = count_result_re.cap(1).toInt();
 QString first_color = count_result_re.cap(2);
 double first_score = count_result_re.cap(3).toDouble();
 QString second_color = count_result_re.cap(4);
 double second_score = count_result_re.cap(5).toDouble();
 
 double white_score = (first_color == "W") ? first_score : second_score;
 double black_score = (first_color == "B") ? first_score : second_score;
 
 QString result_text;
 if (black_score > white_score) {
 result_text = QString("B+%1").arg(black_score - white_score);
 } else if (white_score > black_score) {
 result_text = QString("W+%1").arg(white_score - black_score);
 } else {
 result_text = "Draw";
 }
 
 if (!this->suppress_server_console) output_console->append(QString(">>> COUNTING RESULT: Game %1 - %2 (W:%3 B:%4)").arg(game_id).arg(result_text).arg(white_score).arg(black_score));

 if (docked_pane_mode) {
     if (GameSlot *slot = findSlot(game_id); slot && !slot->game_finished) {
         slot->game_result = result_text;
         slot->game_finished = true;
         slot->clock_timer->stop();
         slot->server_white_score = white_score;
         slot->server_black_score = black_score;
         slot->has_server_score = true;
         slot->is_scoring_mode = true;
         if (game_id == active_slot_game_id) {
             shared_board_window->setServerScore(white_score, black_score);
             // Score first so white_prisoners/black_prisoners include dead stones
             // before updateGameResult() reads them for the comment block.
             shared_board_window->enterScoringModeForResult();
             shared_board_window->updateGameResult(result_text);
         } else {
             // Inactive slot: if Command 22 already gave us territory + dead stones,
             // count territory from the map. Otherwise flood-fill from board_state.
             if (slot->territory_map.isEmpty()) {
                 calculateTerritoryForSlot(slot);
             } else {
                 // CMD22 is authoritative — do NOT cross-reference board_state.
                 // board_state may be incomplete due to mid-stream join / partial
                 // replay.  dead_stone_positions was built directly from CMD22 by
                 // detecting opponent stones sitting in the other side's territory
                 // cells, so it is reliable regardless of board_state accuracy.
                 //
                 // Count all CMD22 territory directly: digit 4 = white territory,
                 // digit 5 = black territory.  Dead stone positions (opponent stone
                 // sitting in territory) are ALSO territory — the stone is removed
                 // and the point scores.  Do NOT exclude dead_stone_positions here;
                 // they are territory AND prisoners simultaneously (Japanese rules).
                 slot->white_territory = 0;
                 slot->black_territory = 0;
                 for (auto it = slot->territory_map.constBegin();
                      it != slot->territory_map.constEnd(); ++it) {
                     if (it.value() == static_cast<int>(WHITE_STONE)) slot->white_territory++;
                     else if (it.value() == static_cast<int>(BLACK_STONE)) slot->black_territory++;
                 }
                 qDebug() << ">>> TERR-LOOP game" << game_id
                          << "CMD22-based (no board_state)"
                          << "counted W-terr:" << slot->white_territory
                          << "B-terr:" << slot->black_territory
                          << "dead:" << slot->dead_stone_positions.size();
             }
             // Prisoners: captures + dead stones.
             // dead_stone_positions comes from CMD22 decode — authoritative.
             // dead-on-d4 = black stones in white territory → white prisoners.
             // dead-on-d5 = white stones in black territory → black prisoners.
             // We don't need board_state to determine color: the CMD22 digit
             // tells us which side's territory the dead stone is in.
             slot->white_prisoners = slot->white_captures;
             slot->black_prisoners = slot->black_captures;
             // Iterate territory_ownership to find dead stone colors from CMD22 digit.
             for (const auto &pos : slot->dead_stone_positions) {
                 int digit = slot->territory_ownership.value(pos, -1);
                 // digit 4 = white territory (IGS CMD22) → stone there is black → white gains prisoner
                 if (digit == 4) slot->white_prisoners++;
                 // digit 5 = black territory (IGS CMD22) → stone there is white → black gains prisoner
                 else if (digit == 5) slot->black_prisoners++;
                 else {
                     // Fallback: use board_state color if ownership digit unavailable
                     int cell = slot->board_state[pos.first][pos.second];
                     if (cell == static_cast<int>(WHITE_STONE)) slot->black_prisoners++;
                     else if (cell == static_cast<int>(BLACK_STONE)) slot->white_prisoners++;
                 }
             }
             slot->final_score = (slot->white_territory + slot->white_prisoners + slot->komi)
                                 - (slot->black_territory + slot->black_prisoners);
             qDebug() << ">>> INACTIVE SLOT SCORE game" << game_id
                      << "W-terr:" << slot->white_territory << "W-pris:" << slot->white_prisoners
                      << "komi:" << slot->komi
                      << "-> W-total:" << (slot->white_territory + slot->white_prisoners + slot->komi)
                      << "B-terr:" << slot->black_territory << "B-pris:" << slot->black_prisoners
                      << "-> B-total:" << (slot->black_territory + slot->black_prisoners)
                      << "final:" << slot->final_score
                      << "(server W:" << white_score << "B:" << black_score << ")";
         }
     }
 } else {
     for (BoardWindow* board : board_windows) {
         if (board->isObserving() && board->getObservedGameId() == game_id) {
             board->setServerScore(white_score, black_score);
             // enterScoringModeForResult() must run first: it calls calculateScore()
             // which sets white_prisoners/black_prisoners (captures + dead stones).
             // updateGameResult() reads those values to build the comment text,
             // so calling it before scoring gave wrong totals (captures only, no dead).
             board->enterScoringModeForResult();
             board->updateGameResult(result_text);
         }
     }
 }

 // Remove finished game from observed games tracking (unhighlight in Games window)
 untrackFinishedGame(game_id);
 }
 
 // Check for resignation results: Format: 9 {Game 92: yanaruuu vs EDW13 : Black resigns.}
 QRegExp resign_result_re("9\\s+\\{Game\\s+(\\d+):\\s+.*:\\s+(Black|White)\\s+resigns\\.\\}");
 if (resign_result_re.indexIn(line) != -1) {
 int game_id = resign_result_re.cap(1).toInt();
 QString who_resigned = resign_result_re.cap(2).trimmed();
 
 QString result_text;
 if (who_resigned == "White") {
 result_text = "B+R"; // White resigned, Black wins
 } else if (who_resigned == "Black") {
 result_text = "W+R"; // Black resigned, White wins
 } else {
 result_text = "?+R"; // Unknown who resigned
 }
 
 if (!this->suppress_server_console) output_console->append(QString(">>> RESIGN RESULT: Game %1 - %2 (%3 resigned)").arg(game_id).arg(result_text).arg(who_resigned));

 // Update board window/slot with result (observing OR playing)
 if (docked_pane_mode) {
     if (GameSlot *slot = findSlot(game_id); slot && !slot->game_finished) {
         slot->game_result = result_text; slot->game_finished = true;
         slot->clock_timer->stop();
         if (game_id == active_slot_game_id) shared_board_window->updateGameResult(result_text);
     }
 } else {
 for (BoardWindow* board : board_windows) {
 if (board->getObservedGameId() == game_id) {
 board->updateGameResult(result_text);
 if (!this->suppress_server_console) output_console->append(QString(">>> Updated board window %1 with result: %2").arg(board->getObservedGameId()).arg(result_text));
 }
 }
 }

 // Remove finished game from observed games tracking (unhighlight in Games window)
 untrackFinishedGame(game_id);
 }

 // Check for simple resignation format: "9 quietone has resigned the game."
 QRegExp simple_resign_re("9\\s+([^\\s]+)\\s+has\\s+resigned\\s+the\\s+game\\.");
 if (simple_resign_re.indexIn(line) != -1) {
 QString who_resigned_name = simple_resign_re.cap(1).trimmed();
 
 // Find which game this belongs to by looking for current playing game
 int game_id = -1;
 QString result_text;

 if (docked_pane_mode) {
     // In docked mode prefer bot_game_id when resigner is bot opponent,
     // otherwise fall back to active_slot_game_id (observer watching a game)
     if (bot_mode_active && bot_game_id != -1 &&
             who_resigned_name.compare(bot_opponent, Qt::CaseInsensitive) == 0) {
         game_id = bot_game_id;
     } else {
         game_id = active_slot_game_id;
     }
     if (game_id != -1) {
         if (GameSlot *slot = findSlot(game_id)) {
             if (slot->white_player.compare(who_resigned_name, Qt::CaseInsensitive) == 0) result_text = "B+R";
             else if (slot->black_player.compare(who_resigned_name, Qt::CaseInsensitive) == 0) result_text = "W+R";
             else result_text = "?+R";
         }
     }
 } else {
 for (BoardWindow* board : board_windows) {
 if (board->isPlaying()) {
 game_id = board->getObservedGameId();
 if (board->getWhitePlayer() == who_resigned_name) result_text = "B+R";
 else if (board->getBlackPlayer() == who_resigned_name) result_text = "W+R";
 else result_text = "?+R";
 break;
 }
 }
 }

 if (game_id != -1) {
 if (!this->suppress_server_console) output_console->append(QString(">>> SIMPLE RESIGN: Game %1 - %2 (%3 resigned)").arg(game_id).arg(result_text).arg(who_resigned_name));

 if (docked_pane_mode) {
     if (GameSlot *slot = findSlot(game_id); slot && !slot->game_finished) {
         slot->game_result = result_text; slot->game_finished = true;
         slot->clock_timer->stop();
         // Always update the board — switch to this slot if not currently visible
         if (game_id == active_slot_game_id)
             shared_board_window->updateGameResult(result_text);
         else {
             // Bot game may not be the active slot; switch to it so result is shown
             switchActiveGame(game_id);
             shared_board_window->updateGameResult(result_text);
         }
     }
     // For bot games: send farewell immediately while opponent is still online.
     // Do this before untrackFinishedGame so bot_game_id is still valid.
     if (bot_mode_active && game_id == bot_game_id && !bot_opponent.isEmpty() && !bot_farewell_sent) {
         bot_farewell_sent = true;
         QString farewell = QString("Thank you for the game %1!").arg(bot_opponent);
         socket->write((QString("tell %1 %2\n").arg(bot_opponent, farewell)).toUtf8());
         socket->flush();
         output_console->append(QString("[BOT] >>> tell %1: %2").arg(bot_opponent, farewell));
         if (GameSlot *bslot = findSlot(bot_game_id)) {
             GameSlot::CommentEntry ce; ce.user = login_username; ce.text = farewell; ce.is_kibitz = false;
             bslot->comments.append(ce);
         }
         if (engine_board)
             engine_board->processComment(login_username, farewell, false);
     }
 } else {
 // Update board window with result
 for (BoardWindow* board : board_windows) {
 if (board->getObservedGameId() == game_id) {
 board->updateGameResult(result_text);
 if (!this->suppress_server_console) output_console->append(QString(">>> Updated board window %1 with result: %2").arg(board->getObservedGameId()).arg(result_text));
 }
 }
 }

 // Remove finished game from observed games tracking (unhighlight in Games window)
 untrackFinishedGame(game_id);
 }
 }
 
 // Check for no-move/no-greeting time loss: "9 <player> lost the game <N> due to no-move."
 // Also covers "due to no-greeting" and any other "due to no-*" IGS variants.
 QRegExp no_move_re("9\\s+(\\S+)\\s+lost the game\\s+(\\d+)\\s+due to no-\\w+\\.");
 if (no_move_re.indexIn(line) != -1) {
     QString loser    = no_move_re.cap(1).trimmed();
     int     game_id  = no_move_re.cap(2).toInt();
     QString result_text;
     if (docked_pane_mode) {
         if (GameSlot *slot = findSlot(game_id); slot && !slot->game_finished) {
             if (slot->white_player.compare(loser, Qt::CaseInsensitive) == 0) result_text = "B+T";
             else result_text = "W+T";
             slot->game_result = result_text; slot->game_finished = true;
             slot->clock_timer->stop();
             if (game_id == active_slot_game_id) shared_board_window->updateGameResult(result_text);
         }
     }
     if (!this->suppress_server_console)
         output_console->append(QString(">>> NO-MOVE TIME LOSS: Game %1 — %2 lost on time (%3)").arg(game_id).arg(loser).arg(result_text));
     untrackFinishedGame(game_id);
 }

 // Check for time forfeit results: Format: 9 {Game 23: ion125 vs gotoplay19 : White forfeits on time.}
 QRegExp forfeit_result_re("9\\s+\\{Game\\s+(\\d+):\\s+.*:\\s+(Black|White)\\s+forfeits on time\\.\\}");
 if (forfeit_result_re.indexIn(line) != -1) {
 int game_id = forfeit_result_re.cap(1).toInt();
 QString who_forfeited = forfeit_result_re.cap(2).trimmed();

 QString result_text;
 if (who_forfeited == "White") {
 result_text = "B+T"; // White forfeited on time, Black wins
 } else if (who_forfeited == "Black") {
 result_text = "W+T"; // Black forfeited on time, White wins
 } else {
 result_text = "?+T"; // Unknown who forfeited
 }

 if (!this->suppress_server_console) output_console->append(QString(">>> TIME FORFEIT RESULT: Game %1 - %2 (%3 forfeited on time)").arg(game_id).arg(result_text).arg(who_forfeited));

 if (docked_pane_mode) {
     if (GameSlot *slot = findSlot(game_id); slot && !slot->game_finished) {
         slot->game_result = result_text; slot->game_finished = true;
         slot->clock_timer->stop();
         if (game_id == active_slot_game_id) shared_board_window->updateGameResult(result_text);
     }
 } else {
 for (BoardWindow* board : board_windows) {
 if (board->getObservedGameId() == game_id) {
 board->updateGameResult(result_text);
 if (!this->suppress_server_console) output_console->append(QString(">>> Updated board window %1 with result: %2").arg(board->getObservedGameId()).arg(result_text));
 }
 }
 }
 untrackFinishedGame(game_id);
 }

 // Check for adjourned games: Format: 9 Game 636: SanchaX vs PandaBot3 has adjourned.
 QRegExp adjourn_re("9\\s+Game\\s+(\\d+):\\s+.*\\s+has\\s+adjourned\\.");
 if (adjourn_re.indexIn(line) != -1) {
 int game_id = adjourn_re.cap(1).toInt();
 QString result_text = "Adjourned";

 if (!this->suppress_server_console) output_console->append(QString(">>> ADJOURNED RESULT: Game %1 - %2").arg(game_id).arg(result_text));

 if (docked_pane_mode) {
     if (GameSlot *slot = findSlot(game_id); slot && !slot->game_finished) {
         slot->game_result = result_text; slot->game_finished = true;
         slot->clock_timer->stop();
         if (game_id == active_slot_game_id) shared_board_window->updateGameResult(result_text);
     }
 } else {
 for (BoardWindow* board : board_windows) {
 if (board->getObservedGameId() == game_id) {
 board->updateGameResult(result_text);
 if (!this->suppress_server_console) output_console->append(QString(">>> Updated board window %1 with result: %2").arg(board->getObservedGameId()).arg(result_text));
 }
 }
 }
 untrackFinishedGame(game_id);
 }
 }

 // "9 Stored games for <user>:" — response to our "stored" command sent at login.
 // Display a prominent notice so the user knows adjourned games are waiting on the server.
 {
 if (line.startsWith("9 Stored games for ") && line.endsWith(":")) {
     stored_header_seen = true;
     stored_game_lines.clear();
 } else if (stored_header_seen) {
     // Table header and separator lines — skip
     if (line.startsWith("9  #") || line.startsWith("9 ---")) {
         // skip
     } else if (line.startsWith("9  ") && line.contains(":")) {
         // Game entry line: "9  1: white [rank] black [rank]  ..."
         // Extract both player names; record the one that isn't us as a known opponent.
         QString entry = line.mid(2).trimmed(); // strip "9 " prefix
         stored_game_lines.append(entry);
         // Format after stripping index: "white [rank] black [rank] ..."
         // Strip leading "N: "
         QString rest = entry.section(':', 1).trimmed();
         QRegExp stored_re("^(\\S+)\\s+\\[.*?\\]\\s+(\\S+)\\s+\\[");
         if (stored_re.indexIn(rest) != -1) {
             QString p1 = stored_re.cap(1);
             QString p2 = stored_re.cap(2);
             if (p1.compare(login_username, Qt::CaseInsensitive) != 0)
                 stored_adjourned_opponents.insert(p1.toLower());
             if (p2.compare(login_username, Qt::CaseInsensitive) != 0)
                 stored_adjourned_opponents.insert(p2.toLower());
         }
     } else {
         // End of stored games block
         if (stored_header_seen) {
             stored_header_seen = false;
             if (stored_game_lines.isEmpty()) {
                 output_console->append(">>> [STORED] No adjourned games on server.");
             } else {
                 output_console->append(QString(">>> [STORED] %1 adjourned game(s) waiting on server — opponent(s) can reconnect to resume:").arg(stored_game_lines.size()));
                 for (const QString &entry : stored_game_lines)
                     output_console->append(QString("    %1").arg(entry));
             }
             stored_game_lines.clear();
         }
     }
 }
 }

 // CMD48: "48 Game N has been adjourned by <player>" — opponent dropped connection.
 // The game may resume within 5 minutes if the player reconnects — do NOT mark finished.
 // Just stop the clock and show Adjourned status; keep the slot alive for resumption.
 // The true end signal is "*SYSTEM*: Disconnected game with X was removed." (5-min timeout).
 {
 QRegExp cmd48_re("^48\\s+Game\\s+(\\d+)\\s+has been adjourned by\\s+(\\S+)");
 if (cmd48_re.indexIn(line) != -1) {
     int game_id = cmd48_re.cap(1).toInt();
     QString who_adjourned = cmd48_re.cap(2).trimmed();
     output_console->append(QString(">>> CMD48 ADJOURN: Game %1 adjourned by %2 (may resume)").arg(game_id).arg(who_adjourned));
     if (docked_pane_mode) {
         if (GameSlot *slot = findSlot(game_id)) {
             slot->adjourned_player = who_adjourned;
             slot->clock_timer->stop();
             // Show adjourned status but keep game_finished=false so slot stays active
             if (game_id == active_slot_game_id && shared_board_window)
                 shared_board_window->processComment("*SYSTEM*", "Game adjourned — waiting for opponent to reconnect...", false);
         }
     }
 }
 }

 // "9 X has restored your old game." / "9 X has restarted your game." — player reconnected
 // and issued the 'load' command. The next "9 Observing game N" will carry the new game ID.
 // Set pending_resume_player so that "Observing game" handler can do the slot reassignment
 // even if the old slot's ID was already recycled (player names may have been overwritten).
 {
 QRegExp restore_re("^9\\s+(\\S+)\\s+has\\s+(?:restored your old game|restarted your game)");
 if (restore_re.indexIn(line) != -1) {
     pending_resume_player = restore_re.cap(1).trimmed();
     output_console->append(QString(">>> ADJOURN RESTORE: %1 reconnected — watching for new game ID").arg(pending_resume_player));
 }
 }

 // CMD18: "18 <game-filename>" — IGS returns stored game filenames in response to "stored".
 // Capture the filename so we can use "load <filename>" instead of "load <opponent>".
 // The filename form (e.g. "woodnstone-weakkyu") works after the 5-min window closes.
 {
     QRegExp cmd18_re("^18\\s+(\\S+-\\S+)$");
     if (!reconnect_opponent.isEmpty() && cmd18_re.indexIn(line) != -1) {
         QString game_file = cmd18_re.cap(1);
         // Only capture if it involves our reconnect opponent
         if (game_file.contains(reconnect_opponent, Qt::CaseInsensitive) ||
             game_file.contains(login_username, Qt::CaseInsensitive)) {
             reconnect_game_file = game_file;
             output_console->append(QString("[BOT] Stored game found: %1 — sending load command").arg(reconnect_game_file));
             socket->write(QString("load %1\n").arg(reconnect_game_file).toUtf8());
         }
     }
 }

 // "You are in disconnected game" — IGS sends this when we reconnect while the game is
 // still in the 5-minute active window. CMD67 should arrive automatically; no load needed.
 if (line.contains("You are in disconnected game") && !reconnect_opponent.isEmpty()) {
     output_console->append(QString("[BOT] IGS: game still active (disconnected window) — waiting for CMD67"));
 }

 // "*SYSTEM*: Disconnected game with <player> was removed." — opponent did not reconnect
 // within the 5-minute window. Now truly mark the game finished and protect the slot
 // from game ID reuse overwriting player names.
 if (line.contains("Disconnected game with") && line.contains("was removed")) {
     // Extract player name to find the right slot
     QRegExp removed_re("Disconnected game with (\\S+) was removed");
     if (removed_re.indexIn(line) != -1) {
         QString dropped_player = removed_re.cap(1).trimmed();
         if (!this->suppress_server_console)
             output_console->append(QString(">>> DISCONNECT REMOVED: %1's game removed — marking finished").arg(dropped_player));
         // If we were waiting for this player to resume, give up now.
         if (pending_resume_player.compare(dropped_player, Qt::CaseInsensitive) == 0)
             pending_resume_player.clear();
         if (docked_pane_mode) {
             for (GameSlot *slot : game_slots) {
                 if (!slot->game_finished &&
                     (slot->white_player.compare(dropped_player, Qt::CaseInsensitive) == 0 ||
                      slot->black_player.compare(dropped_player, Qt::CaseInsensitive) == 0)) {
                     // Dropped player loses by resign; opponent wins
                     QString result_text;
                     if (slot->white_player.compare(dropped_player, Qt::CaseInsensitive) == 0)
                         result_text = "B+R"; // white dropped — black wins
                     else
                         result_text = "W+R"; // black dropped — white wins
                     slot->game_result = result_text; slot->game_finished = true;
                     slot->clock_timer->stop();
                     if (slot->game_id == active_slot_game_id && shared_board_window)
                         shared_board_window->updateGameResult(result_text);
                     untrackFinishedGame(slot->game_id);
                     break;
                 }
             }
         } else {
             for (BoardWindow *board : board_windows) {
                 if (!board->isFinished() &&
                     (board->getWhitePlayer().compare(dropped_player, Qt::CaseInsensitive) == 0 ||
                      board->getBlackPlayer().compare(dropped_player, Qt::CaseInsensitive) == 0)) {
                     QString result_text;
                     if (board->getWhitePlayer().compare(dropped_player, Qt::CaseInsensitive) == 0)
                         result_text = "B+R";
                     else
                         result_text = "W+R";
                     board->updateGameResult(result_text);
                     break;
                 }
             }
         }
     }
 }

 // Fallback: "9 game completed." arrives when IGS finishes scoring but omits CMD20.
 // If the bot sent done but never received CMD20, clean up here so botEndGame() fires.
 if (line == "9 game completed." && bot_mode_active && bot_game_id != -1 && bot_done_sent && !bot_cmd20_received) {
     output_console->append(QString("[BOT] Game %1 completed (no CMD20) — cleaning up via game completed signal").arg(bot_game_id));
     untrackFinishedGame(bot_game_id);
 }

 // Capture who ran out of time: "9 <player> has run out of time."
 {
     QRegExp run_out_re("9\\s+(\\S+)\\s+has run out of time\\.");
     if (run_out_re.indexIn(line) != -1)
         bot_time_forfeit_loser = run_out_re.cap(1).trimmed();
 }

 // Time forfeit path: "9 Removed game file <login>-<opponent> from database."
 // This arrives when the bot (or opponent) runs out of time — no CMD20 is sent.
 // Guard with !bot_cmd20_received: a normal scored game also sends "Removed game file"
 // after CMD20 + "9 game completed." — without this guard it fires as a false positive.
 if (bot_mode_active && bot_game_id != -1 && !bot_cmd20_received && line.startsWith("9 Removed game file ") && line.contains(login_username)) {
     QString result_text;
     if (!bot_time_forfeit_loser.isEmpty()) {
         if (bot_time_forfeit_loser.compare(login_username, Qt::CaseInsensitive) == 0)
             result_text = (bot_color == BLACK_STONE) ? "W+T" : "B+T"; // bot lost on time
         else
             result_text = (bot_color == BLACK_STONE) ? "B+T" : "W+T"; // opponent lost on time
     } else {
         result_text = "?+T";
     }
     output_console->append(QString("[BOT] Game %1 ended via time forfeit — %2 (%3)").arg(bot_game_id).arg(result_text).arg(
         bot_time_forfeit_loser.isEmpty() ? "unknown loser" : bot_time_forfeit_loser + " ran out of time"));
     bot_time_forfeit_loser.clear();
     if (docked_pane_mode) {
         if (GameSlot *slot = findSlot(bot_game_id)) {
             slot->game_result = result_text; slot->game_finished = true;
             slot->clock_timer->stop();
             if (bot_game_id == active_slot_game_id) shared_board_window->updateGameResult(result_text);
         }
     }
     untrackFinishedGame(bot_game_id);
 }

 // DEBUG: Show all Command 9 and 15 messages to understand protocol (DISABLED to reduce spam)
 // if (line.startsWith("9 ")) {
 // if (!this->suppress_server_console) output_console->append(QString(">>> DEBUG CMD9: %1").arg(line));
 // }
 // if (line.startsWith("15 ")) {
 // if (!this->suppress_server_console) output_console->append(QString(">>> DEBUG CMD15: %1").arg(line));
 // }
 
 // Parse IGS Command 9 - Stone removal echo (no game ID — handled by CMD49)
 // "9 Removing @ H15" — log only; CMD49 carries the game ID and does the marking.
 if (line.startsWith("9 ") && line.contains("Removing @")) {
     QString coords_part = line.section('@', 1).trimmed();
     if (!this->suppress_server_console) output_console->append(QString(">>> STONE REMOVAL: %1").arg(coords_part));
 }
 
 // CMD5 error: "5 You cannot remove a liberty." — sent when the bot tries to
 // toggle a coordinate that is empty (opponent already removed that group).
 // Retry the current dead group with an alternate stone from the group.
 if (line.startsWith("5 ") && line.contains("cannot remove a liberty") && bot_mode_active) {
     output_console->append("[BOT] Removal rejected (liberty) — trying alternate stone in group");
     botHandleRemovalRejected();
 }

 // CMD5 recovery: "5 It is not your turn." — IGS rejected our move because the server
 // had not yet committed the opponent's previous move when we sent ours.  KataGo has
 // already placed the stone internally, so we must undo it and re-request genmove.
 if (line.startsWith("5 ") && line.contains("not your turn") && bot_mode_active &&
         !bot_last_sent_vertex.isEmpty()) {
     output_console->append(QString("[BOT] IGS rejected move %1 (not your turn) — sending GTP undo and retrying in 500ms")
         .arg(bot_last_sent_vertex));
     bot_last_sent_vertex.clear();
     KataGoEngine *engine = engine_manager->currentEngine();
     if (engine) {
         engine->enqueueRaw("undo");  // remove the rejected stone from KataGo's board
         StoneColor bc = bot_color;
         QTimer::singleShot(500, this, [this, bc]() {
             KataGoEngine *eng = engine_manager->currentEngine();
             if (eng && bot_game_id != -1)
                 eng->requestGenmove(bc);
         });
     }
 }

 // Parse IGS Command 49 - Dead stone coordinate messages
 // Format: "49 Game <id> <player> is removing @ <coord>"
 if (line.startsWith("49 ") && line.contains("is removing @")) {
 QString pt   = line.section(' ', 7, 7); // coordinate (e.g. H15)
 QString game = line.section(' ', 2, 2); // game ID
 QString who  = line.section(' ', 3, 3); // player name
 
 if (!this->suppress_server_console) output_console->append(QString(">>> IGS REMOVESTONES: Game %1 - %2 removing @ %3").arg(game, who, pt));
 
 // Process the dead stone marking exactly like q5Go
 int game_id = game.toInt();

 // Parse coordinate once (used for both docked and non-docked paths)
 int dead_col = -1, dead_row = -1;
 if (!pt.isEmpty() && pt.length() >= 2) {
 int i = pt[0].toLatin1() - 'A' + 1;
 if (i > 8) i--;
 int j;
 if (pt.length() > 2 && pt[2].toLatin1() >= '0' && pt[2].toLatin1() <= '9')
     j = 19 + 1 - pt.mid(1,2).toInt();
 else
     j = 19 + 1 - pt[1].digitValue();
 dead_col = i - 1;
 dead_row = j - 1;
 }

 if (dead_col >= 0 && dead_row >= 0) {
 if (docked_pane_mode) {
     // Skip bot's own removes in docked mode — already marked in the 1500ms timer.
     // Echoing them again would double-toggle (un-mark) the group.
     bool is_bot_own_remove_docked = (bot_mode_active && bot_game_id == game_id && who == login_username);
     if (GameSlot *slot = findSlot(game_id); slot && !slot->game_finished) {
         if (game_id == active_slot_game_id) {
             if (!is_bot_own_remove_docked) {
                 shared_board_window->markStoneAsDead(dead_col, dead_row);
                 // markStoneAsDead calls calculateScore() which overwrites our KataGo overlay.
                 // Re-apply if bot overlay is active so territory stays correct during scoring.
                 if (bot_overlay_active && bot_game_id == game_id)
                     applyBotTerritoryOverlay(bot_pass2_rendered ? "Pass 2 (restore)" : "Pass 1 (restore)");
             }
         } else {
             // Inactive slot: flood-fill the connected group from board_state
             const int bs = slot->board_size;
             int seed_color = slot->board_state[dead_col][dead_row];
             if (seed_color != static_cast<int>(EMPTY)) {
                 QStack<QPair<int,int>> stack;
                 QSet<QPair<int,int>> visited;
                 stack.push({dead_col, dead_row});
                 while (!stack.isEmpty()) {
                     auto cur = stack.pop();
                     if (visited.contains(cur)) continue;
                     int cx = cur.first, cy = cur.second;
                     if (cx < 0 || cx >= bs || cy < 0 || cy >= bs) continue;
                     if (slot->board_state[cx][cy] != seed_color) continue;
                     visited.insert(cur);
                     slot->dead_stones.insert(cur);
                     slot->dead_stone_positions.insert(cur);
                     stack.push({cx-1,cy}); stack.push({cx+1,cy});
                     stack.push({cx,cy-1}); stack.push({cx,cy+1});
                 }
                 qDebug() << ">>> IS-REMOVING: inactive slot game" << game_id
                          << "marked" << visited.size() << "stones dead at" << pt;
             }
         }
     }
 } else {
 // Skip markStoneAsDead for the bot's own toggles — already marked visually in the 1500ms timer.
 // Applying it again from the server echo would double-toggle (un-mark) the group.
 bool is_bot_own_remove = (bot_mode_active && bot_game_id == game_id && who == login_username);
 for (BoardWindow* board : board_windows) {
 if (board->isObserving() && board->getObservedGameId() == game_id) {
 if (!this->suppress_server_console) output_console->append(QString(">>> Marking dead stone at (%1,%2) from IGS coord %3").arg(dead_col).arg(dead_row).arg(pt));
 if (!is_bot_own_remove) {
     board->markStoneAsDead(dead_col, dead_row);
     // markStoneAsDead calls calculateScore() which overwrites our KataGo overlay.
     // Re-apply if bot overlay is active so territory stays correct during scoring.
     if (bot_overlay_active && bot_game_id == game_id)
         applyBotTerritoryOverlay(bot_pass2_rendered ? "Pass 2 (restore)" : "Pass 1 (restore)");
 }
 break;
 }
 }
 }
 }

 // CMD49 log + sequential removal advancement
 if (bot_mode_active && bot_game_id == game_id) {
     if (who == login_username) {
         output_console->append(QString("[BOT] Server confirmed our removal @ %1").arg(pt));
         // Advance to next group and send its removal command
         bot_remove_index++;
         bot_remove_retry_offset = 0;
         botSendNextRemoval();
     } else {
         output_console->append(QString("[BOT] Opponent %1 removing @ %2 (board updated)").arg(who).arg(pt));
     }
 }
 }
 
 // TODO: Implement proper IGS territory data parsing based on specific protocol triggers
 // The current auto-detection approach was too aggressive and interfered with normal gameplay
 
 // DEBUG: Catch any messages that might contain game result information
 if (line.contains("resign", Qt::CaseInsensitive) || 
 line.contains("concede", Qt::CaseInsensitive) ||
 line.contains("wins by", Qt::CaseInsensitive) ||
 line.contains("lost by", Qt::CaseInsensitive) ||
 line.contains("Game.*finished", Qt::CaseInsensitive) ||
 line.contains("Result", Qt::CaseInsensitive)) {
 if (!this->suppress_server_console) output_console->append(QString(">>> DEBUG RESULT MESSAGE: %1").arg(line));
 }
 
 // Parse match setup messages to determine game type: "CLIENT: <q5go 2.0> match wants handicap 0, komi 5.5, free"
 if (line.contains("CLIENT:") && line.contains("match") && line.contains("wants")) {
 if (line.contains(" free", Qt::CaseInsensitive)) {
 // This is a free game - store this for the next game that gets created
 next_game_is_free = true;
 qDebug() << "[MENU] MATCH SETUP: Detected FREE game from client message";
 } else if (line.contains(" rated", Qt::CaseInsensitive)) {
 next_game_is_free = false;
 qDebug() << "[MENU] MATCH SETUP: Detected RATED game from client message";
 }
 }
 
 // Detect incoming match requests: "9 Match[19x19] in 1 minute requested with BusyBee as White."
 // Bug 32c: Handle both "Match[" (no space) and "Match [" (with space), and "minute"/"minutes"
 if (line.startsWith("9 ") && line.contains("Match") && line.contains("requested with") && line.contains("as ")) {
 output_console->append(QString("[DEBUG] DEBUGGING: Detected potential match request message: %1").arg(line));
 QRegExp match_request("9 Match ?\\[([^\\]]+)\\] in (\\d+) minutes? requested with ([^\\s]+) as (Black|White)\\.");
 if (match_request.indexIn(line) != -1) {
 output_console->append(QString("[OK] REGEX MATCHED: Match request parsed successfully"));
 QString board_info = match_request.cap(1);
 QString time_str = match_request.cap(2);
 QString opponent = match_request.cap(3);
 QString opponent_color = match_request.cap(4);  // Bug 32e: This is opponent's color, not mine!

 // Bug 32e: Invert the color - if opponent wants White, I play Black
 QString my_color = (opponent_color == "White") ? "Black" : "White";

 if (!this->suppress_server_console) output_console->append(QString(">>> INCOMING MATCH REQUEST: %1 wants %2 as %3, I play %4")
 .arg(opponent).arg(board_info).arg(opponent_color).arg(my_color));

 // Find opponent's rank
 QString opponent_rank = findPlayerRank(opponent);

 if (!bot_mode_active) {
     // Show incoming match request dialog (human handles it)
     handleIncomingMatchRequest(opponent, opponent_rank, board_info, time_str, my_color);
 }
 // Bot mode: accept is sent from the "Use <match ...>" suggested-command handler below
 } else {
 output_console->append(QString("[ERROR] REGEX FAILED: Could not parse match request: %1").arg(line));
 }
 }
 
 // Parse server's suggested match command: "9 Use <match BusyBee B 19 1 5> or <decline BusyBee> to respond."
 // Format: match <opp> <color> <size> <main_min> <byo_min>
 if (line.startsWith("9 ") && line.contains("Use <match ") && line.contains("> or <decline")) {
     QRegExp cmd_regex("Use <(match [^>]+)> or");
     if (cmd_regex.indexIn(line) != -1) {
         QString suggested_cmd = cmd_regex.cap(1);
         qDebug() << "[MATCH] Server suggested command:" << suggested_cmd;
         if (bot_mode_active) {
             // Parse: match <opp> <color> <size> <main_min> <byo_min>
             QStringList parts = suggested_cmd.split(' ');
             if (parts.size() >= 6) {
                 QString opp      = parts[1];
                 if (bot_game_id != -1) {
                     // Already in a game — decline to avoid stomping active engine
                     output_console->append(QString("[BOT] Declining match from %1 — game already in progress").arg(opp));
                     socket->write((QString("decline %1\n").arg(opp)).toUtf8());
                     socket->flush();
                 } else if (const QStringList bl = settings->getBotBlacklist();
                            std::any_of(bl.constBegin(), bl.constEnd(),
                         [&opp](const QString &entry){ return entry.compare(opp, Qt::CaseInsensitive) == 0; })) {
                     output_console->append(QString("[BOT] Declining match from %1 — player is blacklisted").arg(opp));
                     socket->write((QString("decline %1\n").arg(opp)).toUtf8());
                     socket->flush();
                 } else {
                     StoneColor myCol = (parts[2].toUpper() == "B") ? BLACK_STONE : WHITE_STONE;
                     int bsize        = parts[3].toInt();
                     int main_s       = parts[4].toInt() * 60;   // minutes → seconds
                     int byo_s        = parts[5].toInt() * 60;   // minutes → seconds
                     // Old match protocol doesn't carry handicap or stones — use IGS defaults
                     botAcceptMatch(opp, suggested_cmd, myCol, bsize, 0, main_s, byo_s, 25, 6.5);
                 }
             }
         } else if (pending_match_dialog) {
             pending_match_dialog->setServerSuggestedCommand(suggested_cmd);
             if (!this->suppress_server_console) output_console->append(QString(">>> Server suggests: %1").arg(suggested_cmd));
         }
     }
 }

 // Parse server's suggested nmatch command: "9 Use <nmatch opp B 0 19 600 300 25 0 0 0> or <decline opp> to respond."
 // Format: nmatch <opp> <color> <hc> <size> <main_s> <byo_s> <stones> ...
 if (line.startsWith("9 ") && line.contains("Use <nmatch ") && line.contains("> or <decline")) {
     QRegExp cmd_regex("Use <(nmatch [^>]+)> or");
     if (cmd_regex.indexIn(line) != -1) {
         QString suggested_cmd = cmd_regex.cap(1);
         qDebug() << "[NMATCH] Server suggested command:" << suggested_cmd;
         if (bot_mode_active) {
             // Parse: nmatch <opp> <color> <hc> <size> <main_s> <byo_s> <stones> ...
             QStringList parts = suggested_cmd.split(' ');
             if (parts.size() >= 8) {
                 QString opp      = parts[1];
                 if (bot_game_id != -1) {
                     // Already in a game — decline to avoid stomping active engine
                     output_console->append(QString("[BOT] Declining nmatch from %1 — game already in progress").arg(opp));
                     socket->write((QString("decline %1\n").arg(opp)).toUtf8());
                     socket->flush();
                 } else if (const QStringList bl = settings->getBotBlacklist();
                            std::any_of(bl.constBegin(), bl.constEnd(),
                         [&opp](const QString &entry){ return entry.compare(opp, Qt::CaseInsensitive) == 0; })) {
                     output_console->append(QString("[BOT] Declining nmatch from %1 — player is blacklisted").arg(opp));
                     socket->write((QString("decline %1\n").arg(opp)).toUtf8());
                     socket->flush();
                 } else {
                     // parts[2] is the color for the bot ("B", "W", or "N" for nigiri).
                     // Use WHITE as placeholder for "N" — CMD 67 will correct it authoritatively.
                     StoneColor myCol = (parts[2].toUpper() == "B") ? BLACK_STONE : WHITE_STONE;
                     int hc           = parts[3].toInt();
                     int bsize        = parts[4].toInt();
                     int main_s       = parts[5].toInt();
                     int byo_s        = parts[6].toInt();
                     int stones       = parts[7].toInt();
                     double def_komi  = (hc >= 2) ? 0.5 : 6.5;
                     botAcceptMatch(opp, suggested_cmd, myCol, bsize, hc, main_s, byo_s, stones, def_komi);
                 }
             }
         } else if (pending_match_dialog) {
             pending_match_dialog->setServerSuggestedCommand(suggested_cmd);
             if (!this->suppress_server_console) output_console->append(QString(">>> Server suggests: %1").arg(suggested_cmd));
         }
     }
 }

 // Detect incoming nmatch requests: "9 NMatch requested with username(B 3 19 60 600 25 0 0 0)."
 if (line.startsWith("9 ") && line.contains("NMatch requested with")) {
 output_console->append(QString("[DEBUG] DEBUGGING: Detected potential nmatch request message: %1").arg(line));
 QRegExp nmatch_request("9 NMatch requested with ([^\\(]+)\\(([^\\)]+)\\)\\.");
 if (nmatch_request.indexIn(line) != -1) {
 QString opponent = nmatch_request.cap(1).trimmed();
 QString nmatch_params = nmatch_request.cap(2);

 if (!this->suppress_server_console) output_console->append(QString(">>> INCOMING NMATCH REQUEST: %1 with params %2")
 .arg(opponent).arg(nmatch_params));

 if (!bot_mode_active) {
     // Find opponent's rank
     QString opponent_rank = findPlayerRank(opponent);
     // Parse nmatch parameters: "B 3 19 60 600 25 0 0 0"
     QStringList params = nmatch_params.split(' ');
     if (params.size() >= 5) {
         QString my_color  = params[0];
         QString handicap  = params[1];
         QString board_size = params[2];
         QString time_seconds = params[3];
         QString byo_seconds  = params[4];
         handleIncomingNmatchRequest(opponent, opponent_rank, my_color, handicap, board_size, time_seconds, byo_seconds);
     }
 }
 // Bot mode: actual accept is sent from the "Use <nmatch ...>" suggested-command handler above
 }
 }
 
 // "9 Creating match [N] with opponent." — IGS confirms the new game ID.
 // Record it so the stale-CMD15 guard does not block the genuine first CMD15 for this game.
 if (line.startsWith("9 ") && line.contains("Creating match [")) {
     QRegExp creating_re("9 Creating match \\[(\\d+)\\]");
     if (creating_re.indexIn(line) != -1)
         newly_confirmed_game_id = creating_re.cap(1).toInt();
 }

 // Debug: Log all Command 9 messages to help identify actual format (DISABLED to reduce spam)
 // if (line.startsWith("9 ")) {
 // output_console->append(QString("[INFO] DEBUG: All Command 9 messages: %1").arg(line));
 // }

 // Parse IGS Command 21 - Shout (broadcast) messages
 // Format: 21 !username!: message
 if (line.startsWith("21 !")) {
     QRegExp shout_re("^21 !([^!]+)!:\\s*(.*)$");
     if (shout_re.indexIn(line) != -1) {
         QString sender  = shout_re.cap(1).trimmed();
         QString message = shout_re.cap(2).trimmed();
         if (!this->suppress_server_console)
             output_console->append(QString("!%1!: %2").arg(sender, message));
         if (shout_window)
             shout_window->addShout(sender, message);
     }
 }

 // Parse IGS Command 21 - Game resume notification
 // Format: 21 {Game <id>: <white> vs <black> @ Move <move_num>}
 if (line.startsWith("21 ")) {
 QRegExp resume_re("21\\s+\\{Game\\s+(\\d+):\\s+(\\S+)\\s+vs\\s+(\\S+)\\s+@\\s+Move\\s+(\\d+)\\}");
 if (resume_re.indexIn(line) != -1) {
     int game_id  = resume_re.cap(1).toInt();
     QString white = resume_re.cap(2).remove('*');
     QString black = resume_re.cap(3).remove('*');
     int at_move   = resume_re.cap(4).toInt();
     if (!this->suppress_server_console)
         output_console->append(QString(">>> GAME RESUMED: Game %1 (%2 vs %3) at move %4")
                                .arg(game_id).arg(white).arg(black).arg(at_move));

     // Only reactivate if we already have a slot for this game (we were observing it).
     // Do NOT auto-observe random server resume broadcasts for games we never requested.
     if (docked_pane_mode) {
         if (GameSlot *slot = findSlot(game_id)) {
             if (!slot->game_finished)
                 observeGame(game_id, white, black, "?", "?");
         }
     } else {
     BoardWindow* existing = nullptr;
     for (BoardWindow* board : board_windows) {
         if (board->getObservedGameId() == game_id && board->isFinished()) {
             existing = board;
             break;
         }
     }
     if (existing) {
         // Reactivate the existing window and re-subscribe on IGS
         existing->stopObserving();
         existing->startObserving(game_id, white, black, "?", "?");
         existing->show();
         existing->raise();
         observed_game_ids.insert(game_id);
         if (games_window) games_window->updateObservedGames(observed_game_ids);
         socket->write((QString("observe %1\n").arg(game_id)).toUtf8());
         if (!this->suppress_server_console)
             output_console->append(QString(">>> SENT: observe %1 (game resumed)").arg(game_id));
     } else {
         // No window open for this game — open a fresh observation
         observeGame(game_id, white, black, "?", "?");
     }
     } // end non-docked path for game resume
 }
 }

 // Parse IGS Command 21 - Resignation and other game results
 if (line.startsWith("21 ")) {
 // Format: 21 {Game 124: Player1* vs Player2* : Black lost by Resign}
 QRegExp result_re("21\\s+\\{Game\\s+(\\d+):\\s+([^:]+):\\s+(.+)\\}");
 if (result_re.indexIn(line) != -1) {
 int game_id = result_re.cap(1).toInt();
 QString result_text = result_re.cap(3).trimmed();
 
 // Convert detailed result to standard format if needed
 QString standard_result = result_text;
 if (result_text.contains("lost by Resign", Qt::CaseInsensitive)) {
 if (result_text.contains("Black lost", Qt::CaseInsensitive)) {
 standard_result = "W+R";
 } else if (result_text.contains("White lost", Qt::CaseInsensitive)) {
 standard_result = "B+R";
 }
 } else if (result_text.contains("lost by time", Qt::CaseInsensitive)) {
 if (result_text.contains("Black lost", Qt::CaseInsensitive)) {
 standard_result = "W+T";
 } else if (result_text.contains("White lost", Qt::CaseInsensitive)) {
 standard_result = "B+T";
 }
 }
 
 if (!this->suppress_server_console) output_console->append(QString(">>> RESIGN/TIME RESULT: Game %1 - %2 (Raw: %3)").arg(game_id).arg(standard_result).arg(result_text));

 if (docked_pane_mode) {
     if (GameSlot *slot = findSlot(game_id)) {
         slot->game_result = standard_result; slot->game_finished = true;
         slot->clock_timer->stop();
         if (game_id == active_slot_game_id) shared_board_window->updateGameResult(standard_result);
     }
 } else {
 for (BoardWindow* board : board_windows) {
 if (board->isObserving() && board->getObservedGameId() == game_id) {
 board->updateGameResult(standard_result);
 }
 }
 }
 untrackFinishedGame(game_id);
 }
 }
 // Parse IGS Command 20 — server-assigned counting score.
 // Format: "20 white_player (W:O): 95.5 to black_player (B:#): 116.0"
 if (line.startsWith("20 ")) {
     QRegExp cmd20_re("20\\s+(\\S+)\\s+\\(W:[^)]+\\):\\s+([\\d\\.]+)\\s+to\\s+(\\S+)\\s+\\(B:[^)]+\\):\\s+([\\d\\.]+)");
     if (cmd20_re.indexIn(line) != -1) {
         QString white_player = cmd20_re.cap(1);
         double  white_score  = cmd20_re.cap(2).toDouble();
         QString black_player = cmd20_re.cap(3);
         double  black_score  = cmd20_re.cap(4).toDouble();
         double  margin       = qAbs(white_score - black_score);
         QString result_text  = (white_score > black_score)
             ? QString("W+%1").arg(margin)
             : QString("B+%1").arg(margin);
         output_console->append(QString("[BOT] Server score CMD20: W:%1 B:%2 → %3")
             .arg(white_score).arg(black_score).arg(result_text));
         bot_cmd20_received = true;
         // Push authoritative score to the board window for the relevant game.
         // Match by BOTH player names to avoid false matches when a player has multiple games.
         // Do NOT call enterScoringModeForResult() here — it calls calculateScore() which
         // clobbers non-scoring games (e.g. W+R games get a territory overlay).
         // For bot games, scoring mode was already entered at CMD9 time.
         int cmd20_game_id = -1;
         if (docked_pane_mode) {
             for (GameSlot *slot : game_slots) {
                 if (slot->game_finished) continue; // skip already-finished slots (rematch same players)
                 if (!slot->is_scoring_mode) continue; // CMD20 follows CMD9/CMD22; slot must be in scoring
                 if (slot->white_player == white_player && slot->black_player == black_player) {
                     cmd20_game_id            = slot->game_id;
                     slot->server_white_score = white_score;
                     slot->server_black_score = black_score;
                     slot->has_server_score   = true;
                     slot->game_result        = result_text;
                     slot->game_finished      = true;
                     slot->clock_timer->stop();
                     // Always switch to this game and show result regardless of active slot
                     if (shared_board_window) {
                         if (slot->game_id != active_slot_game_id)
                             switchActiveGame(slot->game_id);
                         shared_board_window->setServerScore(white_score, black_score);
                         shared_board_window->updateGameResult(result_text);
                     }
                     break;
                 }
             }
         } else {
             for (BoardWindow *board : board_windows) {
                 if (board->getWhitePlayer() == white_player && board->getBlackPlayer() == black_player) {
                     cmd20_game_id = board->getObservedGameId();
                     board->setServerScore(white_score, black_score);
                     board->updateGameResult(result_text);
                     break;
                 }
             }
         }
         if (cmd20_game_id != -1)
             untrackFinishedGame(cmd20_game_id);

         // Scoring discrepancy check — compare CMD20 server score against our local
         // KataGo-based territory calculation from Pass 2. A large gap may indicate
         // an IGS server scoring algorithm failure (e.g. game 350: F16 not counted).
         // Threshold: 10 points (absorbs komi, rounding, minor edge cases).
         if (bot_mode_active && bot_local_white_score >= 0 && bot_local_black_score >= 0) {
             double server_margin = white_score - black_score;   // positive = W ahead
             double local_margin  = bot_local_white_score - bot_local_black_score;
             double discrepancy   = qAbs(server_margin - local_margin);
             if (discrepancy >= 10.0) {
                 // Does the server result disagree with local on WINNER?
                 bool server_w_wins = (white_score > black_score);
                 bool local_w_wins  = (bot_local_white_score > bot_local_black_score);
                 QString anomaly_note = (server_w_wins != local_w_wins)
                     ? "WINNER DISAGREES"
                     : "margin differs";
                 QString local_result = (bot_local_white_score > bot_local_black_score)
                     ? QString("W+%1").arg(qAbs(local_margin), 0, 'f', 0)
                     : QString("B+%1").arg(qAbs(local_margin), 0, 'f', 0);
                 QString warn_msg = QString(
                     "⚠ SCORING ANOMALY game %1: Server %2 (W:%3 B:%4) vs Local %5 (W:%6 B:%7) — gap %8 pts [%9]")
                     .arg(cmd20_game_id)
                     .arg(result_text)
                     .arg(white_score, 0, 'f', 1)
                     .arg(black_score, 0, 'f', 1)
                     .arg(local_result)
                     .arg(bot_local_white_score)
                     .arg(bot_local_black_score)
                     .arg(discrepancy, 0, 'f', 1)
                     .arg(anomaly_note);
                 output_console->append("[BOT] " + warn_msg);
                 // Notify via *SYSTEM* comment so user sees it in Comments & Kibitz
                 if (shared_board_window)
                     shared_board_window->processComment("*SYSTEM*", warn_msg, false);
                 // Append to persistent scoring anomaly log for IGS admin documentation
                 QString log_path = QDir::homePath() + "/xgospel2_scoring_anomalies.log";
                 QFile log_file(log_path);
                 if (log_file.open(QIODevice::Append | QIODevice::Text)) {
                     QTextStream ts(&log_file);
                     ts << QDateTime::currentDateTime().toString(Qt::ISODate) << " "
                        << warn_msg << "\n"
                        << "  Players: W=" << white_player << " B=" << black_player << "\n"
                        << "  CMD20 raw: W:" << white_score << " B:" << black_score << "\n"
                        << "  Local raw: W:" << bot_local_white_score << " B:" << bot_local_black_score << "\n"
                        << "\n";
                 }
             }
         }
     }
 }

 } // End while (socket->canReadLine())

 // Auto-scroll
 output_console->verticalScrollBar()->setValue(output_console->verticalScrollBar()->maximum());
 }
 
 void sendCommand() {
 QString cmd = command_text->text().trimmed();
 if (!cmd.isEmpty()) {
 if (socket->state() == QTcpSocket::ConnectedState) {
 // Handle observe commands specially
 if (cmd.startsWith("observe ")) {
 QString game_num = cmd.section(' ', 1, 1);
 bool ok;
 int game_id = game_num.toInt(&ok);
 if (ok) {
 // Send observe command to IGS and wait for response with player names
 socket->write((cmd + "\n").toUtf8());
 output_console->append(">>> SENT: " + cmd + " (waiting for IGS response with player names)");
 
 // Set flag to indicate we're expecting an observe response
 pending_manual_observe = true;
 pending_observe_game_id = game_id;
 if (!this->suppress_server_console) output_console->append(QString(">>> DEBUG: Set pending_manual_observe=true for game %1").arg(game_id));
 } else {
 if (!this->suppress_server_console) output_console->append(">>> ERROR: Invalid game number for observe command");
 }
 } else {
 // Regular command
 socket->write((cmd + "\n").toUtf8());
            // Suppress ">>> SENT:" for stats commands (output shown via Command 9 response)
            if (!cmd.startsWith("stats ", Qt::CaseInsensitive)) {
                output_console->append(">>> SENT: " + cmd);
            }
 
 // Reset heartbeat counter when user sends command
 resetHeartbeatCounter();
 }
 command_text->clear();
 } else {
 if (!this->suppress_server_console) output_console->append(">>> ERROR: Not connected - use Connection > Connect to IGS first");
 }
 }
 }
 
 void sendCommandString(const QString &cmd_string) {
 if (!cmd_string.isEmpty()) {
 // Intercept "stats <playername>" commands to open player dialog
 // This allows users to type "stats NY" or "stats <any_playername>" in console
 // to open a stats dialog for ANY player (online or offline), just like q5Go
 if (cmd_string.startsWith("stats ", Qt::CaseInsensitive) && cmd_string.length() > 6) {
 QString player_name = cmd_string.mid(6).trimmed();
 if (!player_name.isEmpty() && players_window) {
 qDebug() << "[STATS] Intercepted stats command for player:" << player_name;
 players_window->openStatsDialogForPlayer(player_name);
 // Still send the command to IGS to get the stats data
 }
 }

 if (socket->state() == QTcpSocket::ConnectedState) {
 socket->write((cmd_string + "\n").toUtf8());
            // Suppress ">>> SENT:" for stats commands (output shown via Command 9 response)
            if (!cmd_string.startsWith("stats ", Qt::CaseInsensitive)) {
                output_console->append(">>> SENT: " + cmd_string);
            }
 } else {
 if (!this->suppress_server_console) output_console->append(">>> ERROR: Not connected - use Connection > Connect to IGS first");
 }
 }
 }
 
 void showPlayersWindow() {
 if (!players_window) {
 players_window = new FixedPlayersWindow(this);
 players_window->setNewratingEnabled(newrating_enabled);
 players_window->setOutputConsole(output_console); // Give players window access to console for debug output
 connect(players_window, &FixedPlayersWindow::refreshRequested, this, &FixedXGospelWindow::refreshPlayers);
 connect(players_window, &FixedPlayersWindow::matchRequested, this, &FixedXGospelWindow::requestMatch);
 connect(players_window, &FixedPlayersWindow::statsRequested, this, [this](const QString &player) {
 sendCommandString(QString("stats %1").arg(player));
 });
 connect(players_window, &FixedPlayersWindow::tellRequested, this, &FixedXGospelWindow::sendTell);
        // Feature 33: Connect toggle command signal
        connect(players_window, &FixedPlayersWindow::toggleCommandRequested, this, [this](const QString &parameter) {
            qDebug() << "[TOGGLE] Toggle command requested:" << parameter;
            sendCommandString(QString("toggle %1").arg(parameter));
        });
        // Set local username for toggle buttons
        players_window->setLocalUsername(login_username);
 }
 players_window->show();
 players_window->raise();
 players_window->activateWindow();

 if (connected_to_igs) {
 QTimer::singleShot(500, this, &FixedXGospelWindow::refreshPlayers);
 } else {
 if (!this->suppress_server_console) output_console->append(">>> Connect to IGS first to get live player data");
 }
 }

 void populateGamesFromBufferIfPending() {
 // Called when first Command 7 data arrives after auto-launch
 if (!games_window) {
 return;
 }

 if (buffered_game_lines.isEmpty()) {
 // Wait a bit longer for more data to arrive
 QTimer::singleShot(100, this, &FixedXGospelWindow::populateGamesFromBufferIfPending);
 return;
 }

 // We have data! Populate the games window
 games_window->clearGames();
 int games_added = 0;
 for (const QString &game_line : buffered_game_lines) {
 games_window->addGameFromRawLine(game_line);
 games_added++;
 }
 games_window->resizeColumns();
 // Update observed games highlighting after auto-launch populate (xgospel1 pattern)
 games_window->updateObservedGames(observed_game_ids);

 auto_launch_populate_pending = false; // Mark as complete
 output_console->append(QString("[OK] Auto-launch populated: %1 games").arg(games_added));
 }

 void autoLaunchWindowsMinimized() {
 qDebug() << ">>> AUTO_LAUNCH_FUNC: Called with buffered_game_lines.size()=" << buffered_game_lines.size();
 if (!this->suppress_server_console) output_console->append(">>> [LAUNCH] Auto-launching Players and Games windows (minimized)...");

 // Launch Players window
 if (!players_window) {
 players_window = new FixedPlayersWindow(this);
 players_window->setNewratingEnabled(newrating_enabled);
 players_window->setOutputConsole(output_console);
 connect(players_window, &FixedPlayersWindow::refreshRequested, this, &FixedXGospelWindow::refreshPlayers);
 connect(players_window, &FixedPlayersWindow::matchRequested, this, &FixedXGospelWindow::requestMatch);
 connect(players_window, &FixedPlayersWindow::statsRequested, this, [this](const QString &player) {
 sendCommandString(QString("stats %1").arg(player));
 });
 connect(players_window, &FixedPlayersWindow::tellRequested, this, &FixedXGospelWindow::sendTell);
        // Feature 33: Connect toggle command signal
        connect(players_window, &FixedPlayersWindow::toggleCommandRequested, this, [this](const QString &parameter) {
            qDebug() << "[TOGGLE] Toggle command requested:" << parameter;
            sendCommandString(QString("toggle %1").arg(parameter));
        });
        // Set local username for toggle buttons
        players_window->setLocalUsername(login_username);
 }
 // Show players window normally (don't minimize yet)
 players_window->show();
 if (connected_to_igs) {
 if (!this->suppress_server_console) output_console->append(">>> DEBUG: Scheduling refreshPlayers in 2000ms");
 QTimer::singleShot(2000, this, [this]() {
 refreshPlayers();
 // Minimize after a brief delay to ensure rendering
 QTimer::singleShot(500, players_window, &FixedPlayersWindow::showMinimized);
 });
 }

 // Launch Games window
 if (!games_window) {
 games_window = new FixedGamesWindow(this);
 games_window->setNewratingEnabled(newrating_enabled);
 connect(games_window, &FixedGamesWindow::refreshRequested, this, &FixedXGospelWindow::refreshGames);
 connect(games_window, &FixedGamesWindow::observeRequested, this, &FixedXGospelWindow::observeGame);

 // If we're in auto-launch mode and have buffered Command 7 data, populate from buffer
		games_window->setPlayersWindowRef(players_window);
 if (auto_launch_populate_pending && !buffered_game_lines.isEmpty()) {
 qDebug() << ">>> AUTO_LAUNCH_FUNC: Games window created with" << buffered_game_lines.size() << "buffered games - triggering populate";
 QTimer::singleShot(50, this, &FixedXGospelWindow::populateGamesFromBufferIfPending);
 }
 }
 // Show games window normally (don't minimize yet)
 games_window->show();

 // Create Shout window (always, so shouts are cached from login regardless of preference)
 if (!shout_window) {
     shout_window = new FixedShoutWindow(this);
     connect(shout_window, &FixedShoutWindow::shoutRequested, this, [this](const QString &msg) {
         socket->write(QString("shout %1\n").arg(msg).toUtf8());
         socket->flush();
         if (!this->suppress_server_console)
             output_console->append(QString(">>> SENT: shout %1").arg(msg));
         // IGS does not echo our own shout back as CMD21 — display it locally
         shout_window->addShout(login_username, msg);
     });
     connect(shout_window, &FixedShoutWindow::playerClicked, this, [this](const QString &name) {
         if (players_window) players_window->openStatsDialogForPlayer(name);
     });
 }
 // Only auto-show if user has enabled it in preferences
 if (settings->getAutoLaunchShoutWindow()) {
     shout_window->show();
     QTimer::singleShot(2500, shout_window, &FixedShoutWindow::showMinimized);
 }

 // Execute 'games' command to populate the games window after windows are created
 if (connected_to_igs) {
 // Schedule the games command after a short delay to ensure everything is initialized
 QTimer::singleShot(500, this, [this]() {
 sendCommandString("games");
 // After sending games command, minimize the window after data has time to arrive
 QTimer::singleShot(2000, games_window, &FixedGamesWindow::showMinimized);
 // After window is minimized, send games command again with 1 second delay
 QTimer::singleShot(3000, this, [this]() {
 sendCommandString("games");
 });
 });

 // Seed server clock from UTC+9 (IGS joyjoy.net = JST) immediately so it shows on login.
 // The clock will be corrected to the exact server time if/when IGS sends
 // "9 The current time (GMT) is:" as a side effect of game time events.
 if (!server_time_set) {
     server_time_value    = QDateTime::currentDateTimeUtc().addSecs(9 * 3600);
     server_time_received = QDateTime::currentDateTimeUtc();
     server_time_set      = true;
 }
 }
 }
 
 void refreshPlayers() {
 if (players_window && socket->state() == QTcpSocket::ConnectedState) {
 players_window->clearPlayers();
 waiting_for_players = true;
 player_count = 0;
 tried_fallback_who = false; // Reset fallback flag
 waiting_for_who_supplement = false;  // Bug 35: Reset WHO supplement flag
 userlist_player_names.clear();  // Bug 35: Clear player name tracking set

 // Set parsing mode in players window
 players_window->setGuestMode(login_username == "guest");
 players_window->setFallbackMode(false);

 // Smart approach: Try userlist first (more info), fallback to who
 if (login_username == "guest") {
 // Guest accounts: directly use "who" command
 socket->write("who\n");
 if (!this->suppress_server_console) output_console->append(">>> SENT: who (guest account - basic player info)");
 } else {
 // Registered accounts: try "userlist" first for detailed info
 socket->write("userlist\n");
 if (!this->suppress_server_console) output_console->append(">>> SENT: userlist (registered account - detailed player info)");
 if (!this->suppress_server_console) output_console->append(">>> Note: Will fallback to 'who' if userlist fails");
 }

 // Auto-refresh observer lists (piggyback on players timer).
 // Dock mode: active slot only. Non-dock: all open board windows.
 if (docked_pane_mode) {
     GameSlot *active_s = findSlot(active_slot_game_id);
     if (active_slot_game_id > 0 && active_s && !active_s->game_finished && shared_board_window) {
         shared_board_window->clearObservers();
         requestObservers(active_slot_game_id);
     }
 } else {
     for (BoardWindow *bw : board_windows) {
         int gid = bw->getObservedGameId();
         if (gid > 0 && !bw->isFinished()) {
             bw->clearObservers();
             requestObservers(gid);
         }
     }
 }
 } else if (!connected_to_igs) {
 if (!this->suppress_server_console) output_console->append(">>> ERROR: Connect to IGS first to refresh players");
 }
 }
 
 void requestMatch(const QString &opponent, const QString &opponent_rank) {
 qDebug() << "[MENU] REQUEST MATCH: Called for opponent:" << opponent << "rank:" << opponent_rank;
 if (!connected_to_igs) {
 if (!this->suppress_server_console) output_console->append(">>> ERROR: Connect to IGS first to request matches");
 return;
 }
 
 // Create and show the match dialog (Bug 32a: nullptr parent for independent window)
 MatchDialog *dialog = new MatchDialog(nullptr, my_account_name, my_account_rank, opponent, opponent_rank);

 // Use regular match commands for better compatibility
 // nmatch has issues with parameter validation on IGS
 dialog->setIsNmatch(false);
 
 // Connect dialog signals
 connect(dialog, &MatchDialog::matchOffered, this, [this](const QString &command) {
 if (command == "#FREE_GAME_REQUEST#") {
 // Mark next game as needing "free" command
 next_game_is_free = true;
 if (!this->suppress_server_console) output_console->append(">>> Game will be requested as FREE after match starts");
 } else if (command.startsWith("#KOMI_REQUEST:")) {
 // Extract komi value from marker
 QString komi_str = command.mid(14); // Skip "#KOMI_REQUEST:"
 komi_str.chop(1); // Remove trailing "#"
 double komi = komi_str.toDouble();
 next_game_komi = komi;
 if (!this->suppress_server_console) output_console->append(QString(">>> Komi %1 will be set after match starts").arg(komi));
 } else {
 if (!this->suppress_server_console) output_console->append(QString(">>> SENDING MATCH COMMAND: %1").arg(command));
 sendCommandString(command);
 }
 });
 
 connect(dialog, &MatchDialog::statsRequested, this, [this](const QString &player) {
 sendCommandString(QString("stats %1").arg(player));
 });
 
 connect(dialog, &MatchDialog::suggestRequested, this, [this](const QString &player) {
 // IGS doesn't have suggest command, but we can show a helpful message
 if (!this->suppress_server_console) output_console->append(QString(">>> Suggest feature not available on IGS for %1").arg(player));
 });
 
 // Show the dialog
 dialog->show();
 dialog->raise();
 dialog->activateWindow();
 }
 
 void handleIncomingMatchRequest(const QString &opponent, const QString &opponent_rank,
 const QString &board_info, const QString &time_str, const QString &my_color) {
 // Create match dialog for incoming request (Bug 32a: nullptr parent for independent window)
 MatchDialog *dialog = new MatchDialog(nullptr, my_account_name, my_account_rank, opponent, opponent_rank);
 
 // Parse board info (like "19x19")
 QString size = board_info.split('x').first();
 
 // Set the game parameters from the request
 dialog->getBoardSizeSpin()->setValue(size.toInt());
 dialog->getTimeSpin()->setValue(time_str.toInt());
 dialog->getByoTimeSpin()->setValue(5); // Bug 32e: Default to 5 (more common than 10)
 
 // Set my color based on request
 if (my_color == "Black") {
 dialog->getBlackButton()->setChecked(true);
 } else {
 dialog->getWhiteButton()->setChecked(true);
 }
 
 // This is not an nmatch
 dialog->setIsNmatch(false);
        dialog->setIsIncomingRequest(true);  // Bug 34: Mark as incoming to send just "match"

 // Change button text to show this is incoming
 dialog->getOfferButton()->setText("Accept");
 dialog->getDeclineButton()->setEnabled(true);
 dialog->getCancelButton()->setDisabled(true);

 // Bug 32d: Disable editing of critical parameters for incoming request
 // User must accept/decline with same board size, time, color, and handicap
 dialog->getBoardSizeSpin()->setEnabled(false);
 dialog->getTimeSpin()->setEnabled(false);
 dialog->getByoTimeSpin()->setEnabled(false);
 dialog->getBlackButton()->setEnabled(false);
 dialog->getWhiteButton()->setEnabled(false);
 dialog->getNigiriButton()->setEnabled(false);
 dialog->getHandicapSpin()->setEnabled(false);
 // Note: komi and free/rated can be negotiated, so we leave those editable

 // Connect signals for incoming request (Bug 32d: send match command, not accept)
 connect(dialog, &MatchDialog::matchOffered, this, [this](const QString &command) {
 if (command == "#FREE_GAME_REQUEST#") {
 // Mark next game as needing "free" command
 next_game_is_free = true;
 if (!this->suppress_server_console) output_console->append(">>> Game will be requested as FREE after match starts");
 } else if (command.startsWith("#KOMI_REQUEST:")) {
 // Extract komi value from marker
 QString komi_str = command.mid(14); // Skip "#KOMI_REQUEST:"
 komi_str.chop(1); // Remove trailing "#"
 double komi = komi_str.toDouble();
 next_game_komi = komi;
 if (!this->suppress_server_console) output_console->append(QString(">>> Komi %1 will be set after match starts").arg(komi));
 } else {
 // Send the match command with the dialog's parameters
 if (!this->suppress_server_console) output_console->append(QString(">>> SENDING MATCH COMMAND: %1").arg(command));
 sendCommandString(command);
 }
 });
 
 if (!this->suppress_server_console) output_console->append(QString(">>> SHOWING INCOMING MATCH DIALOG for %1").arg(opponent));

    // Bug 34: Store dialog pointer so we can update it with server's suggested command
    pending_match_dialog = dialog;
    connect(dialog, &QDialog::finished, this, [this]() { pending_match_dialog = nullptr; });
 dialog->show();
 dialog->raise();
 dialog->activateWindow();
 }
 
 void handleIncomingNmatchRequest(const QString &opponent, const QString &opponent_rank,
 const QString &my_color, const QString &handicap,
 const QString &board_size, const QString &time_seconds, const QString &byo_seconds) {
 // Create nmatch dialog for incoming request (Bug 32a: nullptr parent for independent window)
 MatchDialog *dialog = new MatchDialog(nullptr, my_account_name, my_account_rank, opponent, opponent_rank);
 
 // Set the game parameters from nmatch request
 dialog->getBoardSizeSpin()->setValue(board_size.toInt());
 dialog->getTimeSpin()->setValue(time_seconds.toInt() / 60); // Convert seconds to minutes
 dialog->getByoTimeSpin()->setValue(byo_seconds.toInt() / 60); // Convert seconds to minutes 
 dialog->getHandicapSpin()->setValue(handicap.toInt());
 
 // Set my color based on request
 if (my_color == "B") {
 dialog->getBlackButton()->setChecked(true);
 } else if (my_color == "W") {
 dialog->getWhiteButton()->setChecked(true);
 } else if (my_color == "N") {
 dialog->getNigiriButton()->setChecked(true);
 }
 
 // This is an nmatch request
 dialog->setIsNmatch(true);
        dialog->setIsIncomingRequest(true);  // Bug 34: Mark as incoming to send just "nmatch"

 // Change button text to show this is incoming
 dialog->getOfferButton()->setText("Accept");
 dialog->getDeclineButton()->setEnabled(true);
 dialog->getCancelButton()->setDisabled(true);

 // Bug 32d: Disable editing of critical parameters for incoming nmatch
 // User must accept/decline with same board size, time, color, and handicap
 dialog->getBoardSizeSpin()->setEnabled(false);
 dialog->getTimeSpin()->setEnabled(false);
 dialog->getByoTimeSpin()->setEnabled(false);
 dialog->getBlackButton()->setEnabled(false);
 dialog->getWhiteButton()->setEnabled(false);
 dialog->getNigiriButton()->setEnabled(false);
 dialog->getHandicapSpin()->setEnabled(false);
 // Note: komi and free/rated can be negotiated, so we leave those editable

 // Connect signals for incoming nmatch request (Bug 32d: send match command, not accept)
 connect(dialog, &MatchDialog::matchOffered, this, [this](const QString &command) {
 if (command == "#FREE_GAME_REQUEST#") {
 // Mark next game as needing "free" command
 next_game_is_free = true;
 if (!this->suppress_server_console) output_console->append(">>> Game will be requested as FREE after match starts");
 } else if (command.startsWith("#KOMI_REQUEST:")) {
 // Extract komi value from marker
 QString komi_str = command.mid(14); // Skip "#KOMI_REQUEST:"
 komi_str.chop(1); // Remove trailing "#"
 double komi = komi_str.toDouble();
 next_game_komi = komi;
 if (!this->suppress_server_console) output_console->append(QString(">>> Komi %1 will be set after match starts").arg(komi));
 } else {
 // Send the nmatch command with the dialog's parameters
 if (!this->suppress_server_console) output_console->append(QString(">>> SENDING NMATCH COMMAND: %1").arg(command));
 sendCommandString(command);
 }
 });
 
 if (!this->suppress_server_console) output_console->append(QString(">>> SHOWING INCOMING NMATCH DIALOG for %1").arg(opponent));

    // Bug 34: Store dialog pointer so we can update it with server's suggested command
    pending_match_dialog = dialog;
    connect(dialog, &QDialog::finished, this, [this]() { pending_match_dialog = nullptr; });
 dialog->show();
 dialog->raise();
 dialog->activateWindow();
 }
 
 void showGamesWindow() {
 if (!games_window) {
 games_window = new FixedGamesWindow(this);
 games_window->setNewratingEnabled(newrating_enabled);
 connect(games_window, &FixedGamesWindow::refreshRequested, this, &FixedXGospelWindow::refreshGames);
 connect(games_window, &FixedGamesWindow::observeRequested, this, &FixedXGospelWindow::observeGame);
 }
 games_window->show();
		games_window->setPlayersWindowRef(players_window);
 games_window->raise();
 games_window->activateWindow();
 
 if (connected_to_igs) {
 QTimer::singleShot(500, this, &FixedXGospelWindow::refreshGames);
 } else {
 if (!this->suppress_server_console) output_console->append(">>> Connect to IGS first to get live games data");
 }
 }

 void showPreferences() {
 PreferencesDialog *prefs_dialog = new PreferencesDialog(engine_manager, this);
 if (prefs_dialog->exec() == QDialog::Accepted) {
 // Refresh connection menu to reflect updated accounts
 populateConnectionMenu();
 }
 delete prefs_dialog;
 }

 void showAboutDialog() {
 QMessageBox about_box(this);
 about_box.setWindowTitle("About XGospel");
 about_box.setTextFormat(Qt::RichText);
 about_box.setText(QString(
 "<h2>XGospel 2.0</h2>"
 "<p><b>Version:</b> %1</p>"
 "<p><b>Build Date:</b> %2</p>"
 "<p>A Go client for the Internet Go Server (IGS)</p>"
 "<p>Based on q5Go and xgospel1</p>"
 ).arg(XGOSPEL_VERSION, XGOSPEL_BUILD_DATE));
 about_box.setIcon(QMessageBox::Information);
 about_box.setStandardButtons(QMessageBox::Ok);
 about_box.exec();
 }

 void refreshGames() {
 if (games_window && socket->state() == QTcpSocket::ConnectedState) {
 waiting_for_games = true;
 game_count = 0;

 // Send games command to IGS
 socket->write("games\n");

 // Only log to console once per hour to avoid console bloat
 QDateTime now = QDateTime::currentDateTime();
 if (!last_games_console_log.isValid() || last_games_console_log.secsTo(now) >= 3600) {
 if (!this->suppress_server_console) output_console->append(">>> SENT: games (requesting current games list)");
 last_games_console_log = now;
 }
 } else if (!connected_to_igs) {
 if (!this->suppress_server_console) output_console->append(">>> ERROR: Connect to IGS first to refresh games");
 }
 }
 
 // --- Docked-pane helpers ---

 GameSlot *findSlot(int game_id) const {
     for (GameSlot *s : game_slots)
         if (s->game_id == game_id) return s;
     return nullptr;
 }

 // Flood-fill territory directly from slot->board_state, with no board widget.
 // Populates slot->territory_map, slot->white_territory, slot->black_territory.
 // Called for inactive slots when a counted result arrives.
 void calculateTerritoryForSlot(GameSlot *slot) {
     if (!slot) return;
     const int bs = slot->board_size;

     slot->white_territory = 0;
     slot->black_territory = 0;
     slot->territory_map.clear();

     QSet<QPair<int,int>> visited;

     for (int x = 0; x < bs; ++x) {
         for (int y = 0; y < bs; ++y) {
             QPair<int,int> pos(x, y);
             if (visited.contains(pos)) continue;
             if (slot->board_state[x][y] != EMPTY) continue;
             if (slot->dead_stone_positions.contains(pos)) continue;

             // Flood-fill connected empty region
             QStack<QPair<int,int>> stack;
             QSet<QPair<int,int>> region;
             QSet<StoneColor> adj_colors;
             stack.push(pos);
             while (!stack.isEmpty()) {
                 QPair<int,int> cur = stack.pop();
                 int cx = cur.first, cy = cur.second;
                 if (cx < 0 || cx >= bs || cy < 0 || cy >= bs) continue;
                 if (region.contains(cur)) continue;
                 int cell = slot->board_state[cx][cy];
                 if (cell != EMPTY && !slot->dead_stone_positions.contains(cur)) {
                     adj_colors.insert(static_cast<StoneColor>(cell));
                     continue;
                 }
                 region.insert(cur);
                 stack.push({cx-1, cy}); stack.push({cx+1, cy});
                 stack.push({cx, cy-1}); stack.push({cx, cy+1});
             }
             visited.unite(region);

             StoneColor owner = EMPTY;
             if (adj_colors.size() == 1)
                 owner = *adj_colors.begin();

             for (const auto &p : region)
                 slot->territory_map[p] = static_cast<int>(owner);

             if (owner == WHITE_STONE)      slot->white_territory += region.size();
             else if (owner == BLACK_STONE) slot->black_territory += region.size();
         }
     }
     qDebug() << "*** SLOT TERRITORY: game" << slot->game_id
              << "W-terr:" << slot->white_territory
              << "B-terr:" << slot->black_territory
              << "dead:" << slot->dead_stone_positions.size();
 }

 void switchActiveGame(int game_id) {
     if (game_id == active_slot_game_id) return;
     if (!shared_board_window) return;

     GameSlot *old_slot = findSlot(active_slot_game_id);
     if (old_slot) {
         shared_board_window->snapshotToSlot(old_slot);
         updateHoverPixmapForSlot(old_slot);
     }

     GameSlot *new_slot = findSlot(game_id);
     if (!new_slot) return;

     shared_board_window->loadSlot(new_slot);
     active_slot_game_id = game_id;

     GameSelectionDock *dock = shared_board_window->getGameSelectionDock();
     if (dock) dock->setActiveGame(game_id);

     // Bring board window to foreground so user sees the switch
     shared_board_window->raise();
     shared_board_window->activateWindow();

     // Refresh observer list for the newly-active slot immediately.
     if (!new_slot->game_finished) {
         shared_board_window->clearObservers();
         requestObservers(game_id);
     }
 }

 // Send the next queued "moves N" request if no slot is still replaying.
 // Called whenever a slot transitions to LIVE so the queue drains one-by-one.
 void dispatchNextMovesRequest() {
     if (moves_dispatch_queue.isEmpty()) return;
     // Don't dispatch while a "moves N" reply is actively being received (REPLAYING).
     // WAITING_FOR_MOVES0 slots have not had their request sent yet — not blocking.
     for (GameSlot *s : game_slots) {
         if (s->replay_state == GameSlot::REPLAYING)
             return;
     }
     // Drain stale entries (slots already closed) iteratively — never recurse.
     GameSlot *slot = nullptr;
     int next_id = -1;
     while (!moves_dispatch_queue.isEmpty()) {
         next_id = moves_dispatch_queue.takeFirst();
         slot = findSlot(next_id);
         if (slot) break;
         qDebug() << "[MOVES-QUEUE] slot" << next_id << "gone, skipping";
     }
     if (!slot) return;
     games_with_moves_requested.insert(next_id);
     if (next_id == active_slot_game_id && shared_board_window) {
         shared_board_window->clearMoveHistoryBeforeMovesCommand();
         slot->game_root    = shared_board_window->getGameRoot();
         slot->current_node = shared_board_window->getGameRoot();
         slot->move_history.clear();
     }
     QString moves_cmd = QString("moves %1").arg(next_id);
     socket->write((moves_cmd + "\n").toUtf8());
     if (!suppress_server_console)
         output_console->append(QString(">>> SENT: %1 (serialized moves N dispatch)").arg(moves_cmd));
     qDebug() << "[MOVES-QUEUE] dispatched moves" << next_id << "(queue remaining:" << moves_dispatch_queue.size() << ")";
 }

 // Apply a move directly to a slot's board_state array (for inactive slots).
 // Places the stone and removes any captures listed in move.captured.
 void applyMoveToSlotBoard(GameSlot *slot, const GameMove &move) {
     if (!slot) return;
     const int bs = slot->board_size;

     // Pass — no stone to place, but turn still flips
     if (move.x == -1 && move.y == -1) {
         slot->last_move_x = -1;
         slot->last_move_y = -1;
         slot->current_player = (move.color == static_cast<int>(BLACK_STONE))
                                ? WHITE_STONE : BLACK_STONE;
         slot->consecutive_passes++;
         // Game tree: add pass node
         if (slot->current_node) {
             GoBoard pass_board = slot->current_node->getBoard().copy();
             GameNode *pass_node = slot->current_node->addMove(-1, -1,
                                       static_cast<StoneColor>(move.color));
             pass_node->setBoard(pass_board);
             slot->current_node = pass_node;
         }
         return;
     }
     // Handicap setup
     if (move.x == -2) {
         QList<QPair<int,int>> positions = IGSMoveParser::getHandicapPositions(move.y);
         GoBoard handicap_board;
         for (auto &pos : positions) {
             if (pos.first < bs && pos.second < bs) {
                 slot->board_state[pos.first][pos.second] = BLACK_STONE;
                 handicap_board.placeStone(pos.first, pos.second, BLACK_STONE);
             }
         }
         slot->last_move_x = -1;
         slot->last_move_y = -1;
         if (slot->current_node) {
             GameNode *hnode = slot->current_node->addMove(-2, move.y, BLACK_STONE);
             hnode->setBoard(handicap_board);
             slot->current_node = hnode;
         }
         return;
     }
     // Regular move — reset pass counter and update flat board_state and game tree
     slot->consecutive_passes = 0;
     if (move.x >= 0 && move.x < bs && move.y >= 0 && move.y < bs) {
         // Reject placement on an already-occupied square — illegal in Go and a
         // reliable signal of a spurious move (scoring-phase stone removal or
         // cross-game contamination routed here by context pin).
         if (slot->board_state[move.x][move.y] != 0) {
             qDebug() << "[INVALID] Dropping occupied-square placement at"
                      << move.x << move.y << "color" << move.color
                      << "(occupied by" << slot->board_state[move.x][move.y] << ")"
                      << "for game" << slot->game_id;
             return;
         }
         slot->board_state[move.x][move.y] = move.color;
         slot->last_move_x = move.x;
         slot->last_move_y = move.y;
     }
     // Remove captured stones from flat board_state
     if (!move.captured.isEmpty()) {
         for (const QString &cap : move.captured.split(';', Qt::SkipEmptyParts)) {
             QStringList xy = cap.split(',');
             if (xy.size() == 2) {
                 int cx = xy[0].toInt(), cy = xy[1].toInt();
                 if (cx >= 0 && cx < bs && cy >= 0 && cy < bs)
                     slot->board_state[cx][cy] = EMPTY;
             }
         }
     }
     // Game tree: build new node with correct GoBoard (captures via liberty check)
     if (slot->current_node && move.x >= 0 && move.y >= 0) {
         GoBoard new_board = slot->current_node->getBoard().copy();
         StoneColor color = static_cast<StoneColor>(move.color);
         new_board.placeStone(move.x, move.y, color);
         StoneColor opp = (color == BLACK_STONE) ? WHITE_STONE : BLACK_STONE;
         int dx[] = {-1, 1, 0, 0};
         int dy[] = {0, 0, -1, 1};
         for (int d = 0; d < 4; d++) {
             int nx = move.x + dx[d], ny = move.y + dy[d];
             if (nx >= 0 && nx < 19 && ny >= 0 && ny < 19 &&
                 new_board.getStone(nx, ny) == opp &&
                 new_board.countLiberties(nx, ny) == 0)
                 new_board.removeGroup(nx, ny);
         }
         if (new_board.countLiberties(move.x, move.y) == 0)
             new_board.removeGroup(move.x, move.y);
         GameNode *new_node = slot->current_node->addMove(move.x, move.y, color);
         new_node->setBoard(new_board);
         slot->current_node = new_node;
     }
     // Track whose turn it is next (mirrors processMove logic)
     slot->current_player = (move.color == static_cast<int>(BLACK_STONE))
                            ? WHITE_STONE : BLACK_STONE;
 }

 // Generate the hover pixmap for a slot and push it to the dock button.
 void updateHoverPixmapForSlot(GameSlot *slot) {
     if (!slot || !docked_pane_mode || !shared_board_window) return;
     GameSelectionDock *dock = shared_board_window->getGameSelectionDock();
     if (!dock) return;

     int preview_size = settings->getHoverBoardSize();
     QPixmap px = shared_board_window->renderSlotToPixmap(
         preview_size,
         slot->board_state,
         slot->board_size,
         slot->last_move_x,
         slot->last_move_y,
         slot->custom_game_title);
     dock->updateHoverPixmap(slot->game_id, px);
 }

 void observeGame(int game_id, const QString& white, const QString& black,
 const QString& white_rank, const QString& black_rank) {
 if (!connected_to_igs) {
 if (!this->suppress_server_console) output_console->append(">>> ERROR: Connect to IGS first to observe games");
 return;
 }

 // -----------------------------------------------------------------------
 // Docked-pane mode path
 // -----------------------------------------------------------------------
 if (docked_pane_mode) {
     // Already observing this game (live slot)?  Just switch to it.
     // If the slot exists but is finished, IGS has recycled the game ID —
     // remove the stale slot so a fresh one is created below.
     if (GameSlot *existing = findSlot(game_id)) {
         if (!existing->game_finished) {
             switchActiveGame(game_id);
             return;
         }
         GameSelectionDock *dock_s = shared_board_window ? shared_board_window->getGameSelectionDock() : nullptr;
         if (dock_s) dock_s->removeGame(game_id);
         game_slots.removeOne(existing);
         if (active_slot_game_id == game_id) active_slot_game_id = -1;
         delete existing;
     }

     // Create and initialise the slot
     GameSlot *slot = new GameSlot(this);
     slot->game_id      = game_id;
     slot->white_player = white;
     slot->black_player = black;
     slot->white_rank   = white_rank;
     slot->black_rank   = black_rank;
     slot->expected_white_player = white;
     slot->expected_black_player = black;
     slot->my_username  = login_username;
     slot->is_observing = true;
     slot->observation_state = GameSlot::JOINING_GAME;
     slot->observation_start_time = QDateTime::currentDateTime();
     slot->game_start_time        = QDateTime::currentDateTime();
     slot->game_mode   = MODE_NORMAL;
     slot->board_size  = 19;
     slot->current_player = BLACK_STONE;
     slot->game_root   = new GameNode();
     slot->current_node = slot->game_root;
     slot->auto_follow_mode = true;
     slot->komi        = 6.5;
     slot->game_type   = "Free";

     // Apply stored game details from Command 7 if available
     if (game_komi_map.contains(game_id)) {
         slot->komi     = game_komi_map[game_id];
         slot->handicap = game_handicap_map.value(game_id, 0);
         slot->game_type = game_type_map.value(game_id, "Free");
     }

     // Connect the slot's clock tick to update its own time fields
     connect(slot, &GameSlot::clockTick, this, [this, slot]() {
         if (slot->game_finished) return;
         if (slot->last_time_update.isNull()) return;
         int elapsed = static_cast<int>(slot->last_time_update.secsTo(
                           QDateTime::currentDateTime()));
         if (slot->current_player == BLACK_STONE)
             slot->black_time_seconds = std::max(0, slot->black_time_seconds - elapsed);
         else
             slot->white_time_seconds = std::max(0, slot->white_time_seconds - elapsed);
         slot->last_time_update = QDateTime::currentDateTime();
     });

     game_slots.append(slot);

     // Create the shared BoardWindow on the first game; reuse for subsequent ones
     if (!shared_board_window) {
         shared_board_window = new BoardWindow(this, login_username);
         connect(shared_board_window, &BoardWindow::boardClosed,
                 this, &FixedXGospelWindow::closeBoardWindow);
         connect(shared_board_window, &BoardWindow::saveRequested,
                 this, &FixedXGospelWindow::saveBoardGame);
         connect(shared_board_window, &BoardWindow::gameSaved,
                 this, [this](int game_id, const QString &filename) {
                     if (GameSlot *slot = findSlot(game_id))
                         slot->system_messages.append(QString("✓ Game saved to: %1").arg(filename));
                 });
         connect(shared_board_window, &BoardWindow::resignRequested,
                 this, &FixedXGospelWindow::resignGame);
             connect(shared_board_window, &BoardWindow::passRequested,
                     this, [this](int) { socket->write("pass\n"); socket->flush();
                         output_console->append("[IGS] >>> pass (manual Pass button)"); });
             connect(shared_board_window, &BoardWindow::doneRequested,
                     this, [this](int) { socket->write("done\n"); socket->flush();
                         output_console->append("[BOT] >>> done (manual Done button)"); });
             connect(shared_board_window, &BoardWindow::refreshRequested,
                     this, [this](int gid) {
                         socket->write(QString("moves %1\n").arg(gid).toUtf8());
                         output_console->append(QString("[IGS] >>> moves %1 (Refresh Board)").arg(gid)); });
         connect(shared_board_window, &BoardWindow::commentRequested,
                 this, &FixedXGospelWindow::sendComment);
         connect(shared_board_window, &BoardWindow::sayRequested,
                 this, &FixedXGospelWindow::sendSay);
         connect(shared_board_window, &BoardWindow::tellRequested,
                 this, &FixedXGospelWindow::sendTell);
         connect(shared_board_window, &BoardWindow::observersRequested,
                 this, &FixedXGospelWindow::requestObservers);
         connect(shared_board_window, &BoardWindow::observerClicked,
                 this, [this](const QString &name) { if (players_window) players_window->openStatsDialogForPlayer(name); });
         connect(shared_board_window, &BoardWindow::whitePlayerClicked,
                 this, [this](const QString &name) { if (players_window) players_window->openStatsDialogForPlayer(name); });
         connect(shared_board_window, &BoardWindow::blackPlayerClicked,
                 this, [this](const QString &name) { if (players_window) players_window->openStatsDialogForPlayer(name); });
         connect(shared_board_window, &BoardWindow::moveRequested,
                 this, &FixedXGospelWindow::sendMove);

         // Wire dock button clicks to switchActiveGame / close
         GameSelectionDock *dock = shared_board_window->getGameSelectionDock();
         if (dock) {
             dock->show();
             connect(dock, &GameSelectionDock::gameSelected,
                     this, &FixedXGospelWindow::switchActiveGame);
             connect(dock, &GameSelectionDock::gameCloseRequested,
                     this, &FixedXGospelWindow::closeBoardWindow);
         }
         shared_board_window->setSharedWindow(true);
         shared_board_window->show();
     }

     // Add button to the dock
     GameSelectionDock *dock = shared_board_window->getGameSelectionDock();
     if (dock) dock->addGame(game_id, black, black_rank, white, white_rank);

     // Display this game if it's the first (or re-first after all slots closed).
     // Also re-show the board window in case it was hidden by closeBoardWindow.
     if (active_slot_game_id == -1) {
         shared_board_window->loadSlot(slot);
         active_slot_game_id = game_id;
     }
     if (!shared_board_window->isVisible())
         shared_board_window->show();

     // Push initial pixmap after loadSlot so board widget is fully sized
     updateHoverPixmapForSlot(slot);

     most_recently_observed_board = shared_board_window;
     most_recently_observed_game_id = game_id;
     observed_game_ids.insert(game_id);
     if (games_window) games_window->updateObservedGames(observed_game_ids);

     // Send observe command
     QString observe_cmd = QString("observe %1").arg(game_id);
     socket->write((observe_cmd + "\n").toUtf8());
     if (!suppress_server_console)
         output_console->append(QString(">>> SENT: %1 (observing game)").arg(observe_cmd));

     // Defer "moves N" until IGS confirms with "9 Adding game to observation list."
     // Sending moves N in the same tick as observe causes IGS to silently drop it
     // because the observation isn't registered server-side yet.
     slot->replay_state = GameSlot::WAITING_FOR_MOVES0;
     slot->catchup_high = -1;
     slot->pending_catchup_moves.clear();
     games_pending_moves_request.insert(game_id);
     qDebug() << "[SLOT] game" << game_id << "observe sent, waiting for IGS confirmation before moves N";

     return;
 }

 // -----------------------------------------------------------------------
 // Non-docked mode (original path — unchanged)
 // -----------------------------------------------------------------------

 // Check if already observing this game (skip finished games - IGS reuses game IDs)
 for (BoardWindow* board : board_windows) {
 if (board->isObserving() && board->getObservedGameId() == game_id && !board->isFinished()) {
 board->show();
 board->raise();
 board->activateWindow();
 return;
 }
 }

 // Create new board window
 qDebug() << "DEBUG: About to create BoardWindow";
 BoardWindow* board = new BoardWindow(this, login_username);
 qDebug() << "DEBUG: BoardWindow created successfully";
 connect(board, &BoardWindow::boardClosed, this, &FixedXGospelWindow::closeBoardWindow);
 connect(board, &BoardWindow::saveRequested, this, &FixedXGospelWindow::saveBoardGame);
 connect(board, &BoardWindow::resignRequested, this, &FixedXGospelWindow::resignGame);
 connect(board, &BoardWindow::doneRequested, this, [this](int) {
     socket->write("done\n"); socket->flush();
     output_console->append("[IGS] >>> done (manual Done button)"); });
 connect(board, &BoardWindow::refreshRequested, this, [this](int gid) {
     socket->write(QString("moves %1\n").arg(gid).toUtf8());
     output_console->append(QString("[IGS] >>> moves %1 (Refresh Board)").arg(gid)); });
 connect(board, &BoardWindow::commentRequested, this, &FixedXGospelWindow::sendComment);
 connect(board, &BoardWindow::sayRequested, this, &FixedXGospelWindow::sendSay);
 connect(board, &BoardWindow::observersRequested, this, &FixedXGospelWindow::requestObservers);
 connect(board, &BoardWindow::observerClicked, this, [this](const QString &name) { if (players_window) players_window->openStatsDialogForPlayer(name); });
 connect(board, &BoardWindow::moveRequested, this, &FixedXGospelWindow::sendMove);

 qDebug() << "Connected comment signals for board window observing game:" << game_id;

 board_windows.append(board);
 most_recently_observed_board = board; // Track for teaching title assignment
 most_recently_observed_game_id = game_id;
 board->startObserving(game_id, white, black, white_rank, black_rank);
 
 // Apply stored game details from Command 7 if available
 if (game_komi_map.contains(game_id)) {
 double stored_komi = game_komi_map[game_id];
 int stored_handicap = game_handicap_map.value(game_id, 0);
 QString stored_type = game_type_map.value(game_id, "Free");
 
 qDebug() << "*** DEBUG KOMI: Applying stored komi=" << stored_komi << "handicap=" << stored_handicap << "for game" << game_id;
 board->updateGameSetup(stored_handicap, stored_komi, QString("Type: %1").arg(stored_type));
 board->updateGameDetails(stored_type, 0); // byoyomi will be updated separately
 }
 
 board->show();
 
 // Send observe command to IGS
 QString observe_cmd = QString("observe %1").arg(game_id);
 socket->write((observe_cmd + "\n").toUtf8());
 if (!this->suppress_server_console) output_console->append(QString(">>> SENT: %1 (observing game)").arg(observe_cmd));
 if (!this->suppress_server_console) output_console->append(QString(">>> OBSERVING GAME %1: %2 [%3] vs. %4 [%5]")
 .arg(game_id).arg(white).arg(white_rank).arg(black).arg(black_rank));
 if (!this->suppress_server_console) output_console->append(">>> WAITING for IGS to confirm observation before comments will work...");

 // q5Go pattern: Do NOT send "moves N" command here
 // Wait for first Command 15 move to arrive, then send "moves N" from there
 // This prevents duplicates (live moves + history moves)

 // Track this game as observed and update Games window highlighting (xgospel1 pattern)
 observed_game_ids.insert(game_id);
 if (games_window) {
 games_window->updateObservedGames(observed_game_ids);
 }
 if (!this->suppress_server_console) output_console->append(QString(">>> Waiting for first live move before requesting history..."));
 }
 
 // -----------------------------------------------------------------------
 // Engine / local game slots
 // -----------------------------------------------------------------------

 void launchEngineVsEngine() {
     const QList<EngineProfile> profiles = engine_manager->profiles();
     if (profiles.size() < 1) {
         QMessageBox::information(this, "No Engines",
             "Configure at least one engine in Engines → Manage Engines.");
         return;
     }

     EngineVsEngineDialog dlg(profiles, this);
     if (dlg.exec() != QDialog::Accepted) return;

     QString black_id = dlg.blackProfileId();
     QString white_id = dlg.whiteProfileId();
     double  komi     = dlg.komi();
     int     hc       = dlg.handicap();

     QString black_name, white_name;
     for (const EngineProfile &p : profiles) {
         if (p.id == black_id) black_name = p.name;
         if (p.id == white_id) white_name = p.name;
     }

     // Stop any running engines
     engine_manager->detach();
     engine_manager2->detach();
     eve_mode     = false;
     engine_color = WHITE_STONE;  // In EvE, engine_color = White engine's color

     // Share the same profile registry with engine_manager2
     engine_manager2->setProfiles(profiles);

     // Launch board window
     if (engine_board) { engine_board->close(); engine_board = nullptr; }
     engine_board = new BoardWindow(this, "");
     engine_board->setLocalPlayMode(true);
     connect(engine_board, &BoardWindow::boardClosed,
             this, &FixedXGospelWindow::closeBoardWindow);
     connect(engine_board, &BoardWindow::engineGoRequested,
             this, &FixedXGospelWindow::onEngineGoClicked);
     connect(engine_board, &BoardWindow::engineCmdRequested,
             this, &FixedXGospelWindow::onManualEngineCmd);
     board_windows.append(engine_board);

     engine_board->startObserving(engine_game_id, white_name, black_name, "", "");
     engine_board->updateGameSetup(hc, komi, "");
     // Not playing mode — just observing the two engines
     engine_board->setWindowTitle(
         QString("EvE: %1 (B) vs %2 (W)  [Engine starting...]")
             .arg(black_name).arg(white_name));
     engine_board->show();

     // Engine 1 (black_id) starts first; engine 2 (white_id) starts after engine 1 is ready.
     // This is color-independent — works for local/remote in any combination.
     m_eve_white_id  = white_id;
     m_eve_komi      = komi;
     m_eve_handicap  = hc;
     m_eve_black_ready = false;
     m_eve_white_ready = false;
     eve_mode = true;

     // Start engine 1 (Black)
     if (!engine_manager->attach(black_id, komi, hc)) {
         QMessageBox::critical(this, "Engine Error",
             QString("Failed to start Black engine (%1).").arg(black_name));
         eve_mode = false;
         return;
     }

     KataGoEngine *black_engine = engine_manager->currentEngine();

     connect(black_engine, &KataGoEngine::gtpLogLine,
             engine_board, &BoardWindow::appendEngineLog);
     connect(black_engine, &KataGoEngine::engineReady,
             this, &FixedXGospelWindow::onEveBlackReady);
     connect(black_engine, &KataGoEngine::moveReady,
             this, &FixedXGospelWindow::onEveBlackMove);
     connect(black_engine, &KataGoEngine::passMoveReady,
             this, &FixedXGospelWindow::onEveBlackPass);
     connect(black_engine, &KataGoEngine::engineResigned,
             this, &FixedXGospelWindow::onEveBlackResigned);
     connect(black_engine, &KataGoEngine::scoreReady,
             this, &FixedXGospelWindow::onEngineScore);
     connect(black_engine, &KataGoEngine::engineError,
             this, &FixedXGospelWindow::onEngineError);
     connect(black_engine, &KataGoEngine::handicapStonesReady,
             this, &FixedXGospelWindow::onEngineHandicapStones);
     // Engine 2 (White) started in onEveBlackReady() once engine 1 is up

     engine_board->setEngineStatus(
         QString("%1 (B) vs %2 (W)").arg(black_name).arg(white_name), false);
 }

 void onEveBlackReady() {
     m_eve_black_ready = true;
     if (engine_board)
         engine_board->appendEngineLog("--- Black engine ready. Starting White engine... ---", false);

     // Now start white engine serially
     engine_manager2->setProfiles(engine_manager->profiles());
     if (!engine_manager2->attach(m_eve_white_id, m_eve_komi, m_eve_handicap)) {
         QMessageBox::critical(this, "Engine Error", "Failed to start White engine.");
         engine_manager->detach();
         eve_mode = false;
         return;
     }

     KataGoEngine *white_engine = engine_manager2->currentEngine();
     connect(white_engine, &KataGoEngine::gtpLogLine,
             engine_board, &BoardWindow::appendEngineLog);
     connect(white_engine, &KataGoEngine::engineReady,
             this, &FixedXGospelWindow::onEveWhiteReady);
     connect(white_engine, &KataGoEngine::moveReady,
             this, &FixedXGospelWindow::onEveWhiteMove);
     connect(white_engine, &KataGoEngine::passMoveReady,
             this, &FixedXGospelWindow::onEveWhitePass);
     connect(white_engine, &KataGoEngine::engineResigned,
             this, &FixedXGospelWindow::onEveWhiteResigned);
     connect(white_engine, &KataGoEngine::scoreReady,
             this, &FixedXGospelWindow::onEngineScore);
     connect(white_engine, &KataGoEngine::engineError,
             this, &FixedXGospelWindow::onEngineError);
 }
 void onEveWhiteReady() {
     m_eve_white_ready = true;
     if (m_eve_black_ready) onEveBothReady();
 }
 void onEveBothReady() {
     if (!engine_board) return;
     engine_board->setEngineReady(true);
     engine_board->setWindowTitle(engine_board->windowTitle()
         .replace("  [Engine starting...]", "  [Running]"));
     engine_board->setEngineStatus(
         engine_board->windowTitle().section(':', 1).trimmed(), true);
     // Black moves first
     engine_manager->currentEngine()->requestGenmove(BLACK_STONE);
 }

 void onEveBlackMove(int x, int y) {
     if (!engine_board || !eve_mode) return;
     // Render black stone
     GameMove move;
     move.game_id       = engine_game_id;
     move.move_number   = engine_board->getCurrentMove() + 1;
     move.color         = BLACK_STONE;
     move.x = x; move.y = y;
     move.is_live_move  = true;
     move.received_time = QDateTime::currentDateTime();
     engine_board->processMove(move);
     // Tell white engine about black's move then request white's reply
     engine_manager2->currentEngine()->sendPlay(BLACK_STONE, x, y);
     engine_manager2->currentEngine()->requestGenmove(WHITE_STONE);
 }
 void onEveBlackPass() {
     if (!engine_board || !eve_mode) return;
     GameMove move;
     move.game_id = engine_game_id;
     move.move_number = engine_board->getCurrentMove() + 1;
     move.color = BLACK_STONE; move.x = -1; move.y = -1;
     move.is_live_move = true;
     move.received_time = QDateTime::currentDateTime();
     engine_board->processMove(move);
     engine_manager2->currentEngine()->sendPass(BLACK_STONE);
     if (engine_board->getConsecutivePasses() >= 2)
         engine_manager->currentEngine()->requestFinalScore();
     else
         engine_manager2->currentEngine()->requestGenmove(WHITE_STONE);
 }
 void onEveBlackResigned() {
     if (!engine_board) return;
     engine_board->updateGameResult("W+Resign");
     engine_manager->detach();
     engine_manager2->detach();
     eve_mode = false;
 }

 void onEveWhiteMove(int x, int y) {
     if (!engine_board || !eve_mode) return;
     GameMove move;
     move.game_id       = engine_game_id;
     move.move_number   = engine_board->getCurrentMove() + 1;
     move.color         = WHITE_STONE;
     move.x = x; move.y = y;
     move.is_live_move  = true;
     move.received_time = QDateTime::currentDateTime();
     engine_board->processMove(move);
     // Tell black engine about white's move then request black's reply
     engine_manager->currentEngine()->sendPlay(WHITE_STONE, x, y);
     engine_manager->currentEngine()->requestGenmove(BLACK_STONE);
 }
 void onEveWhitePass() {
     if (!engine_board || !eve_mode) return;
     GameMove move;
     move.game_id = engine_game_id;
     move.move_number = engine_board->getCurrentMove() + 1;
     move.color = WHITE_STONE; move.x = -1; move.y = -1;
     move.is_live_move = true;
     move.received_time = QDateTime::currentDateTime();
     engine_board->processMove(move);
     engine_manager->currentEngine()->sendPass(WHITE_STONE);
     if (engine_board->getConsecutivePasses() >= 2)
         engine_manager2->currentEngine()->requestFinalScore();
     else
         engine_manager->currentEngine()->requestGenmove(BLACK_STONE);
 }
 void onEveWhiteResigned() {
     if (!engine_board) return;
     engine_board->updateGameResult("B+Resign");
     engine_manager->detach();
     engine_manager2->detach();
     eve_mode = false;
 }

 void openEnginesPrefs() {
     PreferencesDialog *dlg = new PreferencesDialog(engine_manager, this);
     dlg->setAttribute(Qt::WA_DeleteOnClose);
     // Jump straight to the Engines tab (index 2: Server, App Settings, Engines)
     if (QTabWidget *tw = dlg->findChild<QTabWidget*>())
         tw->setCurrentIndex(2);
     dlg->exec();
 }

 void populateAttachSubmenu() {
     if (!attach_submenu) return;
     attach_submenu->clear();
     const QList<EngineProfile> profiles = engine_manager->profiles();
     if (profiles.isEmpty()) {
         QAction *none = attach_submenu->addAction("(No engines configured)");
         none->setEnabled(false);
         return;
     }
     for (const EngineProfile &p : profiles) {
         QAction *act = attach_submenu->addAction(p.name);
         act->setCheckable(true);
         act->setChecked(p.id == m_pending_engine_profile_id);
         QString pid = p.id;
         connect(act, &QAction::triggered, this, [this, pid]() {
             attachEngine(pid);
         });
     }
 }

 void attachEngine(const QString &profileId) {
     // If a local game is already running, ask before replacing
     if (engine_manager->isAttached()) {
         engine_manager->detach();
     }
     // Attachment happens at game-start time (we need komi/handicap from the dialog)
     // Just record which profile the user selected
     m_pending_engine_profile_id = profileId;
     settings->setSelectedEngineProfile(profileId);
     settings->save();
     // Update menu state
     QString name;
     for (const EngineProfile &p : engine_manager->profiles())
         if (p.id == profileId) { name = p.name; break; }
     if (!this->suppress_server_console)
         output_console->append(QString(">>> Engine selected: %1").arg(name));
 }

 void detachEngine() {
     engine_manager->detach();
     if (!this->suppress_server_console)
         output_console->append(">>> Engine detached.");
 }

 void launchLocalEngineGame() {
     const QList<EngineProfile> profiles = engine_manager->profiles();

     if (profiles.isEmpty()) {
         QMessageBox::information(this, "No Engine",
             "No engine configured. Use Engines → Manage Engines to add one.");
         return;
     }

     // Default selection: last used profile, or first in list
     QString default_id = m_pending_engine_profile_id.isEmpty()
                          ? profiles.first().id
                          : m_pending_engine_profile_id;

     LocalGameDialog dlg(profiles, default_id, this);
     if (dlg.exec() != QDialog::Accepted) return;

     QString profile_id = dlg.selectedProfileId();
     QString engine_name;
     for (const EngineProfile &p : profiles)
         if (p.id == profile_id) { engine_name = p.name; break; }

     // Resolve random color
     StoneColor user_color = dlg.userColor();
     if (user_color == EMPTY)
         user_color = static_cast<StoneColor>((qrand() % 2) + 1);
     engine_color = (user_color == BLACK_STONE) ? WHITE_STONE : BLACK_STONE;
     double komi        = dlg.komi();
     int    hc          = dlg.handicap();
     int    boardsize   = dlg.boardsize();
     int    timepermove = dlg.timePerMove();

     // Remember selection for next time
     m_pending_engine_profile_id = profile_id;

     // Attach engine
     if (!engine_manager->attach(profile_id, komi, hc, boardsize, timepermove)) {
         QMessageBox::critical(this, "Engine Error", "Failed to attach engine.");
         return;
     }
     KataGoEngine *engine = engine_manager->currentEngine();

     // Create board window
     engine_board = new BoardWindow(this, login_username.isEmpty() ? "Player" : login_username);
     engine_board->setLocalPlayMode(true);
     connect(engine_board, &BoardWindow::boardClosed,
             this, &FixedXGospelWindow::closeBoardWindow);
     connect(engine_board, &BoardWindow::moveRequested,
             this, &FixedXGospelWindow::onLocalMove);
     connect(engine_board, &BoardWindow::resignRequested,
             this, &FixedXGospelWindow::onLocalResign);
     connect(engine_board, &BoardWindow::passRequested,
             this, &FixedXGospelWindow::onLocalPass);
     connect(engine_board, &BoardWindow::engineCmdRequested,
             this, &FixedXGospelWindow::onManualEngineCmd);
     connect(engine_board, &BoardWindow::engineGoRequested,
             this, &FixedXGospelWindow::onEngineGoClicked);
     connect(engine_board, &BoardWindow::engineClearRequested,
             this, &FixedXGospelWindow::onEngineClearBoard);
     connect(engine_board, &BoardWindow::undoRequested,
             this, &FixedXGospelWindow::onLocalUndo);
     board_windows.append(engine_board);

     QString user_name  = login_username.isEmpty() ? "Player" : login_username;
     QString white_name = (engine_color == WHITE_STONE) ? engine_name : user_name;
     QString black_name = (engine_color == BLACK_STONE) ? engine_name : user_name;
     engine_board->startObserving(engine_game_id, white_name, black_name, "", "");
     engine_board->updateGameSetup(hc, komi, "");
     engine_board->setPlayingMode(true);
     engine_board->setWindowTitle(QString("vs %1  [Engine starting...]").arg(engine_name));
     engine_board->show();

     // Wire engine signals
     connect(engine, &KataGoEngine::engineReady,
             this, &FixedXGospelWindow::onEngineReady);
     connect(engine, &KataGoEngine::moveReady,
             this, &FixedXGospelWindow::onEngineMove);
     connect(engine, &KataGoEngine::passMoveReady,
             this, &FixedXGospelWindow::onEnginePass);
     connect(engine, &KataGoEngine::engineResigned,
             this, &FixedXGospelWindow::onEngineResigned);
     connect(engine, &KataGoEngine::handicapStonesReady,
             this, &FixedXGospelWindow::onEngineHandicapStones);
     connect(engine, &KataGoEngine::scoreReady,
             this, &FixedXGospelWindow::onEngineScore);
     connect(engine, &KataGoEngine::engineError,
             this, &FixedXGospelWindow::onEngineError);
     connect(engine, &KataGoEngine::illegalMove,
             this, &FixedXGospelWindow::onEngineIllegalMove);
     connect(engine, &KataGoEngine::boardCleared,
             this, &FixedXGospelWindow::onEngineBoardCleared);
     connect(engine, &KataGoEngine::gtpLogLine,
             engine_board, &BoardWindow::appendEngineLog);

     engine_board->setEngineStatus(engine_name, false);  // red dot until ready
 }

 void onEngineReady() {
     if (engine_board) {
         const QList<EngineProfile> &profs = engine_manager->profiles();
         QString name;
         for (const EngineProfile &p : profs)
             if (p.id == m_pending_engine_profile_id) { name = p.name; break; }
         engine_board->setEngineStatus(name, true);
         engine_board->setEngineReady(true);  // Unlock board clicks
     }
     // If engine plays Black (user is White), engine moves first
     if (engine_color == BLACK_STONE && engine_board)
         engine_manager->currentEngine()->requestGenmove(BLACK_STONE);
     // If user plays Black, board is now unlocked and waiting for user's first click
 }

 void onManualEngineCmd(const QString &cmd) {
     if (!engine_manager->isAttached()) return;
     engine_manager->currentEngine()->enqueueRaw(cmd);
 }

 void onEngineClearBoard() {
     if (!engine_manager->isAttached() || !engine_board) return;
     engine_board->setEngineReady(false);  // disable Engine: Go until ACK
     engine_manager->currentEngine()->clearBoard();
 }

 void onEngineBoardCleared() {
     if (engine_board) engine_board->setEngineReady(true);  // re-enable Engine: Go
 }

 void onLocalUndo() {
     if (!engine_manager->isAttached() || !engine_board) return;
     KataGoEngine *eng = engine_manager->currentEngine();
     // Undo engine's move then user's move (two GTP undos)
     eng->enqueueRaw("undo");
     eng->enqueueRaw("undo");
     // Step back two nodes in the game tree
     engine_board->stepBackOneMove();
     engine_board->stepBackOneMove();
     // Disable undo if we're back at the start (after handicap stones)
     engine_board->enableUndoButton(engine_board->getCurrentMove() > 9);
 }

 void onEngineGoClicked() {
     if (!engine_manager->isAttached() || !engine_board) return;
     engine_manager->currentEngine()->requestGenmove(engine_color);
 }

 void onEngineMove(int x, int y) {
     if (!engine_board) return;
     GameMove move;
     move.game_id      = engine_game_id;
     move.move_number  = engine_board->getCurrentMove() + 1;
     move.color        = engine_color;
     move.x            = x;
     move.y            = y;
     move.is_live_move = true;
     move.received_time = QDateTime::currentDateTime();
     engine_board->processMove(move);
     engine_board->enableUndoButton(true);
 }

 void onEnginePass() {
     if (!engine_board) return;
     GameMove move;
     move.game_id      = engine_game_id;
     move.move_number  = engine_board->getCurrentMove() + 1;
     move.color        = engine_color;
     move.x            = -1;
     move.y            = -1;
     move.is_live_move = true;
     move.received_time = QDateTime::currentDateTime();
     engine_board->processMove(move);
     if (engine_board->getConsecutivePasses() >= 2)
         engine_manager->currentEngine()->requestFinalScore();
 }

 void onEngineResigned() {
     if (!engine_board) return;
     // Engine resigned — user wins
     StoneColor user_color = (engine_color == WHITE_STONE) ? BLACK_STONE : WHITE_STONE;
     QString result = (user_color == BLACK_STONE) ? "B+Resign" : "W+Resign";
     engine_board->updateGameResult(result);
     engine_manager->detach();
 }

 void onEngineHandicapStones(QList<QPair<int,int>> stones) {
     if (!engine_board) return;
     for (const QPair<int,int> &pos : stones) {
         GameMove move;
         move.game_id      = engine_game_id;
         move.move_number  = engine_board->getCurrentMove() + 1;
         move.color        = BLACK_STONE;
         move.x            = pos.first;
         move.y            = pos.second;
         move.is_live_move = false;
         move.received_time = QDateTime::currentDateTime();
         engine_board->processMove(move);
     }
 }

 void onEngineScore(const QString &result) {
     if (!engine_board) return;
     engine_board->updateGameResult(result);
 }

 void onEngineError(const QString &msg) {
     // If a bot game is active, send resign to IGS immediately so we don't forfeit on time.
     if (bot_mode_active && bot_game_id != -1 &&
             socket->state() == QTcpSocket::ConnectedState) {
         output_console->append(QString("[BOT] Engine crash during game %1 — sending resign to IGS. Error: %2")
             .arg(bot_game_id).arg(msg));
         socket->write("resign\n");
         socket->flush();
     } else {
         QMessageBox::critical(this, "Engine Error", msg);
         if (engine_board) engine_board->close();
     }
 }

 void onEngineIllegalMove(const QString &vertex) {
     // KataGo rejected our play — undo the optimistic stone from the client board,
     // re-enable the board so the user can try again.
     if (engine_board) {
         engine_board->stepBackOneMove();
         engine_board->setEngineReady(true);
         engine_board->appendEngineLog(
             QString("<<< ? illegal move (%1) — stone removed, please try again").arg(vertex), false);
     }
 }

 void onLocalMove(int game_id, int x, int y) {
     if (game_id != engine_game_id || !engine_manager->isAttached()) return;
     StoneColor user_color = (engine_color == WHITE_STONE) ? BLACK_STONE : WHITE_STONE;

     // Render the user's stone on the board immediately (engine won't echo it back)
     if (engine_board) {
         GameMove move;
         move.game_id       = engine_game_id;
         move.move_number   = engine_board->getCurrentMove() + 1;
         move.color         = user_color;
         move.x             = x;
         move.y             = y;
         move.is_live_move  = true;
         move.received_time = QDateTime::currentDateTime();
         engine_board->processMove(move);
     }

     engine_manager->currentEngine()->sendPlay(user_color, x, y);
     engine_manager->currentEngine()->requestGenmove(engine_color);
 }

 void onLocalPass(int game_id) {
     if (game_id != engine_game_id || !engine_manager->isAttached()) return;
     StoneColor user_color = (engine_color == WHITE_STONE) ? BLACK_STONE : WHITE_STONE;

     // Render user pass on the board
     if (engine_board) {
         GameMove move;
         move.game_id       = engine_game_id;
         move.move_number   = engine_board->getCurrentMove() + 1;
         move.color         = user_color;
         move.x             = -1;
         move.y             = -1;
         move.is_live_move  = true;
         move.received_time = QDateTime::currentDateTime();
         engine_board->processMove(move);
     }

     engine_manager->currentEngine()->sendPass(user_color);
     if (engine_board && engine_board->getConsecutivePasses() >= 2)
         engine_manager->currentEngine()->requestFinalScore();
     else
         engine_manager->currentEngine()->requestGenmove(engine_color);
 }

 void onLocalResign(int game_id) {
     if (game_id != engine_game_id) return;
     engine_manager->detach();
     if (engine_board) {
         StoneColor user_color = (engine_color == WHITE_STONE) ? BLACK_STONE : WHITE_STONE;
         QString result = (user_color == BLACK_STONE) ? "W+Resign" : "B+Resign";
         engine_board->updateGameResult(result);
     }
 }

 void closeBoardWindow(int game_id) {
 qDebug() << "[closeBoardWindow] Called for game" << game_id;

 if (docked_pane_mode) {
     GameSlot *slot = findSlot(game_id);
     if (!slot) return;

     // Never allow a live playing slot to be closed mid-game.
     // Pure observation slots can always be closed (user may have clicked by mistake).
     if (!slot->game_finished && !slot->is_observing && game_id > 0) {
         output_console->append(QString("[WARN] Cannot close game %1 — game is still in progress.").arg(game_id));
         if (shared_board_window)
             shared_board_window->processComment("*SYSTEM*",
                 QString("Cannot close game %1 — game is still in progress.").arg(game_id), false);
         return;
     }

     // Only send unobserve for live (non-finished, non-negative-synthetic) slots.
     if (!slot->game_finished && game_id > 0) {
         QString unobserve_cmd = QString("unobserve %1").arg(game_id);
         socket->write((unobserve_cmd + "\n").toUtf8());
         if (!suppress_server_console)
             output_console->append(QString(">>> SENT: %1 (stopped observing)").arg(unobserve_cmd));
     }
     games_with_moves_requested.remove(game_id);
     games_pending_moves_request.remove(game_id);
     moves_dispatch_queue.removeAll(game_id);

     // Remove from dock and slot list
     GameSelectionDock *dock = shared_board_window ? shared_board_window->getGameSelectionDock() : nullptr;
     if (dock) dock->removeGame(game_id);

     // If closing the active game, switch to another slot first — delete AFTER
     // switchActiveGame so the board window's game_root pointer stays valid until
     // loadSlot() in switchActiveGame has redirected it to the new slot.
     if (active_slot_game_id == game_id) {
         if (shared_board_window)
             shared_board_window->snapshotToSlot(slot);
         game_slots.removeOne(slot);
         active_slot_game_id = -1;

         // Find another slot to display — prefer a live bot game, then any slot.
         GameSlot *next = nullptr;
         for (GameSlot *s : game_slots) {
             if (bot_mode_active && s->game_id == bot_game_id) { next = s; break; }
         }
         if (!next && !game_slots.isEmpty()) next = game_slots.first();

         if (next) {
             switchActiveGame(next->game_id);  // loadSlot redirects board's game_root to next slot
         } else if (shared_board_window) {
             shared_board_window->clearBoard();
             shared_board_window->hide();
         }
         // Detach before delete — shared_board_window->clock_timer may still point here
         if (shared_board_window) shared_board_window->detachSlotClockTimer();
         slot->clock_timer->stop();
         delete slot;
     } else {
         game_slots.removeOne(slot);
         // Detach before delete — shared_board_window->clock_timer may point to this slot's timer
         if (shared_board_window) shared_board_window->detachSlotClockTimer();
         slot->clock_timer->stop();
         delete slot;
     }

     observed_game_ids.remove(game_id);
     if (games_window) {
         games_window->clearSelectionForGame(game_id);
         games_window->updateObservedGames(observed_game_ids);
     }
     return;
 }

 qDebug() << "[closeBoardWindow] board_windows.size=" << board_windows.size();
 for (int i = 0; i < board_windows.size(); i++) {
 if (board_windows[i]->getObservedGameId() == game_id) {
 qDebug() << "[closeBoardWindow] Found matching board at index" << i;
 BoardWindow* board = board_windows.takeAt(i);

 // Clear tracking pointer if this was the most recently observed board
 if (most_recently_observed_board == board) {
 most_recently_observed_board = nullptr;
 qDebug() << "[closeBoardWindow] Cleared most_recently_observed_board pointer (was game" << game_id << ")";
 }

 board->deleteLater();

 // Send unobserve command to IGS
 QString unobserve_cmd = QString("unobserve %1").arg(game_id);
 socket->write((unobserve_cmd + "\n").toUtf8());
 if (!this->suppress_server_console) output_console->append(QString(">>> SENT: %1 (stopped observing)").arg(unobserve_cmd));

 // Remove from observed games tracking and update Games window highlighting (xgospel1 pattern)
 qDebug() << "[closeBoardWindow] Removing game" << game_id << "from observed_game_ids, size before:" << observed_game_ids.size();
 observed_game_ids.remove(game_id);
 qDebug() << "[closeBoardWindow] observed_game_ids size after removal:" << observed_game_ids.size();
 if (games_window) {
 qDebug() << "[closeBoardWindow] Calling games_window->clearSelectionForGame(" << game_id << ")";
 games_window->clearSelectionForGame(game_id);
 qDebug() << "[closeBoardWindow] Calling games_window->updateObservedGames()";
 games_window->updateObservedGames(observed_game_ids);
 } else {
 qDebug() << "[closeBoardWindow] WARNING: games_window is NULL!";
 }

 break;
 }
 }
 qDebug() << "[closeBoardWindow] Finished";
 }

 void untrackFinishedGame(int game_id) {
 // Called when a game ends (result received).
 // Removes the game from live-tracking (observed_game_ids, moves flags, bot cleanup)
 // but intentionally leaves the GameSlot and dock button in place so the user can
 // review or save the completed game.  The slot's game_finished=true flag is the
 // guard that prevents it from being mistaken for a live game.
 // Actual slot eviction happens lazily in the v206/v208 recycle paths when IGS
 // reuses the game ID, or via detachSlotClockTimer() + delete there.
 qDebug() << "[untrackFinishedGame] Game" << game_id << "finished — stopping clock, keeping slot on dock for review";

 // Bot mode: clean up engine if this was the bot game
 if (bot_mode_active && bot_game_id == game_id)
     botEndGame();

 // Clear moves-requested flags so IGS game ID reuse doesn't block the next game's history
 games_with_moves_requested.remove(game_id);
 games_pending_moves_request.remove(game_id);

 // Stop the finished slot's clock timer so it doesn't tick after game over.
 if (docked_pane_mode) {
     if (GameSlot *slot = findSlot(game_id)) {
         if (slot->game_finished) {
             slot->clock_timer->stop();
             // Only detach if THIS is the active slot — detaching for a background
             // finished game incorrectly disconnects the active slot's clock timer.
             if (shared_board_window && game_id == active_slot_game_id)
                 shared_board_window->detachSlotClockTimer();
             qDebug() << "[untrackFinishedGame] Clock stopped for game" << game_id;
             // Re-enable the per-slot close button now that the game is finished.
             GameSelectionDock *fdock = shared_board_window ? shared_board_window->getGameSelectionDock() : nullptr;
             if (fdock) fdock->setGameCloseable(game_id, true);
         }
     }
 }

 if (observed_game_ids.contains(game_id)) {
 observed_game_ids.remove(game_id);
 qDebug() << "[untrackFinishedGame] Removed game" << game_id << "from observed_game_ids, size now:" << observed_game_ids.size();

 // Update Games window to remove highlighting
 if (games_window) {
 games_window->clearSelectionForGame(game_id);
 games_window->updateObservedGames(observed_game_ids);
 } else {
 qDebug() << "[untrackFinishedGame] WARNING: games_window is NULL!";
 }
 } else {
 qDebug() << "[untrackFinishedGame] Game" << game_id << "not in observed_game_ids (already removed or never tracked)";
 }
 }

 // =========================================================
 // Bot Mode (Phase 2)
 // =========================================================

 void toggleBotMode(bool enabled) {
     bot_mode_active = enabled;
     if (bot_mode_action) bot_mode_action->setChecked(enabled);
     settings->setBotModeEnabled(enabled);
     settings->save();
     QString state = enabled ? "ON" : "OFF";
     output_console->append(QString("[BOT] Bot mode %1").arg(state));
     if (socket && socket->state() == QAbstractSocket::ConnectedState) {
         if (enabled) {
             // Advertise availability to challengers
             socket->write("toggle open true\n");
             socket->flush();
         } else {
             if (bot_game_id != -1) {
                 output_console->append("[BOT] Warning: bot mode disabled while a bot game is in progress.");
             } else {
                 // No longer accepting challenges
                 socket->write("toggle open false\n");
                 socket->flush();
                 // Shut down the warm engine now that bot mode is off
                 engine_manager->detach();
             }
         }
     }
 }

 // Convert an IGS rank string to a linear stone scale for handicap calculation.
 // Scale: 30k=-21, ..., 1k=-1, 1d=0, 2d=1, ..., 9d=8, 1p=9, ..., 9p=17
 // Matches IGS server convention: 1d vs 1k = 1 stone difference.
 static int rankToStones(const QString &rank) {
     QString r = rank.trimmed().toLower();
     r.remove('*').remove('+').remove('?');
     if (r == "nr" || r == "bc") return -30;
     QRegExp re("(\\d+)([dkp])");
     if (re.indexIn(r) == -1) return -30;
     int num = re.cap(1).toInt();
     QString type = re.cap(2);
     if (type == "p") return 9 + (num - 1);   // 1p=9 .. 9p=17
     if (type == "d") return num - 1;          // 1d=0 .. 9d=8
     if (type == "k") return -num;             // 1k=-1 .. 30k=-30
     return -30;
 }

 // Expected handicap and color for a bot game given two rank stone values.
 // Returns expected_hc (0-9) and sets bot_should_be_black.
 // IGS rule: diff=0 or 1 → even game (hc=0, nigiri for color).
 //           diff>=2 → weaker player gets Black with hc=diff stones (capped at 9).
 static int expectedHandicap(int bot_stones, int opp_stones, bool &bot_should_be_black) {
     int diff = opp_stones - bot_stones; // positive = opponent stronger
     int abs_diff = qAbs(diff);
     if (abs_diff <= 1) {
         // Even game — 1-rank difference plays without handicap on IGS
         bot_should_be_black = false; // nigiri decides; don't enforce color
         return 0;
     }
     // Handicap game: weaker player is Black
     bot_should_be_black = (diff > 0); // bot weaker → bot Black
     return qMin(abs_diff, 9);
 }

 // Called from the nmatch/match incoming request handlers when bot mode is active.
 // Validates constraints and sends the accept command directly (no dialog).
 void botAcceptMatch(const QString &opponent, const QString &suggested_cmd,
                     StoneColor my_color, int boardsize, int handicap,
                     int main_time, int byoyomi_time, int byoyomi_stones, double komi) {
     if (bot_game_id != -1) {
         // Already in a game — decline
         output_console->append(QString("[BOT] Declining %1 — already in a bot game.").arg(opponent));
         socket->write(QString("decline %1\n").arg(opponent).toUtf8());
         socket->flush();
         return;
     }
     if (boardsize != 19) {
         output_console->append(QString("[BOT] Declining %1 — only 19x19 supported.").arg(opponent));
         socket->write(QString("decline %1\n").arg(opponent).toUtf8());
         socket->flush();
         return;
     }
     if (byoyomi_time < 60) {
         output_console->append(QString("[BOT] Declining %1 — byoyomi period %2s < 60s minimum.").arg(opponent).arg(byoyomi_time));
         socket->write(QString("decline %1\n").arg(opponent).toUtf8());
         socket->flush();
         return;
     }

     // Greylist check: per-opponent maximum handicap with custom tell.
     // For old match protocol (handicap==0), estimate via rank difference since
     // IGS won't reveal the actual handicap until CMD67.
     int effective_handicap = handicap;
     if (handicap == 0 && suggested_cmd.startsWith("match ")) {
         QString my_r  = findPlayerRank(login_username);
         QString opp_r = findPlayerRank(opponent);
         if (my_r != "?" && opp_r != "?") {
             bool dummy = false;
             effective_handicap = expectedHandicap(rankToStones(my_r), rankToStones(opp_r), dummy);
         }
     }
     for (const auto &entry : settings->getBotGreylist()) {
         if (entry.name.compare(opponent, Qt::CaseInsensitive) == 0 && effective_handicap > entry.max_hc) {
             QString hc_note = (effective_handicap != handicap)
                 ? QString("%1 (estimated from ranks)").arg(effective_handicap)
                 : QString::number(effective_handicap);
             output_console->append(QString("[BOT] Declining %1 — %2-stone handicap exceeds greylist limit of %3.")
                 .arg(opponent).arg(hc_note).arg(entry.max_hc));
             socket->write(QString("decline %1\n").arg(opponent).toUtf8());
             socket->flush();
             if (!entry.tell.isEmpty()) {
                 socket->write(QString("tell %1 %2\n").arg(opponent).arg(entry.tell).toUtf8());
                 socket->flush();
                 output_console->append(QString("[BOT] >>> tell %1: %2").arg(opponent).arg(entry.tell));
             }
             return;
         }
     }

     // Fairness check: validate handicap and color against rank difference.
     // Old "match" protocol always sends handicap=0 — IGS assigns it server-side at CMD67.
     // In that case we skip the handicap comparison (we have no information yet) and only
     // enforce the color assignment where the rank gap makes it unambiguous.
     bool is_old_match = (handicap == 0 && suggested_cmd.startsWith("match "));
     QString my_rank_str  = findPlayerRank(login_username);
     QString opp_rank_str = findPlayerRank(opponent);
     if (my_rank_str != "?" && opp_rank_str != "?") {
         int my_stones  = rankToStones(my_rank_str);
         int opp_stones = rankToStones(opp_rank_str);
         bool bot_should_be_black = false;
         int exp_hc = expectedHandicap(my_stones, opp_stones, bot_should_be_black);

         // Skip handicap check for old match protocol — handicap unknown until CMD67
         if (!is_old_match && qAbs(handicap - exp_hc) > 1) {
             output_console->append(QString("[BOT] Declining %1 — unfair handicap: offered %2, expected %3 (me:%4 opp:%5)")
                 .arg(opponent).arg(handicap).arg(exp_hc).arg(my_rank_str).arg(opp_rank_str));
             socket->write(QString("decline %1\n").arg(opponent).toUtf8());
             socket->flush();
             return;
         }

         if (is_old_match && exp_hc > 0) {
             output_console->append(QString("[BOT] Old match protocol from %1 — expected hc %2, trusting IGS to assign (me:%3 opp:%4)")
                 .arg(opponent).arg(exp_hc).arg(my_rank_str).arg(opp_rank_str));
         }

         // Color check: if rank diff >= 2, color is not negotiable
         if (exp_hc >= 2) {
             bool offered_bot_black = (my_color == BLACK_STONE);
             if (offered_bot_black != bot_should_be_black) {
                 output_console->append(QString("[BOT] Declining %1 — wrong color: offered %2, expected %3 (me:%4 opp:%5)")
                     .arg(opponent)
                     .arg(offered_bot_black ? "Black" : "White")
                     .arg(bot_should_be_black ? "Black" : "White")
                     .arg(my_rank_str).arg(opp_rank_str));
                 socket->write(QString("decline %1\n").arg(opponent).toUtf8());
                 socket->flush();
                 return;
             }
         }
     } else {
         output_console->append(QString("[BOT] Warning: cannot verify fairness for %1 (me:%2 opp:%3) — accepting anyway")
             .arg(opponent).arg(my_rank_str).arg(opp_rank_str));
     }

     // Store match params for use after CMD 67 confirms the game
     bot_opponent         = opponent;
     bot_color            = my_color;
     bot_boardsize        = boardsize;
     bot_handicap         = handicap;   // authoritative from match offer; not from stale slot
     bot_main_time        = main_time;
     bot_byoyomi_time     = byoyomi_time;
     bot_byoyomi_stones   = byoyomi_stones;
     bot_komi             = komi;
     bot_time_remaining   = main_time;
     bot_stones_remaining = byoyomi_stones;

     output_console->append(QString("[BOT] Accepting match from %1 — %2 %3x%3 main=%4s byo=%5s/%6 stones")
         .arg(opponent)
         .arg(my_color == BLACK_STONE ? "Black" : "White")
         .arg(boardsize).arg(main_time).arg(byoyomi_time).arg(byoyomi_stones));

     socket->write((suggested_cmd + "\n").toUtf8());
     socket->flush();
 }

 // Called when CMD 67 confirms a new game and bot_mode_active is true.
 // Attaches the engine and configures it for the IGS game.
 void botStartGame(int game_id) {
     bot_game_id = game_id;
     bot_engine_ready = false;  // guard CMD 15 triggers until onBotEngineReady completes

     // Find the board window for this game (dock: shared_board_window; non-dock: board_windows)
     engine_board = nullptr;
     if (docked_pane_mode) {
         engine_board = shared_board_window;
     } else {
         for (BoardWindow *b : board_windows) {
             if (b->getObservedGameId() == game_id) { engine_board = b; break; }
         }
     }

     QString wname = (bot_color == WHITE_STONE) ? login_username : bot_opponent;
     QString bname = (bot_color == BLACK_STONE) ? login_username : bot_opponent;
     if (!docked_pane_mode && engine_board) {
         // Non-dock: startObserving resets to observing mode, then setPlayingMode corrects it
         engine_board->startObserving(game_id, wname, bname, findPlayerRank(wname), findPlayerRank(bname));
         engine_board->setPlayingMode(true);
     } else if (docked_pane_mode && engine_board) {
         // Dock: loadSlot already set is_playing; just ensure player names and labels are current
         engine_board->updatePlayerNames(wname, bname);
     }
     // Greeting is deferred to onBotEngineReady — the game must be fully open on the
     // server before "say" will be delivered. Sending it here (before "9 Creating match")
     // causes IGS to silently drop it.

     // Fetch stats for the opponent in case they weren't in the initial userlist/who.
     // The stats parser will call upsertPlayerRank when the response arrives.
     if (!bot_opponent.isEmpty())
         sendCommandString(QString("stats %1").arg(bot_opponent));

     KataGoEngine *engine = engine_manager->currentEngine();

     if (engine && engine->isReady()) {
         // Warm path: engine already running from previous game — no cold-start latency.
         // Re-wire signals (connections from the previous game are still live but safe to
         // reconnect since disconnect/connect is idempotent for the same pairs).
         output_console->append(QString("[BOT] Game %1 started vs %2 — reusing warm engine").arg(game_id).arg(bot_opponent));
         disconnect(engine, nullptr, this, nullptr);
         connect(engine, &KataGoEngine::boardCleared, this, &FixedXGospelWindow::onBotEngineReady);
         connect(engine, &KataGoEngine::moveReady, this, [this](int x, int y){
             onBotMoveReady(coordsToGtp(x, y));
         });
         connect(engine, &KataGoEngine::passMoveReady,  this, &FixedXGospelWindow::onBotPassReady);
         connect(engine, &KataGoEngine::engineResigned, this, &FixedXGospelWindow::onBotEngineResigned);
         connect(engine, &KataGoEngine::scoreReady,     this, &FixedXGospelWindow::onBotScoreReady);
         connect(engine, &KataGoEngine::ownershipReady, this, &FixedXGospelWindow::onBotOwnershipReady);
         connect(engine, &KataGoEngine::engineError,    this, &FixedXGospelWindow::onEngineError);
         connect(engine, &KataGoEngine::illegalMove,    this, &FixedXGospelWindow::onBotIllegalMove);
         connect(engine, &KataGoEngine::gtpLogLine, this, [this](const QString &line, bool sent){
             BoardWindow *target = engine_board;
             if (!target && docked_pane_mode) target = shared_board_window;
             if (target) target->appendEngineLog(line, sent);
         });
         // clear_board ack fires boardCleared → onBotEngineReady
         engine->clearBoard();
     } else {
         // Cold path: first game of session, or engine died — launch fresh.
         QList<EngineProfile> profiles = engine_manager->profiles();
         if (profiles.isEmpty()) {
             output_console->append("[BOT] ERROR: No engine profiles configured — cannot start bot game.");
             bot_game_id = -1;
             return;
         }
         QString profile_id = m_pending_engine_profile_id.isEmpty() ? profiles.first().id : m_pending_engine_profile_id;
         output_console->append(QString("[BOT] Game %1 started vs %2 — cold-starting engine").arg(game_id).arg(bot_opponent));

         if (!engine_manager->attach(profile_id, bot_komi, 0, bot_boardsize, 0)) {
             output_console->append("[BOT] ERROR: Failed to attach engine.");
             bot_game_id = -1;
             return;
         }
         engine = engine_manager->currentEngine();

         disconnect(engine, nullptr, this, nullptr);
         connect(engine, &KataGoEngine::engineReady,    this, &FixedXGospelWindow::onBotEngineReady);
         connect(engine, &KataGoEngine::moveReady, this, [this](int x, int y){
             onBotMoveReady(coordsToGtp(x, y));
         });
         connect(engine, &KataGoEngine::passMoveReady,  this, &FixedXGospelWindow::onBotPassReady);
         connect(engine, &KataGoEngine::engineResigned, this, &FixedXGospelWindow::onBotEngineResigned);
         connect(engine, &KataGoEngine::scoreReady,     this, &FixedXGospelWindow::onBotScoreReady);
         connect(engine, &KataGoEngine::ownershipReady, this, &FixedXGospelWindow::onBotOwnershipReady);
         connect(engine, &KataGoEngine::engineError,    this, &FixedXGospelWindow::onEngineError);
         connect(engine, &KataGoEngine::illegalMove,    this, &FixedXGospelWindow::onBotIllegalMove);
         connect(engine, &KataGoEngine::gtpLogLine, this, [this](const QString &line, bool sent){
             BoardWindow *target = engine_board;
             if (!target && docked_pane_mode) target = shared_board_window;
             if (target) target->appendEngineLog(line, sent);
         });
     }
 }

 void onBotEngineReady() {
     KataGoEngine *engine = engine_manager->currentEngine();
     if (!engine) return;

     // Komi: use slot value only if CMD 7 has updated it for this specific game
     // (white/black names match our game).  A stale slot from a previous game in
     // the same game ID slot would give the wrong komi.  bot_komi was set from
     // the match-acceptance default (6.5 even / 0.5 hc) and is safe as fallback.
     if (GameSlot *bslot = findSlot(bot_game_id)) {
         QString slot_white = game_white_player_map.value(bot_game_id);
         QString slot_black = game_black_player_map.value(bot_game_id);
         bool cmd7_is_current = (!slot_white.isEmpty() && !slot_black.isEmpty()) &&
             (slot_white.compare(bot_opponent, Qt::CaseInsensitive) == 0 ||
              slot_black.compare(bot_opponent, Qt::CaseInsensitive) == 0);
         if (cmd7_is_current)
             bot_komi = bslot->komi;
         // else keep bot_komi from match-acceptance (6.5 or 0.5)
     }

     output_console->append(QString("[BOT] Engine ready for game %1 — komi=%2 hc=%3")
         .arg(bot_game_id).arg(bot_komi).arg(bot_handicap));

     // Push authoritative handicap/komi to the board display. The slot value may
     // be 0 for a second game vs the same opponent if CMD7 has not arrived yet.
     if (engine_board)
         engine_board->updateGameSetup(bot_handicap, bot_komi, "");

     // Greet the opponent now that the game is fully open on the server.
     // Sending "say" before "9 Creating match [N]" causes IGS to silently drop it.
     {
         QString greeting = QString("Hello %1! Good luck and have fun!").arg(bot_opponent);
         socket->write((QString("say %1\n").arg(greeting)).toUtf8());
         socket->flush();
         output_console->append(QString("[BOT] >>> say: %1").arg(greeting));
         // Save to slot comment list so it persists even if the board is showing another game
         if (GameSlot *bslot = findSlot(bot_game_id)) {
             GameSlot::CommentEntry ce; ce.user = login_username; ce.text = greeting; ce.is_kibitz = false;
             bslot->comments.append(ce);
         }
         if (engine_board)
             engine_board->processComment(login_username, greeting, false);
     }

     // The engine's built-in init already sent: time_settings, boardsize, komi, clear_board.
     // Append kgs-time_settings and kata-set-rules after clear_board.
     engine->enqueueRaw(QString("kgs-time_settings canadian %1 %2 %3")
         .arg(bot_main_time).arg(bot_byoyomi_time).arg(bot_byoyomi_stones));
     engine->enqueueRaw("kata-set-rules japanese");
     // Correct komi (engine init used default 6.5; override with actual game komi)
     engine->enqueueRaw(QString("komi %1").arg(bot_komi));

     if (bot_handicap > 1) {
         // Set handicap stones — IGS standard positions
         QList<QPair<int,int>> hc_positions = IGSMoveParser::getHandicapPositions(bot_handicap);
         QStringList vertices;
         for (const auto &pos : hc_positions)
             vertices << coordsToGtp(pos.first, pos.second);
         engine->enqueueRaw(QString("set_free_handicap %1").arg(vertices.join(' ')));
         output_console->append(QString("[BOT] Sent set_free_handicap: %1").arg(vertices.join(' ')));
     }

     // Allow CMD 15 moves to trigger genmove now that setup is complete.
     bot_engine_ready = true;

     if (bot_color == BLACK_STONE && bot_handicap <= 1) {
         // Even game: Black moves first immediately.
         output_console->append("[BOT] Bot plays Black (even game) — requesting first move");
         engine->requestGenmove(BLACK_STONE);
     } else if (bot_color == WHITE_STONE && bot_handicap >= 2) {
         // Handicap game, bot is White — White moves FIRST after the handicap stones.
         // The handicap placement is move 0 (server-allocated); White does not wait for
         // a Black regular move. Call genmove W immediately now that set_free_handicap
         // has been enqueued.
         output_console->append(QString("[BOT] Bot plays White in %1-stone handicap game — requesting first move").arg(bot_handicap));
         engine->requestGenmove(WHITE_STONE);
     } else if (bot_color == WHITE_STONE) {
         // Even game, bot is White — Black moves first.
         // If Black already played before the engine finished init (fast opponent or slow
         // engine startup), we missed the CMD 15 trigger. Check the move history directly.
         if (GameSlot *bslot = findSlot(bot_game_id)) {
             QString missed_vertex;
             for (int i = bslot->move_history.size() - 1; i >= 0; --i) {
                 const auto &m = bslot->move_history[i];
                 if (m.color == BLACK_STONE && m.x >= 0) {
                     missed_vertex = coordsToGtp(m.x, m.y);
                     break;
                 }
             }
             if (!missed_vertex.isEmpty()) {
                 output_console->append(QString("[BOT] Engine ready late — opponent already played %1, triggering genmove W in 500ms").arg(missed_vertex));
                 QString captured_vertex = missed_vertex;
                 QTimer::singleShot(500, this, [this, captured_vertex]() {
                     onBotOpponentMove(captured_vertex, bot_time_remaining, bot_stones_remaining);
                 });
             }
         }
     }
     // Remaining case (bot=Black, hc>=2): Black placed handicap stones, White moves
     // first via CMD 15. If White already played before engine was ready, catch up now.
     else if (bot_color == BLACK_STONE && bot_handicap >= 2) {
         if (GameSlot *bslot = findSlot(bot_game_id)) {
             QString missed_vertex;
             for (int i = bslot->move_history.size() - 1; i >= 0; --i) {
                 const auto &m = bslot->move_history[i];
                 if (m.color == WHITE_STONE && m.x >= 0) {
                     missed_vertex = coordsToGtp(m.x, m.y);
                     break;
                 }
             }
             if (!missed_vertex.isEmpty()) {
                 output_console->append(QString("[BOT] Engine ready late — opponent already played %1, triggering genmove B in 500ms").arg(missed_vertex));
                 QString captured_vertex = missed_vertex;
                 QTimer::singleShot(500, this, [this, captured_vertex]() {
                     onBotOpponentMove(captured_vertex, bot_time_remaining, bot_stones_remaining);
                 });
             }
         }
     }
 }

 // Called from the CMD 15 move handler when bot_game_id is set and the move is the opponent's.
 void onBotOpponentMove(const QString &gtp_vertex, int time_remaining, int stones_remaining) {
     KataGoEngine *engine = engine_manager->currentEngine();
     if (!engine) return;

     QString color_str = (bot_color == BLACK_STONE) ? "white" : "black";

     // CMD9 scoring confirmed: suppress all moves — game is over for the engine.
     if (bot_scoring_pending) {
         output_console->append(QString("[BOT] Opponent played %1 — scoring phase active, skipping play+genmove")
             .arg(gtp_vertex));
         return;
     }

     // If a kata-raw-nn prefetch is in-flight, let it complete — it takes
     // milliseconds. Cancelling is counterproductive: the command is already
     // queued so clearing m_awaiting_ownership causes the response to be silently
     // dropped, leaving the engine unable to process the next ownership request.
     // onBotOwnershipReady discards the result when bot_scoring_pending is false.

     QString bot_color_str = (bot_color == BLACK_STONE) ? "black" : "white";
     int stones_to_report = qMax(0, stones_remaining);
     // Subtract a network latency buffer so KataGo doesn't use time that will be
     // eaten by IGS round-trip before the move arrives. 2s covers typical IGS latency
     // plus GTP queue overhead; floor at 1 so KataGo always gets a non-zero budget.
     static const int LAG_BUFFER_SECS = 2;
     int adjusted_time = qMax(1, time_remaining - LAG_BUFFER_SECS);
     output_console->append(QString("[BOT] Opponent played %1 — time_left %2s %3 stones — requesting genmove %4")
         .arg(gtp_vertex).arg(time_remaining).arg(stones_to_report).arg(bot_color_str));

     engine->enqueueRaw(QString("play %1 %2").arg(color_str).arg(gtp_vertex));
     engine->enqueueRaw(QString("time_left %1 %2 %3").arg(bot_color_str).arg(adjusted_time).arg(stones_to_report));
     engine->requestGenmove(bot_color);
 }

 void onBotMoveReady(const QString &gtp_vertex) {
     if (bot_game_id == -1) return;

     // Convert GTP vertex (e.g. "K10") to IGS format and send
     // GTP and IGS both use the same letter+number notation (both skip 'I'),
     // so for standard 19x19 the vertex can be sent directly.
     output_console->append(QString("[BOT] Engine plays %1 — sending to IGS").arg(gtp_vertex));
     bot_last_sent_vertex = gtp_vertex;  // track for CMD5 recovery
     socket->write((gtp_vertex + "\n").toUtf8());
     socket->flush();

     // A real move invalidates any prefetched ownership — clear the cache.
     if (!bot_cached_ownership.isEmpty()) {
         bot_cached_ownership.clear();
         output_console->append("[BOT] Real move played — ownership cache cleared");
     }

     // Sync remaining time with engine — clamp stones to 0 (IGS sends -1 during main time)
     QString bot_color_str = (bot_color == BLACK_STONE) ? "black" : "white";
     int stones_to_report = qMax(0, bot_stones_remaining);
     engine_manager->currentEngine()->enqueueRaw(
         QString("time_left %1 %2 %3")
             .arg(bot_color_str).arg(bot_time_remaining).arg(stones_to_report));

     // Update board window if it exists
     if (!engine_board) {
         for (BoardWindow *b : board_windows) {
             if (b->getObservedGameId() == bot_game_id) { engine_board = b; break; }
         }
     }
 }

 void onBotPassReady() {
     if (bot_game_id == -1) return;
     output_console->append("[BOT] Engine passes — sending pass to IGS");
     socket->write("pass\n");
     socket->flush();
     QString bot_color_str = (bot_color == BLACK_STONE) ? "black" : "white";
     int stones_to_report = qMax(0, bot_stones_remaining);
     KataGoEngine *engine = engine_manager->currentEngine();
     engine->enqueueRaw(
         QString("time_left %1 %2 %3")
             .arg(bot_color_str).arg(bot_time_remaining).arg(stones_to_report));
     // Prefetch ownership — if the engine is passing, scoring phase may be imminent.
     // Only start prefetch if: not already scoring, and no request already in flight.
     if (!bot_scoring_pending && !engine->isOwnershipInFlight()) {
         output_console->append("[BOT] Engine passed — prefetching kata-raw-nn ownership");
         bot_cached_ownership.clear();
         engine->requestOwnership();
     }
 }

 void onBotScoreReady(const QString &score) {
     // final_score may still arrive (e.g. user sends it manually) — log but don't act.
     if (bot_game_id == -1) return;
     output_console->append(QString("[BOT] KataGo final_score: %1").arg(score));
 }

 // Replays all IGS-confirmed moves from slot->move_history into the engine,
 // starting from a clean board.  Fully resyncs KataGo's internal board state
 // to the server's authoritative position.  Called by onBotIllegalMove.
 void botReplayMovesIntoEngine() {
     KataGoEngine *engine = engine_manager->currentEngine();
     if (!engine || bot_game_id == -1) return;
     GameSlot *bslot = findSlot(bot_game_id);
     if (!bslot) return;

     output_console->append("[BOT] Resyncing KataGo board from move_history...");
     engine->enqueueRaw("clear_board");
     engine->enqueueRaw(QString("komi %1").arg(bot_komi));
     engine->enqueueRaw(QString("kgs-time_settings canadian %1 %2 %3")
         .arg(bot_main_time).arg(bot_byoyomi_time).arg(bot_byoyomi_stones));
     engine->enqueueRaw("kata-set-rules japanese");

     if (bot_handicap >= 2) {
         QList<QPair<int,int>> hc_positions = IGSMoveParser::getHandicapPositions(bot_handicap);
         QStringList vertices;
         for (const auto &pos : hc_positions)
             vertices << coordsToGtp(pos.first, pos.second);
         engine->enqueueRaw(QString("set_free_handicap %1").arg(vertices.join(' ')));
     }

     // Replay all confirmed moves in order
     for (const auto &m : bslot->move_history) {
         if (m.x < 0) continue;  // skip pass/handicap markers
         QString color_str = (static_cast<StoneColor>(m.color) == BLACK_STONE) ? "black" : "white";
         engine->enqueueRaw(QString("play %1 %2").arg(color_str).arg(coordsToGtp(m.x, m.y)));
     }
     output_console->append(QString("[BOT] Replayed %1 moves — requesting genmove").arg(bslot->move_history.size()));
     engine->requestGenmove(bot_color);
 }

 // Called when KataGo rejects a play command with "? illegal move".
 // Fully resyncs the engine board from the IGS move history.
 void onBotIllegalMove(const QString &vertex) {
     if (bot_game_id == -1) return;
     output_console->append(QString("[BOT] KataGo illegal move (%1) — resyncing board from IGS history").arg(vertex));
     botReplayMovesIntoEngine();
 }

 void onBotOwnershipReady(const QVector<float> &ownership) {
     if (bot_game_id == -1) return;

     if (!bot_scoring_pending) {
         // Proactive prefetch completed before scoring trigger arrived.
         // Cache it — the scoring trigger handler will promote bot_scoring_pending
         // and call this again (or see it from the in-flight path).
         bot_cached_ownership = ownership;
         output_console->append(QString("[BOT] Ownership prefetch complete — %1 values cached")
             .arg(ownership.size()));
         return;
     }

     // Scoring phase result received. Keep bot_scoring_pending=true until done is sent
     // so the CMD49 handler can check it when deciding whether to fire removes+done.

     if (ownership.size() != 361) {
         output_console->append("[BOT] WARNING: Ownership analysis failed — sending done without remove");
         bot_scoring_pending = false;
         QTimer::singleShot(500, this, [this]() {
             bot_done_sent = true; socket->write("done\n"); socket->flush();
             output_console->append("[BOT] >>> done (no ownership)");
         });
         return;
     }

     // Resolve board window.
     BoardWindow *board = engine_board;
     if (!board && docked_pane_mode) board = shared_board_window;
     if (!board) {
         output_console->append("[BOT] WARNING: No board window — sending done without remove");
         QTimer::singleShot(500, this, [this]() {
             bot_done_sent = true; socket->write("done\n"); socket->flush();
             output_console->append("[BOT] >>> done (no board)");
         });
         return;
     }

     output_console->append("[BOT] Ownership analysis complete — identifying dead groups in bot territory");

     const int dx[] = {-1, 1, 0, 0};
     const int dy[] = {0, 0, -1, 1};

     StoneColor opponent_color = (bot_color == BLACK_STONE) ? WHITE_STONE : BLACK_STONE;

     QList<QPair<int,int>> dead_group_seeds;
     QList<QList<QPair<int,int>>> dead_group_members;

     bool use_area_map = settings->getBotUseAreaMapDetection();
     output_console->append(QString("[BOT] Dead stone algorithm: %1")
         .arg(use_area_map ? "geometric area-map (experimental)" : "KataGo ownership + liberty enclosure (classic)"));

     if (use_area_map) {
         // -----------------------------------------------------------------------
         // EXPERIMENTAL: Two-phase Sabaki-style area-map dead stone detection
         //
         // Phase 1 — Geometric flood-fill (deterministic, no KataGo thresholds):
         //   Flood-fill every contiguous empty region on the working board.
         //   A region whose entire border is bot stones is bot territory.
         //   Any opponent stone inside a bot-territory region is a dead candidate.
         //   Remove candidates and repeat until stable (handles chained groups).
         //
         // Phase 2 — KataGo ownership confirmation:
         //   ownership strongly bot (> 0.80)  -> confirmed dead (clear case)
         //   ownership near zero (|own| < 0.40) -> confirmed dead (unresolved aji)
         //   Groups that fail confirmation are left alive (conservative).
         //
         // Mirrors the @sabaki/influence areaMap algorithm: flood-fill empty
         // regions, single-color border check -> sign * indicator. KataGo is
         // demoted to a confirming role rather than the primary detector.
         // -----------------------------------------------------------------------

         const float AREA_CONFIRM_STRONG  = 0.80f;
         const float AREA_CONFIRM_AJI_MAX = 0.40f;

         QSet<QPair<int,int>> removed_cells;
         bool changed = true;
         int pass_num = 0;

         while (changed) {
             changed = false;
             ++pass_num;

             // Build area map on current board (removed_cells treated as empty).
             // areaMap[y][x]: +1 = bot territory, -1 = opponent territory, 0 = dame/mixed.
             QVector<QVector<int>> areaMap(19, QVector<int>(19, 0));
             QSet<QPair<int,int>> area_visited;

             for (int sy = 0; sy < 19; ++sy) {
                 for (int sx = 0; sx < 19; ++sx) {
                     QPair<int,int> start(sx, sy);
                     if (area_visited.contains(start)) continue;

                     StoneColor sc = board->getStoneAt(sx, sy);
                     bool is_removed = removed_cells.contains(start);

                     if (sc != EMPTY && !is_removed) {
                         areaMap[sy][sx] = (sc == bot_color) ? 1 : -1;
                         area_visited.insert(start);
                         continue;
                     }

                     // Flood-fill the contiguous empty/removed region.
                     QQueue<QPair<int,int>> q;
                     QList<QPair<int,int>> region;
                     q.enqueue(start);
                     area_visited.insert(start);
                     int border_sign = 0;
                     int indicator   = 1;

                     while (!q.isEmpty()) {
                         auto cur = q.dequeue();
                         region.append(cur);
                         for (int d = 0; d < 4; ++d) {
                             int nx = cur.first  + dx[d];
                             int ny = cur.second + dy[d];
                             if (nx < 0 || nx >= 19 || ny < 0 || ny >= 19) continue;
                             QPair<int,int> nb(nx, ny);
                             StoneColor nc = board->getStoneAt(nx, ny);
                             bool nb_removed = removed_cells.contains(nb);
                             if (nc != EMPTY && !nb_removed) {
                                 int nsign = (nc == bot_color) ? 1 : -1;
                                 if (border_sign == 0)      border_sign = nsign;
                                 else if (border_sign != nsign) indicator = 0;
                             } else if (!area_visited.contains(nb)) {
                                 area_visited.insert(nb);
                                 q.enqueue(nb);
                             }
                         }
                     }
                     int region_value = border_sign * indicator;
                     for (const auto &rc : region)
                         areaMap[rc.second][rc.first] = region_value;
                 }
             }

             // Find opponent stone groups inside bot-territory regions (value == +1).
             // NOTE: opponent stone cells are assigned areaMap -1 (not +1) so we
             // cannot test areaMap[gy][gx] == 1.  Instead: BFS the full group first,
             // then check whether any stone in the group borders a bot-territory empty
             // cell (areaMap == +1).  This handles groups fully surrounded by other
             // opponent stones as well as groups with direct empty-cell neighbours.
             QSet<QPair<int,int>> group_visited;
             for (int gy = 0; gy < 19; ++gy) {
                 for (int gx = 0; gx < 19; ++gx) {
                     QPair<int,int> gstart(gx, gy);
                     if (group_visited.contains(gstart)) continue;
                     if (removed_cells.contains(gstart)) continue;
                     if (board->getStoneAt(gx, gy) != opponent_color) continue;

                     // BFS the full connected group first.
                     QQueue<QPair<int,int>> gq;
                     QList<QPair<int,int>> group;
                     gq.enqueue(gstart);
                     group_visited.insert(gstart);
                     while (!gq.isEmpty()) {
                         auto cur = gq.dequeue();
                         group.append(cur);
                         for (int d = 0; d < 4; ++d) {
                             int nx = cur.first  + dx[d];
                             int ny = cur.second + dy[d];
                             if (nx < 0 || nx >= 19 || ny < 0 || ny >= 19) continue;
                             QPair<int,int> nb(nx, ny);
                             if (group_visited.contains(nb)) continue;
                             if (removed_cells.contains(nb)) continue;
                             if (board->getStoneAt(nx, ny) == opponent_color) {
                                 group_visited.insert(nb);
                                 gq.enqueue(nb);
                             }
                         }
                     }

                     // Check whether any stone in the group borders a bot-territory
                     // empty cell.  If not, this group is not enclosed in bot territory.
                     bool in_bot_territory = false;
                     for (const auto &s : group) {
                         for (int d = 0; d < 4; ++d) {
                             int nx = s.first + dx[d], ny = s.second + dy[d];
                             if (nx < 0 || nx >= 19 || ny < 0 || ny >= 19) continue;
                             if (board->getStoneAt(nx, ny) == EMPTY
                                 && !removed_cells.contains(qMakePair(nx, ny))
                                 && areaMap[ny][nx] == 1) { in_bot_territory = true; break; }
                         }
                         if (in_bot_territory) break;
                     }
                     if (!in_bot_territory) continue;

                     // Phase 2: KataGo confirmation.
                     int confirmed = 0;
                     for (const auto &s : group) {
                         float own = ownership[s.second * 19 + s.first];
                         float bot_own = (bot_color == BLACK_STONE) ? own : -own;
                         if (bot_own > AREA_CONFIRM_STRONG || qAbs(own) < AREA_CONFIRM_AJI_MAX)
                             ++confirmed;
                     }
                     int pct = (group.size() > 0) ? (confirmed * 100 / group.size()) : 0;
                     bool dead = (pct >= 50);

                     output_console->append(QString("[BOT] [area-map] Pass %1: opponent group of %2 stones in bot territory: %3/%4 (%5%) KataGo-confirmed -> %6")
                         .arg(pass_num).arg(group.size()).arg(confirmed).arg(group.size()).arg(pct)
                         .arg(dead ? "DEAD" : "alive (KataGo disagrees -- skipping)"));

                     if (dead) {
                         for (const auto &s : group)
                             removed_cells.insert(s);
                         dead_group_seeds.append(group.first());
                         dead_group_members.append(group);
                         changed = true;
                     }
                 }
             }
             output_console->append(QString("[BOT] [area-map] Pass %1 complete -- %2 dead group(s) found so far")
                 .arg(pass_num).arg(dead_group_seeds.size()));
         }

     } else {
         // -----------------------------------------------------------------------
         // CLASSIC: KataGo ownership threshold + liberty enclosure BFS
         // -----------------------------------------------------------------------

         const float BOT_TERRITORY_THRESHOLD = 0.70f;
         const int   BOT_MAJORITY_PERCENT    = 65;

         QSet<QPair<int,int>> bot_territory;
         for (int y = 0; y < 19; ++y) {
             for (int x = 0; x < 19; ++x) {
                 float own = ownership[y * 19 + x];
                 bool bot_owns = (bot_color == BLACK_STONE) ? (own >=  BOT_TERRITORY_THRESHOLD)
                                                            : (own <= -BOT_TERRITORY_THRESHOLD);
                 if (bot_owns)
                     bot_territory.insert(qMakePair(x, y));
             }
         }
         output_console->append(QString("[BOT] Bot territory cells (threshold %1): %2")
             .arg(BOT_TERRITORY_THRESHOLD, 0, 'f', 2).arg(bot_territory.size()));

         QSet<QPair<int,int>> all_visited;
         for (int y = 0; y < 19; ++y) {
             for (int x = 0; x < 19; ++x) {
                 QPair<int,int> start(x, y);
                 if (all_visited.contains(start)) continue;
                 if (board->getStoneAt(x, y) != opponent_color) continue;

                 QQueue<QPair<int,int>> queue;
                 QList<QPair<int,int>> group;
                 queue.enqueue(start);
                 all_visited.insert(start);
                 while (!queue.isEmpty()) {
                     auto cur = queue.dequeue();
                     group.append(cur);
                     for (int d = 0; d < 4; ++d) {
                         int nx = cur.first  + dx[d];
                         int ny = cur.second + dy[d];
                         if (nx < 0 || nx >= 19 || ny < 0 || ny >= 19) continue;
                         QPair<int,int> nb(nx, ny);
                         if (all_visited.contains(nb)) continue;
                         if (board->getStoneAt(nx, ny) == opponent_color) {
                             all_visited.insert(nb);
                             queue.enqueue(nb);
                         }
                     }
                 }

                 int in_territory = 0;
                 for (const auto &s : group)
                     if (bot_territory.contains(s)) in_territory++;
                 int pct = (group.size() > 0) ? (in_territory * 100 / group.size()) : 0;
                 bool dead_by_ownership = (pct >= BOT_MAJORITY_PERCENT);

                 bool dead_by_enclosure = false;
                 {
                     QSet<QPair<int,int>> liberty_visited;
                     QQueue<QPair<int,int>> lib_queue;
                     bool escaped = false;
                     for (const auto &s : group) {
                         for (int d = 0; d < 4; ++d) {
                             int nx = s.first  + dx[d];
                             int ny = s.second + dy[d];
                             if (nx < 0 || nx >= 19 || ny < 0 || ny >= 19) continue;
                             QPair<int,int> nb(nx, ny);
                             if (liberty_visited.contains(nb)) continue;
                             if (board->getStoneAt(nx, ny) != EMPTY) continue;
                             liberty_visited.insert(nb);
                             lib_queue.enqueue(nb);
                         }
                     }
                     while (!lib_queue.isEmpty() && !escaped) {
                         auto cur = lib_queue.dequeue();
                         if (!bot_territory.contains(cur)) { escaped = true; break; }
                         for (int d = 0; d < 4; ++d) {
                             int nx = cur.first  + dx[d];
                             int ny = cur.second + dy[d];
                             if (nx < 0 || nx >= 19 || ny < 0 || ny >= 19) continue;
                             QPair<int,int> nb(nx, ny);
                             if (liberty_visited.contains(nb)) continue;
                             if (board->getStoneAt(nx, ny) != EMPTY) continue;
                             liberty_visited.insert(nb);
                             lib_queue.enqueue(nb);
                         }
                     }
                     dead_by_enclosure = !escaped;
                 }

                 bool dead = dead_by_ownership || dead_by_enclosure;
                 QString reason = dead_by_ownership ? (dead_by_enclosure ? "ownership+enclosed" : "ownership")
                                                    : (dead_by_enclosure ? "enclosed" : "");
                 output_console->append(QString("[BOT] Opponent group of %1 stones: %2/%3 (%4%) in bot territory -- %5%6")
                     .arg(group.size()).arg(in_territory).arg(group.size()).arg(pct)
                     .arg(dead ? "DEAD" : "alive")
                     .arg(dead ? QString(" (%1)").arg(reason) : QString()));

                 if (dead) {
                     dead_group_seeds.append(group.first());
                     dead_group_members.append(group);
                 }
             }
         }
     }

     // Build remove list, then fire after 1500ms — gives IGS time to fully open
     // the scoring window after CMD9 before we send remove commands.
     // Both sides act independently: bot sends its removes+done, opponent sends theirs.
     // IGS scores when both dones are received.
     bot_pending_removes.clear();
     bot_pending_remove_positions.clear();
     bot_pending_remove_groups.clear();
     for (int gi = 0; gi < dead_group_seeds.size(); ++gi) {
         const auto &pos = dead_group_seeds[gi];
         int x = pos.first;
         int y = pos.second;
         char letter = (x < 8) ? ('A' + x) : ('A' + x + 1);
         int igs_row = 19 - y;
         bot_pending_removes.append(QString("%1%2").arg(letter).arg(igs_row));
         bot_pending_remove_positions.append(pos);
         bot_pending_remove_groups.append(dead_group_members[gi]);
     }
     if (bot_pending_removes.isEmpty()) {
         output_console->append("[BOT] No dead groups found — sending done in 1500ms");
     } else {
         output_console->append(QString("[BOT] %1 dead group(s) identified — sending <coord> <game_id> + done in 1500ms")
             .arg(bot_pending_removes.size()));
     }

     // Snapshot ownership now (while the parameter is in scope) for use in progressive renders.
     bot_ownership_snapshot = ownership;

     QTimer::singleShot(1500, this, [this]() {
         if (!bot_scoring_pending || bot_done_sent) return;
         bot_scoring_pending = false;
         // Mark bot's own dead groups visually.
         // engine_board is only non-null in Eve/local-engine mode; in normal docked
         // bot games it is null, so fall back to shared_board_window (docked) or
         // the active board window (non-docked).
         BoardWindow *mark_board = engine_board;
         if (!mark_board && docked_pane_mode)
             mark_board = shared_board_window;
         if (!mark_board && !docked_pane_mode) {
             for (BoardWindow *bw : board_windows)
                 if (bw->getObservedGameId() == bot_game_id) { mark_board = bw; break; }
         }
         for (const auto &pos : bot_pending_remove_positions) {
             if (mark_board && !mark_board->getDeadStones().contains(pos))
                 mark_board->markStoneAsDead(pos.first, pos.second);
         }
         // Send removals one at a time. botSendNextRemoval() sends the first and
         // sets bot_done_sent=true + sends "done" once all groups are confirmed.
         bot_remove_index        = 0;
         bot_remove_retry_offset = 0;
         botSendNextRemoval();
     });
 }

 // Build territory map using local flood-fill scoring (same algorithm as the Score button)
 // and apply to engine_board. Called progressively: Pass 1 when bot sends done,
 // Pass 2 when opponent sends done. Dead stones are read from the board widget
 // (marked by markStoneAsDead()) and excluded from the GoBoard so flood-fill treats
 // them as empty intersections — identical to the in_edit_position_mode path.
 // Send the next pending removal to IGS. Called after 1500ms initial delay and
 // again after each CMD49 confirmation. When all groups are sent, sends "done".
 // If IGS rejects a coordinate with "You cannot remove a liberty" (the group was
 // already removed by the opponent), botHandleRemovalRejected() retries with the
 // next stone in the group, or skips the group if all stones are exhausted.
 void botSendNextRemoval() {
     if (bot_game_id == -1) return;
     if (bot_remove_index >= bot_pending_removes.size()) {
         // All groups handled — send done
         bot_done_sent = true;
         socket->write("done\n"); socket->flush();
         output_console->append("[BOT] >>> done");
         bot_pending_removes.clear();
         bot_pending_remove_positions.clear();
         bot_pending_remove_groups.clear();
         applyBotTerritoryOverlay("Pass 1 (bot done)");
         return;
     }
     const QString &coord = bot_pending_removes[bot_remove_index];
     socket->write((QString("%1 %2\n").arg(coord).arg(bot_game_id)).toUtf8());
     socket->flush();
     output_console->append(QString("[BOT] >>> %1 %2 (dead group toggle)").arg(coord).arg(bot_game_id));
 }

 // Called when IGS returns "You cannot remove a liberty" for the current removal.
 // This means the group's seed coordinate is empty (opponent already removed it).
 // Try the next stone in the group as an alternate seed. If all stones exhausted,
 // skip this group (opponent handled it) and move to the next.
 void botHandleRemovalRejected() {
     if (bot_remove_index >= bot_pending_removes.size()) return;

     bot_remove_retry_offset++;
     const QList<QPair<int,int>> &group = bot_pending_remove_groups[bot_remove_index];

     if (bot_remove_retry_offset < group.size()) {
         // Try the next stone in the group
         const auto &pos = group[bot_remove_retry_offset];
         int x = pos.first, y = pos.second;
         char letter = (x < 8) ? ('A' + x) : ('A' + x + 1);
         int igs_row = 19 - y;
         QString alt_coord = QString("%1%2").arg(letter).arg(igs_row);
         bot_pending_removes[bot_remove_index] = alt_coord;
         output_console->append(QString("[BOT] Removal rejected — retrying group %1 with alternate stone %2")
             .arg(bot_remove_index + 1).arg(alt_coord));
         botSendNextRemoval();
     } else {
         // All stones in this group exhausted — opponent already removed it; skip
         output_console->append(QString("[BOT] Group %1 already removed by opponent — skipping")
             .arg(bot_remove_index + 1));
         bot_remove_index++;
         bot_remove_retry_offset = 0;
         botSendNextRemoval();
     }
 }

 void applyBotTerritoryOverlay(const QString &pass_label) {
     BoardWindow *board = engine_board;
     if (!board && docked_pane_mode) board = shared_board_window;
     if (!board) return;

     QSet<QPair<int,int>> dead = board->getDeadStones();

     GoBoard score_board;
     for (int x = 0; x < 19; x++)
         for (int y = 0; y < 19; y++) {
             StoneColor sc = board->getStoneAt(x, y);
             if (sc != EMPTY && !dead.contains(qMakePair(x, y)))
                 score_board.placeStone(x, y, sc);
         }

     QMap<QPair<int,int>, StoneColor> territory_map;
     QSet<QPair<int,int>> dummy_dead, disputed;
     int b_score = 0, w_score = 0;
     ScoringMethod method = (settings->getScoringMethod() == "complex")
                            ? ScoringMethod::Complex : ScoringMethod::Simple;
     ScoreEngine::estimate(score_board, territory_map, dummy_dead, disputed, b_score, w_score, method);

     output_console->append(QString("[BOT] Territory overlay — %1 (local scoring): B:%2 W:%3 dead:%4")
         .arg(pass_label).arg(b_score).arg(w_score).arg(dead.size()));
     // Snapshot Pass-2 scores for CMD20 discrepancy check (both players have typed done).
     if (pass_label.startsWith("Pass 2")) {
         bot_local_white_score = w_score;
         bot_local_black_score = b_score;
     }
     bot_overlay_active = true;
     board->setScoringModeWithTerritory(territory_map);
 }

 void onBotEngineResigned() {
     if (bot_game_id == -1) return;
     output_console->append("[BOT] Engine resigned — sending resign to IGS");
     socket->write("resign\n");
     socket->flush();
 }

 void botEndGame() {
     if (bot_game_id == -1) return;
     output_console->append(QString("[BOT] Game %1 ended — engine kept warm for next game").arg(bot_game_id));

     // Thank the opponent — only if not already sent (resignation sends it early)
     if (!bot_opponent.isEmpty() && !bot_farewell_sent) {
         QString farewell = QString("Thank you for the game %1!").arg(bot_opponent);
         socket->write((QString("tell %1 %2\n").arg(bot_opponent, farewell)).toUtf8());
         socket->flush();
         output_console->append(QString("[BOT] >>> tell %1: %2").arg(bot_opponent, farewell));
         if (GameSlot *bslot = findSlot(bot_game_id)) {
             GameSlot::CommentEntry ce; ce.user = login_username; ce.text = farewell; ce.is_kibitz = false;
             bslot->comments.append(ce);
         }
         if (engine_board)
             engine_board->processComment(login_username, farewell, false);
     }

     bot_game_id          = -1;
     bot_engine_ready     = false;
     bot_scoring_pending  = false;
     bot_done_sent        = false;
     bot_cmd20_received   = false;
     bot_pass2_rendered   = false;
     bot_overlay_active   = false;
     bot_farewell_sent    = false;
     bot_time_forfeit_loser.clear();
     bot_pending_removes.clear();
     bot_pending_remove_positions.clear();
     bot_pending_remove_groups.clear();
     bot_remove_index        = 0;
     bot_remove_retry_offset = 0;
     bot_cached_ownership.clear();
     bot_ownership_snapshot.clear();
     bot_local_white_score = -1;
     bot_local_black_score = -1;
     bot_opponent         = "";
     bot_handicap         = 0;
     bot_time_remaining   = 0;
     bot_stones_remaining = 0;
     // Cancel any in-flight ownership request so its response doesn't fire
     // onBotOwnershipReady after bot_game_id has been cleared to -1.
     if (KataGoEngine *engine = engine_manager->currentEngine()) {
         engine->cancelOwnership();
         engine->enqueueRaw("clear_board");
     }
     engine_board = nullptr;

     // Reopen to new challenges now that the game is finished
     socket->write("toggle open true\n");
     socket->flush();
 }

 // Convert board x,y coordinates to a GTP vertex string.
 // x/y are 0-based from top-left; GTP skips 'I'; pass = x==-1.
 static QString coordsToGtp(int x, int y) {
     if (x < 0 || y < 0) return "pass";
     char col = (x < 8) ? ('A' + x) : ('A' + x + 1); // skip 'I'
     int row = 19 - y;
     return QString("%1%2").arg(col).arg(row);
 }

 // Send a random canned reply to a tell received during bot mode
 void botReplyToTell(const QString &sender) {
     if (!bot_mode_active || bot_tell_responses.isEmpty()) return;
     int idx = QRandomGenerator::global()->bounded(bot_tell_responses.size());
     QString reply = QString("tell %1 %2").arg(sender).arg(bot_tell_responses.at(idx));
     socket->write((reply + "\n").toUtf8());
     socket->flush();
     output_console->append(QString("[BOT] Auto-replied to tell from %1").arg(sender));
 }

 void resignGame(int game_id) {
 qDebug() << "Main window resignGame called for game" << game_id;

 if (socket->state() != QTcpSocket::ConnectedState) {
 if (!this->suppress_server_console) output_console->append(">>> ERROR: Not connected to IGS - cannot resign");
 return;
 }

 // Send resign command to IGS
 QString resign_cmd = "resign";
 socket->write((resign_cmd + "\n").toUtf8());
 if (!this->suppress_server_console) output_console->append(QString(">>> SENT: %1 (resigned game %2)").arg(resign_cmd).arg(game_id));
 qDebug() << "Sent resign command to IGS for game" << game_id;
 }

 void sendComment(int game_id, const QString &message) {
 qDebug() << "Main window sendComment called with game_id:" << game_id << "message:" << message;

 if (socket->state() != QTcpSocket::ConnectedState) {
 if (!this->suppress_server_console) output_console->append(">>> ERROR: Not connected to IGS - cannot send comment");
 qDebug() << "ERROR: Socket not connected";
 return;
 }

 // IGS kibitz command format: "kibitz <game_number> <message>"
 QString comment_cmd = QString("kibitz %1 %2").arg(game_id).arg(message);
 qDebug() << "Sending IGS command:" << comment_cmd;
 socket->write((comment_cmd + "\n").toUtf8());
 if (!this->suppress_server_console) output_console->append(QString(">>> SENT: %1").arg(comment_cmd));
 }
 
 void sendSay(int game_id, const QString &message) {
 qDebug() << "Main window sendSay called with game_id:" << game_id << "message:" << message;
 
 if (socket->state() != QTcpSocket::ConnectedState) {
 if (!this->suppress_server_console) output_console->append(">>> ERROR: Not connected to IGS - cannot send say");
 qDebug() << "ERROR: Socket not connected";
 return;
 }
 
 // IGS say command for private player communication
 QString say_cmd = QString("say %1").arg(message);
 qDebug() << "Sending IGS say command:" << say_cmd;
 socket->write((say_cmd + "\n").toUtf8());
 if (!this->suppress_server_console) output_console->append(QString(">>> SENT: %1").arg(say_cmd));
 if (!this->suppress_server_console) output_console->append(">>> NOTE: Using 'say' for private player communication");
 }

 void sendTell(const QString &player, const QString &message) {
 qDebug() << "Main window sendTell called with player:" << player << "message:" << message;

 if (socket->state() != QTcpSocket::ConnectedState) {
 if (!this->suppress_server_console) output_console->append(">>> ERROR: Not connected to IGS - cannot send tell");
 qDebug() << "ERROR: Socket not connected";
 return;
 }

 // IGS tell command for player-to-player private messaging (not during games)
 // Format: "tell playername message"
 // Note: "say playername message" only works during active games
 // The recipient will receive this as Command 24
 QString tell_cmd = QString("tell %1 %2").arg(player, message);
 qDebug() << "Sending IGS tell command:" << tell_cmd;
 socket->write((tell_cmd + "\n").toUtf8());
 if (!this->suppress_server_console) output_console->append(QString(">>> [MSG] TELL SENT to %1: %2").arg(player, message));
 qDebug() << "Tell command written to socket:" << tell_cmd;
 }

 void requestObservers(int game_id) {
 qDebug() << "Main window requestObservers called for game_id:" << game_id;

 if (socket->state() != QTcpSocket::ConnectedState) {
 if (!this->suppress_server_console) output_console->append(">>> ERROR: Not connected to IGS - cannot request observers");
 qDebug() << "ERROR: Socket not connected";
 return;
 }

 // FIXED: Only send "all <game_id>" command to refresh observer list
 // DO NOT re-send observe/games/moves as that clobbers the existing board state!
 QString all_cmd = QString("all %1").arg(game_id);

 qDebug() << "Sending IGS 'all' command to refresh observers for game" << game_id;
 socket->write((all_cmd + "\n").toUtf8());

 if (!this->suppress_server_console) output_console->append(QString(">>> SENT: %1 (refreshing observer list)")
 .arg(all_cmd));
 
 // Set flag to capture and parse observer responses
 waiting_for_observer_response = true;
 observer_request_game_id = game_id;
 
 // DIAGNOSTIC: Force add test observers to ALL board windows
 if (!this->suppress_server_console) output_console->append(QString(">>> DIAGNOSTIC: Found %1 board windows").arg(board_windows.size()));
 for (BoardWindow* board : board_windows) {
 if (!this->suppress_server_console) output_console->append(QString(">>> DIAGNOSTIC: Board observing=%1 gameId=%2 target=%3")
 .arg(board->isObserving()).arg(board->getObservedGameId()).arg(game_id));
 if (board->isObserving()) {
 board->clearObservers();
 board->addObserver("TestUser1", "5k*");
 board->addObserver("TestUser2", "2d");
 board->addObserver("DiagnosticTest", "1p");
 if (!this->suppress_server_console) output_console->append(">>> DIAGNOSTIC: Added 3 test observers to board window");
 }
 }
 
 // Start a timer to stop after 5 seconds
 QTimer::singleShot(5000, this, [this]() {
 waiting_for_observer_response = false;
 if (!this->suppress_server_console) output_console->append(">>> OBSERVER DEBUG: Finished parsing WHO response");
 });
 }
 
 
 void saveBoardGame(int game_id) {
 if (!this->suppress_server_console) output_console->append(QString(">>> TODO: Save game %1 functionality not yet implemented").arg(game_id));
 }
 
 void sendMove(int game_id, int x, int y) {
 qDebug() << "Main window sendMove called - game:" << game_id << "coordinates:" << x << "," << y;
 
 if (socket->state() != QTcpSocket::ConnectedState) {
 if (!this->suppress_server_console) output_console->append(">>> ERROR: Not connected to IGS - cannot send move");
 return;
 }
 
 // Convert coordinates to IGS format (A-T for columns, 1-19 for rows)
 // x=0,y=0 is top-left, IGS uses A1 = bottom-left
 // Note: IGS skips letter 'I' in coordinate system
 char col;
 if (x < 8) {
 col = 'A' + x; // A-H (x=0-7)
 } else {
 col = 'A' + x + 1; // J-T (x=8-18, skip 'I')
 }
 int row = 19 - y; // y coordinate becomes row number (flipped)
 
 QString igs_move = QString("%1%2").arg(col).arg(row);
 
 qDebug() << "Converted coordinates" << x << "," << y << "to IGS format:" << igs_move;
 
 // Send move command to IGS
 socket->write((igs_move + "\n").toUtf8());
 socket->flush();
 if (!this->suppress_server_console) output_console->append(QString(">>> SENT MOVE: %1 (for game %2)").arg(igs_move).arg(game_id));

 qDebug() << "Move sent to IGS server:" << igs_move;
 }
 
 void updateStatus(const QString& status) {
 QLabel *status_label = findChild<QLabel*>("status_label");
 if (status_label) {
 status_label->setText("Status: " + status);
 }
 }
 
 void updateClocks() {
     QDateTime now_local = QDateTime::currentDateTime();
     QDateTime now_utc   = QDateTime::currentDateTimeUtc();
     if (clock_local)
         clock_local->setText("Local:  " + now_local.toString("ddd MMM dd hh:mm:ss yyyy"));
     if (clock_gmt)
         clock_gmt->setText("GMT:    " + now_utc.toString("ddd MMM dd hh:mm:ss yyyy"));
     if (clock_server) {
         if (server_time_set) {
             qint64 elapsed_secs = server_time_received.secsTo(QDateTime::currentDateTimeUtc());
             QDateTime srv = server_time_value.addSecs(elapsed_secs);
             clock_server->setText("Server: " + srv.toString("ddd MMM dd hh:mm:ss yyyy"));
         }
         // else stays "Server: -- (waiting)" until IGS responds to "time"
     }
 }

 void handleHeartbeat() {
 updateClocks();

 if (!hold_the_line || !connected_to_igs) {
 return;
 }

 heartbeat_counter--;

 if (heartbeat_counter <= 0) {
     if (socket->state() == QTcpSocket::ConnectedState) {
         socket->write("ayt\n");
         if (!this->suppress_server_console) output_console->append(">>> SENT: ayt (heartbeat keep-alive)");
     }
     // During an active bot game use 60s interval — silent drops must be detected
     // quickly so the game isn't lost on time. Otherwise use 15 minutes (899s).
     heartbeat_counter = (bot_mode_active && bot_game_id != -1) ? 59 : 899;
 }
 }
 
 void resetHeartbeatCounter() {
     heartbeat_counter = (bot_mode_active && bot_game_id != -1) ? 59 : 899;
 }
 
 QString findPlayerRank(const QString& player_name) {
 if (!players_window) return "?";
 return players_window->getRankForPlayer(player_name);
 }
};

// Removed moc include to fix build issues
// #include "board_window.moc" // Removed to avoid conflicts with separate board_window.cpp

static void sigsegv_handler(int sig) {
    const char *msg = "\n*** SIGSEGV — stack backtrace ***\n";
    write(STDERR_FILENO, msg, strlen(msg));
    void *bt[64];
    int n = backtrace(bt, 64);
    backtrace_symbols_fd(bt, n, STDERR_FILENO);
    signal(sig, SIG_DFL);
    raise(sig);
}

int main(int argc, char *argv[]) {
 signal(SIGSEGV, sigsegv_handler);
 QApplication app(argc, argv);

 // VERSION BANNER - confirms correct binary is running
 qDebug() << "==========================================================================";
 qDebug() << ">>> XGOSPEL2 VERSION:" << XGOSPEL_VERSION << XGOSPEL_BUILD_DATE;
 qDebug() << ">>> - Fix: dead stone markers survive CMD22 board replacement (color cache)";
 qDebug() << "==========================================================================";

 // Initialize global settings
 settings = new Settings();
 settings->load();

 FixedXGospelWindow window;
 window.show();

 return app.exec();
}

#include "xgospel2_fixed.moc"
