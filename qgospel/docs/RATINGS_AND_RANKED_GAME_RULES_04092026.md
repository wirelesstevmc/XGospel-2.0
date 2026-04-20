# IGS Rating System and Ranked Game Rules Analysis
**Date:** April 9, 2026
**Research Topic:** Understanding IGS server rules for free vs rated games with handicap and unranked players

---

## Background

During development of xgospel2 v50_R13, we encountered unexpected behavior when trying to negotiate free handicap games with unranked players. Despite documentation suggesting Beginner Class (BC) games should be unrated, all test games with BC/NR players were created as rated games by the IGS server.

---

## Documentation Found on the Web

### Source 1: Sensei's Library - IGS Rating System
**URL:** https://senseis.xmp.net/?IGSRatingSystem

### Source 2: Google Search Results
**Query:** "IGS server rules regarding free games for handicap and rated or unranked players"

### Key Rules from Documentation:

#### Excluded Games (Unrated):
- Games played on boards other than 19x19
- Pair go games
- Games played in the "free play room"

#### Rank Difference Rules:
- Games between players with rank difference > 3 stones are generally not rated
- Games with correct handicap for ≤3 stones difference will be rated

#### **Provisional/Beginner Class Rules (CRITICAL):**
> "Games with players holding a provisional rating (e.g., '22k?') or those in the Beginner Class (BC) **do not count as rated games**, but the winner can gain 50 points, and the loser faces no penalty, provided the previous two games were not also non-rated."

#### Handicap Setup:
- Handicap games primarily used on 19x19 boards
- Generally up to 9 stones maximum
- Black player (weaker) places stones using "handicap" button or command

#### Unranked/New Player Status:
- **Provisional Ranks (?):** New players start with `?` after rank (e.g., `22k?`)
- **Beginner Class (BC):** Players with less than 5 wins out of 20 games
- Wins in any standard go game count toward increasing BC count

---

## Test Results from xgospel2

### Test Matrix:

| Player 1 | Player 2 | Handicap | Protocol | Result | Game Type |
|----------|----------|----------|----------|--------|-----------|
| thirdstone (NR/BC) | BusyBee (8K?) | 9 stones | match | Game created | **RATED** |
| thirdstone (NR/BC) | woodnstone (4d*) | 9 stones | match | Game created | **RATED** |
| thirdstone (NR/BC) | woodnstone (4d*) | 9 stones | nmatch (free requested) | Game created | **RATED** |
| weakkyu (7k*) | woodnstone (4d*) | 3 stones | nmatch (free requested) | Game created | **FREE** ✓ |

### Key Observations:

1. **All games with NR/BC player were RATED** despite documentation saying they should be unrated
2. **Server rejected `free` command** with message: `9 That command is currently disabled.`
3. **Two rated players CAN create free handicap games** using nmatch protocol with `free_param=1`
4. **Board size was always 19x19** and **handicap was proper** for rank difference (not the issue)

---

## Evidence: Screenshot Analysis

Three different clients were used to check thirdstone's rank designation:

### Screenshot 1: xgospel1 (Old Client)
**File:** `/tmp/Screenshot_2026-04-09_11-08-10.png`

**Shows:**
- `Stats of thirdstone[NR]`
- `Rating: NR    0`
- `Rated Games: 0`
- `Rank: NR    0`
- **Game History:** Multiple games vs BusyBee marked with `I` flag (RATED)
  - Example: `BusyBee [ 8k I(H) : thirdstone [ NR I(B) H 9 K -5.5 19x19`
  - The `I` flag indicates these are **rated games**

### Screenshot 2: xgospel2 (Our Client)
**File:** `/tmp/Screenshot_2026-04-09_11-10-48.png`

**Shows:**
- `Player: thirdstone [NR]`
- Header shows: `thirdstone [BC]`
- Wins: 1
- Losses: 7
- Account Settings toggle buttons visible

### Screenshot 3: GoPanda2 (Modern Client)
**File:** `/tmp/Screenshot_2026-04-09_11-13-52.png`

**Shows:**
- Stats dialog: `Stats for thirdstone`
- Rank: `BC`
- Win/loss: `1/7`
- Player list shows `thirdstone` with rank `BC`
- Multiple other players also show `BC` designation

---

