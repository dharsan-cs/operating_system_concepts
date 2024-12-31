#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


///this algo makes sure deadlock never happens by using order access like teachnique to prevent circular wait  

sem_t phil_fork[5];
volatile int running = 1;

void fork1(int i) { sem_wait(&phil_fork[i]); }

void fork2(int i) {
  if (i <= 0) {
    sem_wait(&phil_fork[4]);
  } else {
    sem_wait(&phil_fork[i - 1]);
  }
}

void release_fork(int i) {
  sem_post(&phil_fork[i]);
  if (i <= 0) {
    sem_post(&phil_fork[4]);
  } else {
    sem_post(&phil_fork[i - 1]);
  }
}

void *philosopher_thread(void *args) {
  int phil_id = *(int *)args;
  while (running) {
    // thinking
    sleep(1);
    if (phil_id % 2 == 0) {
      fork1(phil_id);
      fork2(phil_id);
    } else {
      fork2(phil_id);
      fork1(phil_id);
    }
    printf("%d philosopher eating\n", phil_id);
    sleep(1);
    release_fork(phil_id);
  }
  printf("%d philospher exiting\n", phil_id);
  return NULL;
}

int main() {
  pthread_t philosophers[5];
  int phil_id[5];

  for (int i = 0; i < 5; i++) {
    if (sem_init(&phil_fork[i], 0, 1) != 0) {
      perror("sem_init failed");
      exit(EXIT_FAILURE);
    }
  }

  for (int i = 0; i < 5; i++) {
    phil_id[i] = i;
    if (pthread_create(&philosophers[i], NULL, philosopher_thread, (void *)&phil_id[i]) != 0) {
      printf("failed to create a thread");
      exit(1);
    }
  }

  sleep(10);
  running = 0;
  for (int i = 0; i < 5; i++) {
    pthread_join(philosophers[i], NULL);
  }

  return 0;
}
