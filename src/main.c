/* main.c — command-line interface for the poker equity engine.
 *
 * Examples:
 *   ./poker_equity --hero "As Kd" --board "Th Jd Qc" --pot 40 --bet 20
 *   ./poker_equity --hero "7s 2h" --opponents 3 --position btn --trials 20000
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "deck.h"
#include "equity.h"
#include "ranges.h"

static int parseCard(const char *text, Card *out) {
    if (strlen(text) != 2) return 0;
    char rc = (char)toupper((unsigned char)text[0]);
    char sc = (char)tolower((unsigned char)text[1]);
    int rank;
    switch (rc) {
        case 'A': rank = 14; break;
        case 'K': rank = 13; break;
        case 'Q': rank = 12; break;
        case 'J': rank = 11; break;
        case 'T': rank = 10; break;
        default:
            if (rc < '2' || rc > '9') return 0;
            rank = rc - '0';
    }
    if (sc != 's' && sc != 'h' && sc != 'd' && sc != 'c') return 0;
    out->rank = rank;
    out->suit = sc;
    return 1;
}

static int parseCards(const char *text, CardList *out) {
    out->count = 0;
    if (text == NULL || text[0] == '\0') return 1;

    char buf[256];
    strncpy(buf, text, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tok = strtok(buf, " ");
    while (tok) {
        Card c;
        if (!parseCard(tok, &c)) {
            fprintf(stderr, "Error: invalid card '%s'\n", tok);
            return 0;
        }
        if (out->count >= CARDLIST_CAP) return 0;
        out->cards[out->count++] = c;
        tok = strtok(NULL, " ");
    }
    return 1;
}

static const char *getArg(int argc, char **argv, const char *flag, const char *def) {
    for (int i = 1; i < argc - 1; i++)
        if (strcmp(flag, argv[i]) == 0) return argv[i + 1];
    return def;
}

static int hasArg(int argc, char **argv, const char *flag) {
    for (int i = 1; i < argc; i++)
        if (strcmp(flag, argv[i]) == 0) return 1;
    return 0;
}

int main(int argc, char **argv) {
    srand((unsigned)time(NULL));
    ranges_init();

    if (!hasArg(argc, argv, "--hero")) {
        fprintf(stderr,
                "Usage: poker_equity --hero \"As Kd\" [--board \"Th Jd Qc\"] "
                "[--opponents N] [--position utg|mp|co|btn|sb|bb|random] "
                "[--trials N] [--pot P] [--bet B] [--stack S] "
                "[--raise-size 2x|3x|pot|allin] [--fold-pct F]\n");
        return 1;
    }

    CardList hero, board;
    if (!parseCards(getArg(argc, argv, "--hero", ""), &hero)) return 1;
    if (!parseCards(getArg(argc, argv, "--board", ""), &board)) return 1;

    if (hero.count != 2) {
        fprintf(stderr, "Error: --hero must specify exactly 2 cards.\n");
        return 1;
    }
    if (board.count != 0 && board.count != 3 && board.count != 4 && board.count != 5) {
        fprintf(stderr, "Error: --board must specify 0, 3, 4, or 5 cards.\n");
        return 1;
    }

    int numOpponents = atoi(getArg(argc, argv, "--opponents", "1"));
    const char *position = getArg(argc, argv, "--position", "random");
    int trials = atoi(getArg(argc, argv, "--trials", "0"));
    if (trials <= 0) trials = trials_for_opponents(numOpponents);

    CardList fullDeck, remainingDeck;
    fresh_deck(&fullDeck);
    remainingDeck.count = 0;
    for (int i = 0; i < fullDeck.count; i++) {
        int dead = 0;
        for (int j = 0; j < hero.count && !dead; j++)
            if (card_equals(fullDeck.cards[i], hero.cards[j])) dead = 1;
        for (int j = 0; j < board.count && !dead; j++)
            if (card_equals(fullDeck.cards[i], board.cards[j])) dead = 1;
        if (!dead) remainingDeck.cards[remainingDeck.count++] = fullDeck.cards[i];
    }

    double equity = simulate_equity(&hero, &board, &remainingDeck, position, trials, numOpponents);

    char heroBuf[16] = "", boardBuf[32] = "", tmp[6];
    for (int i = 0; i < hero.count; i++) { card_label(hero.cards[i], tmp); strcat(heroBuf, tmp); strcat(heroBuf, " "); }
    for (int i = 0; i < board.count; i++) { card_label(board.cards[i], tmp); strcat(boardBuf, tmp); strcat(boardBuf, " "); }
    if (board.count == 0) strcpy(boardBuf, "(none)");

    printf("Hero:      %s\n", heroBuf);
    printf("Board:     %s\n", boardBuf);
    if (strcmp(position, "random") == 0) {
        printf("Opponents: %d (any two cards)\n", numOpponents);
    } else {
        const PositionMeta *meta = position_meta_for(position);
        printf("Opponents: %d (%s)\n", numOpponents, meta ? meta->desc : "unknown position");
    }
    printf("Equity:    %.1f%%  (%d trials)\n", equity * 100, trials);

    if (hasArg(argc, argv, "--pot") && hasArg(argc, argv, "--bet")) {
        double pot = atof(getArg(argc, argv, "--pot", "0"));
        double bet = atof(getArg(argc, argv, "--bet", "0"));
        double stack = atof(getArg(argc, argv, "--stack", "300"));
        const char *raiseSize = getArg(argc, argv, "--raise-size", "pot");
        double foldPct = atof(getArg(argc, argv, "--fold-pct", "35"));

        double req = required_equity_pct(pot, bet);
        double callEv = ev_call(pot, bet, equity);
        int raiseTo = raise_to_amount(pot, bet, stack, raiseSize, 0);
        double raiseEv = ev_raise(pot, bet, raiseTo, equity, foldPct);

        printf("\nPot: $%.1f   Bet faced: $%.1f   Required equity to call: %.1f%%\n", pot, bet, req);
        printf("  EV(fold)                = $0.0\n");
        printf("  EV(call)                = $%+.1f\n", callEv);
        printf("  EV(raise to $%d, %.1f%% fold assumed) = $%+.1f\n", raiseTo, foldPct, raiseEv);

        const char *best = "FOLD";
        double bestEv = 0;
        if (callEv > bestEv) { best = "CALL"; bestEv = callEv; }
        if (raiseEv > bestEv) { best = "RAISE"; bestEv = raiseEv; }
        printf("\n  => %s has the highest EV\n", best);
    }

    return 0;
}
