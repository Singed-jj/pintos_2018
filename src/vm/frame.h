#ifndef VM_FRAME_H
#define VM_FRAME_H


#include <hash.h>
#include "threads/palloc.h"




struct frame {
  uint8_t * kpage;
  uint8_t * upage;

  struct thread * holder;
  struct hash_elem hash_elem;
  struct list_elem l_elem;
};


void frame_init (void);
void * frame_palloc_get_page (enum palloc_flags flags, uint8_t * upage);
void frame_palloc_free_page (uint8_t * kpage);
void find_victim(void);
void clear_victim(struct frame *victim);

unsigned frame_hash(const struct hash_elem *, void *);
bool frame_less(const struct hash_elem *, const struct hash_elem *, void *);
void frame_lock_acquire(void);
void frame_lock_release(void);
#endif /* vm/frame.h */

