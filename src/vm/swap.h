#include <stddef.h>
#include "devices/disk.h"

struct disk *swap_disk;
struct bitmap *swap_map;

void swap_init();
void swap_in(size_t ind, void *kpage);
size_t swap_out(void *kpage);

void swap_counter(void);
