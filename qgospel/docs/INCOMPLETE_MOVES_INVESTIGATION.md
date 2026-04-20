# Investigation: Incomplete Move History Issue

## Problem Summary
xgospel2 is receiving incomplete move history from the `moves <game#>` command response, specifically missing moves 22-55 in the current observation, causing board position to be out of sync with q5Go.

## Current Observation Pattern
- **Received**: Moves 56-57 (current live moves), Move 0 (handicap), Moves 1-21 (historical)
- **Missing**: Moves 22-55 (33 consecutive moves)
- **Expected**: Complete sequence from 0-57

## Root Cause Analysis

### 1. TCP Data Reception Issue (PRIMARY SUSPECT)
**Location**: `xgospel2_fixed.cpp:1044-1047`
```cpp
QByteArray data = socket->readAll();
QString text = QString::fromUtf8(data);
for (const QString& line : text.split('\n')) {
```

**Issue**: `socket->readAll()` may not capture complete multi-line IGS response if it arrives in multiple TCP packets. The `moves` command response contains 55+ lines of move data, which likely exceeds single TCP packet size.

**Evidence**:
- Large `moves` response (55+ moves) could arrive across multiple `onDataReceived()` calls
- Each call only processes `readAll()` from current packet
- Missing moves are consecutive (22-55), suggesting partial response processing
- User confirmed: "This should not happen if we are executing properly processing the command sequence"

### 2. Missing Response Buffering
**Current Implementation**: Immediate line-by-line processing without buffering
**Problem**: Multi-packet responses get truncated between `onDataReceived()` calls

### 3. Command Timing Verification
**Current Sequence** (xgospel2_fixed.cpp:2290-2300):
```cpp
// observe command
QTimer::singleShot(1000, this, [this, game_id]() {
    QString games_cmd = QString("games %1").arg(game_id);
    // games command for komi
});

QTimer::singleShot(1200, this, [this, game_id]() {
    QString moves_cmd = QString("moves %1").arg(game_id);
    // moves command for history - INCOMPLETE RESPONSE
});
```

**Analysis**: Timing appears correct, but response handling is insufficient.

## Evidence from Debug Output
- Move parsing shows gaps: receives 0-21, then jumps to 56-57
- No error messages indicating parsing failures
- All received moves process correctly
- Board reconstruction only shows partial game state

## Recommended Solutions

### Solution 1: Implement Response Buffering (CRITICAL)
Buffer incoming data until complete response received:
```cpp
private:
    QByteArray response_buffer;
    bool awaiting_moves_response;
    QString moves_end_marker; // "Command completed" or similar

void onDataReceived() {
    QByteArray data = socket->readAll();
    response_buffer.append(data);
    
    if (awaiting_moves_response) {
        // Check if complete response received
        if (response_buffer.contains(moves_end_marker)) {
            processMoveHistory(response_buffer);
            response_buffer.clear();
            awaiting_moves_response = false;
        }
        return; // Don't process line-by-line yet
    }
    
    // Normal processing for non-moves responses
    processLines(QString::fromUtf8(data));
}
```

### Solution 2: Add Refresh Button (WORKAROUND)
Immediate fix: Add manual refresh to re-request moves when board is out of sync.

### Solution 3: Response Validation
Add validation to detect incomplete responses:
```cpp
// After moves command, verify we received expected move count
if (received_move_count < expected_move_count) {
    // Re-request moves command
    resendMovesCommand(game_id);
}
```

## Impact Assessment
- **Severity**: High - Prevents accurate board position synchronization
- **Frequency**: Occurs on games with >25 moves (most real games)
- **User Experience**: Board shows wrong position compared to q5Go reference
- **SGF Generation**: Produces incomplete/incorrect game records

## Testing Requirements
1. Observe game with 50+ moves
2. Verify all moves 1-N received in debug output
3. Confirm board position matches q5Go
4. Test SGF export for completeness

## Files Requiring Changes
- `xgospel2_fixed.cpp`: Implement response buffering in onDataReceived
- `board_window.h`: Add refresh button UI element  
- `board_window.cpp`: Add refresh functionality to re-request moves

## Priority
**CRITICAL** - This is the core synchronization issue preventing xgospel2 from matching q5Go board positions.