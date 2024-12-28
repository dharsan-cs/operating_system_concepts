#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>


const char *reader_reverser_queue = "/reader_reverser_queue_01";
const char *reverser_writer_queue = "/reverser_writer_queue_01";
int message_max_size = 256;
char *eof = "EOF";

void reader_process(char *file_name) {
  mqd_t queue = mq_open(reader_reverser_queue, O_WRONLY);
  if (queue == (mqd_t)-1) {
    perror("Reader_prcss q open error");
    exit(1);
  }
  FILE *file = fopen(file_name, "r");
  char buffer[message_max_size];
  while (fgets(buffer, sizeof(buffer), file)) {
    mq_send(queue, buffer, strlen(buffer) + 1, 1);
  }
  mq_send(queue, eof, strlen(eof) + 1, 1);

  if (mq_close(queue) == -1) {
    perror("Reader_prcc q close error");
  }
  fclose(file);
}

void reverser_process() {
  mqd_t read_rev_q = mq_open(reader_reverser_queue, O_RDONLY);
  mqd_t rev_write_q = mq_open(reverser_writer_queue, O_WRONLY);
  unsigned int priority;

  if (read_rev_q == (mqd_t)-1 || rev_write_q == (mqd_t)-1) {
    perror("Reverse_prcc q open error");
    exit(1);
  }
  char buffer[message_max_size];
  while (1) {
    mq_receive(read_rev_q, buffer, sizeof(buffer), &priority);

    if (strcmp(buffer, eof) == 0) {
      mq_send(rev_write_q, eof, strlen(eof) + 1, 1);
      if (mq_close(read_rev_q) == -1 || mq_close(rev_write_q) == -1) {
        perror("Reverse_prcc q close error");
      }
      return;
    }

    int len = strlen(buffer);
    for (int i = 0; i < len / 2; i++) {
      char temp = buffer[i];
      buffer[i] = buffer[len - i - 1];
      buffer[len - i - 1] = temp;
    }
    mq_send(rev_write_q, buffer, strlen(buffer) + 1, 1);
  }
}

void writer(char *file_name) {
  mqd_t queue = mq_open(reverser_writer_queue, O_RDONLY);
  if (queue == (mqd_t)-1) {
    perror("Writer_prcc q open error");
    exit(1);
  }
  FILE *file = fopen(file_name, "w");
  char buffer[message_max_size];
  unsigned int priority;
  while (1) {
    mq_receive(queue, buffer, sizeof(buffer), &priority);

    if (strcmp(buffer, eof) == 0) {
      if (mq_close(queue) == 1) {
        perror("writer_prcc q close error");
      }
      fclose(file);
      return;
    }

    fprintf(file, "%s\n", buffer);
  }
}

int main() {
  mqd_t reader_rev_q, reverser_write_q;
  struct mq_attr reader_rev_q_attr, reverser_write_q_attr;

  reader_rev_q_attr.mq_curmsgs = 0;
  reader_rev_q_attr.mq_msgsize = sizeof(char) * message_max_size;
  reader_rev_q_attr.mq_maxmsg = 5;
  reader_rev_q_attr.mq_flags = 0;

  reverser_write_q_attr.mq_curmsgs = 0;
  reverser_write_q_attr.mq_msgsize = sizeof(char) * message_max_size;
  reverser_write_q_attr.mq_maxmsg = 5;
  reverser_write_q_attr.mq_flags = 0;

  reader_rev_q = mq_open(reader_reverser_queue, O_CREAT | O_RDWR, 0666,
                         &reader_rev_q_attr);
  reverser_write_q = mq_open(reverser_writer_queue, O_CREAT | O_RDWR, 0666,
                             &reverser_write_q_attr);
  if (reader_rev_q == (mqd_t)-1 || reverser_write_q == (mqd_t)-1) {
    perror("Parent_prcc q open error");
    exit(1);
  }

  for (int i = 0; i < 3; i++) {

    pid_t child_id = fork();

    if (child_id < 0) {
      printf("Fork failed\n");
    }

    else if (child_id == 0) {
      switch (i) {
      case 0:
        reader_process("file1.txt");
        return 0;
      case 1:
        reverser_process();
        return 0;
      case 2:
        writer("file2.txt");
        return 0;
      default:
        printf("Got into default\n");
        return 0;
      }
    }
  }

  for (int i = 0; i < 3; i++) {
    wait(NULL);
  }

  if (mq_close(reader_rev_q) == -1 || mq_close(reverser_write_q) == -1) {
    perror("Parent_prcc q close error");
  }
  mq_unlink(reader_reverser_queue);
  mq_unlink(reverser_writer_queue);
}
