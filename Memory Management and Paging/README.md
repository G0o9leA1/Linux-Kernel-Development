# Source Code
This the source code of Operating Systems Organization.

### Project Website
For any detailed information about this project, please refer to the project website.
https://www.cse.wustl.edu/~brian.kocoloski/courses/cse422s/labs/08_monitoring_framework.html

### General Description
In this project we
  - Implement a kernel module that leverages the mmap() system call to perform virtual memory mappings for a process. Processes will interact with your module via a special device file, in this case /dev/paging. When a process issues the mmap() call to this file, our module code will be invoked
  - Our module can perform two main tasks when it is invoked in this fashion: (1) allocate physical memory for the process, and (2) map a new virtual address from this process to the new physical memory that we allocated for task (1)
  - Our module can be configured to operate in one of two modes: (1) demand paging, or (2) pre paging. 
  - Evaluate the performance differences between  two different configurations, focusing on how they affect the system call execution time for the mmap call, and the runtime of a matrix multiplication application that uses the memory we map for it


## Copyright:
  - If you are students of Washington University in St. Louis, you should cite properly and do not violate the academic integrity when you using this code.

  - Please contact the author through [email](mailto:Li.z@wustl.edu) if you want to use this code in other ways
