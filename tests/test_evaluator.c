/* tests/test_evaluator.c — benchmark tests, mirroring the JS/Python/C++ suites.
 * Build & run via `make test`. */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "deck.h"
#include "equity.h"
#include "evaluator.h"
#include "ranges.h"

static int g_passed = 0;
static int g_failed = 0;

static void check(int condition, const char *message) {
    if (condition) {
        g_passed++;
        printf("  \xE2\x9C\x93 %s\n", message);
    } else {
        g_failed++;
        printf("  \xE2\x9C\x97 %s\n", message);
    }
}

static void approx(double actual, double expected, double tolerance, const char *message) {
    char buf[160];
    snprintf(buf, sizeof(buf), "%s (got %.4f, expected ~%.2f)", message, actual, expected);
    check(fabs(actual - expected) <= tolerance, buf);
}

static double equityHeadsUp(Card hero[2], Card villain[2], int trials) {
    CardList fullDeck, remaining;
    fresh_deck(&fullDeck);
    remaining.count = 0;
    for (int i = 0; i < fullDeck.count; i++) {
        int dead = card_equals(fullDeck.cards[i], hero[0]) || card_equals(fullDeck.cards[i], hero[1]) ||
                   card_equals(fullDeck.cards[i], villain[0]) || card_equals(fullDeck.cards[i], villain[1]);
        if (!dead) remaining.cards[remaining.count++] = fullDeck.cards[i];
    }

    int wins = 0, ties = 0;
    for (int t = 0; t < trials; t++) {
        CardList board, rest;
        draw_n(&remaining, 5, &board, &rest);

        Card h7[7] = {hero[0], hero[1], board.cards[0], board.cards[1], board.cards[2], board.cards[3], board.cards[4]};
        Card v7[7] = {villain[0], villain[1], board.cards[0], board.cards[1], board.cards[2], board.cards[3], board.cards[4]};

        long long h = best7_score(h7);
        long long v = best7_score(v7);
        if (h > v) wins++;
        else if (h == v) ties++;
    }
    return (wins + ties / 2.0) / trials;
}

int main(void) {
    srand((unsigned)time(NULL));
    ranges_init();

    printf("Hand category ordering:\n");
    {
        Card royalFlush[5] = {{14, 's'}, {13, 's'}, {12, 's'}, {11, 's'}, {10, 's'}};
        Card quads[5] = {{9, 's'}, {9, 'h'}, {9, 'd'}, {9, 'c'}, {2, 's'}};
        Card fullHouse[5] = {{5, 's'}, {5, 'h'}, {5, 'd'}, {3, 'c'}, {3, 's'}};
        Card flush[5] = {{12, 'h'}, {9, 'h'}, {7, 'h'}, {4, 'h'}, {2, 'h'}};
        Card straight[5] = {{9, 's'}, {8, 'h'}, {7, 'd'}, {6, 'c'}, {5, 's'}};

        check(hand_score5(royalFlush) > hand_score5(quads), "royal flush beats quads");
        check(hand_score5(quads) > hand_score5(fullHouse), "quads beat a full house");
        check(hand_score5(fullHouse) > hand_score5(flush), "full house beats a flush");
        check(hand_score5(flush) > hand_score5(straight), "flush beats a straight");
    }

    printf("\n7-card best-hand selection:\n");
    {
        Card cards7[7] = {{14, 'h'}, {13, 'd'}, {2, 's'}, {3, 'c'}, {4, 'd'}, {5, 'h'}, {9, 'c'}};
        long long score = best7_score(cards7);
        long long category = score;
        for (int i = 0; i < 5; i++) category /= 15;
        check(category == 4, "finds the wheel (A-2-3-4-5) straight buried in a 7-card hand");
    }

    printf("\nEquity vs known benchmarks (Monte Carlo, so allow tolerance):\n");
    {
        Card aa[2] = {{14, 's'}, {14, 'h'}};
        Card kk[2] = {{13, 's'}, {13, 'h'}};
        Card s72o[2] = {{7, 's'}, {2, 'h'}};

        approx(equityHeadsUp(aa, kk, 20000), 0.82, 0.02, "AA vs KK, all-in preflop, ~82%");
        approx(equityHeadsUp(s72o, aa, 20000), 0.12, 0.03, "72o vs AA, all-in preflop, ~12%");

        CardList fullDeck, remainingVsRandom, remainingAA;
        fresh_deck(&fullDeck);

        remainingVsRandom.count = 0;
        for (int i = 0; i < fullDeck.count; i++) {
            Card c = fullDeck.cards[i];
            if (!(c.rank == 7 && c.suit == 's') && !(c.rank == 2 && c.suit == 'h'))
                remainingVsRandom.cards[remainingVsRandom.count++] = c;
        }
        CardList heroS72o, boardEmpty;
        heroS72o.count = 2; heroS72o.cards[0] = s72o[0]; heroS72o.cards[1] = s72o[1];
        boardEmpty.count = 0;
        double eq72Random = simulate_equity(&heroS72o, &boardEmpty, &remainingVsRandom, "random", 20000, 1);
        approx(eq72Random, 0.345, 0.02, "72o vs a random hand, heads-up, ~35%");

        remainingAA.count = 0;
        for (int i = 0; i < fullDeck.count; i++) {
            Card c = fullDeck.cards[i];
            if (!(c.rank == 14 && c.suit == 's') && !(c.rank == 14 && c.suit == 'h'))
                remainingAA.cards[remainingAA.count++] = c;
        }
        CardList heroAA;
        heroAA.count = 2; heroAA.cards[0] = aa[0]; heroAA.cards[1] = aa[1];
        double eqAA5 = simulate_equity(&heroAA, &boardEmpty, &remainingAA, "random", 8000, 5);
        approx(eqAA5, 0.49, 0.03, "AA vs 5 random opponents (multiway), ~49%");
    }

    printf("\n%d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}
