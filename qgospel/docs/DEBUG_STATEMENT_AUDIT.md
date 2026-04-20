# XGospel2 Debug Statement Audit
Generated: 2026-02-07

## Purpose
Review all qDebug() statements to determine which should be:
- **KEEP**: Useful for future development/debugging
- **REMOVE**: Completed debugging, no longer needed
- **SUPPRESS**: Add boolean flag to enable/disable

---

## BOARD_WINDOW.CPP (336 total statements)

### Category A: Initialization/Constructor (Lines 41-538)
**Count**: ~15 statements
**Purpose**: Track window creation, UI setup, timer initialization
**Examples**:
- Line 41: Board texture loading success
- Line 43: Board texture not found warning
- Line 516-538: Constructor step-by-step tracking

**Recommendation**: 
- KEEP: Texture loading messages (lines 41-43) - useful for config issues
- REMOVE: Constructor step tracking (lines 516-538) - completed debugging
- REMOVE: "Timer created/connected" - no longer needed

---

### Category B: Stone Placement (Lines 84-93)
**Count**: ~5 statements  
**Purpose**: Track placeMoveAt() validation and success
**Examples**:
- Line 84: "GoBoardWidget::placeMoveAt called with x=... y=..."
- Line 90: "Stone placed successfully"
- Line 93: "ERROR: Invalid coordinates"

**Recommendation**:
- KEEP: Error message (line 93) - alerts to coordinate bugs
- REMOVE: Success tracking (lines 84, 90) - routine operation

---

### Category C: Rendering Debug (Lines 172-264)
**Count**: ~10 statements
**Purpose**: Stone size, grid lines, positioning calculations
**Examples**:
- Line 172: "=== STONE SIZE DEBUG ==="
- Line 225: "=== GRID DEBUG ==="
- Line 264: "=== STONE POSITIONING DEBUG ==="

**Recommendation**:
- REMOVE: All rendering debug - completed, stones render correctly

---

### Category D: Edit Mode (Lines 473-487)
**Count**: ~5 statements
**Purpose**: Track edit mode stone placement/removal
**Examples**:
- Line 473: "Edit mode: Placed stone"
- Line 487: "Edit mode: Removed stone with right click"

**Recommendation**:
- KEEP: Edit mode operations - useful for future edit features
- SUPPRESS: Add bool flag "debug_edit_mode" in settings

---

### Category E: Observation State (Lines 912-1095)
**Count**: ~15 statements
**Purpose**: Track observation state machine (JOINING, RECONSTRUCTING, LIVE)
**Examples**:
- Line 912: "🎯 OBSERVATION STATE: Changed to JOINING_GAME"
- Line 1060: "🎯 OBSERVATION STATE: Changed to RECONSTRUCTING"
- Line 1078: "🎯 OBSERVATION STATE: Changed to LIVE_OBSERVATION"

**Recommendation**:
- KEEP: State transitions - critical for understanding observation bugs
- SUPPRESS: Add bool flag "debug_observation_state" in settings

---

### Category F: Move Processing (Lines 1099-2100)
**Count**: ~200 statements
**Purpose**: Track move validation, game tree updates, captures
**Examples**:
- Line 1099: "DEBUG: BoardWindow::processMove called"
- Line 1140: "DEBUG: Placing X handicap stones"
- Line 1205: "DEBUG: Pass move X by BLACK/WHITE"
- Line 1273: "*** 3 CONSECUTIVE PASSES DETECTED - entering scoring mode"
- Line 1305: "🗂️ MOVE HISTORY: Adding move X"

**Recommendation**:
- KEEP: Scoring mode detection (line 1273) - important game state
- KEEP: Error messages (invalid color, wrong game, etc)
- REMOVE: Move-by-move tracking - routine operation
- SUPPRESS: Add bool flag "debug_move_processing" in settings

---

### Category G: SGF Loading (Lines 949-1000)
**Count**: ~8 statements
**Purpose**: Track SGF file loading and game name extraction
**Examples**:
- Line 949: ">>> loadSGF called with game_name:"
- Line 964: ">>> Calling setCustomGameTitle with:"
- Line 1000: "Loaded SGF: filename with X moves"

**Recommendation**:
- KEEP: File loading confirmation (line 1000) - user feedback
- REMOVE: Internal call tracking (lines 949, 964)

---

### Category H: Navigation/Slider (Lines 2400-2600)
**Count**: ~30 statements
**Purpose**: Track move slider, navigation buttons, position changes
**Examples**:
- "goToMove() called with move_index=X"
- "Move slider value changed"
- "Navigation: Forward/Back button clicked"

**Recommendation**:
- REMOVE: Navigation tracking - completed debugging

---

### Category I: Territory/Scoring (Lines 2700-3000)
**Count**: ~25 statements
**Purpose**: Track territory data reception, dead stone marking
**Examples**:
- "Receiving territory data for game X"
- "[DEAD-MARKER-DEBUG] setDeadStones called with X dead stones"
- "Territory data row X processed"

**Recommendation**:
- KEEP: Territory data reception start/end - useful for protocol debugging
- SUPPRESS: Add bool flag "debug_scoring" in settings
- REMOVE: Row-by-row processing

---

## XGOSPEL2_FIXED.CPP (similar analysis needed)

### Category J: Connection/Protocol (~50 statements)
**Purpose**: IGS login, command parsing, protocol handling
**Recommendation**: KEEP errors, SUPPRESS verbose parsing

### Category K: Game/Player Window Updates (~40 statements)
**Purpose**: Track window population, refresh operations
**Recommendation**: REMOVE most, KEEP errors

### Category L: Match Protocol (~30 statements)  
**Purpose**: Track match requests, responses, game setup
**Recommendation**: KEEP for future match debugging

---

## RECOMMENDED SETTINGS FLAGS

Add to Settings class:
```cpp
bool debug_observation_state = false;
bool debug_move_processing = false;
bool debug_edit_mode = false;
bool debug_scoring = false;
bool debug_protocol = false;
bool debug_match = false;
```

Add to Preferences dialog under new "Debug" section.

---

## SUMMARY COUNTS

Total statements found: ~336
- **KEEP**: ~50 (errors, critical state changes, user feedback)
- **REMOVE**: ~200 (completed debugging, routine operations)
- **SUPPRESS with flag**: ~86 (useful but verbose)

