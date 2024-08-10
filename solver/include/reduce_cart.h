#ifndef REDUCE_CART_H
#define REDUCE_CART_H

#include <stddef.h>
#include "card_option.h"

void remove_uncommon_sellers(CardOption **options, size_t count);

#endif