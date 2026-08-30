#ifndef POKER_EVALUATOR_H
#define POKER_EVALUATOR_H
/* evaluator.h — 7-card poker hand evaluator.
 *
 * Scores any 5-card hand into a single comparable integer (higher =
 * stronger), then finds the best 5-card hand within 7 cards by checking
 * all 21 combinations. Same encoding scheme as the JS reference
 * implementation: category (0-8) is the most significant digit, kickers
 * break ties within it.
 */

#include "deck.h"

long long hand_score5(const Card cards[5]);
long long best7_score(const Card cards7[7]);

#endif
