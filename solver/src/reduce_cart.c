#include "reduce_cart.h"
#include "constants.h"

void remove_uncommon_sellers(CardOption **items, size_t item_count, size_t unique_seller_count) {
    int *seller_counts = calloc(unique_seller_count, sizeof(int));
    if (seller_counts == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(1);
    }
    for (size_t i = 0; i < item_count; i++) {
        CardOption *item = items[i];
        for (size_t j = 0; j < item->seller_count; j++) {
            Seller seller = item->sellers[j];
            seller_counts[seller.id] += 1;
        }
    }
    for (size_t i = 0; i < item_count; i++) {
        CardOption *item = items[i];
        int total_cards = 0;
        Seller *sellers_to_remove = malloc(sizeof(Seller) * item->seller_count);
        if (sellers_to_remove == NULL) {
            fprintf(stderr, "Failed to allocate memory.\n");
            exit(1);
        }
        size_t remove_size = 0;
        for (size_t j = 0; j < item->seller_count; j++) {
            Seller seller = item->sellers[j];
            // keeps the cheapest sellers (the same number as amount required)
            // in case it is cheaper to buy from all of them even with delivery costs
            if ((total_cards >= item->amount) && (seller_counts[seller.id] <= MIN_DUPLICATE_SELLERS)) {
                remove_size++;
                sellers_to_remove[remove_size - 1] = seller;
            }
            total_cards = seller.available;
        }
        // Remove uncommon sellers and shift sellers in the left to the left before decreasing the list_size
        for (size_t j = 0; j < remove_size; j++) {
            for (size_t k = 0; k < item->seller_count; k++) {
                if (sellers_to_remove[j].id == item->sellers[k].id) {
                    item->sellers[k] = (Seller) {};
                    for (size_t l = k + 1; l < item->seller_count; l++) {
                        item->sellers[l - 1] = item->sellers[l];
                        item->sellers[l] = (Seller) {};
                    }
                    item->seller_count -= 1;
                }
            }
        }
        free(sellers_to_remove);
        // Shrink sellers array to minimise memory usage after deleting uncommon sellers
        Seller *temp = realloc(item->sellers, item->seller_count * sizeof(Seller));
        item->sellers = temp;
    }
    free(seller_counts);
}