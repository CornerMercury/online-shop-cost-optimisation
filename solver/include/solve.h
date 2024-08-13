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
    char *seller_name;
    ItemInfo item_infos[MAX_UNIQUE_CARDS_PER_SELLER];
    size_t item_info_count;
} ItemInfoKey;

typedef struct {
    ItemInfoKey *array;
    int card_cost;
    int delivery_cost;
    size_t seller_count;
} SellerArray;

typedef struct {
    struct hashmap *map;
    int card_cost;
    int delivery_cost;
    size_t seller_count;
} SellerMap;

typedef struct {
    SellerArray *best_seller_array;
    int min_cost;
    SellerMap *current_seller_map;
} State;


int solve(CardOption **items, size_t item_count, State *state);

void free_state(State *state);

#endif