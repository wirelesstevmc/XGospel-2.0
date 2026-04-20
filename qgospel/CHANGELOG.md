# XGospel2 Changelog

All notable changes to the XGospel2 Qt5-based IGS Go client will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [v50_R18-GITHUB-PREP] - 2026-04-18

### Changed
- **Code organization and attribution**
  - Renamed parser files from `q5go_*` prefix to `xgospel2_*` prefix
  - Files renamed: `q5go_parser.{h,cpp}` → `xgospel2_parser.{h,cpp}`
  - Files renamed: `q5go_games_parser.{h,cpp}` → `xgospel2_games_parser.{h,cpp}`
  - Added proper attribution headers crediting q5Go project (https://github.com/bernds/q5Go)
  - Updated all `#include` statements and Makefile references
  - Organized documentation files into `docs/` subdirectory
  - Organized debug binaries into `debug/` subdirectory
  - Created `cleanup_for_github.sh` script for release preparation

### Technical Details
- xgospel2_parser.h/cpp: Added attribution header, updated header guards
- xgospel2_games_parser.h/cpp: Added attribution header, updated header guards
- xgospel2_fixed.cpp:40-41: Updated include statements
- Makefile:23-24,126,129-130: Updated source file references and dependencies
- cleanup_for_github.sh: Bash script for automated cleanup (docs, debug, artifacts)

### Notes
- Version updated from v50_R17 to v50_R18 for GitHub release preparation
- All features from v50_R17 are included in this release
- Parser files remain functionally identical, only naming and attribution changed

---

## [v50_R17-KIBITZ-FIX] - 2026-04-17

### Fixed
- **CRITICAL: Kibitz command missing game number**
  - Fixed kibitz failing with "You need to specify a game #" error
  - Root cause: sendComment() was sending "kibitz <message>" without game number
  - Old command: `kibitz hello everyone`
  - New command: `kibitz 207 hello everyone` (includes game ID)
  - IGS requires format: "kibitz <game_number> <message>"
  - Location: xgospel2_fixed.cpp:5905
  - Result: Kibitz now works correctly when observing games

### Notes
- Version updated from v50_R16 to v50_R17 to reflect kibitz fix as standalone release
- All features from v50_R16 are included in this release

---

## [v50_R16-PLAYER-STATUS-SYNC-FIX] - 2026-04-14

### Added
- **Feature: "Suppress All Output" console option**
  - Added "Suppress All Output" checkbox in Console menu
  - Controls all three suppression options at once (Games, Moves, Debug)
  - Provides quick way to silence all console output with one click
  - Located at top of Console menu with separator for clarity
  - Location: xgospel2_fixed.cpp:2950-2987

### Fixed
- **CRITICAL: Player game status sync timing issue**
  - Fixed intermittent player status updates that required multiple refresh cycles
  - Root cause: Games list and players list load asynchronously in unpredictable order
  - Old behavior: syncAllPlayerStatuses() only called after games list completed
  - Problem: If games list completed before players list, status updates were lost
  - New behavior: syncAllPlayerStatuses() called after BOTH lists complete
  - Location: xgospel2_fixed.cpp:4053, 4107
  - Result: Player game statuses now update immediately on first refresh

- **CRITICAL: Observing status being wiped out**
  - Fixed ALL players' observing status showing "--" instead of game numbers
  - Root cause: resetAllPlayerGameStatuses() was wiping observing column data
  - Old behavior: Reset both "pl" (playing) AND "ob" (observing) columns to "--"
  - Problem: WHO/userlist provides observing data, but sync was erasing it
  - New behavior: Only reset "pl" column, preserve "ob" column from WHO/userlist
  - Location: xgospel2_fixed.cpp:1182-1189
  - Result: ALL players now show correct observing status (e.g., "mituo2100 observing 204")

### Technical Details
- xgospel2_fixed.cpp:4053, 4107: Added syncAllPlayerStatuses() call after players list completes
- This ensures status sync happens regardless of which list (games or players) completes first
- Eliminates race condition where player status would show "--" despite being in a game
- Example: Player "tomyaki" in game 300 now shows "300" immediately after refresh

- xgospel2_fixed.cpp:1182-1189: Modified resetAllPlayerGameStatuses() to preserve observing data
- Only resets "pl" (playing) column - the data we're syncing from games list
- Preserves "ob" (observing) column - the data that comes from WHO/userlist command
- WHO command format includes observing: "Q -- 204 mituo2100 13s 7k" = observing game 204
- userlist (cmd42) also provides obs_str field with observing game numbers

- xgospel2_fixed.cpp:2199-2220: Simplified syncAllPlayerStatuses() function
- Removed observed_game_ids and login_username parameters (no longer needed)
- Observing data now comes entirely from WHO/userlist parsing
- No need to separately track local player observing - WHO already provides it

### Testing
- Test that player game status appears immediately after first refresh
- Verify status updates work when games list completes before players list
- Verify status updates work when players list completes before games list
- **NEW:** Test that ALL players' observing status shows correctly (not just local player)
- **NEW:** Verify observing column shows game numbers like "204", "109", etc.
- **NEW:** Check that players observing multiple games show correct data

---

## [v50_R15X-PLAYER-FILTER-FIXES] - 2026-04-11

### Added
- **Feature: "Hide Guests" filter checkbox**
  - Added checkbox to players window to hide guest accounts (guestXXXX)
  - Filter persists across refreshes
  - Checkbox appears next to "open" filter in UI
  - Significantly reduces clutter by filtering out all guest accounts
  - Guest detection: case-insensitive check for names starting with "guest"

- **Feature: Player game status tracking**
  - Players now show which game they're playing in the "pl" (playing) column
  - Status automatically updates when games start or end
  - Synced from games list after each refresh
  - Example: Player "tomyaki" playing in game 245 shows "245" in pl column
  - All player statuses reset to "--" then updated from current games list

- **Feature: Game count indicator**
  - Games window title now shows count: "Games Online (345)"
  - Updates in real-time as games are added
  - Resets to (0) when games list is cleared
  - Easy comparison with other clients to verify parsing accuracy

### Fixed
- **CRITICAL: BC/NR player filtering bug**
  - Fixed regex filter that was excluding BC/NR players from players list
  - Old regex: `line.contains(QRegExp("[0-9]+[dkp*]"))` required numbers before rank suffix
  - New regex: Also accepts `BC` and `NR` without numbers
  - This was THE root cause of intermittent BC/NR player visibility
  - Location: xgospel2_fixed.cpp:3841

- **Guest filter persistence**
  - Fixed issue where guests reappeared after refresh despite checkbox being checked
  - Added safety check to skip filter during data population (while sorting disabled)
  - Filter now properly reapplies after players list completes loading

- **Games window refresh**
  - Fixed issue where games window stopped repainting after implementing player status sync
  - syncAllPlayerStatuses() was being called at wrong point in data flow
  - Now called inside games completion block after all games are loaded

### Changed
- **Filter application strategy**
  - All filters (open, hide guests, rank range) now reapply together after player status updates
  - Prevents filters from conflicting with each other
  - Single batch update instead of multiple individual updates improves performance

- **Player status update flow**
  - Added resetAllPlayerGameStatuses() to clear all pl/ob columns to "--" before sync
  - Added syncAllPlayerStatuses() to update from current games list
  - Added updatePlayerGameStatus() to update individual player status
  - Games window maintains reference to players window for status updates

### Technical Details
- xgospel2_fixed.cpp:3841: BC/NR regex filter fix
- xgospel2_fixed.cpp:706: Added hide_guests_checkbox member
- xgospel2_fixed.cpp:874-877: Hide guests checkbox UI initialization
- xgospel2_fixed.cpp:1687-1727: applyHideGuestsFilter() function
- xgospel2_fixed.cpp:1159-1195: updatePlayerGameStatus() function
- xgospel2_fixed.cpp:1197-1220: resetAllPlayerGameStatuses() function
- xgospel2_fixed.cpp:1919: Added players_window_ref to FixedGamesWindow
- xgospel2_fixed.cpp:2193-2196: setPlayersWindowRef() function
- xgospel2_fixed.cpp:2198-2219: syncAllPlayerStatuses() function
- xgospel2_fixed.cpp:2221-2224: updateGamesCount() function
- xgospel2_fixed.cpp:2147-2150: Update player status when games added to table
- xgospel2_fixed.cpp:4101-4102: Sync player statuses and update count after games list completes
- xgospel2_fixed.cpp:5415, 5665: Link games_window to players_window on creation

### Known Issues
- **Intermittent player game status updates** ✅ FIXED in v50_R16
  - ~~Some players may intermittently not show correct game status after refresh~~
  - ~~Example: Player tomyaki in game 300 may show "--" instead of "300"~~
  - ~~Root cause: Timing issue between games list and players list updates~~
  - **RESOLUTION:** Added syncAllPlayerStatuses() call after players list completes
  - See v50_R16 changelog for details

- **Intermittent BC/NR player visibility** (from v50_R15)
  - Some BC/NR players may intermittently not appear after auto-refresh
  - Root cause: Race condition in proxy model index mapping during filter application
  - Workaround: Click "Refresh Players" button or sort by different column
  - Proper fix planned: Refactor to use QSortFilterProxyModel::filterAcceptsRow()
  - Estimated effort: 2-3 hours

### Notes
- Session included accidental source restore from backup, requiring re-application of all fixes
- Created comprehensive reapply script: reapply_all_fixes.sh
- All features successfully restored and compiled
- Backup recommended: v50_R15X_player_filtering_fixes

---

## [v50_R15-PLAYERS-BC-FILTER] - 2026-04-10

### Added
- **Feature 35: BC/NR/Guest players now visible in players list**
  - Implemented hybrid userlist + who approach
  - userlist provides detailed info for ranked players
  - WHO supplements with BC/NR/guest players (limited info)
  - Duplicate players are automatically filtered out
  - BC players now appear with rank "BC" (Beginner Class)
  - Guest players now show all guests online (not just first one)
  - thirdstone and other BC players now consistently visible

- **Feature 35a: Rank range filter (q5Go style)**
  - Added "From" and "To" rank dropdown menus
  - Filter players by rank range (e.g., BC to 9p)
  - Default range: BC to 9p (shows all players from beginners to professionals)
  - Rank ranges work with professional (9p-1p), dan (10d-1d), kyu (1k-30k), and BC ranks
  - Numeric rank conversion for accurate filtering (Pro: 9p=900, Dan: 1d=10, Kyu: 1k=9, BC=-30)
  - Filters persist across player list refreshes
  - Filters automatically reapply when user changes sort order

### Changed
- **Players list population strategy**
  - Now uses BOTH userlist AND who commands
  - userlist executes first (detailed player data)
  - WHO executes as supplement (BC/NR/guests only)
  - Player count includes all players from both sources
  - No duplicate players shown (tracked via QSet)

- **Players list sorting behavior**
  - Sorting disabled during population for better performance
  - Sorting re-enabled and applied after all data loads
  - Default sort: by rank (strongest to weakest)
  - Sort indicator appears on Rank column header after loading
  - Filters reapply automatically when user sorts by different columns

### Fixed
- **WHO format parsing improvements**
  - Fixed column misalignment where player names appeared in Idle column
  - Correctly handle WHO format with optional number field (5, 6, or 7 parts)
  - 7-part format: [number] [flag1] [flag2] [flag3] [name] [idle] [rank]
  - 6-part format: [flag1] [flag2] [flag3] [name] [idle] [rank]
  - 5-part format: [flag2] [flag3] [name] [idle] [rank]
  - Filter out userlist format entries (8-10+ parts) from WHO parser
  - Filter out game list entries (cmd 7) with bracket notation

- **WHO command termination detection**
  - Fixed WHO terminator not being detected: "27  ******** N Players M Games ********"
  - Removed overly restrictive `!line.contains("27 ")` check
  - Added explicit terminator pattern: contains "Players" + "Games" + "**"
  - Now properly triggers completion, sorting, and filter application
  - Resolved intermittent "thirdstone missing" issue caused by incomplete loading

- **Filter persistence across operations**
  - Filters now reapply when user clicks column headers to sort
  - Connected to QHeaderView::sortIndicatorChanged signal
  - Both "open" filter and rank range filter reapply on sort changes
  - Prevents rows from disappearing when changing sort order
  - Default rank filter changed from "BC to 3p" to "BC to 9p" (shows all players)

### Known Issues
- **Intermittent BC/NR player visibility**
  - Some BC/NR players may intermittently not appear in the players list after auto-refresh
  - Root cause: Race condition in proxy model index mapping during filter application
  - Players ARE parsed and added to the model, but setRowHidden() may map to wrong proxy row
  - Workaround: Click "Refresh Players" button or sort by different column to make player appear
  - Proper fix planned for future release: Refactor to use QSortFilterProxyModel::filterAcceptsRow()
  - See "Future Improvements" section below for technical details

### Technical Details
- xgospel2_fixed.cpp:2418-2419: Added waiting_for_who_supplement and userlist_player_names state variables
- xgospel2_fixed.cpp:2614-2615: Initialize hybrid tracking in constructor
- xgospel2_fixed.cpp:703-706: Added from_rank_combo and to_rank_combo UI members
- xgospel2_fixed.cpp:858-896: Implemented rank filter UI with combo boxes
- xgospel2_fixed.cpp:1117-1124: Added reapplyRankRangeFilter() wrapper function
- xgospel2_fixed.cpp:1581-1627: Added applyRankRangeFilter() function
- xgospel2_fixed.cpp:1629-1655: Added getRankNumericValue() helper function
- xgospel2_fixed.cpp:3819-3846: Modified player parsing to track names and skip duplicates
- xgospel2_fixed.cpp:3891-3898: Fixed WHO terminator detection in end-of-list handler
- xgospel2_fixed.cpp:3911-3930: Modified completion handler to trigger WHO supplement
- xgospel2_fixed.cpp:1085-1087: Disable sorting during clearPlayers() for better performance
- xgospel2_fixed.cpp:1117-1122: Re-enable sorting in sortByRank() and force proxy model sort
- xgospel2_fixed.cpp:850-858: Connect to sortIndicatorChanged signal to reapply filters
- q5go_parser.cpp:443-448: Fixed WHO format prefix detection
- q5go_parser.cpp:467-468: Added upper bound check for WHO format
- q5go_parser.cpp:475-495: Fixed WHO field index calculation for 7-part format
- xgospel2_fixed.cpp:891: Changed default rank filter from "3p" to "9p"
- xgospel2_fixed.cpp:1607-1615: Added safety check to skip filter while sorting disabled

### Future Improvements

**Planned Fix for Intermittent Player Visibility Issue:**

The current implementation uses `QTreeView::setRowHidden()` with manual proxy model index mapping, which causes race conditions during sorting/filtering. The proper Qt approach is to override `QSortFilterProxyModel::filterAcceptsRow()`.

**Refactoring Effort Estimate: 2-3 hours**

**Required Changes:**

1. **Modify FixedRankSortProxyModel class** (xgospel2_fixed.cpp:52-67)
   - Add member variables to store filter state:
     - `bool open_filter_enabled`
     - `QString rank_from`, `QString rank_to`
   - Add public setter methods:
     - `setOpenFilterEnabled(bool enabled)`
     - `setRankRange(QString from, QString to)`
   - Override `filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent)`
     - Check column 0 (Stat) for "X" flag if open_filter_enabled
     - Check column 2 (Rank) against rank_from/rank_to range
     - Return true if row passes all active filters
     - Return false to hide row
   - Move getRankNumericValue() to proxy model class as helper

2. **Simplify FixedPlayersWindow filter functions** (xgospel2_fixed.cpp:1534-1684)
   - applyOpenFilter(): Just call proxy_model->setOpenFilterEnabled() + invalidateFilter()
   - applyRankRangeFilter(): Just call proxy_model->setRankRange() + invalidateFilter()
   - Remove all manual setRowHidden() calls
   - Remove proxy index mapping code
   - Let Qt's proxy model handle visibility automatically

3. **Benefits:**
   - Eliminates race conditions completely
   - Cleaner, more maintainable code
   - Better performance (Qt optimized)
   - Filters work correctly during all operations

4. **Testing:**
   - Verify open filter hides playing/X-marked players
   - Verify rank range filter works across full range (BC to 9p)
   - Test filter persistence across refresh
   - Test sorting with filters active
   - Verify no players disappear intermittently

---
