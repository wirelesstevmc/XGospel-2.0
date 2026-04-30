# XGospel2 Changelog

All notable changes to the XGospel2 Qt5-based IGS Go client will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [v51_R1-COMPLEX-SCORING] - 2026-04-30

### Changed
- **Major version bump to v51** — SGF editor, game tree navigation, complex scoring engine,
  and disputed territory visualisation represent a significant milestone in development.

### Fixed
- **Update button segfault**: The SGF edit window's "Update" button crashed when the source
  live game window had been closed before Update was clicked. `source_board_window` was a
  dangling pointer — the null check did not catch it. Fix: connect `QObject::destroyed` on
  the source window to null out the pointer in the edit window before memory is freed.
- **Disputed marker colour**: Changed from grey to red for better visibility when comparing
  complex scoring output against q5Go side-by-side.

---

## [v50_R21-COMPLEX-SCORING] - 2026-04-29

### Added
- **Complex scoring engine (calc_scoring_markers_complex port from q5Go)**
  - Full port of q5Go's `go_board::calc_scoring_markers_complex()` into `score_engine.cpp`
  - False-eye detection: iterative loop identifies groups touching exactly one candidate
    territory point with external liberties; those points are removed from territory and
    flagged as `falseeye`
  - Dead-stone propagation: territory containing dead stones is "real"; liveness propagates
    outward from real territory through bordering units and their adjacent empty points
  - Seki exclusion: groups with `m_seki` flag set contribute a `seki_neighbours` exclusion
    mask passed to `finish_scoring_markers`; seki points are not counted for either side
  - Benson's algorithm (`benson()` + `find_eas()`) fully ported but kept behind `#if 0`
    matching q5Go upstream — can be activated when needed
  - New `sc_mark` values: `seki`, `falseeye` — populated by complex path, empty in simple path

- **False-eye upstream bug fix (q5Go improvement)**
  - q5Go's original false-eye loop only sets `changed = true` when a territory unit becomes
    completely empty, leaving trimmed-but-non-empty units' removed points stranded in
    `cand_territory`.  Those stranded points survived to `finish_scoring_markers` uncategorised
    and appeared as disputed — the 1-point error visible in q5Go's complex scoring.
  - Fix: `changed` is set whenever *any* point is removed from any territory unit.
    `cand_territory` is fully rebuilt from actual unit contents after each pass.
    Result: no points can persist as phantom candidates between iterations.

- **Disputed-point visual marker**
  - `GoBoardWidget` gains `disputed_positions` member and `setDisputedPoints()` setter
  - `drawDisputedMarkers()`: hollow grey square (cell_size/3), 2px grey outline, no fill —
    visually distinct from filled territory boxes and dead-stone markers
  - Drawn in `paintEvent` after `drawDeadStoneMarkers`, only when `scoring_mode_enabled`
  - `exitScoringMode()` clears disputed points from the widget

- **Scoring method preference (Simple vs Complex)**
  - `ScoringMethod` enum in `score_engine.h`: `Simple` (default), `Complex`
  - `ScoreEngine::estimate()` extended with `disputed_out` (QSet) and `method` parameters
  - `settings.h`: `getScoringMethod()` / `setScoringMethod()` — key `scoring_method`
  - Preferences dialog: new "Scoring" group in Application Settings tab with combo box:
    "Simple (flood-fill, fast)" / "Complex (false-eye + seki detection)"
  - Setting persists across sessions; both methods can be compared by re-scoring same position

### Technical Details
- score_engine.h: `ScoringMethod` enum; `estimate()` signature extended
- score_engine.cpp: `stone_unit` gains `m_n_vital` (short) + `m_seki` (bool); `sc_mark`
  gains `seki` + `falseeye`; `bit_array` gains `subset_of()`; `ScoreBoard` gains
  `enclosed_area`, `find_eas()`, `benson()`, `calc_scoring_markers_complex()`;
  `finish_scoring_markers()` now accepts `const bit_array *do_not_count` (nullptr for simple)
- board_window.h: `disputed_positions` member; `setDisputedPoints()` inline setter;
  `drawDisputedMarkers()` declaration
- board_window.cpp: `drawDisputedMarkers()` implementation; all three `ScoreEngine::estimate()`
  call sites updated (markStoneAsDead, onScoreClicked, receiveScoreEnd path unchanged);
  method read from settings at each call
- preferences_dialog.h/cpp: `m_scoring_method_combo`; Scoring group UI; saved in onApply/onOk

### Notes
- Default scoring method remains Simple — no behavior change for existing users
- Complex method produces identical results to Simple for positions with no seki/false-eyes
- The Benson #if 0 gate matches q5Go upstream; activating it seeds m_seki for live-group
  detection but is not needed for the false-eye + propagation path currently in use

