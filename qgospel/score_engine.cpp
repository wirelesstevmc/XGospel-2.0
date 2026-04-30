/* Score engine for xgospel2.
   Core algorithm ported from q5Go's go_board scoring (goboard.cc / goboard.h),
   authored by Bernd Schmidt <bernds_cb1@t-online.de>, licensed GPLv3+.
   Both the simple and complex scoring paths are included.
   Benson's algorithm is ported but currently gated behind a #if 0 matching q5Go upstream. */

#include "score_engine.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <vector>
#include <map>

// ---------------------------------------------------------------------------
// Anonymous namespace — all ported q5Go internals live here
// ---------------------------------------------------------------------------
namespace {

// ---- bit_array (verbatim from q5Go bitarray.h, minus debug/grow/extract/insert) ----

inline unsigned ba_popcnt(uint64_t val) {
    unsigned cnt = 0;
    while (val != 0) { val &= val - 1; cnt++; }
    return cnt;
}

class bit_array {
    unsigned m_n_bits;
    int m_n_elts;
    uint64_t *m_bits;
    uint64_t m_last_mask;

    void calc_mask(unsigned sz) {
        uint64_t mask = 0;
        mask = ~mask;
        mask >>= 63 - (sz + 63) % 64;
        m_last_mask = mask;
    }
public:
    bit_array(unsigned sz, bool set = false)
        : m_n_bits(sz), m_n_elts((sz + 63) / 64), m_bits(new uint64_t[m_n_elts]())
    {
        calc_mask(sz);
        if (set && m_n_elts > 0) {
            memset(m_bits, -1, m_n_elts * sizeof(uint64_t));
            m_bits[m_n_elts - 1] &= m_last_mask;
        }
    }
    bit_array(const bit_array &other)
        : m_n_bits(other.m_n_bits), m_n_elts(other.m_n_elts),
          m_bits(new uint64_t[m_n_elts]), m_last_mask(other.m_last_mask)
    {
        memcpy(m_bits, other.m_bits, m_n_elts * sizeof(uint64_t));
    }
    bit_array(bit_array &&other) noexcept
        : m_n_bits(other.m_n_bits), m_n_elts(other.m_n_elts),
          m_bits(other.m_bits), m_last_mask(other.m_last_mask)
    {
        other.m_n_bits = 0; other.m_n_elts = 0; other.m_bits = nullptr;
    }
    ~bit_array() { delete[] m_bits; }

    bit_array &operator=(bit_array other) {
        std::swap(m_n_elts, other.m_n_elts);
        std::swap(m_n_bits, other.m_n_bits);
        std::swap(m_bits, other.m_bits);
        std::swap(m_last_mask, other.m_last_mask);
        return *this;
    }
    bool operator==(const bit_array &other) const {
        if (other.m_n_bits != m_n_bits) return false;
        return memcmp(m_bits, other.m_bits, m_n_elts * sizeof(uint64_t)) == 0;
    }
    bool operator!=(const bit_array &other) const { return !(*this == other); }

    unsigned bitsize() const { return m_n_bits; }

    void clear() { for (int i = 0; i < m_n_elts; i++) m_bits[i] = 0; }

    bool test_bit(unsigned n) const {
        if (n >= m_n_bits) return false;
        return (m_bits[n / 64] >> (n % 64)) & 1;
    }
    void set_bit(unsigned n) {
        if (n >= m_n_bits) return;
        m_bits[n / 64] |= (uint64_t)1 << (n % 64);
    }
    void clear_bit(unsigned n) {
        if (n >= m_n_bits) return;
        m_bits[n / 64] &= ~((uint64_t)1 << (n % 64));
    }

    // Returns true if every set bit in *this is also set in other.
    bool subset_of(const bit_array &other) const {
        int limit = std::min(m_n_elts, other.m_n_elts);
        int i;
        for (i = 0; i < limit; i++)
            if (m_bits[i] & ~other.m_bits[i]) return false;
        for (; i < m_n_elts; i++)
            if (m_bits[i] != 0) return false;
        return true;
    }

