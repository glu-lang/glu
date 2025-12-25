//
// This is the implementation file for the header.
//

#include "simple.h"

size_t getPizzaCount(void)
{
    return 3;
}

struct Pizza *createFavoritePizza(enum PizzaSize size)
{
    static struct Pizza pizza;
    pizza.size = size;
    pizza.name = "Babylone";
    pizza.price = 14.50;
    return &pizza;
}

char const *getCC(void)
{
    return __VERSION__;
}
