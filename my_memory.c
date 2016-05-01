#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<math.h>
#include<assert.h>

//#define DEBUG
#define NUM_LEVELS 11

typedef struct node node;
typedef struct llist llist;

struct node {
	void *start_addr;
	void *end_addr;
	int size;
	node *next;
	node *prev;
};

struct llist {
	node *head;
	node *tail;
};

//API
void setup(int, int, void*);
void *my_malloc(int);
void my_free(void *);
//Top function
node *find_buddy_node(int);
//Internal
void my_free_buddy(void *);
void init_buddy_list();
llist *init_buddy_freelist(int);
llist *init_buddy_freelist_take2(void);
void merge_buddy_nodes(int, node *, node *);
int update_buddy_list(int, node *);
int delete_at_buddy_head(int);
int add_buddy_in_order_of_start_address(llist *, void *, void *, int);
int add_buddy_node_in_order_of_start_address(llist *, node *);
// Link List Operations
int add(llist *, void *, void *, int);
int add_node(llist *, node *);
int rmv_by_id(llist *, int);
// Debug
void print_queue(llist*);
void print_buddy_array(void);
//Global Variables
llist *ll;
llist* buddy_array[NUM_LEVELS] = { NULL };
int _type;
int _mem_size;
void* _start_of_address_space;

// Function Definitions
void setup(int malloc_type, int mem_size, void* start_of_memory) {
	_type = malloc_type;
	_mem_size = mem_size;
	_start_of_address_space = start_of_memory;
	if (_type == 3)
		init_buddy_list();
	else
	{
		printf("Buddy system is type 3\n");
		EXIT_FAILURE;
	}
}

void *my_malloc(int size) {

	node *temp = malloc(sizeof(node));
	switch (_type) {
	case 0:
		temp = NULL;
		break;
	case 1:
		temp = NULL;
		break;
	case 2:
		temp = NULL;
		break;
	case 3:
		temp = find_buddy_node(size);
		break;
	default:
		temp = find_buddy_node(size);
		break;
	}

	if (temp == NULL ) {
		return (void *) -1;
	} else {
		int node_start_address = (int)temp->start_addr;
		void *ram_sec_strt = _start_of_address_space + (int) temp->start_addr;
		*(int*) (ram_sec_strt) = size;
		return (void *) (_start_of_address_space + node_start_address + 4); //node_start_address + 4;
	}
}

void my_free(void* ptr)
{
	my_free_buddy(ptr);
}


//find the free node from the free list which can be used to accomodate the request using buddy algo.
node *find_buddy_node(int size) {
	// Min Request = 1 KB
	int buddy_index = ceil(log(size+4) / log(2)) - (log(1024) / log(2));

	llist *templ = malloc(sizeof(llist));
	node *tempn = malloc(sizeof(node));

	templ = buddy_array[buddy_index];

	if (templ == NULL ) // There is no free block at the given level. Move up a level and search
	{
		while (templ == NULL ) {
			// Try one higher
			buddy_index = buddy_index + 1;
			// Pointer to free list
			templ = buddy_array[buddy_index];
		}
		if (buddy_index < NUM_LEVELS) {
			// Node that needs to be moved from higher level to lower level
			tempn = templ->head;
			while (size < (int) 1024 * pow(2, buddy_index - 1)) {
				// Add to free list at a LOWER level
				// First check if needs to be initialized
				if (buddy_array[buddy_index - 1] == NULL ) {

					buddy_array[buddy_index - 1] = init_buddy_freelist_take2();

					int mid_addr = ((int) tempn->start_addr
							+ (int) tempn->end_addr) / 2;

					add(buddy_array[buddy_index - 1], tempn->start_addr,
							mid_addr, tempn->size / 2);
					//print_queue(buddy_array[buddy_index - 1]);
					add(buddy_array[buddy_index - 1], mid_addr, tempn->end_addr,
							tempn->size / 2);
					//print_queue(buddy_array[buddy_index - 1]);
				} else {
					// Add buddies in order
					add_buddy_in_order_of_start_address(
							buddy_array[buddy_index - 1], tempn->start_addr,
							(int) tempn->end_addr / 2, tempn->size / 2);
					add_buddy_in_order_of_start_address(
							buddy_array[buddy_index - 1],
							(int) tempn->end_addr / 2, tempn->end_addr,
							tempn->size / 2);
				}
				// Allocate => Remove from free list at higher level
				delete_at_buddy_head(buddy_index);
				buddy_index = buddy_index - 1;
				templ = buddy_array[buddy_index];
				tempn = templ->head;
			}
			// Delete the node that is being returned from the free list
			delete_at_buddy_head(buddy_index);
		} else {
			//printf("Out of memory\n");
			tempn = NULL;
		}
	} else {
		tempn = templ->head;  // There is a free block. Give it the first one.
		// Delete the node that is being returned from the free list
		delete_at_buddy_head(buddy_index);
	}
	//printf("ALLOCATION DONE\n");
	return tempn;
}

