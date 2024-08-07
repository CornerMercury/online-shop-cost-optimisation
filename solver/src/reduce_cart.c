#include "reduce_cart.h"
#include "hashmap.h"
#include "constants.h"

typedef struct {
    char *name;
    int count;
} SellerCount;

int hashmap_compare(const void *a, const void *b, void *udata) {
    const char *sa = a;
    const char *sb = b;
    return strcmp(sa, sb);
}

uint64_t hashmap_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const SellerCount *seller = item;
    return hashmap_sip(seller->name, strlen(seller->name), seed0, seed1);
}

void remove_uncommon_sellers(CardOption *cart, size_t count) {
    struct hashmap *seller_counts = hashmap_new(sizeof(SellerCount *), 0, 0, 0, hashmap_hash, hashmap_compare, NULL,
                                                NULL);
    for (size_t i = 0; i < count; i++) {
        CardOption card_option = cart[i];
        for (size_t j = 0; j < card_option.seller_count; j++) {
            Seller seller = card_option.sellers[j];
            const SellerCount *seller_count = hashmap_get(seller_counts, &(SellerCount) {.name=seller.name});
            int new_count;
            if (seller_count) {
                new_count = seller_count->count + 1;
            } else {
                new_count = 1;
            }
            hashmap_set(seller_counts, &(SellerCount) {.name=seller.name, .count=new_count});
        }
    }

    for (size_t i = 0; i < count; i++) {
        CardOption card_option = cart[i];
        int total_cards = 0;
        Seller *sellers_to_remove = malloc(sizeof(Seller) * hashmap_count(seller_counts));
        if (sellers_to_remove == NULL) {
            fprintf(stderr, "Failed to allocate memory.\n");
            exit(1);
        }
        size_t remove_size = 0;
        for (size_t j = 0; j < card_option.seller_count; j++) {
            Seller seller = card_option.sellers[j];
            // keeps the cheapest sellers (the same number as amount required)
            // in case it is cheaper to buy from all of them even with delivery costs
            const SellerCount *seller_count_struct = hashmap_get(seller_counts, &(SellerCount) {.name=seller.name});
            if ((total_cards >= card_option.amount) && (seller_count_struct->count <= MIN_DUPLICATE_SELLERS)) {
                remove_size++;
                sellers_to_remove[remove_size - 1] = seller;
            }
            // valgrind hates this for some reason
            total_cards += seller.available;
        }
        for (size_t j = 0; j < remove_size; j++) {
            for (size_t k = 0; k < card_option.seller_count; k++) {
                if (sellers_to_remove[j].name == card_option.sellers[k].name) {
                    free(card_option.sellers[k].name);
                    card_option.sellers[k] = (Seller){};
                    for (size_t l = k + 1; l < card_option.seller_count; l++) {
                        card_option.sellers[l - 1] = card_option.sellers[l];
                        card_option.sellers[l] = (Seller){};
                    }
                    card_option.seller_count -= 1;
                }
            }
        }
        free(sellers_to_remove);
    }
    hashmap_free(seller_counts);
}