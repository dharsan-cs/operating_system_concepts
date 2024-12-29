#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

const char *shared_memory_name = "shared_memory";

char *get_current_time() {
  time_t current_time = time(NULL);
  struct tm *tm_info = localtime(&current_time);
  char *buffer = malloc(80);
  strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", tm_info);
  return buffer;
}

void child_process() {
  size_t size = sizeof(sem_t);
  int shm_fd = shm_open(shared_memory_name, O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open failed");
    exit(EXIT_FAILURE);
  }

  sem_t *sem_ptr =
      mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (sem_ptr == MAP_FAILED) {
    perror("mapping failed");
    exit(EXIT_FAILURE);
  }

  // entry section (bounded wait ,not a busy wait)
  sem_wait(sem_ptr);
  // critical section
  char *current_time = get_current_time();
  printf("%d child_process in critical_section at : %s\n", getpid(), current_time);
  sleep(2);
  // exit section( incrments sem count ,wakes up any waiting process in queue)
  sem_post(sem_ptr);
  return;
}

int main() {
  size_t size = sizeof(sem_t);
  int shm_fd = shm_open(shared_memory_name, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open failed");
    exit(EXIT_FAILURE);
  }
  if (ftruncate(shm_fd, size) == -1) {
    perror("trunction failed");
    exit(EXIT_FAILURE);
  }

  sem_t *sem_ptr =
      mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (sem_ptr == MAP_FAILED) {
    perror("mapping failed");
    exit(EXIT_FAILURE);
  }

  unsigned int sem_count = 2;
  if (sem_init(sem_ptr, 1, sem_count) != 0) {
    perror("Sem_init failed");
    exit(EXIT_FAILURE);
  }

  int num_child = 5;
  for (int i = 0; i < num_child; i++) {
    pid_t pid = fork();

    if (pid < 0) {
      perror("fork failed");
      exit(EXIT_FAILURE);
    }

    else if (pid == 0) {
      child_process();
      return 0;
    }
  }

  for (int i = 0; i < num_child; i++) {
    wait(NULL);
  }

  munmap(sem_ptr, size);
  shm_unlink(shared_memory_name);
}
