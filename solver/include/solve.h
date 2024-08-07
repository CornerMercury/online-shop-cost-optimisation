#ifndef SOLVE_H
#define SOLVE_H

#include "hashmap.h"
#include "card_option.h"

typedef struct {
    int amount;
    uint64_t total_cost;
    char* url;
} CardInfo;

typedef struct {
    struct hashmap *map;
    uint64_t card_cost;
    uint64_t delivery_cost;
    size_t seller_count;
} SellerMap;

typedef struct {
    SellerMap *best_seller_map;
    uint64_t min_cost;
    SellerMap *current_seller_map;
} State;


void solve(CardOption *cart, size_t cart_count, State *state);

void free_state(State *state);

#endif