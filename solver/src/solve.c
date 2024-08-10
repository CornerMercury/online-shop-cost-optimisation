#include "solve.h"
#include <stdlib.h>
#include <stdio.h>
#include "reduce_cart.h"
#include "constants.h"

typedef struct {
    char *seller_name;
    ItemInfo item_infos[MAX_UNIQUE_CARDS_PER_SELLER];
    size_t item_info_count;
} ItemInfoKey;

static int hashmap_compare(const void *a, const void *b, void *udata) {
    const ItemInfoKey *item_info_key_a = a;
    const ItemInfoKey *item_info_key_b = b;
    return strcmp(item_info_key_a->seller_name, item_info_key_b->seller_name);
}

static uint64_t hashmap_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const ItemInfoKey *item_info_key = item;
    return hashmap_sip(item_info_key->seller_name, strlen(item_info_key->seller_name), seed0, seed1);
}

void init_seller_map(SellerMap *seller_map) {
    seller_map->map = hashmap_new(sizeof(ItemInfoKey), 0, 0, 0, hashmap_hash, hashmap_compare, NULL, NULL);
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
}


// DFS on first layer, used for setup and printing
int solve(CardOption **items, size_t item_count, State *state) {
    init_state(state);
    SellerMap *current_seller_map = state->current_seller_map;
    if (item_count == 0) {
        fprintf(stderr, "The number of items is 0\n");
        return EXIT_FAILURE;
    }
    remove_uncommon_sellers(items, item_count);
    // Populate hashmap
    for (size_t i = 0; i < item_count; i++) {
        for (size_t j = 0; j < items[i]->seller_count; j++) {
            char *seller_name = items[i]->sellers[j].name;
            if (!hashmap_get(current_seller_map->map, &(ItemInfoKey) {.seller_name=seller_name})) {
                hashmap_set(current_seller_map->map, &(ItemInfoKey) {.seller_name=seller_name, .item_infos={}, .item_info_count=0});
            }
        }
    }
    return EXIT_SUCCESS;
}