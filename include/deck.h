#ifndef POKER_DECK_H
#define POKER_DECK_H
/* deck.h — card representation and deck utilities (pure C99, no deps). */

typedef struct {
    int rank;  /* 2-14 (14 = Ace) */
    char suit; /* 's','h','d','c' */
} Card;

/* A fixed-capacity list of cards. count tracks how many of cards[] are used. */
#define CARDLIST_CAP 52
typedef struct {
    Card cards[CARDLIST_CAP];
    int count;
} CardList;

/* Writes a short label like "A", "K", "10", "7" into buf (must be >= 3 bytes). */
void rank_label(int rank, char *buf);

/* Writes a full card label like "As" or "10h" into buf (must be >= 4 bytes). */
void card_label(Card c, char *buf);

/* Fills out with all 52 cards. */
void fresh_deck(CardList *out);

/* Draws n random cards from deck (via partial Fisher-Yates on a local copy).
 * Does not mutate deck. drawnOut gets n cards, restOut gets the remainder. */
void draw_n(const CardList *deck, int n, CardList *drawnOut, CardList *restOut);

/* True if the two cards have the same rank and suit. */
int card_equals(Card a, Card b);

#endif