// Function to free memory - Buddy System
void my_free_buddy(void *ptr) {
	ptr = (void *) (ptr - 4);		//compensate for the header
	int size = *(int *) ptr;//get size
	// Min Request = 1 KB
	int buddy_index = ceil(log(size+4) / log(2)) - (log(1024) / log(2));
	int block_size = (int) 1024 * pow(2, buddy_index);

	*(int *) (ptr) = 0;	//clear the size stored at that memory location.
	node *new = malloc(sizeof(node));
	new->start_addr = ptr - _start_of_address_space;//_start_of_address_space+(int)ptr;
	new->end_addr = new->start_addr + block_size;	// + 4;
	new->size = block_size;	// + 4;

	llist *templ = malloc(sizeof(llist));
	templ = buddy_array[buddy_index];

	if (templ == NULL ) {  // empty list so only this node needs to be inserted => initialize list and add head and tail
		templ = init_buddy_freelist_take2();
		templ->head = new;
		templ->tail = new;
		buddy_array[buddy_index] = templ;
	} else { // add in order of start address
		node *tempn = malloc(sizeof(node));
		tempn = templ->head;
		while (tempn != NULL && tempn->start_addr < new->start_addr) {
			tempn = tempn->next;
		}
		if (tempn == NULL ) { //to be put at tail
			templ->tail->next = new;
			new->prev = templ->tail;
			templ->tail = new;
			templ->tail->next = NULL;
		} else if (templ->head == tempn) { //to be put at head
			templ->head->prev = new;
			new->next = templ->head;
			templ->head = new;
			templ->head->prev = NULL;
		} else { //to be put in between
			new->prev = tempn->prev;
			new->next = tempn;
			tempn->prev->next = new;
			tempn->prev = new;
		}
	}

    // if there are buddies, recursively coalesce using merge operation
	if (((int) (new->start_addr) / ((int) pow(2, buddy_index) * 1024) % 2)
			!= 0) { // odd
		if (new->prev != NULL && new->prev->end_addr == new->start_addr) {
			merge_buddy_nodes(buddy_index, new->prev, new); // merge at this level
		}
	} else // even
	{
		if (new->next != NULL && new->next->start_addr == new->end_addr) { //merging with next node
			merge_buddy_nodes(buddy_index, new, new->next); // merge at this level
		}
	}
}

// Initialize a linked free list for a specific block size
llist* init_buddy_freelist(int block_size) {
	llist *freelist;
	freelist = malloc(sizeof(llist));
	freelist->head = NULL;
	freelist->tail = NULL;
	add(freelist, 0, block_size, block_size);
	return freelist;
}

// Initialize a linked free list  without mentioning block size
llist* init_buddy_freelist_take2(void) {
	llist *freelist;
	freelist = malloc(sizeof(llist));
	freelist->head = NULL;
	freelist->tail = NULL;
	//	add(freelist,0,0,0);
	return freelist;
}

// Initialize an array of pointers. Each pointer points to a free list of a specific size. A 1 MB linked list is set up as free
void init_buddy_list() {
	assert(1024*1024 == _mem_size);
	// 	Point last array element to free list of 1 MB
	buddy_array[10] = init_buddy_freelist(_mem_size);
	// 	Maintain consistency with other 3 algorithms. Hence ll now points to this free 1 MB block of memory
	ll = buddy_array[10];
}

