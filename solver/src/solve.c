#include "solve.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "reduce_cart.h"
#include <time.h>

#define MIN(a, b) (((a)<(b))?(a):(b))

void init_seller_array(SellerArray *seller_array, size_t unique_seller_count) {
    seller_array->array = malloc(sizeof(SellerItems) * unique_seller_count);
    if (seller_array->array == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        exit(-1);
    }
    for (size_t i = 0; i < unique_seller_count; i++) {
        seller_array->array[i] = (SellerItems){.item_infos={}, .item_info_count=0, .item_count=0};
    }
    seller_array->card_cost = 0;
    seller_array->delivery_cost = 0;
    seller_array->seller_count = 0;
}

void init_state(State *state, size_t unique_seller_count) {
    SellerArray *best_seller_array = malloc(sizeof(SellerArray));
    if (best_seller_array == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        exit(-1);
    }
    init_seller_array(best_seller_array, unique_seller_count);
    SellerArray *current_seller_array = malloc(sizeof(SellerArray));
    if (current_seller_array == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        exit(-1);
    }
    init_seller_array(current_seller_array, unique_seller_count);
    state->best_seller_array = best_seller_array;
    state->min_cost = (int) MAX_COST + 1;
    state->current_seller_array = current_seller_array;
}

void free_state(State *state) {
    free(state->best_seller_array->array);
    free(state->best_seller_array);
    free(state->current_seller_array->array);
    free(state->current_seller_array);
}

int compare_items(const void *a, const void *b) {
    int int_a = (*(CardOption **) a)->sellers[0].cost;
    int int_b = (*(CardOption **) b)->sellers[0].cost;
    if (int_a == int_b) return 0;
    else if (int_a < int_b) return -1;
    else return 1;
}

void depth_first_search(CardOption **items, size_t item_count, State *state, int *sum_of_min_costs, size_t unique_seller_count) {
    SellerArray *current_seller_array = state->current_seller_array;
    if (current_seller_array->seller_count > MAX_SELLERS) {
        return;
    }
    int total = current_seller_array->card_cost + current_seller_array->delivery_cost + sum_of_min_costs[item_count];

    if (total >= state->min_cost) {
        return;
    }
    // Found a solution that beats the current best solution
    if (item_count == 0) {
        state->min_cost = total;
        // Copy current_seller_map into the best_seller_array
        state->best_seller_array->card_cost = current_seller_array->card_cost;
        state->best_seller_array->delivery_cost = current_seller_array->delivery_cost;
        state->best_seller_array->seller_count = current_seller_array->seller_count;
        for (size_t i = 0; i < unique_seller_count; i++) {
            state->best_seller_array->array[i] = state->current_seller_array->array[i];
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
        current_seller_array->card_cost += total_cost;
        size_t seller_items_count = current_seller_array->array[seller.id].item_count;

        // Calculate the delivery cost
        // Assumes there is only SMALL_DEL and MED_DEL, add more if needed
        int previous_delivery_from_this_seller = 0;
        if (seller_items_count == 0) {
            current_seller_array->seller_count += 1;
        }
        else if (seller_items_count > 0 && seller_items_count <= 4) {
            previous_delivery_from_this_seller = SMALL_DEL;
        } else if (seller_items_count > 4) {
            previous_delivery_from_this_seller = MED_DEL;
        }

        int new_delivery_from_this_seller = 0;
        if (seller_items_count + (size_t) amount <= 4) {
            new_delivery_from_this_seller = SMALL_DEL;
        }
        else if (seller_items_count + (size_t) amount > 4) {
            new_delivery_from_this_seller = MED_DEL;
        }
        
        int change_in_delivery = new_delivery_from_this_seller - previous_delivery_from_this_seller;

        current_seller_array->delivery_cost += change_in_delivery;
        current_seller_array->array[seller.id].item_infos[seller_items_count] = (ItemInfo) {.amount=amount, .total_cost=total_cost, .url=current_item->url};
        current_seller_array->array[seller.id].item_info_count += 1;
        current_seller_array->array[seller.id].item_count += amount;

        int new_item_count = item_count;
        if (current_item->amount == 0) {
            new_item_count -= 1;
        }
        depth_first_search(items, new_item_count, state, sum_of_min_costs, unique_seller_count);

        // revert assumption of this seller
        current_seller_array->array[seller.id].item_count -= amount;
        current_seller_array->array[seller.id].item_info_count -= 1;
        current_seller_array->delivery_cost -= change_in_delivery;
        current_seller_array->card_cost -= total_cost;
        if (seller_items_count == 0) {
            current_seller_array->seller_count -= 1;
        }
        current_item->sellers[i].available += amount;
        current_item->amount += amount;
    }
}

// DFS on first layer, used for setup and printing
int solve(CardOption **items, size_t item_count, State *state, size_t unique_seller_count) {
    if (item_count == 0) {
        fprintf(stderr, "The number of items is 0\n");
        return EXIT_FAILURE;
    }

    // setup states
    remove_uncommon_sellers(items, item_count, unique_seller_count);
    // Sort so it exceeds the min_cost faster (less nodes need to be checked)
    qsort(items, item_count, sizeof(CardOption *), compare_items);
    init_state(state, unique_seller_count);
    SellerArray *current_seller_array = state->current_seller_array;

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
        current_seller_array->card_cost += total_cost;
        size_t seller_items_count = current_seller_array->array[seller.id].item_count;

        // Calculate the delivery cost
        // Assumes there is only SMALL_DEL and MED_DEL, add more if needed
        int previous_delivery_from_this_seller = 0;
        if (seller_items_count == 0) {
            current_seller_array->seller_count += 1;
        }
        else if (seller_items_count > 0 && seller_items_count <= 4) {
            previous_delivery_from_this_seller = SMALL_DEL;
        } else if (seller_items_count > 4) {
            previous_delivery_from_this_seller = MED_DEL;
        }

        int new_delivery_from_this_seller = 0;
        if (seller_items_count + (size_t) amount <= 4) {
            new_delivery_from_this_seller = SMALL_DEL;
        }
        else if (seller_items_count + (size_t) amount > 4) {
            new_delivery_from_this_seller = MED_DEL;
        }
        
        int change_in_delivery = new_delivery_from_this_seller - previous_delivery_from_this_seller;

        current_seller_array->delivery_cost += change_in_delivery;
        current_seller_array->array[seller.id].item_infos[seller_items_count] = (ItemInfo) {.amount=amount, .total_cost=total_cost, .url=current_item->url};
        current_seller_array->array[seller.id].item_info_count += 1;
        current_seller_array->array[seller.id].item_count += amount;

        int new_item_count = item_count;
        if (current_item->amount == 0) {
            new_item_count -= 1;
        }
        depth_first_search(items, new_item_count, state, sum_of_min_costs, unique_seller_count);

        // revert assumption of this seller
        current_seller_array->array[seller.id].item_count -= amount;
        current_seller_array->array[seller.id].item_info_count -= 1;
        current_seller_array->delivery_cost -= change_in_delivery;
        current_seller_array->card_cost -= total_cost;
        if (seller_items_count == 0) {
            current_seller_array->seller_count -= 1;
        }
        current_item->sellers[i].available += amount;
        current_item->amount += amount;
    }

    clock_t end = clock();
    printf("%fs\n", (double)(end - begin) / CLOCKS_PER_SEC);
    free(sum_of_min_costs);
    return EXIT_SUCCESS;
}