#include "solve.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "reduce_cart.h"
#include "hashmap.h"
#include <time.h>

#define MIN(a, b) (((a)<(b))?(a):(b))

static int hashmap_compare(const void *a, const void *b, void *udata) {
    const ItemInfoKey *item_info_key_a = a;
    const ItemInfoKey *item_info_key_b = b;
    return strcmp(item_info_key_a->seller_name, item_info_key_b->seller_name);
}

static uint64_t hashmap_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const ItemInfoKey *item_info_key = item;
    return hashmap_sip(item_info_key->seller_name, strlen(item_info_key->seller_name), seed0, seed1);
}

void init_seller_array(SellerArray *seller_array) {
    // Malloc to nothing so it can be freed without having to check if it has been malloced yet
    seller_array->array = malloc(0);
    if (seller_array->array == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        exit(-1);
    }
    seller_array->card_cost = 0;
    seller_array->delivery_cost = 0;
    seller_array->seller_count = 0;
}

void init_seller_map(SellerMap *seller_map) {
    seller_map->map = hashmap_new(sizeof(ItemInfoKey), 0, 0, 0, hashmap_hash, hashmap_compare, NULL, NULL);
    seller_map->card_cost = 0;
    seller_map->delivery_cost = 0;
    seller_map->seller_count = 0;
}

void init_state(State *state) {
    SellerArray *best_seller_array = malloc(sizeof(SellerArray));
    if (best_seller_array == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        exit(-1);
    }
    init_seller_array(best_seller_array);
    SellerMap *current_seller_map = malloc(sizeof(SellerMap));
    if (current_seller_map == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        exit(-1);
    }
    init_seller_map(current_seller_map);
    state->best_seller_array = best_seller_array;
    state->min_cost = INT_MAX;
    state->current_seller_map = current_seller_map;
}

void free_state(State *state) {
    free(state->best_seller_array->array);
    free(state->best_seller_array);
    hashmap_free(state->current_seller_map->map);
    free(state->current_seller_map);
}

int compare_items(const void *a, const void *b) {
    int int_a = (*(CardOption **) a)->sellers[0].cost;
    int int_b = (*(CardOption **) b)->sellers[0].cost;
    if (int_a == int_b) return 0;
    else if (int_a < int_b) return -1;
    else return 1;
}

void depth_first_search(CardOption **items, size_t item_count, State *state, int *sum_of_min_costs) {
    SellerMap *current_seller_map = state->current_seller_map;
    if (current_seller_map->seller_count > MAX_SELLERS) {
        return;
    }
    int total = current_seller_map->card_cost + current_seller_map->delivery_cost + sum_of_min_costs[item_count];

    if (total >= state->min_cost) {
        return;
    }

    // Found a solution that beats the current best solution
    if (item_count == 0) {
        state->min_cost = total;
        // Copy current_seller_map into the best_seller_array
        state->best_seller_array->card_cost = current_seller_map->card_cost;
        state->best_seller_array->delivery_cost = current_seller_map->delivery_cost;
        int seller_count = current_seller_map->seller_count;
        state->best_seller_array->seller_count = seller_count;
        state->best_seller_array->array = realloc(state->best_seller_array->array, sizeof(ItemInfoKey) * seller_count);
        if (state->best_seller_array->array == NULL) {
            fprintf(stderr, "Failed to reallocate memory.\n");
            exit(1);
        }
        size_t iter = 0;
        void *item;
        size_t i = 0;
        while (hashmap_iter(state->current_seller_map->map, &iter, &item) && i < (size_t) seller_count) {
            const ItemInfoKey *p_seller_ref = item;
            if (p_seller_ref->item_info_count != 0) {
                ItemInfoKey seller_ref = *p_seller_ref;
                state->best_seller_array->array[i] = seller_ref;
                i++;
            }
        }
        return;
    }

    CardOption *current_item = items[item_count - 1];
    for (size_t i = 0; i < current_item->seller_count; i++) {
        Seller seller = current_item->sellers[i];
        if (seller.available == 0) {
            continue;
        }
        int amount = MIN(seller.available, current_item->amount);

        // Assume you use this seller
        current_item->sellers[i].available -= amount;
        current_item->amount -= amount;
        int total_cost = amount * seller.cost;
        current_seller_map->card_cost += total_cost;
        const ItemInfoKey *p_seller_ref = hashmap_get(current_seller_map->map,
                                                      &(ItemInfoKey) {.seller_name=seller.name});
        ItemInfoKey seller_ref = *p_seller_ref;
        size_t seller_items_count = seller_ref.item_info_count;

        // Calculate the delivery cost
        // Assumes there is only SMALL_DEL and MED_DEL, add more if needed
        int delivery_change = 0;
        if (seller_items_count == 0) {
            delivery_change += SMALL_DEL;
            current_seller_map->seller_count += 1;
        }
        if (seller_items_count + (size_t) amount > 4 && seller_items_count <= 4) {
            delivery_change += MED_DEL;
        }

        current_seller_map->delivery_cost += delivery_change;
        seller_ref.item_infos[seller_ref.item_info_count] = (ItemInfo) {.amount=amount, .total_cost=total_cost, .url=current_item->url};
        seller_ref.item_info_count += 1;
        hashmap_set(current_seller_map->map, &seller_ref);

        int new_item_count = item_count;
        if (current_item->amount == 0) {
            new_item_count -= 1;
        }
        depth_first_search(items, new_item_count, state, sum_of_min_costs);

        // revert assumption of this seller
        seller_ref.item_info_count -= 1;
        hashmap_set(current_seller_map->map, &seller_ref);
        current_seller_map->delivery_cost -= delivery_change;
        current_seller_map->card_cost -= total_cost;
        if (seller_items_count == 0) {
            current_seller_map->seller_count -= 1;
        }
        current_item->sellers[i].available += amount;
        current_item->amount += amount;
    }
}

