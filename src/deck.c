#include "deck.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// test
static const char kSuits[4] = {'s', 'h', 'd', 'c'};

void rank_label(int rank, char *buf) {
    switch (rank) {
        case 14: strcpy(buf, "A"); return;
        case 13: strcpy(buf, "K"); return;
        case 12: strcpy(buf, "Q"); return;
        case 11: strcpy(buf, "J"); return;
        default: sprintf(buf, "%d", rank); return;
    }
}

void card_label(Card c, char *buf) {
    char rbuf[3];
    rank_label(c.rank, rbuf);
    sprintf(buf, "%s%c", rbuf, c.suit);
}

void fresh_deck(CardList *out) {
    int i = 0;
    for (int r = 2; r <= 14; r++) {
        for (int s = 0; s < 4; s++) {
            out->cards[i].rank = r;
            out->cards[i].suit = kSuits[s];
            i++;
        }
    }
    out->count = i; /* 52 */
}

void draw_n(const CardList *deck, int n, CardList *drawnOut, CardList *restOut) {
    Card pool[CARDLIST_CAP];
    int count = deck->count;
    memcpy(pool, deck->cards, sizeof(Card) * count);

    /* partial Fisher-Yates: only need the first n slots randomized */
    for (int i = 0; i < n && i < count; i++) {
        int j = i + rand() % (count - i);
        Card tmp = pool[i];
        pool[i] = pool[j];
        pool[j] = tmp;
    }

    drawnOut->count = n;
    for (int i = 0; i < n; i++) drawnOut->cards[i] = pool[i];

    restOut->count = count - n;
    for (int i = 0; i < count - n; i++) restOut->cards[i] = pool[n + i];
}

int card_equals(Card a, Card b) {
    return a.rank == b.rank && a.suit == b.suit;
}
