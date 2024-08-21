#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "card_option.h"
#include "solve.h"

int read_file(const char *filename, char **content) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    *content = (char *) malloc(file_size + 1);
    if (!*content) {
        fprintf(stderr, "Failed to allocate memory\n");
        fclose(file);
        return 1;
    }
    size_t bytes_read = fread(*content, 1, file_size, file);
    if (bytes_read != (size_t) file_size) {
        fprintf(stderr, "Failed to read file\n");
        free(*content);
        fclose(file);
        return 1;
    }
    (*content)[file_size] = '\0';
    fclose(file);
    return 0;
}

void output_solution(SellerArray solution, char **id_to_name, size_t unique_seller_count) {
    for (size_t i = 0; i < unique_seller_count; i++) {
        SellerItems items = solution.array[i];
        if (items.item_info_count == 0) {
            continue;
        }
        printf("%s:\n", id_to_name[i]);
        for (size_t j = 0; j < items.item_info_count; j++) {
            ItemInfo info = items.item_infos[j];
            printf("    x%d %s&amount=%d €%.2f\n", info.amount, info.url, info.amount, (double) info.total_cost / 100);
        }
    }
    printf("\nTotal cost: €%.2f\n", (double) (solution.card_cost + solution.delivery_cost) / 100);
    printf("Total card cost: €%.2f\n", (double) solution.card_cost / 100);
    printf("Total delivery cost: €%.2f\n", (double) solution.delivery_cost / 100);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Command in form 'solver <filename>\n");
        return EXIT_FAILURE;
    }

    char file_path[256];
    // append filename to path
    snprintf(file_path, sizeof(file_path), "%s%s", "../.jsonCarts/", argv[1]);

    char *json_str = NULL;
    if (read_file(file_path, &json_str) == 1) {
        return EXIT_FAILURE;
    }

    size_t item_count;
    char **id_to_name = malloc(0);
    size_t unique_seller_count = 0;
    CardOption **items = parse_json_list(json_str, &item_count, &id_to_name, &unique_seller_count);
    free(json_str);
    State state;

    if (!solve(items, item_count, &state, unique_seller_count)) {
        output_solution(*state.best_seller_array, id_to_name, unique_seller_count);
    };
    free_card_options(items, item_count, id_to_name, unique_seller_count);
    free_state(&state);
    return EXIT_SUCCESS;
}