    bool ior(const bit_array &other, int shift, const bit_array &mask) {
        if (shift == 0) return ior(other);
        shift = -shift;
        shift += other.m_n_elts * 64;
        int wordshift = (shift + 63) / 64 - other.m_n_elts - 1;
        int bitshift  = shift % 64;
        uint64_t last = 0;
        if (wordshift >= 0 && wordshift < other.m_n_elts) last = other.m_bits[wordshift];
        if (wordshift >= 0 && wordshift < mask.m_n_elts)  last &= mask.m_bits[wordshift];
        bool changed = false;
        for (int i = 0; i < m_n_elts; i++) {
            wordshift++;
            uint64_t curr = 0;
            if (wordshift >= 0 && wordshift < other.m_n_elts) curr = other.m_bits[wordshift];
            if (wordshift >= 0 && wordshift < mask.m_n_elts)  curr &= mask.m_bits[wordshift];
            uint64_t val = m_bits[i], val1 = val;
            if (bitshift != 0) { val |= curr << (64 - bitshift); val |= last >> bitshift; }
            else                { val |= curr; }
            if (i + 1 == m_n_elts) val &= m_last_mask;
            changed |= val != val1;
            m_bits[i] = val;
            last = curr;
        }
        return changed;
    }
    bool ior(const bit_array &other, int shift) { return ior(other, shift, other); }
    bool ior(const bit_array &other) {
        int limit = std::min(m_n_elts, other.m_n_elts);
        bool changed = false;
        for (int i = 0; i < limit; i++) {
            uint64_t val = m_bits[i], val1 = val;
            val |= other.m_bits[i];
            if (i + 1 == m_n_elts) val &= m_last_mask;
            changed |= val != val1;
            m_bits[i] = val;
        }
        return changed;
    }
    bool and1(const bit_array &other) {
        int limit = std::min(m_n_elts, other.m_n_elts);
        bool changed = false;
        for (int i = 0; i < limit; i++) {
            uint64_t val = m_bits[i], val1 = val;
            val &= other.m_bits[i];
            changed |= val != val1;
            m_bits[i] = val;
        }
        return changed;
    }
    bool andnot(const bit_array &other) {
        unsigned limit = std::min(m_n_elts, other.m_n_elts);
        bool changed = false;
        for (unsigned i = 0; i < limit; i++) {
            uint64_t val = m_bits[i], val1 = val;
            val &= ~other.m_bits[i];
            changed |= val != val1;
            m_bits[i] = val;
        }
        return changed;
    }
    bool intersect_p(const bit_array &other) const {
        unsigned limit = std::min(m_n_elts, other.m_n_elts);
        for (unsigned i = 0; i < limit; i++)
            if (m_bits[i] & other.m_bits[i]) return true;
        return false;
    }

    unsigned popcnt() const {
        unsigned cnt = 0;
        for (int i = 0; i < m_n_elts; i++) cnt += ba_popcnt(m_bits[i]);
        return cnt;
    }
    unsigned ffs(int test = 0) const {
        int elt = test / 64, bitpos = test % 64;
        uint64_t mask = (uint64_t)1 << bitpos;
        while (elt < m_n_elts) {
            uint64_t v = m_bits[elt];
            while (mask != 0) { if (v & mask) return test; mask <<= 1; test++; }
            mask = 1; elt++;
        }
        return m_n_bits;
    }
    unsigned ffz(int test = 0) const {
        int elt = test / 64, bitpos = test % 64;
        uint64_t mask = (uint64_t)1 << bitpos;
        while (elt < m_n_elts) {
            uint64_t v = m_bits[elt];
            while (mask != 0) { if (!(v & mask)) return test; mask <<= 1; test++; }
            mask = 1; elt++;
        }
        return m_n_bits;
    }
};

// ---- Precomputed edge masks (one per board size, cached statically) ----

static std::map<std::pair<int,int>, bit_array*> s_mask_left;
static std::map<std::pair<int,int>, bit_array*> s_mask_right;

static const bit_array *get_mask_left(int w, int h) {
    auto key = std::make_pair(w, h);
    auto it = s_mask_left.find(key);
    if (it != s_mask_left.end()) return it->second;
    bit_array *m = new bit_array(w * h, true);
    for (int i = 0; i < h; i++) m->clear_bit(i * w);
    s_mask_left[key] = m;
    return m;
}
static const bit_array *get_mask_right(int w, int h) {
    auto key = std::make_pair(w, h);
    auto it = s_mask_right.find(key);
    if (it != s_mask_right.end()) return it->second;
    bit_array *m = new bit_array(w * h, true);
    for (int i = 0; i < h; i++) m->clear_bit(i * w + w - 1);
    s_mask_right[key] = m;
    return m;
}

// ---- Simplified go_board (scoring only) ----

enum sc_color { sc_none, sc_black, sc_white };
enum class sc_mark { none = 0, dead, terr, seki, falseeye };

struct stone_unit {
    bit_array m_stones;
    short     m_n_vital;       // Benson vital region count (>= 2 → pass-alive)
    bool      m_alive;
    bool      m_seki;
    bool      m_any_terr, m_real_terr, m_seki_neighbour;
    stone_unit(const bit_array &s)
        : m_stones(s), m_n_vital(0), m_alive(true), m_seki(false),
          m_any_terr(false), m_real_terr(false), m_seki_neighbour(false) {}
};

struct terr_unit {
    bit_array m_terr;
    bool m_nb_w, m_nb_b, m_contains_dead;
    terr_unit(const bit_array &t, bool nw, bool nb, bool cd)
        : m_terr(t), m_nb_w(nw), m_nb_b(nb), m_contains_dead(cd) {}
};

class ScoreBoard {
    int m_sz;
    const bit_array *m_mask_left;
    const bit_array *m_mask_right;

