#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

sem_t tabaco, paper, match, more, exit_synch;

volatile int running = 1;

void *agent_func(void *arg) {
  int choice;
  while (running) {
    srand(time(NULL));
    choice = rand() % 3;
    switch (choice) {
    case 0:
      sem_post(&tabaco);
      sem_post(&paper);
      break;

    case 1:
      sem_post(&paper);
      sem_post(&match);
      break;

    case 2:
      sem_post(&tabaco);
      sem_post(&match);
      break;
    }
    sem_wait(&more);
  }
  printf("agent exiting\n");
  return NULL;
}

// breaking the hold and wait condition of deadlock (deadlock prevention)
void *tabaco_holder(void *args) {
  while (running) {
    sem_wait(&paper);
    if (running != 1) {
      break;
    }
    if (sem_trywait(&match) == 0) {
      printf("tabaco_holder smoking\n");
      sleep(1);
      sem_post(&more);
    } else {
      sem_post(&paper);
    }
  }
  printf("tabaco_holder exiting\n");
  return NULL;
}

void *paper_holder(void *args) {
  while (running) {
    sem_wait(&match);
    if (running != 1) {
      break;
    }
    if (sem_trywait(&tabaco) == 0) {
      printf("paper_holder smoking\n");
      sleep(1);
      sem_post(&more);
    } else {
      sem_post(&match);
    }
  }
  printf("paper_holder exiting\n");
  return NULL;
}

void *match_holder(void *args) {
  while (running) {
    sem_wait(&tabaco);
    if (running != 1) {
      break;
    }
    if (sem_trywait(&paper) == 0) {
      printf("match_holder smoking\n");
      sleep(1);
      sem_post(&more);
    } else {
      sem_post(&tabaco);
    }
  }
  printf("match_holder exiting\n");
  return NULL;
}

void unblock_all_semphore() {
  sem_post(&tabaco);
  sem_post(&match);
  sem_post(&paper);
  sem_post(&more);
}

int main() {
  pthread_t tabaco_thread, paper_thread, match_thread, agent;

  if (sem_init(&tabaco, 0, 0) != 0 || sem_init(&paper, 0, 0) != 0) {
    perror("semaphore initialization failed");
    exit(EXIT_FAILURE);
  }

  if (sem_init(&match, 0, 0) != 0 || sem_init(&more, 0, 0) != 0) {
    perror("semaphore initialization failed");
    exit(EXIT_FAILURE);
  }

  if (sem_init(&exit_synch, 0, 0) != 0) {
    perror("semaphore initialization failed");
    exit(EXIT_FAILURE);
  }

  pthread_create(&agent, NULL, agent_func, NULL);
  pthread_create(&tabaco_thread, NULL, tabaco_holder, NULL);
  pthread_create(&paper_thread, NULL, paper_holder, NULL);
  pthread_create(&match_thread, NULL, match_holder, NULL);

  sleep(10);
  running = 0;
  printf("Exit signal generated\n");
  for (int i = 0; i < 3; i++) {
    unblock_all_semphore();
  }
  
  pthread_join(agent, NULL);
  pthread_join(tabaco_thread, NULL);
  pthread_join(paper_thread, NULL);
  pthread_join(match_thread, NULL);
}
