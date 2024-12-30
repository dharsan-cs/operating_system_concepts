#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


typedef struct sleeping_barber {
  int num_waiting_chair;
  sem_t waiting_chair_sem;
  sem_t haircut_sem;
  sem_t barber_sem;
} sleeping_barber;

void *customer_thread_func(void *args) {
  sleeping_barber *ptr = (sleeping_barber *)args;
  pthread_t self_id = pthread_self();

  sem_wait(&(ptr->waiting_chair_sem));
  if (ptr->num_waiting_chair == 0) {
    printf("%d - exiting\n", (int)self_id);
    sem_post(&(ptr->waiting_chair_sem));
    return 0;
  } else {
    ptr->num_waiting_chair--;
    sem_post(&(ptr->waiting_chair_sem));
  }

  printf("%d - in Waiting_chair\n", (int)self_id);
  sem_wait(&(ptr->haircut_sem));

  sem_wait(&(ptr->waiting_chair_sem));
  ptr->num_waiting_chair++;
  sem_post(&(ptr->waiting_chair_sem));

  sem_post(&(ptr->barber_sem));
  printf("%d - in Barber_chair\n", (int)self_id);
  sleep(2);

  sem_post(&(ptr->haircut_sem));
  return 0;
}

void *barber_thread_func(void *args) {
  sleeping_barber *ptr = (sleeping_barber *)args;
  pthread_t self_id = pthread_self();
  while (1) {
    sem_wait(&(ptr->barber_sem));
    printf("%d-Barber is cutting hair\n", (int)self_id);
  }
}

int main() {
  sleeping_barber slp_bar;

  slp_bar.num_waiting_chair = 3;
  if (sem_init(&slp_bar.haircut_sem, 0, 1) != 0 ||
      sem_init(&slp_bar.waiting_chair_sem, 0, 1) != 0) {
    perror("Sem init failed");
    exit(EXIT_FAILURE);
  }
  if (sem_init(&slp_bar.barber_sem, 0, 0) != 0) {
    perror("Sem init failed");
    exit(EXIT_FAILURE);
  }

  pthread_t customer_thread[5], barber_thread;
  pthread_create(&barber_thread, NULL, barber_thread_func, &slp_bar);
  for (int i = 0; i < 5; i++) {
    pthread_create(&customer_thread[i], NULL, customer_thread_func, &slp_bar);
  }
  for (int i = 0; i < 5; i++) {
    pthread_join(customer_thread[i], NULL);
  }
  pthread_cancel(barber_thread);
}
