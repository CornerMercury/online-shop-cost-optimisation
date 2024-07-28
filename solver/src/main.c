#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "card_option.h"

char *read_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Failed to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *data = (char *) malloc(length + 1);
    if (data == NULL) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    fread(data, 1, length, file);
    data[length] = '\0';

    fclose(file);
    return data;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        perror("Command in form 'solver <filename>");
        return 1;
    }
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s%s", "../.jsonCarts/", argv[1]);
    char *json_str = read_file(file_path);
    int count;
    CardOption *cart = parse_json_list(json_str, &count);
    free(json_str);
    free_card_options(cart, count);
    return 0;
}