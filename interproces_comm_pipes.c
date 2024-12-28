#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  int pipe_fd[2];
  int write_end = 1, read_end = 0;

  if (pipe(pipe_fd) == -1) {
    perror("pipe");
    exit(1);
  }

  pid_t child_process_id = fork();

  if (child_process_id < 0) {
    printf("Fork failed");
    exit(1);
  }

  else if (child_process_id == 0) {
    //write end of the pipe is open ,if buffer is empty while trying to read child process get blocked 
    // buffer for reading message form the pipe
    char buffer[256];

    // reading message from the pipe
    if (read(pipe_fd[read_end], buffer, sizeof(buffer)) == 0) {
      printf("child process : pipe is empty\n");
    } else {
      printf("Chile process read : %s\n", buffer);
    }
    // closing the ends of the pipe
    close(pipe_fd[read_end]);
    close(pipe_fd[write_end]);
  }

  else {
    // closing the read end of the process
    close(pipe_fd[read_end]);

    // writing message to the pipe
    char *message = "hi child process how r u";
    if (write(pipe_fd[write_end], message, strlen(message) + 1) == -1) {
      perror("Parent process");
    }

    // closing the write end of the pipe
    close(pipe_fd[write_end]);

    wait(NULL);
  }
}