#include "card_option.h"

Seller *parse_sellers(cJSON *sellers_json, int *seller_count) {
    *seller_count = cJSON_GetArraySize(sellers_json);
    Seller *sellers = (Seller *) malloc(*seller_count * sizeof(Seller));
    if (sellers == NULL) {
        perror("Failed to allocate memory for sellers");
        return NULL;
    }

    for (int i = 0; i < *seller_count; i++) {
        cJSON *item = cJSON_GetArrayItem(sellers_json, i);
        cJSON *available = cJSON_GetObjectItemCaseSensitive(item, "available");
        cJSON *cost = cJSON_GetObjectItemCaseSensitive(item, "cost");
        cJSON *seller_name = cJSON_GetObjectItemCaseSensitive(item, "seller_name");

        sellers[i].available = available ? available->valueint : 0;
        sellers[i].cost = cost ? cost->valueint : 0;
        sellers[i].seller_name = seller_name ? strdup(seller_name->valuestring) : NULL;
    }

    return sellers;
}

CardOption *parse_json_list(const char *json_str, int *count) {
    cJSON *json = cJSON_Parse(json_str);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            perror(error_ptr);
        }
        return NULL;
    }

    if (!cJSON_IsArray(json)) {
        perror("JSON is not an array");
        cJSON_Delete(json);
        return NULL;
    }

    int array_size = cJSON_GetArraySize(json);
    CardOption *options = (CardOption *) malloc(array_size * sizeof(CardOption));
    if (options == NULL) {
        perror("Failed to allocate memory for card options");
        cJSON_Delete(json);
        return NULL;
    }

    for (int i = 0; i < array_size; i++) {
        cJSON *item = cJSON_GetArrayItem(json, i);
        cJSON *sellers_json = cJSON_GetObjectItemCaseSensitive(item, "sellers");
        cJSON *amount = cJSON_GetObjectItemCaseSensitive(item, "amount");
        cJSON *url = cJSON_GetObjectItemCaseSensitive(item, "url");

        options[i].sellers = parse_sellers(sellers_json, &options[i].seller_count);
        options[i].amount = amount ? amount->valueint : 0;
        options[i].url = url ? strdup(url->valuestring) : NULL;
    }

    *count = array_size;
    cJSON_Delete(json);
    return options;
}

void free_card_options(CardOption *options, int count) {
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < options[i].seller_count; j++) {
            free(options[i].sellers[j].seller_name);
        }
        free(options[i].sellers);
        free(options[i].url);
    }
    free(options);
}
