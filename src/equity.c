#include "equity.h"
#include <math.h>
#include <string.h>
#include "evaluator.h"
#include "ranges.h"

double simulate_equity(const CardList *hero, const CardList *board, const CardList *remainingDeck,
                        const char *positionKey, int trials, int numOpponents) {
    double equitySum = 0.0;
    int neededBoardCards = 5 - board->count;
    if (numOpponents > MAX_OPPONENTS) numOpponents = MAX_OPPONENTS;

    for (int t = 0; t < trials; t++) {
        CardList deckWorking = *remainingDeck;
        CardList opponents[MAX_OPPONENTS];

        for (int k = 0; k < numOpponents; k++) {
            CardList rest;
            deal_one_opponent(&deckWorking, positionKey, &opponents[k], &rest);
            deckWorking = rest;
        }

        Card fullBoard[5];
        for (int i = 0; i < board->count; i++) fullBoard[i] = board->cards[i];
        if (neededBoardCards > 0) {
            CardList extra, rest;
            draw_n(&deckWorking, neededBoardCards, &extra, &rest);
            for (int i = 0; i < neededBoardCards; i++) fullBoard[board->count + i] = extra.cards[i];
        }

        Card heroCards7[7];
        heroCards7[0] = hero->cards[0];
        heroCards7[1] = hero->cards[1];
        for (int i = 0; i < 5; i++) heroCards7[2 + i] = fullBoard[i];
        long long heroScore = best7_score(heroCards7);

        long long maxScore = heroScore;
        long long oppScores[MAX_OPPONENTS];
        for (int k = 0; k < numOpponents; k++) {
            Card oppCards7[7];
            oppCards7[0] = opponents[k].cards[0];
            oppCards7[1] = opponents[k].cards[1];
            for (int i = 0; i < 5; i++) oppCards7[2 + i] = fullBoard[i];
            oppScores[k] = best7_score(oppCards7);
            if (oppScores[k] > maxScore) maxScore = oppScores[k];
        }

        if (heroScore == maxScore) {
            int winners = 1;
            for (int k = 0; k < numOpponents; k++)
                if (oppScores[k] == maxScore) winners++;
            equitySum += 1.0 / winners;
        }
    }

    return equitySum / trials;
}

int trials_for_opponents(int n) {
    if (n <= 1) return 3000;
    if (n == 2) return 2000;
    if (n == 3) return 1400;
    if (n == 4) return 1000;
    if (n == 5) return 800;
    return 600;
}

double required_equity_pct(double pot, double bet) {
    if (pot + 2 * bet <= 0) return 0.0;
    return (bet / (pot + 2 * bet)) * 100;
}

int raise_to_amount(double pot, double bet, double stack, const char *sizeMode, double customTo) {
    double raiseTo;
    if (strcmp(sizeMode, "2x") == 0) raiseTo = bet * 2;
    else if (strcmp(sizeMode, "3x") == 0) raiseTo = bet * 3;
    else if (strcmp(sizeMode, "pot") == 0) raiseTo = pot + 3 * bet; /* standard "raise-to-pot" formula */
    else if (strcmp(sizeMode, "allin") == 0) raiseTo = stack;
    else raiseTo = customTo > 0 ? customTo : bet * 2; /* custom */

    if (raiseTo > stack) raiseTo = stack;
    if (raiseTo < bet + 1) raiseTo = bet + 1;
    return (int)(raiseTo + 0.5); /* round */
}

double ev_raise(double pot, double bet, double raiseTo, double equity, double foldPct) {
    double f = foldPct / 100;
    if (f < 0) f = 0;
    if (f > 1) f = 1;
    double evIfEveryoneFolds = pot + bet;
    double evIfCalledToShowdown = equity * (pot + raiseTo) - (1 - equity) * raiseTo;
    return f * evIfEveryoneFolds + (1 - f) * evIfCalledToShowdown;
}

double ev_call(double pot, double bet, double equity) {
    return equity * (pot + bet) - (1 - equity) * bet;
}
