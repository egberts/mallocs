#ifndef _WB_BUDDY_SIZE_H
#define _WB_BUDDY_SIZE_H
#include <stddef.h>
#include "config.h"
#include "wb_buddy_size.h"

size_t weighted_buddy_class_size(size_t class);


size_t weighted_buddy_class_for(size_t size, size_t nr_classes);
#endif  // _WB_BUDDY_SIZE_H