---

## [v50_R20-patch1-KOMI-GUARD] - 2026-04-29

### Fixed
- **CRITICAL: Komi corruption via IGS game ID reuse**
  - Root cause: IGS recycles game numbers. When a finished game's board window remained open,
    a new game assigned the same ID would send a Command 7 line with different players and komi.
    The Command 7 handler matched only on game ID and blindly overwrote the stale board's komi,
    handicap, and game type with values from the new game.
  - Discovery: Pro teaching game kansai1 vs kansai1 (game 167, komi 6.5) was observed and played
    to conclusion (W resigns). After the game ended IGS reused game ID 167 for a new game
    (RCC vs krka, komi -5.5). Periodic Command 7 updates then overwrote the open kansai1 board's
    komi to -5.5. When the SGF was saved it recorded `KM[-5.500000]` instead of `KM[6.500000]`,
    flipping the game result from W+2.5 to B+9.5. Confirmed via session log at line ~2310555:
    `"7 [167]         RCC [ 1d ] vs.        krka [ 1k ] (  2   19  0 -5.5  5  I) (  0)"`.
  - Fix: Command 7 board-update loop now guards on two conditions before applying any update:
    1. `!board->isFinished()` — finished/stale boards are never updated.
    2. Player name match — if neither white nor black from Command 7 matches the board's players,
       the game ID was reused; update is skipped and a diagnostic message is logged.
  - Location: xgospel2_fixed.cpp, Command 7 handler, board-update loop (~line 4873).
  - Impact: Protects komi, handicap, and game type from silent corruption for any game where
    the board window is kept open after the game ends (common during post-game analysis).

---

## [v50_R20-SGF-EDITOR] - 2026-04-27

### Added
- **SGF Editor: Full edit window launched from observed/played games**
  - "Edit Game" button opens a dedicated analysis board (separate BoardWindow in edit mode)
  - Inherits player names, game metadata, and full game tree from source window
  - Horizontal game tree navigation strip at bottom (stone icons, scrollable)
  - Active node highlighted in red; user-edited nodes drawn with blue border
  - Click any node in strip to jump directly to that position

- **SGF Editor: Edit Position mode (free stone placement)**
  - "Edit Position" button transforms panel from 4-button view to 5-button edit layout
  - Stones placed in alternating color with full capture rule enforcement
  - Ghost cursor (semi-transparent stone) follows mouse in edit mode
  - Red circle marks last move from unedited game record (preserved through edits)
  - Blue circle marks last stone placed in current edit session
  - Both markers visible simultaneously and survive clearBoard() re-renders

- **SGF Editor: Per-stone Undo in edit position mode**
  - "Undo" button (orange) appears only in edit position mode
  - Undoes one stone at a time back to the start of the edit session
  - Board state, blue marker, red marker, and next-player color all correctly restored
  - Stack cleared on entering/leaving edit position mode

- **SGF Editor: Append and Pass**
  - "Append" commits current scratchpad board state as a new edited GameNode
  - "Pass" creates a pass node (x=-1, y=-1) marked as edited
  - Edited nodes flagged with `isEdited()` for distinct rendering in tree strip

- **SGF Editor: Cancel Edit and Update**
  - "Cancel Edit" discards scratchpad, returns to view mode at current node
  - "Update" regenerates SGF from source board, re-parses, reloads tree in edit window

- **IGS game resume detection**
  - Parser now handles `21 {Game <id>: <white> vs <black> @ Move <move_num>}` resume notification
  - If a board window for that game exists and is marked finished (adjourned), it is reactivated and re-subscribed via `observe <id>`
  - If no window exists, a fresh observation is opened automatically
  - Handles game ID reassignment that IGS performs on resume

### Fixed
- **Say/Tell protocol revert after game ends**
  - Incoming Command 24 (`say`) messages were still routing to finished game board windows
  - Root cause: routing condition checked `isPlaying()` but not `isFinished()`
  - Fix: added `&& !board->isFinished()` guard — messages now fall through to player dialog `tell` channel as soon as game result is received, even with board window still open
  - Location: xgospel2_fixed.cpp Command 24 handler

- **Red circle (last game move marker) disappearing during edit**
  - `clearBoard()` was resetting `last_move_x/y = -1`
  - Fix: after every `clearBoard()` + re-render in edit mode, `setLastMove()` is called with `current_node` coordinates to restore red marker

- **Blue circle (last edit stone marker) lost after Undo**
  - Undo stack stored only `GoBoard` snapshots, not the previous blue marker position
  - Fix: stack entries now carry `{GoBoard, edit_x, edit_y}`; Undo restores both board and blue marker