    bit_array m_stones_b;
    bit_array m_stones_w;

    std::vector<stone_unit> m_units_b;
    std::vector<stone_unit> m_units_w;
    std::vector<terr_unit>  m_units_t;
    std::vector<terr_unit>  m_units_st;

    std::vector<sc_mark>   m_marks;
    std::vector<int>        m_mark_extra;  // 0=white terr, 1=black terr

    int m_score_b = 0;
    int m_score_w = 0;

public:
    ScoreBoard(int sz)
        : m_sz(sz),
          m_mask_left(get_mask_left(sz, sz)),
          m_mask_right(get_mask_right(sz, sz)),
          m_stones_b(sz * sz),
          m_stones_w(sz * sz)
    {}

    unsigned bitsize() const { return m_sz * m_sz; }
    int bitpos(int x, int y) const { return x + y * m_sz; }

    void set_stone(int x, int y, sc_color col) {
        int bp = bitpos(x, y);
        if (col != sc_white) m_stones_w.clear_bit(bp);
        if (col != sc_black) m_stones_b.clear_bit(bp);
        if (col == sc_white) m_stones_w.set_bit(bp);
        else if (col == sc_black) m_stones_b.set_bit(bp);
    }

    sc_color stone_at(int x, int y) const {
        int bp = bitpos(x, y);
        if (m_stones_b.test_bit(bp)) return sc_black;
        if (m_stones_w.test_bit(bp)) return sc_white;
        return sc_none;
    }

    sc_mark mark_at(int x, int y) const {
        if (m_marks.empty()) return sc_mark::none;
        return m_marks[bitpos(x, y)];
    }
    int mark_extra_at(int x, int y) const {
        if (m_marks.empty()) return 0;
        return m_mark_extra[bitpos(x, y)];
    }

    int score_b() const { return m_score_b; }
    int score_w() const { return m_score_w; }

    void identify_units();
    void calc_scoring_markers_simple();
    void calc_scoring_markers_complex();

private:
    // ---- flood-fill primitives ----
    void flood_step(bit_array &next, const bit_array &fill) {
        next.ior(fill, -1, *m_mask_left);
        next.ior(fill, 1, *m_mask_right);
        next.ior(fill, m_sz);
        next.ior(fill, -m_sz);
    }
    void flood_fill(bit_array &fill, const bit_array &boundary) {
        bit_array next(fill);
        for (;;) {
            flood_step(next, fill);
            next.andnot(boundary);
            if (next == fill) break;
            fill = next;
        }
    }
    bit_array init_fill(int bp, const bit_array &bounds, bool in) {
        bit_array fill(bitsize());
        int rem_len = m_sz - bp % m_sz;
        for (unsigned y = bp; y < bitsize(); y += m_sz) {
            if (bounds.test_bit(y) != in) break;
            for (int i = 0; i < rem_len; i++) {
                if (bounds.test_bit(y + i) != in) break;
                fill.set_bit(y + i);
            }
        }
        return fill;
    }
    void scoring_flood_fill(bit_array &fill,
                            const bit_array &w_stones, const bit_array &b_stones,
                            bool &nb_w, bool &nb_b)
    {
        bit_array next(fill);
        for (;;) {
            flood_step(next, fill);
            if (next.intersect_p(w_stones)) nb_w = true;
            if (next.intersect_p(b_stones)) nb_b = true;
            next.andnot(w_stones);
            next.andnot(b_stones);
            if (next == fill) break;
            fill = next;
        }
    }
    void init_marks(bool do_clear) {
        if (m_marks.empty()) {
            m_marks.resize(bitsize(), sc_mark::none);
            m_mark_extra.resize(bitsize(), 0);
            do_clear = true;
        }
        if (do_clear) {
            std::fill(m_marks.begin(), m_marks.end(), sc_mark::none);
            std::fill(m_mark_extra.begin(), m_mark_extra.end(), 0);
        }
    }

