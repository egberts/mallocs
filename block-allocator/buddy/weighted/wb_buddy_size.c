#include <stddef.h>
#include "config.h"
#include "wb_buddy_size.h"

size_t
weighted_buddy_class_size(size_t class)
{
    size_t order = class / 2;
    size_t size = (size_t)1 << order;

    if (class & 1)
        size += size / 2;

    return size;
}


size_t
weighted_buddy_class_for(size_t size, size_t nr_classes)
{
    size_t class;

    if (!size)
        return SIZE_MAX;

    for (class = 0; class < nr_classes; ++class) {
        if (weighted_buddy_class_size(class) >= size)
            return class;
    }

    return SIZE_MAX;
}