### Technical Details
- board_window.h: Added `GameTreeStrip` class, `GoBoardWidget` hover/edit extensions, `BoardWindow` edit members
- board_window.cpp: `setupEditUI()`, `switchToEditPositionMode()`, `switchToViewMode()`, `updateGameTreeStrip()`, `onBoardClicked()`, `onAppendClicked()`, `onPassClicked()`, `onCancelEditClicked()`, `onUpdateClicked()`, `onUndoEditClicked()`
- game_tree.h/cpp: Added `m_edited` flag to `GameNode` with `isEdited()`/`setEdited()` accessors
- xgospel2_fixed.cpp: Command 21 resume handler; Command 24 `isFinished()` guard

### Notes
- Version updated from v50_R19 to v50_R20
- Score estimation (Benson's algorithm + flood-fill territory) deferred to Phase 2
- Variation support in game tree strip deferred to Phase 2
- All features from v50_R19 are included in this release

---

## [v50_R19-FILTER-PREFERENCES] - 2026-04-22

### Added
- **Feature: Player filter preference persistence**
  - All player list filters now save and restore across sessions
  - "Open" filter checkbox state persisted
  - "Hide Guests" filter checkbox state persisted
  - Rank range "From" and "To" selections persisted
  - Filter states loaded on startup and automatically applied
  - Preferences saved immediately when filters change
  - Settings keys: `players_filter_open`, `players_filter_hide_guests`, `players_filter_rank_from`, `players_filter_rank_to`
  - Default values: open=false, hide_guests=false, rank_from="BC", rank_to="9p"

### Fixed
- **Player dialog toggle flags resetting after refresh**
  - Fixed local player toggle flags (Looking, Open, Quiet, Shout) resetting to OFF/FALSE after players list refresh
  - Root cause: Players list refresh doesn't update dialog stats, stat field becomes stale
  - Solution: Auto-request fresh stats for local player if their dialog is open after player list refresh
  - Added parseToggleStatesFromStat() function to extract flags from IGS stat field
  - IGS stat field flags: '!' = Looking, 'X' = Not Open, 'Q' = Quiet, 'S' = Shout
  - parseToggleStatesFromStat() called in constructor before setupUI()
  - parseToggleStatesFromStat() called in updateStatsData() before rebuilding UI
  - Auto-stats request triggered at end of players list load if local dialog open
  - Added getLocalUsername() method to FixedPlayersWindow
  - Location: xgospel2_fixed.cpp:131-147, 530, 1573-1576, 4221-4235
  - Result: Toggle button states now refresh automatically and stay correct

- **CRITICAL: Filter conflict causing lost state after refresh**
  - Fixed three filters (open, rank range, hide guests) overriding each other
  - Root cause: Sequential filter application caused later filters to unconditionally override earlier ones
  - Old behavior: `applyRankRangeFilter()` would show all players in rank range, ignoring open filter
  - Problem: Filters were fighting each other instead of cooperating
  - New behavior: Unified `applyAllFilters()` function combines all conditions in single pass
  - A player is now hidden if ANY condition is true:
    - Open filter is on AND (player is playing OR has X mark)
    - Guest filter is on AND player is a guest
    - Player's rank is outside selected range
  - Grey formatting for X-marked players now applied in same pass as visibility logic
  - Result: All filters work together dynamically without conflicts or requiring refresh

### Changed
- **Filter application strategy**
  - Replaced three separate filter functions with unified `applyAllFilters()`
  - All checkbox/combo signals now call unified filter function
  - Filter preferences saved to settings when changed (using lambda captures)
  - Preferences loaded in constructor after setupUI()
  - Eliminated race conditions between filter types

### Technical Details
- xgospel2_fixed.cpp:723-738: Load filter preferences from settings in constructor
- xgospel2_fixed.cpp:871-873: Modified open filter checkbox to save preference on toggle
- xgospel2_fixed.cpp:876-879: Modified hide guests checkbox to save preference on toggle
- xgospel2_fixed.cpp:901-910: Modified rank combos to save preferences on change
- xgospel2_fixed.cpp:1225-1310: Unified `applyAllFilters()` function (replaces three separate functions)
- settings.h:35-40: Using `writeEntry()`, `readEntry()`, `writeBoolEntry()`, `readBoolEntry()`

### Notes
- Version updated from v50_R18 to v50_R19
- All features from v50_R18 are included in this release
- Filter preferences persist in `~/.config/xgospel2/xgospel2.conf`
- Setting checkbox states during preference load triggers save (harmless redundancy)

---

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
