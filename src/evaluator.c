#include "evaluator.h"
#include <string.h>
//Eval
/* All 21 ways to choose 5 indices out of 7, built once on first use. */
static int g_combos[21][5];
static int g_combosBuilt = 0;

static void buildCombos(void) {
    int idx = 0;
    for (int a = 0; a < 7; a++)
        for (int b = a + 1; b < 7; b++)
            for (int c = b + 1; c < 7; c++)
                for (int d = c + 1; d < 7; d++)
                    for (int e = d + 1; e < 7; e++) {
                        g_combos[idx][0] = a;
                        g_combos[idx][1] = b;
                        g_combos[idx][2] = c;
                        g_combos[idx][3] = d;
                        g_combos[idx][4] = e;
                        idx++;
                    }
    g_combosBuilt = 1;
}

static void sortDescInt(int *arr, int n) {
    for (int i = 1; i < n; i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] < key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

typedef struct {
    int rank;
    int count;
} RankGroup;

long long hand_score5(const Card cards[5]) {
    int ranksSorted[5];
    for (int i = 0; i < 5; i++) ranksSorted[i] = cards[i].rank;
    sortDescInt(ranksSorted, 5);

    int isFlush = 1;
    for (int i = 1; i < 5; i++)
        if (cards[i].suit != cards[0].suit) { isFlush = 0; break; }

    /* unique ranks, descending (ranksSorted is already sorted) */
    int uniq[5], uniqCount = 0;
    for (int i = 0; i < 5; i++) {
        if (uniqCount == 0 || uniq[uniqCount - 1] != ranksSorted[i]) uniq[uniqCount++] = ranksSorted[i];
    }

    int isStraight = 0, straightHigh = 0;
    if (uniqCount == 5) {
        if (uniq[0] - uniq[4] == 4) {
            isStraight = 1;
            straightHigh = uniq[0];
        } else if (uniq[0] == 14 && uniq[1] == 5 && uniq[2] == 4 && uniq[3] == 3 && uniq[4] == 2) {
            isStraight = 1; /* wheel: A-2-3-4-5 */
            straightHigh = 5;
        }
    }

    /* rank -> count, using a 15-slot table indexed by rank */
    int counts[15];
    memset(counts, 0, sizeof(counts));
    for (int i = 0; i < 5; i++) counts[ranksSorted[i]]++;

    RankGroup groups[5];
    int groupCount = 0;
    for (int r = 14; r >= 2; r--) {
        if (counts[r] > 0) groups[groupCount++] = (RankGroup){r, counts[r]};
    }
    /* stable-ish insertion sort by (count desc, rank desc); ranks already desc so this
     * only needs to reorder by count. */
    for (int i = 1; i < groupCount; i++) {
        RankGroup key = groups[i];
        int j = i - 1;
        while (j >= 0 && groups[j].count < key.count) {
            groups[j + 1] = groups[j];
            j--;
        }
        groups[j + 1] = key;
    }

    int category;
    int tie[5] = {0, 0, 0, 0, 0};

    if (isStraight && isFlush) {
        category = 8;
        tie[0] = straightHigh;
    } else if (groups[0].count == 4) {
        category = 7;
        tie[0] = groups[0].rank;
        tie[1] = groups[1].rank;
    } else if (groups[0].count == 3 && groupCount > 1 && groups[1].count == 2) {
        category = 6;
        tie[0] = groups[0].rank;
        tie[1] = groups[1].rank;
    } else if (isFlush) {
        category = 5;
        for (int i = 0; i < 5; i++) tie[i] = ranksSorted[i];
    } else if (isStraight) {
        category = 4;
        tie[0] = straightHigh;
    } else if (groups[0].count == 3) {
        category = 3;
        tie[0] = groups[0].rank;
        tie[1] = groups[1].rank;
        tie[2] = groups[2].rank;
    } else if (groups[0].count == 2 && groupCount > 1 && groups[1].count == 2) {
        int p0 = groups[0].rank > groups[1].rank ? groups[0].rank : groups[1].rank;
        int p1 = groups[0].rank > groups[1].rank ? groups[1].rank : groups[0].rank;
        category = 2;
        tie[0] = p0;
        tie[1] = p1;
        tie[2] = groups[2].rank;
    } else if (groups[0].count == 2) {
        category = 1;
        tie[0] = groups[0].rank;
        tie[1] = groups[1].rank;
        tie[2] = groups[2].rank;
        tie[3] = groups[3].rank;
    } else {
        category = 0;
        for (int i = 0; i < 5; i++) tie[i] = ranksSorted[i];
    }

    long long score = category;
    for (int i = 0; i < 5; i++) score = score * 15 + tie[i];
    return score;
}

long long best7_score(const Card cards7[7]) {
    if (!g_combosBuilt) buildCombos();

    long long best = -1;
    for (int c = 0; c < 21; c++) {
        Card hand[5];
        for (int i = 0; i < 5; i++) hand[i] = cards7[g_combos[c][i]];
        long long score = hand_score5(hand);
        if (score > best) best = score;
    }
    return best;
}
