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
#include <QtGui/QIcon>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QFileDialog>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QStyledItemDelegate>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtGui/QCursor>
#include <QtGui/QCloseEvent>
#include <QtCore/QSortFilterProxyModel>
#include <QtCore/QTimer>
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

// Version information - update these with each release
const QString XGOSPEL_VERSION = "v50_R19-FILTER-PREFERENCES";
const QString XGOSPEL_BUILD_DATE = "2026-04-22";

class FixedRankSortProxyModel : public QSortFilterProxyModel {
public:
 FixedRankSortProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent) {}
 
protected:
 bool lessThan(const QModelIndex &left, const QModelIndex &right) const override {
 if (left.column() == 2) { // Rank column
 QString leftKey = sourceModel()->data(left, Qt::UserRole).toString();
 QString rightKey = sourceModel()->data(right, Qt::UserRole).toString();
 if (!leftKey.isEmpty() && !rightKey.isEmpty()) {
 return leftKey < rightKey;
 }
 }
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

 // Player name and rank - centered header
 player_name_label = new QLabel(QString("<b>%1</b> [<b>%2</b>]").arg(player_name, player_rank));
 player_name_label->setAlignment(Qt::AlignCenter);
 player_name_label->setStyleSheet("font-size: 11px; margin-bottom: 2px;");
 header_container->addWidget(player_name_label);

 // Compact 2-column grid for player stats
 QGridLayout *grid = new QGridLayout();
 grid->setSpacing(1);
 grid->setVerticalSpacing(0);
 grid->setHorizontalSpacing(12);
 grid->setMargin(0);

 int row = 0;

 // Row 0: Wins | Losses
 QLabel *wins_lbl = new QLabel(QString("<b>Wins:</b> %1").arg(wins == 0 && losses == 0 ? "--" : QString::number(wins)));
 wins_lbl->setStyleSheet("font-size: 9px; ");
 grid->addWidget(wins_lbl, row, 0, Qt::AlignLeft);

 QLabel *losses_lbl = new QLabel(QString("<b>Losses:</b> %1").arg(wins == 0 && losses == 0 ? "--" : QString::number(losses)));
 losses_lbl->setStyleSheet("font-size: 9px; ");
 grid->addWidget(losses_lbl, row, 1, Qt::AlignLeft);
 row++;

 // Row 1: Rated Games | Country (if available)
 if (!rated_record.isEmpty() && rated_record != "0") {
 rated_label = new QLabel(QString("<b>Rated:</b> %1").arg(rated_record));
 rated_label->setStyleSheet("font-size: 9px; ");
 grid->addWidget(rated_label, row, 0, Qt::AlignLeft);
 } else {
 rated_label = nullptr;
 }

 if (!country.isEmpty()) {
 QLabel *country_lbl = new QLabel(QString("<b>Country:</b> %1").arg(country));
 country_lbl->setStyleSheet("font-size: 9px; ");
 grid->addWidget(country_lbl, row, 1, Qt::AlignLeft);
 row++;
 } else if (rated_label) {
 row++;
 }

 // Row 2: Info | Idle - Creating 2x4 grid array
 qDebug() << "[STATS-DISPLAY] Info field value:'" << info << "' isEmpty:" << info.isEmpty();
 if (!info.isEmpty() && info != "<None>") {
 qDebug() << "[STATS-DISPLAY] Creating Info label with text:" << info;
 // HTML-escape the info text to handle quotes and special characters
 QString escaped_info = QString(info).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;");
 QLabel *info_lbl = new QLabel(QString("<b>Info:</b> %1").arg(escaped_info));
 info_lbl->setStyleSheet("font-size: 9px; ");
 grid->addWidget(info_lbl, row, 0, Qt::AlignLeft);
 } else {
 qDebug() << "[STATS-DISPLAY] Skipping Info label (empty or <None>)";
 }

 if (!idle_time.isEmpty()) {
 QLabel *idle_lbl = new QLabel(QString("<b>Idle:</b> %1").arg(idle_time));
 idle_lbl->setStyleSheet("font-size: 9px; ");
 grid->addWidget(idle_lbl, row, 1, Qt::AlignLeft);
 }
 if ((!info.isEmpty() && info != "<None>") || !idle_time.isEmpty()) {
 row++;
 }

 // Row 3: Playing | Observing
 if (!playing.isEmpty() && playing != "0" && playing != "--") {
 QLabel *playing_lbl = new QLabel(QString("<b>Playing:</b> %1").arg(playing));
 playing_lbl->setStyleSheet("font-size: 9px; ");
 grid->addWidget(playing_lbl, row, 0, Qt::AlignLeft);
 }

 if (!observing.isEmpty() && observing != "0" && observing != "--") {
 QLabel *obs_lbl = new QLabel(QString("<b>Observing:</b> %1").arg(observing));
 obs_lbl->setStyleSheet("font-size: 9px; ");
 grid->addWidget(obs_lbl, row, 1, Qt::AlignLeft);
 }
 if ((!playing.isEmpty() && playing != "0" && playing != "--") ||
 (!observing.isEmpty() && observing != "0" && observing != "--")) {
 row++;
 }

 // Row 4: Last log (for offline players) - spans both columns
 if (!last_log.isEmpty()) {
 QLabel *lastlog_lbl = new QLabel(QString("<b>Last log:</b> %1").arg(last_log));
 lastlog_lbl->setStyleSheet("font-size: 9px; ");
 lastlog_lbl->setMinimumWidth(400); // Ensure enough space for full timestamp
 grid->addWidget(lastlog_lbl, row, 0, 1, 2, Qt::AlignLeft); // Span 2 columns
 row++;
 }

 header_container->addLayout(grid);

 // Match Preferences (full width below grid)
 if (!match_prefs.isEmpty()) {
 match_prefs_label = new QLabel(QString("<b>Match Prefs:</b> %1").arg(match_prefs));
 match_prefs_label->setStyleSheet("font-size: 8px; margin-top: 2px;");
 match_prefs_label->setWordWrap(true);
 header_container->addWidget(match_prefs_label);
 } else {
 match_prefs_label = nullptr;
 }

 main_layout->addWidget(header_frame);

 // Message/Tell section
 QGroupBox *tell_group = new QGroupBox("Send Message");
 QVBoxLayout *tell_layout = new QVBoxLayout(tell_group);

 message_display = new QTextEdit;
 message_display->setReadOnly(true);
 message_display->setMaximumHeight(120);
 message_display->setStyleSheet(
 "QTextEdit {" " " " border: 1px solid #ccc;" " font-family: monospace;" " font-size: 10px;" "}"
 );
 tell_layout->addWidget(message_display);

 QHBoxLayout *input_layout = new QHBoxLayout;
 message_input = new QLineEdit;
 message_input->setPlaceholderText("Type message to send...");
 message_input->setMinimumHeight(30);
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

 // With the new compact grid layout, rebuild the dialog to show updated stats
 QWidget *central = this;
 QLayout *oldLayout = central->layout();
 if (oldLayout) {
 QLayoutItem *item;
 while ((item = oldLayout->takeAt(0)) != nullptr) {
 delete item->widget();
 delete item;
 }
 delete oldLayout;
 }

 // Re-parse toggle states from stat field before rebuilding UI
 parseToggleStatesFromStat();
 setupUI();

 qDebug() << "[STATS-UPDATE] Rebuilt dialog - Rank:" << player_rank << "Country:" << country << "Last log:" << last_log
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
 // Only do this if players_model has data (i.e., player list has been loaded)
 if (!dialogExists && players_model && players_model->rowCount() > 0) {
 qDebug() << "[NOTIFY] Dialog doesn't exist for sender:" << sender << "- auto-opening";
 openStatsDialogForPlayer(sender);
 // After opening, find the newly created dialog
 for (PlayerStatsDialog* dialog : open_dialogs) {
 if (dialog && dialog->getPlayerName().compare(sender, Qt::CaseInsensitive) == 0) {
 targetDialog = dialog;
 break;
 }
 }
 } else if (!dialogExists && (!players_model || players_model->rowCount() == 0)) {
 qDebug() << "[WAIT] Players list not loaded yet, deferring auto-dialog for:" << sender;
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

 // Force IMMEDIATE synchronous repaint (not deferred update())
 // Use repaint() instead of update() to force immediate painting even when window lacks focus
 if (games_table && games_table->viewport()) {
 qDebug() << "[updateObservedGames] Forcing viewport repaint";
 games_table->viewport()->repaint();
 qDebug() << "[updateObservedGames] Viewport repaint complete";
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
 komi_spin->setRange(0.0, 9.5);
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

class FixedXGospelWindow : public QMainWindow {
 Q_OBJECT

private:
 QTextEdit *output_console;
 QLineEdit *command_text;
 QTcpSocket *socket;
 FixedPlayersWindow *players_window;
 FixedGamesWindow *games_window;
    MatchDialog *pending_match_dialog;  // Bug 34: Track pending match dialog for server command
 QList<BoardWindow*> board_windows;
 BoardWindow *most_recently_observed_board; // Track most recent observation for teaching title assignment
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
 
 // Observer list parsing state
 bool waiting_for_observer_response;
 int observer_request_game_id;
 bool parsing_observers;
 int observer_game_id;
 
 // Manual observe command state
 bool pending_manual_observe;
 int pending_observe_game_id;
 
 // IGS territory data state (following q5Go protocol)
 bool receiving_territory_data;
 int territory_data_column;
 BoardWindow* territory_board;
 QString territory_player_name; // Track first player name from Command 22 to match board
 
 // TCP buffering variables (from q5Go)
 char *saved_data;
 int len_saved_data;

 // Connection menu for dynamic account management
 QMenu *connection_menu;

public:
 FixedXGospelWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
 setWindowTitle("XGospel Console");
 setMinimumSize(700, 500);
 players_window = nullptr;
 games_window = nullptr;
        pending_match_dialog = nullptr;  // Bug 34: Initialize pending match dialog tracker
 move_parser = new IGSMoveParser();
 current_game_context = -1; // No game context initially
 next_game_is_free = false; // Default to rated until we see match setup
 next_game_komi = 0.0; // Default to server default komi
 my_account_name = ""; // Will be set when we connect
 my_account_rank = "NR"; // Default to unranked
 most_recently_observed_board = nullptr; // Initialize teaching title tracking
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
 
 // Initialize observer parsing state
 parsing_observers = false;
 observer_game_id = -1;
 waiting_for_observer_response = false;
 observer_request_game_id = -1;
 
 // Initialize manual observe state
 pending_manual_observe = false;
 pending_observe_game_id = -1;
 
 // Initialize IGS territory data state
 receiving_territory_data = false;
 territory_data_column = 0;
 territory_board = nullptr;
 territory_player_name = "";
 
 // Initialize TCP buffering
 saved_data = nullptr;
 len_saved_data = 0;
 
 socket = new QTcpSocket(this);
 
 connect(socket, &QTcpSocket::connected, this, &FixedXGospelWindow::onConnected);
 connect(socket, &QTcpSocket::disconnected, this, &FixedXGospelWindow::onDisconnected);
 connect(socket, &QTcpSocket::readyRead, this, &FixedXGospelWindow::onDataReceived);
 
 // Connect heartbeat timer - 1 second intervals like q5Go
 connect(heartbeat_timer, &QTimer::timeout, this, &FixedXGospelWindow::handleHeartbeat);
 heartbeat_timer->setInterval(1000); // 1 second
 
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

 connection_menu = menu->addMenu("Connection");
 populateConnectionMenu();

 QMenu *windows_menu = menu->addMenu("Windows");
 QAction *players_action = windows_menu->addAction("Show Players");
 connect(players_action, &QAction::triggered, this, &FixedXGospelWindow::showPlayersWindow);

 QAction *games_action = windows_menu->addAction("Show Games");
 connect(games_action, &QAction::triggered, this, &FixedXGospelWindow::showGamesWindow);

 // Console menu with suppression options
 QMenu *console_menu = menu->addMenu("Console");

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
 if (socket->state() == QTcpSocket::ConnectedState) {
 socket->disconnectFromHost();
 }
 }
 
 void onConnected() {
 if (!this->suppress_server_console) output_console->append(">>> CONNECTED to IGS successfully!");
 updateStatus(QString("Connected - logging in as %1").arg(login_username));
 connected_to_igs = true;

 // Set account name for match dialogs
 my_account_name = login_username;

 // Start heartbeat system
 hold_the_line = true;
 heartbeat_counter = 899; // Reset to 15 minutes
 heartbeat_timer->start();
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
 if (!this->suppress_server_console) output_console->append(">>> DISCONNECTED from IGS");
 updateStatus("Not connected");
 connected_to_igs = false;
 auto_launched_windows = false;
 waiting_for_players = false;
 waiting_for_games = false;
 
 // Stop heartbeat system
 hold_the_line = false;
 heartbeat_timer->stop();
 if (!this->suppress_server_console) output_console->append(">>> Heartbeat system stopped");
 }
 
 void onDataReceived() {
 // Use q5Go's buffering approach to prevent TCP packet fragmentation issues
 for (;;) {
 int available = socket->bytesAvailable();
 if (available == 0) {
 return;
 }
 
 // If we can't read a complete line, buffer the incomplete data
 if (!socket->canReadLine()) {
 char *new_data = new char[available + len_saved_data];
 if (saved_data) {
 memcpy(new_data, saved_data, len_saved_data);
 delete[] saved_data;
 }
 int nread = socket->read(new_data + len_saved_data, available);
 saved_data = new_data;
 len_saved_data += nread;
 return; // Wait for more data
 }
 
 // Process complete lines
 while (socket->canReadLine()) {
 int size = socket->bytesAvailable() + 1;
 char *c = new char[size + len_saved_data];
 if (len_saved_data) {
 memcpy(c, saved_data, len_saved_data);
 }
 socket->readLine(c + len_saved_data, size);
 QString line = QString::fromUtf8(c);
 delete[] c;
 
 // Clean up saved data after processing
 len_saved_data = 0;
 if (saved_data) {
 delete[] saved_data;
 saved_data = nullptr;
 }
 
 // Remove trailing newline characters
 line = line.trimmed();
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
 if (line.contains("Adding game to observation list") && pending_manual_observe) {
 if (!this->suppress_server_console) output_console->append(QString(">>> DEBUG: IGS confirmed observation, waiting for game info..."));
 }
 
 // IGS Command 9 Observer Parsing (based on q5Go protocol)
 // Pattern: "9 Observing game 89 (player1 vs. player2) :"
 if (line.startsWith("9 Observing game ") && line.contains(" (") && line.contains(") :")) {
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
 // Update existing board window with correct player names
 for (BoardWindow* board : board_windows) {
 if (board->isObserving() && board->getObservedGameId() == game_id) {
 board->updatePlayerNames(white_player, black_player);
 if (!this->suppress_server_console) output_console->append(QString(">>> UPDATED PLAYER NAMES for game %1").arg(game_id));
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
 for (BoardWindow* board : board_windows) {
 if (board->isObserving() && board->getObservedGameId() == extracted_game_id) {
 board->clearObservers();
 }
 }
 }
 }
 // Parse observer entries when we're in observer parsing mode
 else if (parsing_observers && line.startsWith("9 ") && !line.contains("Found") && !line.contains("Observing")) {
 // Remove "9 " prefix and parse alternating name/rank pairs
 QString observer_line = line.mid(2).trimmed(); // Remove "9 " prefix
 QStringList words = observer_line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
 
 if (!this->suppress_server_console) output_console->append(QString(">>> PARSING OBSERVER LINE: %1 (%2 words)").arg(observer_line).arg(words.size()));
 
 // Parse alternating name/rank pairs
 for (int i = 0; i < words.size(); i += 2) {
 if (i + 1 < words.size()) {
 QString observer_name = words[i];
 QString observer_rank = words[i + 1];
 
 if (!this->suppress_server_console) output_console->append(QString(">>> FOUND OBSERVER: %1 [%2]").arg(observer_name, observer_rank));
 
 // Add to board windows
 for (BoardWindow* board : board_windows) {
 if (board->isObserving() && board->getObservedGameId() == observer_game_id) {
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
 
 // Try alternate parsing approach - look for any line with rank patterns during observer requests
 if (waiting_for_observer_response && !parsing_observers) {
 // Look for lines containing rank patterns like "username 5k" or "username[2d]"
 QRegExp rank_pattern("([a-zA-Z0-9_]+)\\s*\\[?([0-9]+[kdp]\\*?)\\]?");
 if (rank_pattern.indexIn(line) != -1) {
 QString observer_name = rank_pattern.cap(1);
 QString observer_rank = rank_pattern.cap(2);
 if (!observer_name.isEmpty() && !observer_rank.isEmpty()) {
 if (!this->suppress_server_console) output_console->append(QString(">>> ALTERNATE PARSING - FOUND OBSERVER: %1 [%2]").arg(observer_name, observer_rank));
 
 // Add to board windows
 for (BoardWindow* board : board_windows) {
 if (board->isObserving() && board->getObservedGameId() == observer_request_game_id) {
 board->addObserver(observer_name, observer_rank);
 }
 }
 }
 }
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

 // First try to send to a playing board (if this is a say during a game)
 int boards_checked = 0;
 int boards_delivered = 0;
 for (BoardWindow* board : board_windows) {
 boards_checked++;
 if (board->isPlaying()) {
 // Check if sender is one of the players in this game
 QString white = board->getWhitePlayer();
 QString black = board->getBlackPlayer();

 if (!this->suppress_server_console) output_console->append(QString(">>> DEBUG: Checking board %1: white=%2 black=%3 sender=%4")
 .arg(boards_checked).arg(white, black, sender));

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

 // If not delivered to a game board, treat it as a tell/private message
 // and broadcast to Player Stats Dialogs
 if (boards_delivered == 0) {
 if (!this->suppress_server_console) output_console->append(QString(">>> [TELL] TELL MESSAGE from %1 (no matching game board)").arg(sender));
 qDebug() << "[DEBUG] Checking players_window for broadcast - players_window=" << players_window;
 if (players_window) {
 qDebug() << "[BROADCAST] Broadcasting TELL message to player dialogs - sender:" << sender << "message:" << message;
 players_window->broadcastIncomingTell(sender, message);
 } else {
 qDebug() << "[ERROR] ERROR: players_window is NULL - cannot broadcast tell!";
 }
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

 // Broadcast tell to all open player stats dialogs
 qDebug() << "[TELL] Tell message received from" << sender << ":" << message;
 if (players_window) {
 qDebug() << "[BROADCAST] Calling broadcastIncomingTell on players_window";
 players_window->broadcastIncomingTell(sender, message);
 } else {
 qDebug() << "[ERROR] ERROR: players_window is NULL!";
 }
 }
 }

 // Show all data in high contrast console
 // Determine if this line contains games data (Command 7), moves data (Command 15), players data (Commands 27/42), or stats data (Command 9)
	bool is_games_data = line.startsWith("7 ");
	bool is_moves_data = line.startsWith("15 ");
	bool is_players_data = line.startsWith("27 ") || line.startsWith("42 ");
	bool is_stats_data = line.startsWith("9 ");

	// Check if this is an important message that should never be suppressed
	bool is_important = line.startsWith("ayt") ||  // Keep-alive heartbeat
	                    line.contains("!!*Pandanet*!!") ||  // Server announcements
	                    line.contains("Handicap and komi");  // Important game status messages

	// Only output to console if not suppressed
	// - Games data suppressed by suppress_games_console
	// - Moves data suppressed by suppress_moves_console
	// - Players data suppressed by suppress_server_console (debug output)
	// - Stats data (Command 9) suppressed by suppress_server_console (debug output)
	// - Raw server output (command responses, help, etc.) is NEVER suppressed
	// - Important messages are NEVER suppressed
	// Note: suppress_server_console now affects debug messages (>>>), players list data, AND stats data
	bool suppress_this_line = !is_important &&
	                          ((is_games_data && suppress_games_console) ||
	                           (is_moves_data && suppress_moves_console) ||
	                           (is_players_data && suppress_server_console) ||
	                           (is_stats_data && suppress_server_console));

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

 games_window->addGameFromRawLine(line);
 game_count++;

 if (game_count % 5 == 0) {
 if (!this->suppress_server_console) output_console->append(QString(">>> Parsed %1 games so far...").arg(game_count));
 }
 }

 // Check for end of games list
 if ((line.contains("**") && !line.contains("[")) ||
 line.contains("games listed") ||
 (line.contains("1 1") && line.length() < 10) ||
 line.startsWith("Type 'games'") ||
 line.contains("end of list") ||
 line.startsWith("##") ||
 (line.trimmed() == "1 1")) {

 waiting_for_games = false;
 games_window->resizeColumns();
 // Update observed games highlighting after refresh (xgospel1 pattern)
 games_window->updateObservedGames(observed_game_ids);

 // Add delayed secondary refresh to fix occasional rendering issues
 // 500ms delay allows better server-client handshaking timing
 QTimer::singleShot(500, games_window, [this]() {
 if (games_window && games_window->isVisible()) {
 games_window->updateObservedGames(observed_game_ids);
 }
 });

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
 
 // Update current game context for move routing
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
 connect(board, &BoardWindow::commentRequested, this, &FixedXGospelWindow::sendComment);
 connect(board, &BoardWindow::sayRequested, this, &FixedXGospelWindow::sendSay);
 connect(board, &BoardWindow::observersRequested, this, &FixedXGospelWindow::requestObservers);
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
 if (game_komi_map.contains(game_id)) {
 // Command 7 data available - use authoritative type from game list
 double stored_komi = game_komi_map[game_id];
 QString stored_type = game_type_map.value(game_id, game_type);
 board->updateGameSetup(0, stored_komi, stored_type);
 board->updateGameDetails(stored_type, 0);
 qDebug() << "[GAME] KOMI APPLIED: Game" << game_id << "komi:" << stored_komi << "type:" << stored_type << "(from Command 7)";
 } else {
 // No Command 7 yet - use detected game type from match setup or Command 15
 QString detected_type = next_game_is_free ? "Free" : (game_type_map.contains(game_id) ? game_type_map[game_id] : game_type);
 board->updateGameSetup(0, 0.5, detected_type);
 board->updateGameDetails(detected_type, 0);
 qDebug() << "[GAME] DEFAULT SETUP: Game" << game_id << "komi: 0.5 type:" << detected_type << "(will update from Command 7)";

 // Reset for next game
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
 }
 }
 
 // Find the board window observing this game and update capture counts, game type, and byoyomi
 for (BoardWindow* board : board_windows) {
 if (board->getObservedGameId() == game_id) {
 board->updateCaptures(white_captures, black_captures);
 
 // Use stored game type from Command 7 if available
 QString correct_game_type = game_type_map.value(game_id, game_type);
 qDebug() << "[MENU] GAME TYPE UPDATE: Game" << game_id << "using type" << correct_game_type << "(15 format was:" << game_type << ")";
 board->updateGameDetails(correct_game_type, 0);
 board->updateByoyomi(white_time, black_time, white_byo_moves, black_byo_moves);
 qDebug() << "DEBUG: Updated game info for game" << game_id 
 << "- White:" << white_captures << "captures, time:" << white_time << "byo:" << white_byo_moves
 << "Black:" << black_captures << "captures, time:" << black_time << "byo:" << black_byo_moves
 << "Type:" << game_type;
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

 if (current_game_context != -1) {
 qDebug() << "[BOARD-LOOKUP] Looking for board for game" << current_game_context << "- have" << board_windows.size() << "boards";
 // q5Go pattern: Find the board for the current game context
 for (BoardWindow* board : board_windows) {
 int board_game_id = board->getObservedGameId();
 qDebug() << "[BOARD-LOOKUP] Checking board with game_id" << board_game_id;
 if (board_game_id == current_game_context) {
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
 connect(target_board, &BoardWindow::observersRequested, this, &FixedXGospelWindow::requestObservers);
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

 // q5Go pattern: Send "moves N" command on first move for this game
 if (target_board && !games_with_moves_requested.contains(current_game_context)) {
 games_with_moves_requested.insert(current_game_context);

 // CRITICAL FIX: Clear the board BEFORE sending moves command
 // This prevents duplicate/bogus moves from appearing in SGF
 target_board->clearMoveHistoryBeforeMovesCommand();
 qDebug() << "[q5Go] Cleared board for game" << current_game_context << "before requesting history";

 QString moves_cmd = QString("moves %1").arg(current_game_context);
 socket->write((moves_cmd + "\n").toUtf8());
 if (!this->suppress_server_console) output_console->append(QString(">>> [q5Go] SENT: %1 (first live move arrived, requesting history)").arg(moves_cmd));

 // CRITICAL: Skip processing ALL live moves until history arrives
 qDebug() << "[q5Go] Skipping live move" << move_part << "- waiting for history";
 target_board = nullptr; // Prevent processing below
 }
 }

 if (target_board) {
 // Route move to target board
 move.game_id = current_game_context;
 target_board->processMove(move);
 qDebug() << "[MENU] MOVE ROUTED: Game" << current_game_context << "move" << move_part;
 } else {
 qDebug() << "[WARN] MOVE SKIPPED:" << move_part << "- No target board for game context" << current_game_context;
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
 for (BoardWindow* board : board_windows) {
 if (board->isObserving()) {
 board->processComment(kibitz_user, kibitz_message, true);
 if (!this->suppress_server_console) output_console->append(QString(">>> Sent kibitz to board window"));
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
 
 // Send to board windows
 for (BoardWindow* board : board_windows) {
 if (board->isObserving() && board->getObservedGameId() == current_kibitz_game) {
 board->processComment(current_kibitzer, kibitz_message, true);
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

 if (is_say) {
 output_console->append(QString(">>> [OK] SAY SUCCESS: Delivered \"%1: %2\" to %3 playing board(s)")
 .arg(kibitz_user, kibitz_message).arg(boards_updated));
 } else {
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
 int move_count = gamesre.cap(6).toInt();
 int board_size = gamesre.cap(7).toInt();
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
 // Update any board window (both observing AND playing) for this game with complete details
 for (BoardWindow* board : board_windows) {
 if (board->getObservedGameId() == game_id) {
 qDebug() << "*** DEBUG KOMI: Calling updateGameSetup with komi=" << komi_val << "for game" << game_id;
 board->updateGameSetup(handicap_val, komi_val, QString("Byoyomi: %1s").arg(byoyomi_val));
 qDebug() << ">>> [CMD7] UPDATING BOARD GAME TYPE: Game" << game_id << "setting type to" << game_type;
 board->updateGameDetails(game_type, byoyomi_val);
 if (!this->suppress_server_console) output_console->append(QString(">>> [CMD7] Board for game %1 updated to type: %2").arg(game_id).arg(game_type));
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
 // Update all observing board windows
 for (BoardWindow* board : board_windows) {
 if (board->isObserving()) {
 board->updateGameSetup(board->getHandicap(), komi_val);
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
 QRegExp cmd22_header_re("^22\\s+(\\w+)\\s+"); // Capture player name
 QRegExp cmd22_row_re("^22\\s+(\\d+):\\s+([012345]+)$"); // Match "22 0: 0550501..."

 if (cmd22_header_re.indexIn(line) != -1) {
 // This is one of the two header lines
 QString player_name = cmd22_header_re.cap(1);

 if (!receiving_territory_data) {
 // First header line - begin territory data reception
 receiving_territory_data = true;
 territory_data_column = 0;
 territory_player_name = player_name; // Save first player name
 if (!this->suppress_server_console) output_console->append(QString(">>> CMD22: Territory data beginning for player: %1").arg(player_name));

 // Find the board window matching this player name
 territory_board = nullptr;
 for (BoardWindow* board : board_windows) {
 if (board->isObserving()) {
 // Match by player name (could be white or black)
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

 if (!territory_board) {
 if (!this->suppress_server_console) output_console->append(QString(">>> CMD22 WARNING: Could not find board matching player %1").arg(player_name));
 }
 }
 } else if (cmd22_row_re.indexIn(line) != -1 && receiving_territory_data) {
 // This is a territory row - process it
 int row_number = cmd22_row_re.cap(1).toInt();
 QString row_data = cmd22_row_re.cap(2);

 if (!this->suppress_server_console) output_console->append(QString(">>> CMD22: Row %1 = %2").arg(row_number).arg(row_data));

 if (territory_board) {
 territory_board->receiveScoreLine(row_number, row_data);
 }

 // If this is the last row (18), end territory reception
 if (row_number == 18) {
 if (territory_board) {
 territory_board->receiveScoreEnd();
 if (!this->suppress_server_console) output_console->append(QString(">>> CMD22: Territory data complete for game %1").arg(territory_board->getObservedGameId()));
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

 // Apply teaching title to the board for the current game context
 // IGS sends "Game is titled:" via Command 9 messages interleaved with Command 15 moves
 // Use current_game_context to identify which game the title belongs to
 if (current_game_context != -1) {
 // Find the board window for this specific game
 BoardWindow *target_board = nullptr;
 for (BoardWindow* board : board_windows) {
 if (board->getObservedGameId() == current_game_context) {
 target_board = board;
 break;
 }
 }

 if (target_board != nullptr) {
 target_board->setCustomGameTitle(game_title);
 if (!this->suppress_server_console) output_console->append(QString(">>> UPDATED BOARD TITLE for game %1: %2").arg(current_game_context).arg(game_title));
 } else {
 if (!this->suppress_server_console) output_console->append(QString(">>> WARNING: CUSTOM TITLE for game %1 but no board found: %2").arg(current_game_context).arg(game_title));
 }
 } else {
 if (!this->suppress_server_console) output_console->append(QString(">>> WARNING: CUSTOM TITLE DETECTED but no current game context: %1").arg(game_title));
 }
 }
 }

 // Check for scoring mode trigger
 if (line.contains("check your score with the score command")) {
 if (!this->suppress_server_console) output_console->append(">>> IGS REMOVESTONES: Game entered scoring phase - following q5Go protocol");

 // Implement q5Go's slot_removestones functionality
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

 // VERSION CONFIRMATION - verify correct binary is handling Command 9
 qDebug() << ">>> VERSION: BUILD-2026-01-03-v26-ENHANCED-SHADOWS (Command 9 handler executing)";

 // Update board window with result AND enter scoring mode with client-side territory calculation
 // Following q5Go pattern: enter scoring mode on Command 9, use calc_scoring_markers_complex()
 qDebug() << ">>> CMD9 DEBUG: About to iterate through" << board_windows.size() << "board windows for game" << game_id;
 for (BoardWindow* board : board_windows) {
 bool is_observing = board->isObserving();
 int observed_game_id = board->getObservedGameId();
 bool already_scoring = board->isScoringMode();

 qDebug() << ">>> CMD9 DEBUG: Board check - isObserving:" << is_observing
 << "observedGameId:" << observed_game_id
 << "targetGameId:" << game_id
 << "alreadyScoring:" << already_scoring;

 if (board->isObserving() && board->getObservedGameId() == game_id) {
 qDebug() << ">>> CMD9 DEBUG: Found matching board for game" << game_id;
 board->updateGameResult(result_text);

 // Store server scores for display
 board->setServerScore(white_score, black_score);
 qDebug() << ">>> CMD9: Set server scores - White:" << white_score << "Black:" << black_score;

 // TEMPORARY WORKAROUND: Removed enterScoringMode() and calculateTerritoryMarkers() calls
 // These were causing cross-contamination between observed games - one game finishing
 // would incorrectly trigger scoring mode on other observed games still in progress
 // Now relying solely on Command 22 territory data from server to enter scoring mode
 qDebug() << ">>> CMD9: Stored server scores, waiting for Command 22 territory data to trigger scoring mode";
 }
 }
 qDebug() << ">>> CMD9 DEBUG: Finished iterating through board windows";

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

 // Update board window with result (observing OR playing)
 for (BoardWindow* board : board_windows) {
 if (board->getObservedGameId() == game_id) {
 board->updateGameResult(result_text);
 if (!this->suppress_server_console) output_console->append(QString(">>> Updated board window %1 with result: %2").arg(board->getObservedGameId()).arg(result_text));
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
 
 for (BoardWindow* board : board_windows) {
 if (board->isPlaying()) {
 game_id = board->getObservedGameId();
 
 // Determine who resigned based on player names
 if (board->getWhitePlayer() == who_resigned_name) {
 result_text = "B+R"; // White resigned, Black wins
 } else if (board->getBlackPlayer() == who_resigned_name) {
 result_text = "W+R"; // Black resigned, White wins
 } else {
 result_text = "?+R"; // Unknown who resigned
 }
 break;
 }
 }
 
 if (game_id != -1) {
 if (!this->suppress_server_console) output_console->append(QString(">>> SIMPLE RESIGN: Game %1 - %2 (%3 resigned)").arg(game_id).arg(result_text).arg(who_resigned_name));

 // Update board window with result
 for (BoardWindow* board : board_windows) {
 if (board->getObservedGameId() == game_id) {
 board->updateGameResult(result_text);
 if (!this->suppress_server_console) output_console->append(QString(">>> Updated board window %1 with result: %2").arg(board->getObservedGameId()).arg(result_text));
 }
 }

 // Remove finished game from observed games tracking (unhighlight in Games window)
 untrackFinishedGame(game_id);
 }
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

 // Update board window with result (observing OR playing)
 for (BoardWindow* board : board_windows) {
 if (board->getObservedGameId() == game_id) {
 board->updateGameResult(result_text);
 if (!this->suppress_server_console) output_console->append(QString(">>> Updated board window %1 with result: %2").arg(board->getObservedGameId()).arg(result_text));
 }
 }

 // Remove finished game from observed games tracking (unhighlight in Games window)
 untrackFinishedGame(game_id);
 }

 // Check for adjourned games: Format: 9 Game 636: SanchaX vs PandaBot3 has adjourned.
 QRegExp adjourn_re("9\\s+Game\\s+(\\d+):\\s+.*\\s+has\\s+adjourned\\.");
 if (adjourn_re.indexIn(line) != -1) {
 int game_id = adjourn_re.cap(1).toInt();
 QString result_text = "Adjourned";

 if (!this->suppress_server_console) output_console->append(QString(">>> ADJOURNED RESULT: Game %1 - %2").arg(game_id).arg(result_text));

 // Update board window with result (observing OR playing)
 for (BoardWindow* board : board_windows) {
 if (board->getObservedGameId() == game_id) {
 board->updateGameResult(result_text);
 if (!this->suppress_server_console) output_console->append(QString(">>> Updated board window %1 with result: %2").arg(board->getObservedGameId()).arg(result_text));
 }
 }

 // Remove finished game from observed games tracking (unhighlight in Games window)
 untrackFinishedGame(game_id);
 }
 }
 
 // DEBUG: Show all Command 9 and 15 messages to understand protocol (DISABLED to reduce spam)
 // if (line.startsWith("9 ")) {
 // if (!this->suppress_server_console) output_console->append(QString(">>> DEBUG CMD9: %1").arg(line));
 // }
 // if (line.startsWith("15 ")) {
 // if (!this->suppress_server_console) output_console->append(QString(">>> DEBUG CMD15: %1").arg(line));
 // }
 
 // Parse IGS Command 9 - Stone removal detection
 if (line.startsWith("9 ") && line.contains("Removing @")) {
 QString coords_part = line.section('@', 1).trimmed();
 if (!this->suppress_server_console) output_console->append(QString(">>> STONE REMOVAL: %1").arg(coords_part));
 // TODO: Mark stones as dead on board
 }
 
 // Parse IGS Command 15 - Dead stone coordinate messages (q5Go removestones protocol)
 if (line.startsWith("15 ") && line.contains("is removing @")) {
 // Extract coordinates from pattern: "15 GameID Player is removing @ A1"
 QString pt = line.section(' ', 6, 6); // Get coordinate (A1, B2, etc.)
 QString game = line.section(' ', 1, 1); // Get game ID
 QString who = line.section(' ', 2, 2); // Get player name
 
 if (!this->suppress_server_console) output_console->append(QString(">>> IGS REMOVESTONES: Game %1 - %2 removing @ %3").arg(game, who, pt));
 
 // Process the dead stone marking exactly like q5Go
 int game_id = game.toInt();
 for (BoardWindow* board : board_windows) {
 if (board->isObserving() && board->getObservedGameId() == game_id) {
 // Follow q5Go's slot_removestones() logic exactly
 if (!this->suppress_server_console) output_console->append(QString(">>> Processing dead stone marking: %1 @ %2 (Game %3)").arg(who, pt, QString::number(game_id)));
 
 // Convert IGS coordinate (A1, B2) to board position (following q5Go logic)
 if (!pt.isEmpty() && pt.length() >= 2) {
 // q5Go coordinate parsing logic
 int i = pt[0].toLatin1() - 'A' + 1;
 // skip 'I' in IGS coordinates
 if (i > 8) i--;
 
 int j;
 if (pt.length() > 2 && pt[2].toLatin1() >= '0' && pt[2].toLatin1() <= '9') {
 j = 19 + 1 - pt.mid(1,2).toInt(); // board_size = 19 for standard Go
 } else {
 j = 19 + 1 - pt[1].digitValue();
 }
 
 // Convert to 0-based coordinates
 int col = i - 1;
 int row = j - 1;
 
 if (!this->suppress_server_console) output_console->append(QString(">>> Marking dead stone at (%1,%2) from IGS coord %3").arg(col).arg(row).arg(pt));
 
 // Mark stone as dead exactly like q5Go does
 board->markStoneAsDead(col, row);
 
 // Send kibitz message like q5Go does
 if (!this->suppress_server_console) output_console->append(QString(">>> removing @ %1").arg(pt));
 }
 break;
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

 // Show incoming match request dialog
 handleIncomingMatchRequest(opponent, opponent_rank, board_info, time_str, my_color);
 } else {
 output_console->append(QString("[ERROR] REGEX FAILED: Could not parse match request: %1").arg(line));
 }
 }
 
 // Bug 34: Parse server's suggested match command: "9 Use <match BusyBee B 19 1 5> or <decline BusyBee> to respond."
 if (line.startsWith("9 ") && line.contains("Use <match ") && line.contains("> or <decline")) {
     QRegExp cmd_regex("Use <(match [^>]+)> or");
     if (cmd_regex.indexIn(line) != -1) {
         QString suggested_cmd = cmd_regex.cap(1);
         qDebug() << "[MATCH] Server suggested command:" << suggested_cmd;
         if (pending_match_dialog) {
             pending_match_dialog->setServerSuggestedCommand(suggested_cmd);
             if (!this->suppress_server_console) output_console->append(QString(">>> Server suggests: %1").arg(suggested_cmd));
         }
     }
 }

 // Bug 34: Parse server's suggested nmatch command: "9 Use <nmatch woodnstone B 9 19 60 300 25 0 0 0> or <decline woodnstone> to respond."
 if (line.startsWith("9 ") && line.contains("Use <nmatch ") && line.contains("> or <decline")) {
     QRegExp cmd_regex("Use <(nmatch [^>]+)> or");
     if (cmd_regex.indexIn(line) != -1) {
         QString suggested_cmd = cmd_regex.cap(1);
         qDebug() << "[NMATCH] Server suggested command:" << suggested_cmd;
         if (pending_match_dialog) {
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
 
 // Find opponent's rank 
 QString opponent_rank = findPlayerRank(opponent);
 
 // Parse nmatch parameters: "B 3 19 60 600 25 0 0 0"
 QStringList params = nmatch_params.split(' ');
 if (params.size() >= 5) {
 QString my_color = params[0];
 QString handicap = params[1];
 QString board_size = params[2];
 QString time_seconds = params[3];
 QString byo_seconds = params[4];
 
 // Show incoming nmatch request dialog
 handleIncomingNmatchRequest(opponent, opponent_rank, my_color, handicap, board_size, time_seconds, byo_seconds);
 }
 }
 }
 
 // Debug: Log all Command 9 messages to help identify actual format (DISABLED to reduce spam)
 // if (line.startsWith("9 ")) {
 // output_console->append(QString("[INFO] DEBUG: All Command 9 messages: %1").arg(line));
 // }
 
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

 // Update board window with result
 for (BoardWindow* board : board_windows) {
 if (board->isObserving() && board->getObservedGameId() == game_id) {
 board->updateGameResult(standard_result);
 }
 }

 // Remove finished game from observed games tracking (unhighlight in Games window)
 untrackFinishedGame(game_id);
 }
 }
 } // End while (socket->canReadLine())
 } // End for (;;)
 
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
 PreferencesDialog *prefs_dialog = new PreferencesDialog(this);
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
 games_window->clearGames();
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
 
 void observeGame(int game_id, const QString& white, const QString& black, 
 const QString& white_rank, const QString& black_rank) {
 if (!connected_to_igs) {
 if (!this->suppress_server_console) output_console->append(">>> ERROR: Connect to IGS first to observe games");
 return;
 }
 
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
 connect(board, &BoardWindow::commentRequested, this, &FixedXGospelWindow::sendComment);
 connect(board, &BoardWindow::sayRequested, this, &FixedXGospelWindow::sendSay);
 connect(board, &BoardWindow::observersRequested, this, &FixedXGospelWindow::requestObservers);
 connect(board, &BoardWindow::moveRequested, this, &FixedXGospelWindow::sendMove);
 
 qDebug() << "Connected comment signals for board window observing game:" << game_id;

 board_windows.append(board);
 most_recently_observed_board = board; // Track for teaching title assignment
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
 
 void closeBoardWindow(int game_id) {
 qDebug() << "[closeBoardWindow] Called for game" << game_id << ", board_windows.size=" << board_windows.size();
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
 // Remove finished game from observed games tracking
 // Called when a game ends (result received) to unhighlight it in Games window
 // Board window can stay open for review, but game is no longer actively observed
 qDebug() << "[untrackFinishedGame] Removing finished game" << game_id << "from observed tracking";

 if (observed_game_ids.contains(game_id)) {
 observed_game_ids.remove(game_id);
 qDebug() << "[untrackFinishedGame] Removed game" << game_id << ", observed_game_ids size now:" << observed_game_ids.size();

 // Update Games window to remove highlighting
 if (games_window) {
 qDebug() << "[untrackFinishedGame] Calling games_window->clearSelectionForGame()";
 games_window->clearSelectionForGame(game_id);
 qDebug() << "[untrackFinishedGame] Calling games_window->updateObservedGames()";
 games_window->updateObservedGames(observed_game_ids);
 } else {
 qDebug() << "[untrackFinishedGame] WARNING: games_window is NULL!";
 }
 } else {
 qDebug() << "[untrackFinishedGame] Game" << game_id << "not in observed_game_ids (already removed or never tracked)";
 }
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
 if (!this->suppress_server_console) output_console->append(QString(">>> SENT MOVE: %1 (for game %2)").arg(igs_move).arg(game_id));
 
 qDebug() << "Move sent to IGS server:" << igs_move;
 }
 
 void updateStatus(const QString& status) {
 QLabel *status_label = findChild<QLabel*>("status_label");
 if (status_label) {
 status_label->setText("Status: " + status);
 }
 }
 
 void handleHeartbeat() {
 if (!hold_the_line || !connected_to_igs) {
 return;
 }
 
 heartbeat_counter--;
 
 // Every 5 minutes (300 seconds), check status
 if (heartbeat_counter % 300 == 0) {
 if (!this->suppress_server_console) output_console->append(QString(">>> Heartbeat check: %1 seconds remaining").arg(heartbeat_counter));
 
 // If counter reaches 0, send AYT and reset
 if (heartbeat_counter <= 0) {
 if (socket->state() == QTcpSocket::ConnectedState) {
 socket->write("ayt\n");
 if (!this->suppress_server_console) output_console->append(">>> SENT: ayt (heartbeat keep-alive)");
 }
 
 // Reset counter to 15 minutes (899 seconds)
 heartbeat_counter = 899;
 
 // Stop sending AYT after 1 hour if not observing games
 if (board_windows.isEmpty()) {
 if (!this->suppress_server_console) output_console->append(">>> No games being observed - reducing heartbeat frequency");
 // Could modify behavior here if needed
 }
 }
 }
 }
 
 void resetHeartbeatCounter() {
 heartbeat_counter = 899;
 }
 
 QString findPlayerRank(const QString& player_name) {
 // Search through players_model to find the rank for this player
 if (!players_window) return "?";
 
 // For now, check if we have the information in a map or return a default
 // This is a simplified approach - we'd need to access the players_model properly
 // in a real implementation, but this prevents crashes
 
 // Common known ranks for testing
 if (player_name == "weakkyu") return "7k?";
 if (player_name == "quietone") return "7k?";
 
 return "?"; // Default if not found
 }
};

// Removed moc include to fix build issues
// #include "board_window.moc" // Removed to avoid conflicts with separate board_window.cpp

int main(int argc, char *argv[]) {
 QApplication app(argc, argv);

 // VERSION BANNER - confirms correct binary is running
 qDebug() << "==========================================================================";
 qDebug() << ">>> XGOSPEL2 VERSION: BUILD-2026-01-14-v38-BOARD-TOP-TITLE";
 qDebug() << ">>> NEW: Teaching title moved to top of board frame (xgospel placement)";
 qDebug() << ">>> NEW: Title appears above board, not in info panel";
 qDebug() << ">>> - All three vertical borders now resizable (board, comments, observers)";
 qDebug() << ">>> - Removed fixed 300px width limit on info panel";
 qDebug() << ">>> - Initial split: 75% board, 25% info panel";
 qDebug() << ">>> - Single consolidated player info frame (no borders)";
 qDebug() << ">>> - Minimal padding (1-2px) for space efficiency";
 qDebug() << ">>> - Grid lines: Qt::black cosmetic pen (1 device pixel, no AA)";
 qDebug() << "==========================================================================";

 // Initialize global settings
 settings = new Settings();
 settings->load();

 FixedXGospelWindow window;
 window.show();

 return app.exec();
}

#include "xgospel2_fixed.moc"