## Critical Discovery

### NR vs BC Designation

**Finding:** `NR` and `BC` are the same thing, displayed differently by different clients:
- **Old clients (xgospel1):** Display as `NR` (Not Rated)
- **Modern clients (xgospel2, GoPanda2):** Display as `BC` (Beginner Class)
- **IGS server designation:** `BC` (Beginner Class)

**thirdstone's actual status:** Beginner Class (BC) with 1 win, 7 losses

### The Smoking Gun: Games ARE Rated

Looking at xgospel1's game history, **ALL games show the `I` flag**, which indicates **RATED games**:

```
BusyBee [ 8k I(H) : thirdstone [ NR I(B) H 9 K -5.5 19x19 HeResign 09-14-02 R
```

According to the documentation:
> "Games with players holding a provisional rating (e.g., '22k?') or those in the Beginner Class (BC) do not count as rated games"

**But IGS is marking them as `I` (rated)!**

This directly contradicts the Sensei's Library documentation.

---

## Analysis: Documentation vs Reality

### Hypothesis 1: Documentation is Outdated (Most Likely)

**Evidence supporting this:**
1. Sensei's Library page may reflect **old IGS policy** (pre-2020?)
2. IGS/Pandanet may have changed rules to improve ranking system
3. Modern policy: **Force new players to play rated games** to establish rank faster
4. Prevents "eternal beginners" who avoid ranking

**Possible policy change:**
- **Old rule:** BC games are unrated (free)
- **New rule:** BC games ARE rated (to build rating pool faster)

### Hypothesis 2: Special Rating Calculation vs Unrated

The documentation mentions:
> "the winner can gain 50 points, and the loser faces no penalty"

**Possible interpretation:**
- Games show as "Rated" (`I` flag) in game lists
- But use **special 50-point calculation** instead of normal ELO
- So they're "rated" but with simplified math
- Game is still marked as `I` (rated) because it affects ratings

**This could mean:**
- BC games ARE rated (contra documentation saying "do not count as rated")
- But use different calculation than standard ELO
- Documentation is technically wrong about "not rated"

### Hypothesis 3: Anti-Sandbagging Policy

**Why IGS might force BC games to be rated:**
- Prevents strong players from creating BC accounts to manipulate rankings
- Prevents abuse of "free game" system
- Forces all players into rating pool for integrity
- Accelerates rank establishment for legitimate new players

---

## Server Behavior: Actual Rules (Based on Testing)

Based on our testing, IGS appears to enforce these rules:

```
IF (either player is BC/NR/unranked) THEN
    game_type = RATED (forced, cannot be overridden by client)
    message = "That command is currently disabled" (if client tries to send "free")
ELSE IF (both players have established ratings * AND nmatch free_param=1) THEN
    game_type = FREE
ELSE
    game_type = RATED (default)
END
```

### Rank Status Types (from testing):
- **BC (Beginner Class):** < 5 wins out of games played, displayed as "NR" in old clients
- **? (Provisional):** Declared rank, not yet established (e.g., "8K?")
- **\* (Established):** Earned rating from 20+ games (e.g., "7k*", "4d*")

### Free Game Rules (from testing):
- **BC player + any opponent:** FORCED RATED (cannot be free)
- **? player + any opponent:** FORCED RATED (not tested, but likely)
- **\* player + \* player + nmatch:** CAN BE FREE (if both request it)
- **\* player + \* player + old match:** RATED (no free parameter in old protocol)

---

## Verification Methods

To definitively determine which hypothesis is correct:

### Method 1: Check Rating Point Changes
1. Note thirdstone's current rating (if BC shows numeric rating)
2. Play a game and **win**
3. Check rating increase:
   - **Exactly 50 points** → Special BC rating system active (Hypothesis 2)
   - **Different amount** → Normal ELO, documentation outdated (Hypothesis 1)

### Method 2: Query IGS Server Directly
```
help rated
help bc
help beginner
help free
```

Type these commands on IGS to see current official documentation.

### Method 3: Ask IGS Administration
```
tell admin [question about BC rating rules]
```

Contact IGS staff for clarification on current BC game rating policy.

### Method 4: Test Different Game Start Methods
Instead of using `match` command, try:
- Manual game setup (if IGS still supports it)
- Different rooms on IGS (teaching room, etc.)
- Teaching game mode

