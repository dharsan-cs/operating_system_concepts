#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct message {
  char parent_message[100];
  char child_message[100];
} message;

const char *shared_memory_name = "shared_memory";
// shared memory ipc

void child_process() {
  // opening the shared memory
  int shm_fd = shm_open(shared_memory_name, O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("child process shm_open failed");
    exit(EXIT_FAILURE);
  }
  // accessing the shared memory
  message *ptr =
      mmap(0, sizeof(message), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (ptr == MAP_FAILED) {
    perror("mapping failed");
    exit(EXIT_FAILURE);
  }

  //
  strcpy(ptr->child_message, "hello from child");
  return;
}

int main() {

  const size_t size = sizeof(message);
  // shared memory object is created
  int shm_fd = shm_open(shared_memory_name, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shared memory object intialisation failed");
    exit(EXIT_FAILURE);
  }

  // shared memory size is configured
  if (ftruncate(shm_fd, size) == -1) {
    perror("shared memory trunctio failed");
    exit(EXIT_FAILURE);
  }

  // mapping a integer variable in shared memory
  message *ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (ptr == MAP_FAILED) {
    perror("mapping failed");
    exit(EXIT_FAILURE);
  }

  pid_t pid = fork();

  if (pid < 0) {
    perror("fork failed");
    exit(EXIT_FAILURE);
  }

  else if (pid == 0) {
    child_process();
    return 0;
  }

  else {
    strcpy(ptr->parent_message, "HI im Parent");
    wait(NULL);
    printf("message object par_msgg : %s\n", ptr->child_message);
    printf("message object child_msgg : %s\n", ptr->parent_message);
    return 0;
  }

  shm_unlink(shared_memory_name);
}