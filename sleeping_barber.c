#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct sleeping_barber {
  sem_t waiting_chair_sem;
  sem_t haircut_sem;
  sem_t barber_sem;
  sem_t customer_sem;
} sleeping_barber;

volatile int running = 1;
const int num_chairs = 3;
const int num_customers = 5;

void *customer_thread_func(void *args) {
  sleeping_barber *ptr = (sleeping_barber *)args;
  pthread_t self_id = pthread_self();

  if (sem_trywait(&(ptr->waiting_chair_sem)) == 0) {
    printf("%d customer - in Waiting_chair\n", (int)self_id);
    sem_wait(&(ptr->haircut_sem));
    sem_post(&(ptr->waiting_chair_sem));
    sem_post(&(ptr->barber_sem));
    printf("%d customer - in Barber_chair\n", (int)self_id);
    sem_wait(&(ptr->customer_sem));
    sem_post(&(ptr->haircut_sem));
    printf("%d customer - exiting\n", (int)self_id);
  } else {
    printf("%d customer - exiting(no waiting chair)\n", (int)self_id);
  }
  return NULL;
}

void *barber_thread_func(void *args) {
  sleeping_barber *ptr = (sleeping_barber *)args;
  pthread_t self_id = pthread_self();
  while (running) {
    sem_wait(&(ptr->barber_sem));
    if (!running) {
      break;
    }
    // simulating haircut
    printf("%d Barber is cutting hair\n", (int)self_id);
    sleep(1);
    sem_post(&(ptr->customer_sem));
  }
  printf("%d barber - exiting\n", (int)self_id);
  return NULL;
}

int main() {
  sleeping_barber slp_bar;
  if (sem_init(&slp_bar.haircut_sem, 0, 1) != 0) {
    perror("Sem init failed");
    exit(EXIT_FAILURE);
  }

  if (sem_init(&slp_bar.waiting_chair_sem, 0, num_chairs) != 0) {
    perror("Sem init failed");
    exit(EXIT_FAILURE);
  }

  if (sem_init(&slp_bar.barber_sem, 0, 0) != 0) {
    perror("Sem init failed");
    exit(EXIT_FAILURE);
  }

  if (sem_init(&slp_bar.customer_sem, 0, 0) != 0) {
    perror("Sem init failed");
    exit(EXIT_FAILURE);
  }

  pthread_t customer_thread[num_customers], barber_thread;
  pthread_create(&barber_thread, NULL, barber_thread_func, &slp_bar);
  for (int i = 0; i < 5; i++) {
    pthread_create(&customer_thread[i], NULL, customer_thread_func, &slp_bar);
  }
  for (int i = 0; i < 5; i++) {
    pthread_join(customer_thread[i], NULL);
  }
  running = 0;
  sem_post(&slp_bar.barber_sem);
  pthread_join(barber_thread, NULL);
  
}
