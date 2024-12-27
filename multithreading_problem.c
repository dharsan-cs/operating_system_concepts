#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// objective - read string from file1 ( using thread1 ) ,reverse the string
// (using thread 2) and write the reverse string to file2 (using thread3)

int max_line = 100;
char string[100][256];
// string_status ,0-not read ,1-read ,2-reversed ,3-written
int string_status[100];
int string_read = 1;
int string_reversed = 2;
int string_written = 3;
int no_more_string = 4;

// thread function to read data from file1
void *read_thread_function(void *args) {
  printf("Read thread started\n");
  char *file_path = (char *)args;
  FILE *file_ptr = fopen(file_path, "r");
  char line[256];
  int i = 0;

  while (fgets(line, sizeof(line), file_ptr) && i < max_line - 1) {
    strcpy(string[i], line);
    string_status[i] = string_read;
    i = i + 1;
  }

  string_status[i] = no_more_string;
  fclose(file_ptr);
  return 0;
}

// thread function to reverse a string
void *reverse_thread_function(void *args) {
  printf("Reverse thread started\n");
  int i = 0;
  while (1) {
    // making the thread wait untill the string is read
    while (string_status[i] != string_read && string_status[i] != no_more_string) {
      // making it sleep for 2ms so other threads will the processor to make updation
      usleep(2);
    }

    // returning if there is no more strings to reverse
    if (string_status[i] == no_more_string) {
      return 0;
    }

    // reversing the string
    int n = strlen(string[i]);
    for (int j = 0; j < n / 2; j++) {
      char temp = string[i][j];
      string[i][j] = string[i][n - j - 1];
      string[i][n - j - 1] = temp;
    }

    // updating the staus as reversed
    string_status[i] = string_reversed;
    i = i + 1;
  }
}

// thread function to write the reversed string to file2
void *write_thread_function(void *args) {
  printf("write thread started\n");
  char *file_path = (char *)args;
  FILE *file_ptr = fopen(file_path, "w");
  int i = 0;

  while (1) { // making the thread wait untill the string is reversed
    while (string_status[i] != string_reversed && string_status[i] != no_more_string) {
      // making it sleep for 2ms so other threads will the processor to make updation
      usleep(2);
    }

    // returning if there is no more strings to write
    if (string_status[i] == no_more_string) {
      return 0;
    }

    // writing the string to file2
    fprintf(file_ptr, "%s\n", string[i]);
    string_status[i] = string_written;
    i = i + 1;
  }

  fclose(file_ptr);
}

int main() {
  char *read_file = "file1.txt";
  char *write_file = "file2.txt";

  // initializing the string_status to unread
  for (int i = 0; i < max_line; i++) {
    string_status[i] = 0;
  }

  pthread_t read_thread, write_thread, reverse_thread;

  // creating threads
  pthread_create(&read_thread, NULL, read_thread_function, (void *)read_file);
  pthread_create(&reverse_thread, NULL, reverse_thread_function, NULL);
  pthread_create(&write_thread, NULL, write_thread_function,
                 (void *)write_file);

  // waiting for threads to complete execution
  pthread_join(read_thread, NULL);
  pthread_join(reverse_thread, NULL);
  pthread_join(write_thread, NULL);

  //
  printf("Execution over");
  return 0;
}
