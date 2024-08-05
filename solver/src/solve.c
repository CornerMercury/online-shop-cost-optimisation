#include "solve.h"
#include <stdlib.h>
#include <stdio.h>

void init_seller_map(SellerMap *seller_map) {
    seller_map->map = hashmap_new(sizeof(CardInfo *), 0, 0, 0, 0, 0, NULL, NULL);
    seller_map->card_cost = 0;
    seller_map->delivery_cost = 0;
    seller_map->seller_count = 0;
}

void init_state(State *state) {
    SellerMap *best_seller_map = malloc(sizeof(SellerMap));
    if (best_seller_map == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        exit(-1);
    }
    init_seller_map(best_seller_map);
    SellerMap *current_seller_map = malloc(sizeof(SellerMap));
    if (current_seller_map == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        exit(-1);
    }
    init_seller_map(current_seller_map);
    state->best_seller_map = best_seller_map;
    state->min_cost = INT64_MAX;
    state->current_seller_map = current_seller_map;
}

void free_state(State *state) {
    hashmap_free(state->best_seller_map->map);
    free(state->best_seller_map);
    hashmap_free(state->current_seller_map->map);
    free(state->current_seller_map);
    free(state);
}



void solve(CardOption **cart, State *state) {
    init_state(state);
    return;
}