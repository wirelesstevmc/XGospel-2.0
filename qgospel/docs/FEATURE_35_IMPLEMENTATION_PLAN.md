# Feature 35: Players List BC/NR/Guest Support + Rank Range Filtering
**Date:** April 9, 2026
**Status:** Implementation Plan
**Complexity:** High - requires hybrid data source approach + new UI components

---

## Problem Statement

### Current Issues
1. **BC/NR players missing from list** - `userlist` command only returns players with established ratings
2. **Guest players incomplete** - Only 1 guest shows instead of all guests online
3. **No rank range filtering** - Can't filter players by rank range (e.g., "BC to 1p-3p")
4. **q5Go has this feature** - See `/home/cahill/Claude_Projects/q5Go_screenshots/q5Go_players_2026-04-07_15-44-00.png`

### Root Cause Analysis
- **userlist**: Returns detailed info (Win/Loss, Country, Match Prefs) but ONLY for ranked players
- **who**: Returns ALL players (BC/NR/guests included) but with minimal info (Name, Rank, Idle, Flags only)
- **Current implementation**: Uses userlist OR who (fallback), not both together

---

## Solution Design: Hybrid Approach

### Strategy
Use BOTH commands in sequence:
1. Send `userlist` (for registered accounts)
2. Wait for userlist completion
3. Send `who` as supplement
4. Merge results, skipping duplicates
5. Display all players with available info

### Data Merge Rules
```
IF player in userlist:
    Show full details (Name, Rank, Win/Loss, Country, Match Prefs, etc.)
ELSE IF player in who only:
    Show limited details (Name, Rank, Idle, Status flags)
    Set other fields to "--" or blank
```

---

## Implementation Details

### Phase 1: Hybrid userlist + who

#### Step 1.1: Add State Tracking Variables
**File:** `xgospel2_fixed.cpp`
**Location:** Line ~2417 (with other state variables)

**Already Added:**
```cpp
bool waiting_for_who_supplement;  // Line 2418
QSet<QString> userlist_player_names;  // Line 2419
```

**Initialization:** Line ~2503
```cpp
waiting_for_who_supplement = false;  // Already added
userlist_player_names.clear();  // Need to add in constructor
```

#### Step 1.2: Track Player Names from userlist
**File:** `xgospel2_fixed.cpp`
**Function:** `FixedPlayersWindow::addPlayerToTable()` (Line ~945)

**Add after line 946:**
```cpp
void addPlayerToTable(const Q5GoPlayer& player) {
    if (player.name.isEmpty() || player.rank.isEmpty()) return;

    // Bug 35: Track player names from userlist to avoid duplicates
    // This will be accessed by main window to build exclusion set
    // No code change needed here - tracking happens in main window
```

**Actual tracking location:** In main window where `addPlayerFromRawLine` is called (Line ~3707)

**Add after line 3707:**
```cpp
players_window->addPlayerFromRawLine(line);

// Bug 35: Track player names from userlist (before WHO supplement)
if (!waiting_for_who_supplement) {
    // Extract player name from parsed line to add to exclusion set
    // This prevents duplicates when merging WHO data later
    Q5GoPlayer temp_player;
    Q5GoParser parser;
    // Try userlist format first
    if (parser.parseUserlistLine(line, temp_player)) {
        userlist_player_names.insert(temp_player.name);
    } else if (parser.parseWhoFormatLine(line, temp_player)) {
        userlist_player_names.insert(temp_player.name);
    }
}

player_count++;
```

#### Step 1.3: Trigger WHO Supplement After userlist Completes
**File:** `xgospel2_fixed.cpp`
**Location:** Line ~3746 (end of players list detection)

**Replace lines 3746-3751:**
```cpp
} else {
    waiting_for_players = false;
    players_window->resizeColumns();
    players_window->sortByRank();
    players_window->reapplyOpenFilter();
    if (!this->suppress_server_console) output_console->append(QString(">>> COMPLETED: Parsed %1 total players from IGS").arg(player_count));
}
```

**With:**
```cpp
} else {
    // Bug 35: After userlist completes, supplement with WHO for BC/NR/guest players
    if (!waiting_for_who_supplement && login_username != "guest" && player_count > 5) {
        // userlist succeeded, now get BC/NR/guest players from WHO
        if (!this->suppress_server_console) output_console->append(QString(">>> USERLIST complete (%1 players), now fetching BC/NR/guests with WHO...").arg(player_count));
        waiting_for_who_supplement = true;
        players_window->setFallbackMode(true);  // Switch to WHO parsing mode
        socket->write("who\n");
        if (!this->suppress_server_console) output_console->append(">>> SENT: who (supplement for BC/NR/guest players)");
    } else {
        // Either WHO supplement is done, or we're in guest mode
        waiting_for_players = false;
        waiting_for_who_supplement = false;
        userlist_player_names.clear();  // Clear tracking set
        players_window->resizeColumns();
        players_window->sortByRank();
        players_window->reapplyOpenFilter();
        if (!this->suppress_server_console) output_console->append(QString(">>> COMPLETED: Parsed %1 total players from IGS").arg(player_count));
    }
}
```

