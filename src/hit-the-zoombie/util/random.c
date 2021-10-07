#include "random.h"

#include <stdlib.h>

inline int random_range_int(int min, int max)
{
    return (rand() % (max - min + 1)) + min;
}
