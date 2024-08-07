#ifndef CARD_OPTION_H
#define CARD_OPTION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

typedef struct {
    int available;
    int cost;
    char *name;
} Seller;

typedef struct {
    Seller *sellers;
    size_t seller_count;
    int amount;
    char *url;
} CardOption;

CardOption *parse_json_list(const char *json_str, size_t *count);

void free_card_options(CardOption *options, size_t count);

#endif