#define shared_mem_size 10000000
const char* name = "my_shared_memory";
struct shared_data {
	volatile int write_guard;
	volatile int read_guard;
	volatile int delete_guard;
	volatile int data[shared_mem_size];
};
