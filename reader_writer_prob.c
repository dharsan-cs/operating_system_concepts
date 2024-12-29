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

char *get_current_time() {
  time_t current_time = time(NULL);
  struct tm *tm_info = localtime(&current_time);
  char *buffer = malloc(80);
  strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", tm_info);
  return buffer;
}

const char *shared_memory_name = "shared_memory";

typedef struct synch_mech {
  int num_readers;
  sem_t reader_sem;
  sem_t writer_sem;
} synch_mech;

synch_mech *access_shared_resource() {
  int shm_fd = shm_open(shared_memory_name, O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open failed");
    exit(EXIT_FAILURE);
  }
  synch_mech *ptr = mmap(NULL, sizeof(synch_mech), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (ptr == MAP_FAILED) {
    perror("mapping failed");
    exit(EXIT_FAILURE);
  }
  return ptr;
}

void reader_process() {
  synch_mech *ptr = access_shared_resource();

  sem_wait(&(ptr->reader_sem));
  if (ptr->num_readers == 0) {
    sem_wait(&(ptr->writer_sem));
  }
  ptr->num_readers++;
  sem_post(&(ptr->reader_sem));

  char *current_time = get_current_time();
  printf("%d Reader_process reading at : %s\n", getpid(), current_time);
  sleep(2);

  sem_wait(&(ptr->reader_sem));
  ptr->num_readers--;
  if (ptr->num_readers == 0) {
    sem_post(&(ptr->writer_sem));
  }
  sem_post(&(ptr->reader_sem));
}

void writer_prcess() {
  synch_mech *ptr = access_shared_resource();
  sem_wait(&(ptr->writer_sem));
  char *current_time = get_current_time();
  printf("%d Writer_process writing at : %s\n", getpid(), current_time);
  sleep(2);
  sem_post(&(ptr->writer_sem));
}




int main() {
  int shm_fd = shm_open(shared_memory_name, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open failed");
    exit(EXIT_FAILURE);
  }
  if (ftruncate(shm_fd, sizeof(synch_mech)) == -1) {
    perror("trunction failed");
    exit(EXIT_FAILURE);
  }

  synch_mech *ptr = mmap(NULL, sizeof(synch_mech), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (ptr == MAP_FAILED) {
    perror("mapping failed");
    exit(EXIT_FAILURE);
  }

  if (sem_init(&(ptr->reader_sem), 1, 1) != 0 || sem_init(&(ptr->writer_sem), 1, 1) != 0) {
    perror("Sem_init failed");
    exit(EXIT_FAILURE);
  }
  ptr->num_readers = 0;

  int num_child = 10;
  for (int i = 0; i < num_child; i++) {
    pid_t pid = fork();

    if (pid < 0) {
      perror("fork failed");
      exit(EXIT_FAILURE);
    }

    else if (pid == 0) {
      if (i % 2 == 0) {
        writer_prcess();
        return 0;
      } else {
        reader_process();
        return 0;
      }
    }
  }

  for (int i = 0; i < num_child; i++) {
    wait(NULL);
  }
  munmap(ptr, sizeof(synch_mech));
  shm_unlink(shared_memory_name);
}