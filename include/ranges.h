#ifndef POKER_RANGES_H
#define POKER_RANGES_H
/* ranges.h — position-based opponent range modeling via the Chen formula.
 * Illustrative approximation of typical open-raise ranges, not a solved
 * GTO range. */

#include "deck.h"

typedef struct {
    char key[8];
    char label[8];
    double pct;
    char desc[80];
    double cutoff;
} PositionMeta;

#define NUM_POSITIONS 6

double chen_score(int r1, int r2, int suited);

/* Must be called once before using position_meta_for() or deal_one_opponent()
 * with a non-"random" position key. Idempotent. */
void ranges_init(void);

/* Returns NULL if key is not a known position. */
const PositionMeta *position_meta_for(const char *key);

/* All positions, in table order (for "players still to act" calculations). */
extern const char *const POSITIONS_ORDER[NUM_POSITIONS];

int fits_position_range(Card c1, Card c2, double cutoffScore);

/* Deals one opponent's two-card hand, respecting the given position's range
 * via rejection sampling. positionKey == "random" means no filter. */
void deal_one_opponent(const CardList *deckWorking, const char *positionKey, CardList *handOut,
                        CardList *restOut);

#endif