// DFS on first layer, used for setup and printing
int solve(CardOption **items, size_t item_count, State *state) {
    if (item_count == 0) {
        fprintf(stderr, "The number of items is 0\n");
        return EXIT_FAILURE;
    }

    // setup states
    remove_uncommon_sellers(items, item_count);
    // Sort so it exceeds the min_cost faster (less nodes need to be checked)
    qsort(items, item_count, sizeof(CardOption *), compare_items);
    init_state(state);
    SellerMap *current_seller_map = state->current_seller_map;

    // Populate hashmap
    for (size_t i = 0; i < item_count; i++) {
        for (size_t j = 0; j < items[i]->seller_count; j++) {
            char *seller_name = items[i]->sellers[j].name;
            if (!hashmap_get(current_seller_map->map, &(ItemInfoKey) {.seller_name=seller_name})) {
                hashmap_set(current_seller_map->map,
                            &(ItemInfoKey) {.seller_name=seller_name, .item_infos={}, .item_info_count=0});
            }
        }
    }

    // Precompute the minimum cost ignoring delivery costs
    // for the first i cards and 0 for no cards
    int *sum_of_min_costs = malloc(sizeof(int) * (item_count + 1));
    if (sum_of_min_costs == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        return EXIT_FAILURE;
    }
    sum_of_min_costs[0] = 0;
    for (size_t i = 0; i < item_count; i++) {
        int sum = 0;
        for (size_t j = 0; j <= i; j++) {
            sum += items[j]->sellers[0].cost * items[j]->amount;
        }
        sum_of_min_costs[i + 1] = sum;
    }
    clock_t begin = clock();
    CardOption *current_item = items[item_count - 1];
    for (size_t i = 0; i < current_item->seller_count; i++) {
        printf("%zu/%zu\n", i, current_item->seller_count);
        Seller seller = current_item->sellers[i];
        if (seller.available == 0) {
            continue;
        }
        int amount = MIN(seller.available, current_item->amount);

        // Assume you use this seller
        current_item->sellers[i].available -= amount;
        current_item->amount -= amount;
        int total_cost = amount * seller.cost;
        current_seller_map->card_cost += total_cost;
        const ItemInfoKey *p_seller_ref = hashmap_get(current_seller_map->map,
                                                      &(ItemInfoKey) {.seller_name=seller.name});
        ItemInfoKey seller_ref = *p_seller_ref;
        size_t seller_items_count = seller_ref.item_info_count;

        // Calculate the delivery cost
        // Assumes there is only SMALL_DEL and MED_DEL, add more if needed
        int delivery_change = 0;
        if (seller_items_count == 0) {
            delivery_change += SMALL_DEL;
            current_seller_map->seller_count += 1;
        }
        if (seller_items_count + (size_t) amount > 4 && seller_items_count <= 4) {
            delivery_change += MED_DEL;
        }

        current_seller_map->delivery_cost += delivery_change;
        seller_ref.item_infos[seller_ref.item_info_count] = (ItemInfo) {.amount=amount, .total_cost=total_cost, .url=current_item->url};
        seller_ref.item_info_count += 1;
        hashmap_set(current_seller_map->map, &seller_ref);

        int new_item_count = item_count;
        if (current_item->amount == 0) {
            new_item_count -= 1;
        }
        depth_first_search(items, new_item_count, state, sum_of_min_costs);

        // revert assumption of this seller
        seller_ref.item_info_count -= 1;
        hashmap_set(current_seller_map->map, &seller_ref);
        current_seller_map->delivery_cost -= delivery_change;
        current_seller_map->card_cost -= total_cost;
        if (seller_items_count == 0) {
            current_seller_map->seller_count -= 1;
        }
        current_item->sellers[i].available += amount;
        current_item->amount += amount;
    }

    clock_t end = clock();
    printf("%fs\n", (double)(end - begin) / CLOCKS_PER_SEC);
    free(sum_of_min_costs);
    return EXIT_SUCCESS;
}