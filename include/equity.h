#ifndef POKER_EQUITY_H
#define POKER_EQUITY_H
/* equity.h — Monte Carlo equity simulation and pot-odds / EV math. */

#include "deck.h"

#define MAX_OPPONENTS 8

/* Estimates hero's equity against numOpponents by simulating `trials`
 * random runouts. Hero must beat every opponent to win; ties split equity
 * fractionally across everyone tied for best (correct multiway handling).
 *
 * positionKey: "random" or a key known to ranges.h, applied to every
 * opponent as a simplification — only one opponent (the raiser) is truly
 * known to hold that range. hero must have count==2, board count in
 * {0,3,4,5}, numOpponents <= MAX_OPPONENTS. */
double simulate_equity(const CardList *hero, const CardList *board, const CardList *remainingDeck,
                        const char *positionKey, int trials, int numOpponents);

/* Trial count scales down as opponent count rises, to keep runtime bounded. */
int trials_for_opponents(int n);

/* Standard pot-odds breakeven: equity needed for a call to break even. */
double required_equity_pct(double pot, double bet);

/* Resolves a raise-to amount from a sizing mode, clamped to the effective
 * stack. sizeMode: "2x", "3x", "pot", "allin", or "custom" (uses customTo). */
int raise_to_amount(double pot, double bet, double stack, const char *sizeMode, double customTo);

/* Simplified raise EV: assumes either everyone folds (at foldPct) or
 * everyone remaining calls and it goes to showdown at the given equity. */
double ev_raise(double pot, double bet, double raiseTo, double equity, double foldPct);

/* EV of a straight call, given equity against the field. */
double ev_call(double pot, double bet, double equity);

#endif