#### Step 1.4: Skip Duplicates When Processing WHO Supplement
**File:** `xgospel2_fixed.cpp`
**Location:** Line ~3707 (where players are added)

**Modify player addition logic:**
```cpp
if (waiting_for_players && players_window) {
    // Look for any line that might contain player data
    if (line.contains("27 ") ||
        (line.length() > 20 && line.contains(QRegExp("[0-9]+[dkp*]")) &&
         line.contains(QRegExp("[a-zA-Z][a-zA-Z0-9]{2,10}")))) {

        // Bug 35: When processing WHO supplement, check for duplicates
        bool should_add = true;
        if (waiting_for_who_supplement) {
            // Extract player name to check against userlist
            Q5GoPlayer temp_player;
            Q5GoParser parser;
            if (parser.parseWhoFormatLine(line, temp_player) ||
                parser.parseUserlistLine(line, temp_player)) {
                // Skip if player was already added from userlist
                if (userlist_player_names.contains(temp_player.name)) {
                    should_add = false;
                }
            }
        }

        if (should_add) {
            players_window->addPlayerFromRawLine(line);

            // Bug 35: Track names from userlist (but not from WHO supplement)
            if (!waiting_for_who_supplement) {
                Q5GoPlayer temp_player;
                Q5GoParser parser;
                if (parser.parseUserlistLine(line, temp_player) ||
                    parser.parseWhoFormatLine(line, temp_player)) {
                    userlist_player_names.insert(temp_player.name);
                }
            }

            player_count++;

            if (player_count % 10 == 0) {
                if (!this->suppress_server_console) output_console->append(QString(">>> Parsed %1 players so far...").arg(player_count));
            }
        }
    }
```

---

### Phase 2: Rank Range Filter UI

#### Step 2.1: Add Rank Range ComboBoxes
**File:** `xgospel2_fixed.cpp`
**Class:** `FixedPlayersWindow`
**Location:** Line ~703 (with other UI members)

**Add member variables:**
```cpp
QCheckBox *open_filter_checkbox;  // Filter to show only "open" players
QComboBox *from_rank_combo;  // Bug 35: From rank filter
QComboBox *to_rank_combo;    // Bug 35: To rank filter
bool is_guest_mode;
```

**Initialize in constructor** (Line ~857, after open_filter_checkbox):
```cpp
// "Open" filter checkbox (q5Go style)
open_filter_checkbox = new QCheckBox("open");
open_filter_checkbox->setChecked(false);
connect(open_filter_checkbox, &QCheckBox::toggled, this, &FixedPlayersWindow::applyOpenFilter);

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

// Default: BC to 1p-3p (like q5Go screenshot)
from_rank_combo->setCurrentText("BC");
to_rank_combo->setCurrentText("3p");

connect(from_rank_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &FixedPlayersWindow::applyRankRangeFilter);
connect(to_rank_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &FixedPlayersWindow::applyRankRangeFilter);

// Update layout (replace existing filter_layout)
QHBoxLayout *filter_layout = new QHBoxLayout();
filter_layout->addWidget(new QLabel("From:"));
filter_layout->addWidget(from_rank_combo);
filter_layout->addWidget(new QLabel("To:"));
filter_layout->addWidget(to_rank_combo);
filter_layout->addStretch();
filter_layout->addWidget(open_filter_checkbox);
layout->addLayout(filter_layout);
```

#### Step 2.2: Implement Rank Range Filtering
**File:** `xgospel2_fixed.cpp`
**Class:** `FixedPlayersWindow`
**Location:** After `applyOpenFilter()` function (Line ~1500)

**Add new function:**
```cpp
void applyRankRangeFilter() {
    // Bug 35: Filter players by rank range
    if (!players_model || !proxy_model) return;

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
            players_table->setRowHidden(proxy_index.row(), !in_range);
        }
    }
}

int getRankNumericValue(const QString &rank) {
    // Convert rank to numeric value (higher = stronger)
    // Pro: 9p=1000, 8p=900, ..., 1p=100
    // Dan: 10d=99, 9d=90, ..., 1d=10
    // Kyu: 1k=9, 2k=8, ..., 30k=-20
    // BC: -30

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
```

