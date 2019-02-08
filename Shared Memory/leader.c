/*
* Author: Chengyue Gong, Zixuan Li, Wentao Wang
* Orgnization: Washington University in St. Louis
*/ 

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "structure.h"

void printArray(volatile int *arr, int n) {
	int i;
	for (i = 0; i < n; i++) {
		printf("%d ", arr[i]);
	}
	printf("\n");
}

int main()
{
	int i;
	int fd = shm_open(name, O_RDWR | O_CREAT, S_IRWXU);
	ftruncate(fd, sizeof(struct shared_data));
	void *ret_val = mmap(NULL, sizeof(struct shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ret_val == MAP_FAILED) {
		perror("mmap failed");	
		exit(1);
	}
	struct shared_data *shared_ptr = (struct shared_data *)ret_val;
	if (shared_ptr == NULL) {
		perror("malloc failed");
		exit(1);
	}
	while (shared_ptr->write_guard == 0) {} // waits for the follower to be created
	int *local_arr = (int *)malloc(sizeof(int)*shared_mem_size);
	srand(1);
	for (i = 0; i < shared_mem_size; i++) {
		local_arr[i] = rand();
		//d->data[i] = local_arr[i];
	}
	memcpy((void *)shared_ptr->data, (void *)local_arr, sizeof(int)*shared_mem_size);
	shared_ptr->read_guard = 1; // notify the follower to start reading
	//printArray(shared_ptr->data, shared_mem_size);
	while(shared_ptr->delete_guard == 0) {} // waits for the follower to finish reading
	shm_unlink(name);
	return 0;
}
