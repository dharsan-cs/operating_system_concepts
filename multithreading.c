#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *thread_function(void *args) {
  int *id = (int *)args;
  printf("hello linux from %d thread\n", *id);
  return 0;
}

int main() {
  int num_thread = 5;
  pthread_t threads[num_thread];
  int thread_id[num_thread];

  // creating threads
  for (int i = 0; i < num_thread; i++) {
    thread_id[i] = i + 1;
    if (pthread_create(&threads[i], NULL, thread_function, &thread_id[i]) != 0) {
      printf("failed to create a thread");
      exit(1);
    }
  }

  // waiting for threads to complete execution
  for (int i = 0; i < num_thread; i++) {
    pthread_join(threads[i], NULL);
  }
  
  return 0;
}
