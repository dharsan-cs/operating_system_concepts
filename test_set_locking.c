#include <errno.h>
#include <fcntl.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//
typedef struct shared_data {
  int count;
  atomic_flag lock;
} shared_data;

//
char *shared_memory_name = "/shared_memory";

void init_shared_data(shared_data *ptr) {
  ptr->count = 0;
  atomic_flag_clear(&ptr->lock);
}

void child_process(int pid) {
  int shm_fd = shm_open(shared_memory_name, O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open failed");
    exit(EXIT_FAILURE);
  }

  shared_data *ptr = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (ptr == MAP_FAILED) {
    perror("mapping failed");
    exit(EXIT_FAILURE);
  }

  // entry section(busy waiting /non bounded waiting)
  while (atomic_flag_test_and_set(&(ptr->lock))) {
  }
  // critical section
  if (pid % 2 == 0) {
    ptr->count++;
  } else {
    ptr->count--;
  }
  // exit section
  atomic_flag_clear(&(ptr->lock));
  return;
}

int main() {
  size_t size = sizeof(shared_data);
  int shm_fd = shm_open(shared_memory_name, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open failed");
    exit(EXIT_FAILURE);
  }
  if (ftruncate(shm_fd, size) == -1) {
    perror("trunction failed");
    exit(EXIT_FAILURE);
  }

  shared_data *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (ptr == MAP_FAILED) {
    perror("mapping failed");
    exit(EXIT_FAILURE);
  }
  init_shared_data(ptr);
  int num_child = 10;
  for (int i = 0; i < num_child; i++) {
    pid_t pid = fork();

    if (pid < 0) {
      perror("fork failed");
      exit(EXIT_FAILURE);
    }

    else if (pid == 0) {
      child_process(i);
      return 0;
    }
  }

  for (int i = 0; i < num_child; i++) {
    wait(NULL);
  }

  printf("count(expected zero): %d\n", ptr->count);
  munmap(ptr, size);
  shm_unlink(shared_memory_name);
}