---

## Testing Plan

### Test Case 1: Hybrid userlist + who
1. Login as registered account (e.g., woodnstone)
2. Open Players window
3. Verify userlist executes first
4. Verify WHO executes after userlist completes
5. Verify BC/NR players appear in list
6. Verify multiple guest players appear
7. Verify no duplicate players

### Test Case 2: Rank Range Filter
1. Set From: BC, To: 3p
2. Verify only players BC-3p are visible
3. Set From: 5k, To: 1d
4. Verify only players 5k-1d are visible
5. Verify filter persists across refresh

### Test Case 3: Combined Filters
1. Enable "open" checkbox
2. Set rank range
3. Verify both filters apply correctly
4. Refresh players list
5. Verify filters persist

---

## Code Locations Reference

### Files to Modify
- **xgospel2_fixed.cpp** (main implementation)

### Key Line Numbers (v50_R14)
- **2417-2419**: State tracking variables (DONE)
- **2503**: Variable initialization (PARTIAL - need userlist_player_names.clear())
- **703**: UI member variables (NEED TO ADD combos)
- **857-864**: Filter UI layout (NEED TO MODIFY)
- **3707-3712**: Player parsing and tracking (NEED TO MODIFY)
- **3746-3751**: End of players list handler (NEED TO MODIFY)
- **~1500**: Add applyRankRangeFilter() and getRankNumericValue() functions

### New Functions to Add
1. `applyRankRangeFilter()` - Apply rank range hiding
2. `getRankNumericValue()` - Convert rank string to numeric value for comparison

---

## Potential Issues & Solutions

### Issue 1: Duplicate Players
**Problem:** Same player appears twice (from userlist and who)
**Solution:** Use `userlist_player_names` QSet to track and skip duplicates

### Issue 2: WHO Format Parsing
**Problem:** WHO returns multiple players per line with `|` separator
**Solution:** Already handled in `addPlayerFromRawLine()` line ~879

### Issue 3: BC Players Missing Data
**Problem:** BC players won't have Win/Loss, Country, etc.
**Solution:** q5Go parser sets these to empty/default, which displays as "--"

### Issue 4: Filter State Persistence
**Problem:** Filters reset after refresh
**Solution:** Don't call `clearPlayers()` when supplementing with WHO

---

## Version and CHANGELOG Entry

**Version:** v50_R15-PLAYERS-BC-FILTER
**Build Date:** 2026-04-09

**CHANGELOG Template:**
```markdown
## [v50_R15-PLAYERS-BC-FILTER] - 2026-04-09

### Added
- **Feature 35: BC/NR/Guest players now visible in players list**
  - Implemented hybrid userlist + who approach
  - userlist provides detailed info for ranked players
  - WHO supplements with BC/NR/guest players (limited info)
  - Duplicate players are automatically filtered out

- **Feature 35a: Rank range filter (q5Go style)**
  - Added "From" and "To" rank dropdown menus
  - Filter players by rank range (e.g., BC to 1p-3p)
  - Default range: BC to 3p (shows all players)
  - Rank ranges work with professional, dan, kyu, and BC ranks

### Changed
- **Players list population strategy**
  - Now uses BOTH userlist AND who commands
  - userlist executes first (detailed player data)
  - WHO executes as supplement (BC/NR/guests only)
  - Player count includes all players from both sources

### Technical Details
- xgospel2_fixed.cpp:2418-2419: Added waiting_for_who_supplement and userlist_player_names
- xgospel2_fixed.cpp:703-705: Added from_rank_combo and to_rank_combo UI members
- xgospel2_fixed.cpp:857-890: Implemented rank filter UI with combo boxes
- xgospel2_fixed.cpp:~1500: Added applyRankRangeFilter() and getRankNumericValue()
- xgospel2_fixed.cpp:3707-3730: Modified player parsing to track names and skip duplicates
- xgospel2_fixed.cpp:3746-3765: Modified end-of-list handler to trigger WHO supplement
```

---

## Next Steps

1. ✅ Review this implementation plan
2. ⏳ Implement Phase 1 (hybrid approach)
3. ⏳ Test Phase 1 with BC/NR/guest players
4. ⏳ Implement Phase 2 (rank range filter)
5. ⏳ Test Phase 2 with various rank ranges
6. ⏳ Update CHANGELOG.md
7. ⏳ Build and final testing
8. ⏳ Backup

**Estimated Implementation Time:** 30-45 minutes
**Estimated Testing Time:** 15-20 minutes
**Total Session Time:** ~1 hour