### Method 5: True BC vs NR Test
Create/use an account that is **explicitly BC** with some games played:
- Play 10+ games with < 5 wins to ensure BC status
- Test if this behaves differently than thirdstone
- Compare with brand new account (0 games)

---

## Conclusions

### What We Know for Certain:

1. ✅ **BC and NR are the same designation** (old vs new client display)
2. ✅ **thirdstone is Beginner Class (BC)** with 1 win, 7 losses
3. ✅ **All BC games showed `I` (rated) flag** in game history
4. ✅ **IGS rejects `free` command for BC players** ("That command is currently disabled")
5. ✅ **Two established-rating players CAN create free handicap games** via nmatch
6. ✅ **xgospel2 client is working correctly** - this is IGS server policy

### What We Suspect:

1. **Sensei's Library documentation is OUTDATED** (most likely)
2. **Modern IGS forces BC games to be rated** (policy change)
3. **BC games may use special 50-point calculation** (but still marked as "rated")
4. **Anti-sandbagging measure** to prevent rank manipulation

### What Needs Further Testing:

1. ❓ Do BC games use 50-point special calculation or normal ELO?
2. ❓ Can BC players create free games through other methods (teaching room, etc.)?
3. ❓ Do provisional (?) players have same restrictions as BC?
4. ❓ What is IGS's current official documentation on BC rating rules?
5. ❓ When did IGS change this policy (if it changed)?

---

## Implications for xgospel2 Development

### Current Status: ✅ Client Working Correctly

The xgospel2 client is properly implementing IGS protocol:
- Match/nmatch acceptance works correctly
- Server's suggested commands are used
- Free game requests are sent properly via nmatch `free_param=1`
- Server's rejection of free games is an IGS policy, not client bug

### No Code Changes Needed

The AUTO-FREE feature was correctly disabled because:
- IGS rejects the `free` command for BC players
- nmatch protocol handles free/rated via parameter, not post-game command
- Server policy overrides client requests for BC games

### Documentation Recommendations

1. **Update user documentation** to reflect IGS's actual BC game policy
2. **Note that BC/NR players cannot create free handicap games** (IGS policy)
3. **Recommend nmatch protocol** for players wanting free handicap games
4. **Explain rank status types:** BC/NR, ?, and *

---

## Appendix: Client Compatibility Notes

### Tested Clients:

| Client | BC Display | Match Protocol | Free Games Support |
|--------|------------|----------------|-------------------|
| xgospel1 (old) | Shows as "NR" | Old match only | N/A (no free option) |
| xgospel2 (ours) | Shows as "BC" | Both match/nmatch | ✓ nmatch free_param |
| q5Go | Shows as "BC" | Both match/nmatch | ✓ Free game checkbox |
| GoPanda2 | Shows as "BC" | Both match/nmatch | ✓ Free game option |

### Protocol Differences:

**Old Match Protocol:**
```
match opponent color boardsize time byotime
```
- No free/rated parameter in command
- Requires post-game `free` command to change status
- IGS rejects `free` command for BC players

**New Nmatch Protocol:**
```
nmatch opponent color handicap komi boardsize time_sec byo_sec byo_stones free_param reserved1 reserved2
```
- Parameter 8 (9th position): `0`=rated, `1`=free
- Both players must send `free_param=1` for free game
- IGS still overrides to rated if either player is BC

---

## References

1. Sensei's Library - IGS Rating System: https://senseis.xmp.net/?IGSRatingSystem
2. Pandanet-IGS official site (various pages on rating rules)
3. xgospel2 test results - v50_R13 (April 8-9, 2026)
4. Screenshots from xgospel1, xgospel2, GoPanda2 (April 9, 2026)

---

## Future Research Questions

1. Has IGS published updated BC rating documentation anywhere?
2. When did the BC rating policy change (if it changed)?
3. Are there any IGS forums or mailing lists discussing this?
4. Do other Go servers (KGS, OGS) have similar BC restrictions?
5. What is the exact rating calculation for BC games (50-point special or normal ELO)?

---

**Document Status:** Research in progress
**Last Updated:** April 9, 2026
**Next Steps:** Verify rating point changes, query IGS server for official docs, test with true BC account
