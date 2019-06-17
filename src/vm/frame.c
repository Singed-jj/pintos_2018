#include <hash.h>
#include <list.h>
#include <stdio.h>
#include "lib/kernel/list.h"

#include "vm/frame.h"
#include "vm/page.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"


static struct hash frame_table;
static struct lock frame_lock;
static struct list frame_list;

void
frame_init(void)
{
  lock_init (&frame_lock);
  hash_init (&frame_table, frame_hash, frame_less, NULL);
  list_init(&frame_list);

}

void *
frame_palloc_get_page(enum palloc_flags flags, uint8_t *upage)
{
  void * allocated_frame;
  struct frame * f = malloc(sizeof(struct frame));
  allocated_frame = palloc_get_page(PAL_USER | flags);
  if(allocated_frame == NULL){

    find_victim();
    allocated_frame = palloc_get_page(PAL_USER | flags);
    
    // there is no frame left. should do swap.
    //return NULL;
  }
  f -> kpage = allocated_frame;
  f -> upage = upage;
  f -> holder = thread_current();
  
  struct hash_elem *e =  hash_insert(&frame_table, &f->hash_elem);

  if (!e){
    list_push_back(&frame_list,&f->l_elem);
    //printf("list_size %d \n",list_size(&frame_list));
    return allocated_frame;
  }
  free(f);

  return hash_entry(e, struct frame, hash_elem);
}

void
frame_palloc_free_page (uint8_t *kpage)
{
  //printf("reached free page 101 \n");
  ASSERT(kpage != NULL);

  struct frame * f = (struct frame *)malloc(sizeof(struct frame));
  f -> kpage = kpage;
  struct hash_elem *e = hash_delete(&frame_table, &(f->hash_elem));
  
  
  if (!e){
    free (f);
    return;
  }
  list_remove(&hash_entry(e,struct frame, hash_elem)->l_elem);
  palloc_free_page (kpage);
  free(hash_entry(e, struct frame, hash_elem));
  free (f);
}

void find_victim(void){

  struct list_elem *elem;
  struct frame *f1;
  bool done =false;
  int i;
  for(i=0;i<2;i++){
    for(elem=list_begin(&frame_list);elem!=list_end(&frame_list);elem=list_next(elem)){
      f1=list_entry(elem, struct frame, l_elem);
      if(!pagedir_is_accessed(f1->holder->pagedir, f1->upage)){
        done =true;
        break;
      }
      pagedir_set_accessed(f1->holder->pagedir,f1->upage,false);
    }
  }

  if(done)
    clear_victim(f1);

}

void clear_victim(struct frame *victim){
  //printf("clear victim called\n");
  pagedir_clear_page(victim->holder->pagedir, victim->upage);
  size_t index = swap_out(victim->kpage);
  struct page *p1;
  p1 = page_find(&victim->holder->page_table,victim->upage);
  p1->on_kmem=false;
  p1->swap_index=index;
  p1->kpage=NULL;
  //printf("swapped out index %d \n", index);
  //printf("swapped out kmem %d \n", p1->on_kmem);
  frame_palloc_free_page(victim->kpage);
}

unsigned frame_hash(const struct hash_elem *fr, void *aux UNUSED){
  struct frame* f = hash_entry(fr, struct frame, hash_elem);
  return hash_bytes (&f->kpage, sizeof(f->kpage));
}

bool frame_less(const struct hash_elem *first, const struct hash_elem *second, void *aux UNUSED){
  const struct frame *f1 = hash_entry(first, struct frame, hash_elem);
  const struct frame *f2 = hash_entry(second, struct frame, hash_elem);
  return f1->kpage < f2->kpage;
}

void
frame_lock_acquire(void){
  lock_acquire(&frame_lock);
}

void
frame_lock_release(void){
  lock_release(&frame_lock);
}
