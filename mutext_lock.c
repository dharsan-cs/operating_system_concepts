
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t lock;
int count = 0;

void *thread_function(void *args) {
  // entry section (no busy wait) ,(no bounded wait)
  pthread_mutex_lock(&lock);
  // critical section
  count++;
  // exit section
  pthread_mutex_unlock(&lock);
  return 0;
}

int main() {
  pthread_t thread[5];
  for (int i = 0; i < 5; i++) {
    pthread_create(&thread[i], NULL, thread_function, NULL);
  }
  for (int i = 0; i < 5; i++) {
    pthread_join(thread[i], NULL);
  }
  printf("count : %d", count);
}