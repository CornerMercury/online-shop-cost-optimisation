
# Online Shop Cost Optimisation

Outdated! Use [this](https://github.com/CornerMercury/online-shop-cost-optimisation-LP) instead! It's a lot faster.

A brute force algorithm to minimise cost for the delivery of a large amount of items, particularly optimised for situations where the delivery cost is not negligible and sellers are common between items.

An example of this is [Cardmarket](https://www.cardmarket.com/en/). I made this project for this site, so feel free to ask for the webscraper.




## Usage

### JSON Format

Scrape the item data into this format.

- Each item in the outer array represents an item you want to buy.
- Cost is stored in the base unit, so cents or pence etc.
```json
[
  {
    "sellers": [
        {
        "available": 1,
        "cost": 1,
        "name": "seller"
      }
    ],
    "amount": 1,
    "url": "https://google.com"
  }
]
```

### Edit Constants
- In ./solver/include/constants.h, edit these values to your liking.

### Run on Linux

- Set the C compiler in the makefile
```makefile
CC = gcc   # <-- Compiler
CFLAGS = -I./include -Wall -Wextra -Werror -Wno-error=unused-parameter -O3 -march=native
```

- Install [make](https://www.gnu.org/software/make/), then run:
```sh
make
```
- Run the solver by supplying a json file located in .jsonCarts:
```sh
./solver <filename>
```