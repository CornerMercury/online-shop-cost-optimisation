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

    size_t count;
    CardOption **cart = parse_json_list(json_str, &count);
    free(json_str);
    State state;
    solve(cart, count, &state);
    free_card_options(cart, count);
    free_state(&state);
    return EXIT_SUCCESS;
}