int add(llist *ll, void *start, void *end, int size) {
	node *new = malloc(sizeof(node));
	if (new == NULL )
		return -1;
	new->start_addr = start;
	new->end_addr = end;
	new->size = size;
	new->next = NULL;
	new->prev = NULL;
	if (ll->tail != NULL ) {
		new->prev = ll->tail;
		ll->tail->next = new;
	}
	ll->tail = new;
	if (ll->head == NULL ) /* first value */
		ll->head = new;
	return 0;
}


int add_node(llist *ll, node *new) {
	if (new == NULL )
		return -1;
	else {
		if (ll->tail != NULL ) {
			new->prev = ll->tail;
			ll->tail->next = new;
		}
		ll->tail = new;
		if (ll->head == NULL ) /* first value */
			ll->head = new;
		return 0;
	}
}

int delete_at_buddy_head(int level) {
	llist* temp = buddy_array[level];
	temp->head = temp->head->next;
	if (temp->head == NULL )
		buddy_array[level] = NULL; // SKA - 04/30/2016 temp->head; // test
	return 0;
}

// Function to add into free list based on start address (assumption : at least one node is in list already
int add_buddy_in_order_of_start_address(llist *ll, void* start, void *end,
		int size) {
	#ifdef DEBUG
	printf("%s\n", __func__);
	#endif
	node *new = malloc(sizeof(node));
	node* temp = malloc(sizeof(node));
	if (new == NULL || temp == NULL )
		return -1;

	new->start_addr = start;
	new->end_addr = end;
	new->size = size;

	if (ll->head == NULL ) {	//empty list
		ll->head = new;
		ll->tail = new;
		new->next = NULL;
		new->prev = NULL;
	} else if (ll->head->start_addr > start) {	//to be added at head
		new->next = ll->head;
		ll->head->prev = new;
		new->prev = NULL;
		ll->head = new;
	} else if (ll->tail->start_addr < start) {	//to be added at the tail
		new->next = NULL;
		new->prev = ll->tail;
		ll->tail->next = new;
		ll->tail = new;
	} else {	//add in the middle with
		while (temp != NULL && start < temp->start_addr)
			temp = temp->next;
		//start < temp->start_addr => new to be put before temp
		temp->prev->next = new;
		new->prev = temp->prev;
		new->next = temp;
		temp->prev = new;
	}
	return 0;
}

int add_buddy_node_in_order_of_start_address(llist *ll, node* new) 
{
	#ifdef DEBUG
	printf("%s\n", __func__);
	#endif
	node* temp = malloc(sizeof(node));
	if (temp == NULL )
		return -1;

	if (ll->head == NULL ) {	//empty list
		ll->head = new;
		ll->tail = new;
		new->next = NULL;
		new->prev = NULL;
	} else if (ll->head->start_addr > new->start_addr) {	//to be added at head
		new->next = ll->head;
		ll->head->prev = new;
		new->prev = NULL;
		ll->head = new;
	} else if (ll->tail->start_addr < new->start_addr) {	//to be added at the tail
		new->next = NULL;
		new->prev = ll->tail;
		ll->tail->next = new;
		ll->tail = new;
	} else {	//add in the middle with
		while (temp != NULL && new->start_addr < temp->start_addr)
			temp = temp->next;
		//start < temp->start_addr => new to be put before temp
		temp->prev->next = new;
		new->prev = temp->prev;
		new->next = temp;
		temp->prev = new;
	}
	return 0;
}

// Function to update buddy list after a merge
int update_buddy_list(int index, node *tempn) {
	#ifdef DEBUG
	printf("%s:level %d\n", __func__, index);
	#endif
	llist *templ = malloc(sizeof(llist));
	//llist *templ2 = malloc(sizeof(llist));
	if (templ == NULL)
		return -1;

	templ = buddy_array[index];
	//templ2 = buddy_array[index + 1];
	// Take the misfit, move it up one level
	if (tempn != NULL ) {
		if (templ == NULL ) // Need to initialize the level above and add the misfit as first node
		{
			buddy_array[index] = init_buddy_freelist_take2();
			add_node(buddy_array[index], tempn);
		} else	// just add in order
		{
			add_buddy_node_in_order_of_start_address(templ, tempn);
		}
	}
	return 0;
}

