#ifndef SOLVE_H
#define SOLVE_H

#include "card_option.h"
#include "constants.h"

typedef struct {
    int amount;
    int total_cost;
    char *url;
} ItemInfo;

typedef struct {
    ItemInfo item_infos[MAX_UNIQUE_CARDS_PER_SELLER];
    size_t item_info_count;
} SellerItems;

typedef struct {
    SellerItems *array;
    int card_cost;
    int delivery_cost;
    size_t seller_count;
} SellerArray;

typedef struct {
    SellerArray *best_seller_array;
    int min_cost;
    SellerArray *current_seller_array;
} State;


int solve(CardOption **items, size_t item_count, State *state, size_t unique_seller_count);

void free_state(State *state);

#endif