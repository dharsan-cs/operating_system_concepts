#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int read_end = 0;
int write_end = 1;

// reads a string from file1 and push it reader_processor_pipe
void read_process(char *file_name, int *pipe_fd) {
  close(pipe_fd[read_end]);
  FILE *file_ptr = fopen(file_name, "r");
  char string[256];
  while (fgets(string, sizeof(string), file_ptr)) {
    if (write(pipe_fd[write_end], string, strlen(string) + 1) == -1) {
      perror("Reader");
    }
  }
  if (write(pipe_fd[write_end], "eof", strlen("eof") + 1) == -1) {
    perror("error");
  }
  close(pipe_fd[write_end]);
  return;
}

void reverse_process(int *read_processor_pipe, int *processor_writer_pipe) {
  close(processor_writer_pipe[read_end]);
  close(read_processor_pipe[write_end]);

  fcntl(read_processor_pipe[read_end], F_SETFL, O_NONBLOCK);
  char string[256], temp;
  int n;

  while (1) {
    printf("1\n");

    if (read(read_processor_pipe[read_end], string, sizeof(string)) == -1) {
      perror("reverser");
    }

    printf("1.1\n");

    if (strcmp(string, "eof") == 0) {
      if (write(processor_writer_pipe[write_end], string, strlen(string) + 1) ==
          -1) {
        perror("Reverser");
      }
      return;
    }

    printf("1.2\n");

    n = strlen(string) - 1;
    for (int i = 0; i < n / 2; i++) {
      temp = string[i];
      string[i] = string[n - i - 1];
      string[n - i - 1] = temp;
    }

    printf("1.3\n");

    if (write(processor_writer_pipe[write_end], string, strlen(string) + 1) ==
        -1) {
      perror("Reverse");
    }

    printf("2\n");
  }

  close(read_processor_pipe[read_end]);
  close(read_processor_pipe[write_end]);
  close(processor_writer_pipe[write_end]);
}

void writer_process(char *file_name, int *processor_writer_pipe) {
  close(processor_writer_pipe[write_end]);

  FILE *file_ptr = fopen(file_name, "w");
  char string[256];
  while (1) {
    printf("a\n");

    if (read(processor_writer_pipe[read_end], string, sizeof(string)) == -1) {
      perror("writer\n");
    }

    printf("a1\n");

    if (strcmp(string, "eof") == 0) {
      return;
    }

    printf("a2\n");

    fprintf(file_ptr, "%s\n", string);

    printf("b\n");
  }
  fclose(file_ptr);
  close(processor_writer_pipe[read_end]);
  close(processor_writer_pipe[write_end]);
}

int main() {
  int read_processor_pipe[2], processor_writer_pipe[2];

  if (pipe(read_processor_pipe) || pipe(processor_writer_pipe)) {
    perror("pipe");
    exit(1);
  }

  for (int i = 0; i < 3; i++) {
    pid_t pid = fork();

    // if fork failed
    if (pid < 0) {
      printf("Fork failed");
      exit(1);
    }

    // child process
    else if (pid == 0) {
      switch (i) {
      case 0:
        read_process("file1.txt", read_processor_pipe);
        return 0;
      case 1:
        reverse_process(read_processor_pipe, processor_writer_pipe);
        return 0;
      case 2:
        writer_process("file2.txt", processor_writer_pipe);
        return 0;
      default:
        printf("Got into default");
        return 0;
      }
    }
  }
  printf("xyz\n");
  // Close all pipe ends in the parent process
  close(read_processor_pipe[read_end]);
  close(read_processor_pipe[write_end]);
  close(processor_writer_pipe[read_end]);
  close(processor_writer_pipe[write_end]);

  printf("pipes closed\n");
  for (int i = 0; i < 3; i++) {
    wait(NULL);
  }
  return 0;
}