// Function to recursively merge buddies
void merge_buddy_nodes(int buddy_level, node *first, node *second) {
	#ifdef DEBUG
	printf("%s: level %d\n", __func__, buddy_level);
	#endif
	llist *templ = malloc(sizeof(llist));
	templ = buddy_array[buddy_level];

	buddy_level = buddy_level + 1;

	if (buddy_level > NUM_LEVELS-1)
		return; // base case
	else {
		node *node_at_upper_level = malloc(sizeof(node));
		node_at_upper_level->start_addr = first->start_addr;
		node_at_upper_level->end_addr = second->end_addr;
		node_at_upper_level->size = first->size + second->size;
		node_at_upper_level->next = NULL;  // this will be updated in update_buddy_list
		node_at_upper_level->prev = NULL;  // this will be updated in update_buddy_list

		int stat = -1;

		if (templ != NULL ) {
			stat = rmv_by_id(templ, (int) first->start_addr);
			stat = rmv_by_id(templ, (int) second->start_addr);

			if (stat == -1 )
				buddy_array[buddy_level - 1] = NULL; // list is empty after removing nodes
		} else {
			buddy_array[buddy_level - 1] = NULL; // List is empty;
			return;
		}
		// update next level => add node in order of start address
		update_buddy_list(buddy_level, node_at_upper_level);

		// check for merge again
		if (((int) (node_at_upper_level->start_addr)
				/ ((int) pow(2, buddy_level) * 1024) % 2) != 0) { // odd
			printf("Odd\n");
			if (node_at_upper_level->prev != NULL
					&& node_at_upper_level->prev->end_addr
							== node_at_upper_level->start_addr) { //merging with previous node
				merge_buddy_nodes(buddy_level, node_at_upper_level->prev,
						node_at_upper_level);
			}
		} else {	// even
			if (node_at_upper_level->next != NULL
					&& node_at_upper_level->next->start_addr
							== node_at_upper_level->end_addr) { //merging with next node
				merge_buddy_nodes(buddy_level, node_at_upper_level,
						node_at_upper_level->next);
			} 
		}
	}
}

void print_queue(llist *ll) {
	node* current = malloc(sizeof(node));
	current = ll->head;

	while (current != NULL ) {
		printf("(%d,%d,%d)->", (int) current->start_addr,
				(int) current->end_addr, current->size);
		current = current->next;
	}
	printf("\n");
}

void print_buddy_array(void) {
	int i;
	for (i = 0; i < NUM_LEVELS; i++) {
		printf("Pointer at level %d is %d", i, (int) buddy_array[i]);
		if (buddy_array[i] != NULL )
			print_queue(buddy_array[i]);
		else
			printf("\n");
	}
}

// This function removes node based on start address. If the head is removed, it will return -1 indicating list is empty.
int rmv_by_id(llist *ll, int id) {

	if (ll->head == NULL ) {
		return -1 ; // can't remove anything, list is empty
	}
	node *temp = malloc(sizeof(node));
	temp = ll->head;

	if ((int)ll->head->start_addr == id) { //if the head has to be removed
		ll->head = temp->next;
		if (temp->next != NULL ) //if temp != tail
			temp->next->prev = NULL; //ll->head; // SKA
		else { //if temp == tail
			ll->tail = ll->head;
		}
		free(temp);
		if (ll->head == NULL ) {
			ll->tail = NULL;
			return -1 ;  // List is now empty
		} else
			return id;
	}
	while (temp != NULL && (int)temp->start_addr != id) {
		temp = temp->next;
	}
	if (temp == NULL ) {
		return -1;
	}
	if (temp->prev != NULL )
		temp->prev->next = temp->next;
	if (temp->next != NULL )
		temp->next->prev = temp->prev;
	else
		ll->tail = temp->prev;

	free(temp);
	return id;
}
