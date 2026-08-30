#include "ranges.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

double chen_score(int r1, int r2, int suited) {
    double hp[15];
    for (int i = 0; i < 15; i++) hp[i] = i / 2.0;
    hp[14] = 10;
    hp[13] = 8;
    hp[12] = 7;
    hp[11] = 6;
    hp[10] = 5;

    if (r1 == r2) {
        double s = hp[r1] * 2;
        return s > 5 ? s : 5;
    }

    int hi = r1 > r2 ? r1 : r2;
    int lo = r1 > r2 ? r2 : r1;
    double score = hp[hi];
    if (suited) score += 2;

    int gap = hi - lo - 1;
    if (gap == 1) score -= 1;
    else if (gap == 2) score -= 2;
    else if (gap == 3) score -= 4;
    else if (gap >= 4) score -= 5;

    if (gap <= 1 && hi < 12) score += 1; /* connector/1-gapper bonus, both below queen */

    return score;
}

typedef struct {
    double score;
    int combos;
} HandEntry;

#define HAND_TABLE_SIZE 169
static HandEntry g_handTable[HAND_TABLE_SIZE];
static int g_handTableBuilt = 0;

static int compareHandEntry(const void *a, const void *b) {
    double sa = ((const HandEntry *)a)->score;
    double sb = ((const HandEntry *)b)->score;
    if (sa < sb) return 1;   /* descending */
    if (sa > sb) return -1;
    return 0;
}

static void buildHandTable(void) {
    int idx = 0;
    for (int hi = 2; hi <= 14; hi++) {
        for (int lo = 2; lo <= hi; lo++) {
            if (hi == lo) {
                g_handTable[idx].score = chen_score(hi, lo, 0);
                g_handTable[idx].combos = 6;
                idx++;
            } else {
                g_handTable[idx].score = chen_score(hi, lo, 1);
                g_handTable[idx].combos = 4;
                idx++;
                g_handTable[idx].score = chen_score(hi, lo, 0);
                g_handTable[idx].combos = 12;
                idx++;
            }
        }
    }
    qsort(g_handTable, HAND_TABLE_SIZE, sizeof(HandEntry), compareHandEntry);
    g_handTableBuilt = 1;
}

static double chenCutoffForPercentile(double targetPct) {
    if (!g_handTableBuilt) buildHandTable();
    int cumulative = 0;
    for (int i = 0; i < HAND_TABLE_SIZE; i++) {
        cumulative += g_handTable[i].combos;
        if ((double)cumulative / 1326.0 >= targetPct) return g_handTable[i].score;
    }
    return g_handTable[HAND_TABLE_SIZE - 1].score;
}

const char *const POSITIONS_ORDER[NUM_POSITIONS] = {"utg", "mp", "co", "btn", "sb", "bb"};

static PositionMeta g_positions[NUM_POSITIONS];
static int g_positionsBuilt = 0;

void ranges_init(void) {
    if (g_positionsBuilt) return;
    if (!g_handTableBuilt) buildHandTable();

    struct { const char *key, *label, *desc; double pct; } raw[NUM_POSITIONS] = {
        {"utg", "UTG", "~11% of hands - premium-heavy", 0.107},
        {"mp", "MP", "~18% of hands - still fairly tight", 0.175},
        {"bb", "BB", "~26% of hands - defend/3-bet mix", 0.243},
        {"co", "CO", "~41% of hands - opening up", 0.259},
        {"sb", "SB", "~43% of hands - wide but first to act postflop", 0.409},
        {"btn", "BTN", "~50% of hands - widest, positional steal range", 0.436},
    };

    for (int i = 0; i < NUM_POSITIONS; i++) {
        strncpy(g_positions[i].key, raw[i].key, sizeof(g_positions[i].key) - 1);
        strncpy(g_positions[i].label, raw[i].label, sizeof(g_positions[i].label) - 1);
        strncpy(g_positions[i].desc, raw[i].desc, sizeof(g_positions[i].desc) - 1);
        g_positions[i].pct = raw[i].pct;
        g_positions[i].cutoff = chenCutoffForPercentile(raw[i].pct);
    }
    g_positionsBuilt = 1;
}

const PositionMeta *position_meta_for(const char *key) {
    if (!g_positionsBuilt) ranges_init();
    for (int i = 0; i < NUM_POSITIONS; i++)
        if (strcmp(g_positions[i].key, key) == 0) return &g_positions[i];
    return NULL;
}

int fits_position_range(Card c1, Card c2, double cutoffScore) {
    return chen_score(c1.rank, c2.rank, c1.suit == c2.suit) >= cutoffScore;
}

void deal_one_opponent(const CardList *deckWorking, const char *positionKey, CardList *handOut,
                        CardList *restOut) {
    if (strcmp(positionKey, "random") == 0) {
        draw_n(deckWorking, 2, handOut, restOut);
        return;
    }

    const PositionMeta *meta = position_meta_for(positionKey);
    double cutoff = meta ? meta->cutoff : -100.0; /* unknown key => accept anything */

    for (int tries = 0; tries < 25; tries++) {
        draw_n(deckWorking, 2, handOut, restOut);
        if (fits_position_range(handOut->cards[0], handOut->cards[1], cutoff)) return;
    }
    draw_n(deckWorking, 2, handOut, restOut); /* fallback */
}
