
#include <stdint.h>
#include <hash.h>
#include "filesys/file.h"
#include "threads/synch.h"

struct page
{

  uint8_t * kpage;
  uint8_t * upage;

  struct thread * t;  
  bool dirty_bit;
  //bool swapped;
  size_t swap_index;
  bool on_kmem;  

  struct hash_elem hash_elem;
};

bool page_table_init(struct hash *);
struct page * page_create(void *, void*);
struct page * page_find(struct hash *, void*);
static unsigned page_hash(const struct hash_elem *, void * aux);
bool page_less (const struct hash_elem*, const struct hash_elem*, void * aux);

void page_remove(struct hash *);
void page_remove_helper(struct hash_elem *, void * aux);

void * stack_grow(void * upage);