    // ---- territory helpers ----
    void find_territory_units(const bit_array &w_stones, const bit_array &b_stones);
    void finish_scoring_markers(const bit_array *do_not_count = nullptr);

    // ---- Benson's algorithm helpers ----
    struct enclosed_area {
        bit_array area;
        bit_array border;
        enclosed_area(const bit_array &a, const bit_array &b) : area(a), border(b) {}
        enclosed_area(bit_array &&a, bit_array &&b) : area(std::move(a)), border(std::move(b)) {}
    };
    std::vector<enclosed_area> find_eas(const bit_array &stones, const bit_array &other_stones);
    void benson(std::vector<stone_unit> &units, const bit_array &other_stones);
};

void ScoreBoard::identify_units() {
    m_units_w.clear();
    m_units_b.clear();
    bit_array handled(bitsize());
    for (int y = 0; y < m_sz; y++) {
        for (int x = 0; x < m_sz; x++) {
            int i = bitpos(x, y);
            if (handled.test_bit(i)) continue;
            const bit_array *stones;
            sc_color col;
            if (m_stones_w.test_bit(i))      { col = sc_white; stones = &m_stones_w; }
            else if (m_stones_b.test_bit(i)) { col = sc_black; stones = &m_stones_b; }
            else continue;

            bit_array unit = init_fill(i, *stones, true);
            bit_array next(unit);
            for (;;) {
                flood_step(next, unit);
                next.and1(*stones);
                if (next == unit) break;
                unit = next;
            }
            handled.ior(unit);
            std::vector<stone_unit> &units = (col == sc_black) ? m_units_b : m_units_w;
            units.emplace_back(next);
        }
    }
}

void ScoreBoard::find_territory_units(const bit_array &w_stones, const bit_array &b_stones) {
    m_units_t.clear();
    m_units_st.clear();
    bit_array handled(w_stones);
    handled.ior(b_stones);
    bit_array dead_stones = m_stones_w;
    dead_stones.ior(m_stones_b);
    dead_stones.andnot(w_stones);
    dead_stones.andnot(b_stones);

    init_marks(false);
    for (unsigned i = 0; i < bitsize(); i++) {
        i = handled.ffz(i);
        if (i == bitsize()) break;
        bit_array fill(bitsize());
        fill.set_bit(i);
        bool nb_w = false, nb_b = false;
        scoring_flood_fill(fill, w_stones, b_stones, nb_w, nb_b);
        if (nb_w != nb_b)
            m_units_t.emplace_back(fill, nb_w, nb_b, fill.intersect_p(dead_stones));
        else
            m_units_st.emplace_back(fill, nb_w, nb_b, false);
        handled.ior(fill);
    }
    for (unsigned i = 0; i < bitsize(); i++) {
        i = dead_stones.ffs(i);
        if (i == bitsize()) break;
        m_marks[i] = sc_mark::dead;
    }
}

void ScoreBoard::finish_scoring_markers(const bit_array *do_not_count) {
    bit_array terr_w(bitsize());
    bit_array terr_b(bitsize());
    for (const auto &t : m_units_t) {
        bool counted = true;
        if (do_not_count && t.m_terr.intersect_p(*do_not_count))
            counted = false;
        if (counted) {
            if (t.m_nb_b) terr_b.ior(t.m_terr);
            else          terr_w.ior(t.m_terr);
        }
    }
    m_score_w += terr_w.popcnt();
    m_score_b += terr_b.popcnt();
    for (unsigned i = 0; i < bitsize(); i++) {
        if (terr_w.test_bit(i)) { m_marks[i] = sc_mark::terr; m_mark_extra[i] = 0; }
        if (terr_b.test_bit(i)) { m_marks[i] = sc_mark::terr; m_mark_extra[i] = 1; }
    }
}

void ScoreBoard::calc_scoring_markers_simple() {
    init_marks(true);
    m_score_b = m_score_w = 0;
    bit_array w_stones(bitsize());
    bit_array b_stones(bitsize());
    for (auto &it : m_units_w) {
        it.m_any_terr = it.m_real_terr = it.m_seki_neighbour = false;
        if (it.m_alive) w_stones.ior(it.m_stones);
    }
    for (auto &it : m_units_b) {
        it.m_any_terr = it.m_real_terr = it.m_seki_neighbour = false;
        if (it.m_alive) b_stones.ior(it.m_stones);
    }
    find_territory_units(w_stones, b_stones);
    finish_scoring_markers(nullptr);
    m_units_t.clear();
    m_units_st.clear();
}

// ---- Benson's algorithm ----
// Enclosed areas: flood-fill through space not containing stones of 'stones' color,
// then strip opposite-color stones.  Returns (area, border) pairs.
std::vector<ScoreBoard::enclosed_area>
ScoreBoard::find_eas(const bit_array &stones, const bit_array &other_stones)
{
    std::vector<enclosed_area> ea;
    bit_array handled = stones;
    for (unsigned i = 0; i < bitsize(); i++) {
        i = handled.ffz(i);
        if (i == bitsize()) break;

        bit_array fill = init_fill(i, stones, false);
        flood_fill(fill, stones);

        bit_array border(fill);
        flood_step(border, fill);
        border.andnot(fill);
        fill.andnot(other_stones);

        handled.ior(fill);
        ea.emplace_back(std::move(fill), std::move(border));
    }
    return ea;
}

// Benson's algorithm: sets m_n_vital on each unit.  If m_n_vital >= 2 the unit
// is unconditionally alive (pass-alive).  Currently used only to seed seki detection
// in calc_scoring_markers_complex; the Benson call itself is gated behind #if 0
// matching q5Go upstream behavior.
void ScoreBoard::benson(std::vector<stone_unit> &units, const bit_array &other_stones)
{
    std::vector<bit_array> unit_liberties;
    std::vector<size_t> tentative;
    tentative.reserve(units.size());
    unit_liberties.reserve(units.size());

    for (const auto &it : units) {
        unit_liberties.emplace_back(bitsize());
        bit_array &liberties = unit_liberties.back();
        flood_step(liberties, it.m_stones);
        liberties.andnot(m_stones_w);
        liberties.andnot(m_stones_b);
        tentative.push_back(tentative.size());
    }
    bit_array stones(bitsize());
    for (auto it : tentative) stones.ior(units[it].m_stones);
    auto eas = find_eas(stones, other_stones);

    for (;;) {
        bool changed = false;
        for (auto it : tentative) units[it].m_n_vital = 0;
        for (const auto &ea : eas) {
            for (auto idx : tentative) {
                if (ea.area.subset_of(unit_liberties[idx]))
                    units[idx].m_n_vital++;
            }
        }
        tentative.erase(
            std::remove_if(tentative.begin(), tentative.end(),
                           [&](size_t idx) {
                               bool remove = units[idx].m_n_vital < 2;
                               changed |= remove;
                               return remove;
                           }),
            tentative.end());
        stones.clear();
        for (auto it : tentative) stones.ior(units[it].m_stones);

        eas.erase(
            std::remove_if(eas.begin(), eas.end(),
                           [&](enclosed_area &ea) {
                               bool remove = !ea.border.subset_of(stones);
                               changed |= remove;
                               return remove;
                           }),
            eas.end());

        if (!changed) break;
    }
}

// ---- Complex scoring ----
// Ported from q5Go go_board::calc_scoring_markers_complex().
// Key differences from the q5Go original:
//   1. After the false-eye removal loop we rebuild cand_territory from actual unit
//      contents.  This prevents points stripped mid-loop from surviving as
//      phantom disputed points — the 1-point discrepancy visible in q5Go.
//   2. The #if 0 Benson call and the #if 0 seki-neighbour detection block are
//      preserved as-is from q5Go (not activated) to match upstream behavior.
void ScoreBoard::calc_scoring_markers_complex()
{
    init_marks(true);
    m_score_b = 0;
    m_score_w = 0;

    std::vector<stone_unit *> live_units;

    bit_array w_stones(bitsize());
    bit_array b_stones(bitsize());
    bit_array dead_stones(bitsize());

    for (auto &it : m_units_w) {
        it.m_any_terr = it.m_real_terr = it.m_seki_neighbour = false;
        if (it.m_alive) {
            w_stones.ior(it.m_stones);
            live_units.push_back(&it);
        } else {
            dead_stones.ior(it.m_stones);
        }
    }
    for (auto &it : m_units_b) {
        it.m_any_terr = it.m_real_terr = it.m_seki_neighbour = false;
        if (it.m_alive) {
            b_stones.ior(it.m_stones);
            live_units.push_back(&it);
        } else {
            dead_stones.ior(it.m_stones);
        }
    }
    find_territory_units(w_stones, b_stones);

#if 0
    // Benson's algorithm — not activated, matching q5Go upstream.
    benson(m_units_w, m_stones_b);
    benson(m_units_b, m_stones_w);
#endif

    // Collect candidate territory (all single-color adjacent regions).
    bit_array cand_territory(bitsize());
    for (const auto &t : m_units_t)
        cand_territory.ior(t.m_terr);

    bit_array false_eyes(bitsize());

    // Detect false eyes: a live unit that borders exactly one candidate territory
    // point, and that point has liberties outside the unit, is a false eye.
    // Iterate until no new false eyes are found.
    for (;;) {
        bool changed = false;
        for (const auto it : live_units) {
            bit_array borders(bitsize());
            flood_step(borders, it->m_stones);
            borders.and1(cand_territory);
            if (borders.popcnt() != 1) continue;

            bit_array b_libs(bitsize());
            flood_step(b_libs, borders);
            b_libs.andnot(it->m_stones);
            if (b_libs.popcnt() == 0) continue;

            // This single bordering point is a false eye.
            false_eyes.ior(borders);
            for (auto &t : m_units_t) {
                if (!t.m_terr.intersect_p(borders)) continue;
                t.m_terr.andnot(borders);
                // Mark changed regardless of whether the unit became empty.
                // (q5Go only set changed when the unit went to zero, which left
                // trimmed points stranded in cand_territory for the next
                // iteration, causing the 1-point disputed-territory error.)
                changed = true;
            }
        }
        if (!changed) break;

        // Rebuild cand_territory from actual unit contents after each pass.
        // This ensures points removed mid-loop cannot persist as phantom candidates.
        cand_territory.clear();
        for (const auto &t : m_units_t)
            cand_territory.ior(t.m_terr);
    }

    // Mark false-eye points so the caller can display them distinctly.
    for (unsigned i = 0; i < bitsize(); i++)
        if (false_eyes.test_bit(i))
            m_marks[i] = sc_mark::falseeye;

    bit_array real_territory(bitsize());
    bit_array nonseki_stones(bitsize());

    // Territory containing dead stones is "real"; propagate liveness from it.
    for (const auto &t : m_units_t) {
        if (t.m_terr.popcnt() == 0) continue;   // may have been emptied by false-eye pass

        bit_array borders(bitsize());
        flood_step(borders, t.m_terr);

        if (t.m_contains_dead)
            real_territory.ior(t.m_terr);

        if (t.m_nb_b) {
            for (auto &it : m_units_b) {
                if (!it.m_alive || !it.m_stones.intersect_p(borders)) continue;
                it.m_any_terr = true;
            }
        } else {
            for (auto &it : m_units_w) {
                if (!it.m_alive || !it.m_stones.intersect_p(borders)) continue;
                it.m_any_terr = true;
            }
        }
    }

    // Seki stones: any unit with m_seki flag set (user-toggled; Benson gate is off).
    bit_array seki_stones(bitsize());
    for (auto it : live_units)
        if (it->m_seki)
            seki_stones.ior(it->m_stones);
    for (unsigned i = 0; i < bitsize(); i++)
        if (seki_stones.test_bit(i))
            m_marks[i] = sc_mark::seki;

    // Propagate: units bordering real territory are not in seki; their neighbouring
    // empty points are also real territory.  Iterate until stable.
    for (;;) {
        bool changed = false;
        for (auto it : live_units) {
            if (it->m_real_terr) continue;
            bit_array borders(bitsize());
            flood_step(borders, it->m_stones);
            if (borders.intersect_p(real_territory)) {
                nonseki_stones.ior(it->m_stones);
                it->m_real_terr = true;
                changed = true;
            }
        }
        if (!changed) break;

        for (auto &t : m_units_t) {
            if (t.m_contains_dead) continue;
            bit_array borders(bitsize());
            flood_step(borders, t.m_terr);
            if (borders.intersect_p(nonseki_stones)) {
                real_territory.ior(t.m_terr);
                t.m_contains_dead = true;  // repurposed as "confirmed real" flag
                changed = true;
            }
        }
        if (!changed) break;
    }

    // Build the exclusion set: points adjacent to seki-flagged units.
    // The #if 0 block (seki_neighbour detection) matches q5Go upstream.
    bit_array seki_neighbours(bitsize());
#if 0
    for (const auto &it : m_units_w) {
        if (it.m_any_terr || !it.m_alive) continue;
        bit_array nb(bitsize());
        flood_step(nb, it.m_stones);
        for (auto &nit : m_units_b)
            if (nit.m_alive && !nit.m_real_terr && nb.intersect_p(nit.m_stones))
                nit.m_seki_neighbour = true;
    }
    for (const auto &it : m_units_b) {
        if (it.m_any_terr || !it.m_alive) continue;
        bit_array nb(bitsize());
        flood_step(nb, it.m_stones);
        for (auto &nit : m_units_w)
            if (nit.m_alive && !nit.m_real_terr && nb.intersect_p(nit.m_stones))
                nit.m_seki_neighbour = true;
    }
    for (const auto it : live_units)
        if (it->m_seki_neighbour)
            flood_step(seki_neighbours, it->m_stones);
#else
    for (const auto it : live_units)
        if (it->m_seki)
            flood_step(seki_neighbours, it->m_stones);
#endif

    finish_scoring_markers(&seki_neighbours);
    m_units_t.clear();
    m_units_st.clear();
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// ScoreEngine::estimate — public bridge
// ---------------------------------------------------------------------------

void ScoreEngine::estimate(const GoBoard &board,
                           QMap<QPair<int,int>, StoneColor> &territory_out,
                           QSet<QPair<int,int>>             &dead_stones_out,
                           QSet<QPair<int,int>>             &disputed_out,
                           int &black_score,
                           int &white_score,
                           ScoringMethod method)
{
    const int SZ = 19;
    ScoreBoard sb(SZ);

    // Load all stones from GoBoard into ScoreBoard
    for (int x = 0; x < SZ; x++) {
        for (int y = 0; y < SZ; y++) {
            StoneColor sc = board.getStone(x, y);
            if (sc == BLACK_STONE)      sb.set_stone(x, y, sc_black);
            else if (sc == WHITE_STONE) sb.set_stone(x, y, sc_white);
        }
    }

    sb.identify_units();

    if (method == ScoringMethod::Complex)
        sb.calc_scoring_markers_complex();
    else
        sb.calc_scoring_markers_simple();

    // Extract results
    territory_out.clear();
    dead_stones_out.clear();
    disputed_out.clear();
    black_score = sb.score_b();
    white_score = sb.score_w();

    for (int x = 0; x < SZ; x++) {
        for (int y = 0; y < SZ; y++) {
            sc_mark m = sb.mark_at(x, y);
            if (m == sc_mark::terr) {
                int extra = sb.mark_extra_at(x, y);
                // extra==0 → white territory, extra==1 → black territory
                StoneColor owner = (extra == 1) ? BLACK_STONE : WHITE_STONE;
                territory_out.insert(qMakePair(x, y), owner);
            } else if (m == sc_mark::dead) {
                dead_stones_out.insert(qMakePair(x, y));
            } else if (m == sc_mark::seki || m == sc_mark::falseeye) {
                disputed_out.insert(qMakePair(x, y));
            }
        }
    }
}
