#include "devices/disk.h"
#include "threads/vaddr.h"
#include "vm/swap.h"
#include "threads/synch.h"

static size_t swap_size;
static struct lock swap_lock;
static int counter_swap_in;
static int counter_swap_out;

void swap_init(){
	swap_disk = disk_get(1,1);
	swap_size = disk_size(swap_disk)*DISK_SECTOR_SIZE/PGSIZE;
	swap_map = bitmap_create(swap_size);

	lock_init(&swap_lock);
	counter_swap_in = 0;
	counter_swap_out = 0;
}

void swap_in(size_t index, void *kpage){
	lock_acquire(&swap_lock);
	int i;
	//printf("start of swap in\n");
	for(i=0; i < PGSIZE/DISK_SECTOR_SIZE;i++)
		disk_read(swap_disk,index * PGSIZE/DISK_SECTOR_SIZE + i, kpage+(DISK_SECTOR_SIZE*i));
	
	counter_swap_in ++;
	bitmap_set(swap_map,index,false);
	lock_release(&swap_lock);
}

size_t swap_out(void *kpage){
	lock_acquire(&swap_lock);
	
	//printf("start of swap out \n");
	size_t index=bitmap_scan_and_flip(swap_map,0,1,false);
	int i;
	for(i=0; i < PGSIZE/DISK_SECTOR_SIZE;i++)
		disk_write(swap_disk,index * PGSIZE/DISK_SECTOR_SIZE + i, kpage+(DISK_SECTOR_SIZE*i));

	counter_swap_out ++;
	lock_release(&swap_lock);
	return index;
}

void swap_free(size_t index){
	lock_acquire(&swap_lock);
	bitmap_set(swap_map,index,false);
	lock_release(&swap_lock);
}

void swap_counter(){
   if (!counter_swap_in && ! counter_swap_out)
     return;
   printf("swap_in :: %zu\n",counter_swap_in);
   printf("swap_out :: %zu\n",counter_swap_out);
}
