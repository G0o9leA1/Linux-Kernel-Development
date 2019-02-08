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
	int fd = shm_open(name, O_RDWR, S_IRWXU);
	void* ret_val = mmap(NULL, sizeof(struct shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ret_val == MAP_FAILED) {
		perror("mmap falied");	
		exit(1);
	}
	struct shared_data *shared_ptr = (struct shared_data *)ret_val;
	shared_ptr->write_guard = 1; // notify the leader to start writing
	while (shared_ptr->read_guard = 0) {} // waits for the leader to write the data
	//printArray(shared_ptr->data, shared_mem_size);
	int *local_arr = (int *)malloc(sizeof(int)*shared_mem_size);
	for (i = 0; i < shared_mem_size; i++) {
		local_arr[i] = shared_ptr->data[i];
	}
	shared_ptr->delete_guard = 1; // notify the leader that it is finished
	return 0;
}
