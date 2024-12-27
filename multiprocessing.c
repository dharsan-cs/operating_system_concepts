#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  int child_process_id = fork();

  if (child_process_id < 0) {
    printf("Fork failed");
    return 0;
  }

  else if (child_process_id == 0) {
    printf("Child process id : %d\n", getpid());
    return 0;
  }

  else {
    printf("Parend process id : %d\n", getpid());
    printf("Child id from parent : %d\n", child_process_id);
    wait(NULL);
    return 0;
  }
}