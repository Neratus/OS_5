#include "bakery.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_CLIENTS 100

// Глобальные переменные для алгоритма булочной
int choosing[MAX_CLIENTS] = {0};
int number[MAX_CLIENTS] = {0};
int next_ticket = 1;
pthread_mutex_t bakery_mutex = PTHREAD_MUTEX_INITIALIZER;

// Алгоритм булочной - получение номера
int get_ticket(int client_id) {
  pthread_mutex_lock(&bakery_mutex);

  choosing[client_id] = 1;

  int max_num = 0;
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (number[i] > max_num) {
      max_num = number[i];
    }
  }

  number[client_id] = max_num + 1;
  choosing[client_id] = 0;

  pthread_mutex_unlock(&bakery_mutex);

  return number[client_id];
}

// Алгоритм булочной - ожидание своей очереди
void wait_for_turn(int client_id) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    while (choosing[i]) {
      usleep(1000);
    }

    while (number[i] != 0 &&
           (number[i] < number[client_id] ||
            (number[i] == number[client_id] && i < client_id))) {
      usleep(1000);
    }
  }
}

// Обработка запроса с использованием алгоритма булочной
BAKERY_RESPONSE *submit_request_1_svc(BAKERY_REQUEST *argp,
                                      struct svc_req *rqstp) {
  (void)rqstp;

  static BAKERY_RESPONSE result;

  printf("\n=== Bakery: New client %d arrived ===\n", argp->client_id);
  printf("Request type: %s, Details: %s\n", argp->request_type, argp->details);

  // 1. Получаем номерок (алгоритм булочной)
  int ticket = get_ticket(argp->client_id);
  printf("Client %d got ticket: %d\n", argp->client_id, ticket);

  printf("Client %d waiting for turn...\n", argp->client_id);
  wait_for_turn(argp->client_id);

  printf("Client %d (ticket %d) is being served\n", argp->client_id, ticket);

  sleep(2);

  result.client_id = argp->client_id;
  result.ticket_number = ticket;
  result.queue_position = ticket;

  char buffer[200];
  snprintf(buffer, sizeof(buffer),
           "Request '%s: %s' processed for client %d with ticket %d",
           argp->request_type, argp->details, argp->client_id, ticket);
  strcpy(result.status, buffer);

  pthread_mutex_lock(&bakery_mutex);
  number[argp->client_id] = 0;
  pthread_mutex_unlock(&bakery_mutex);

  printf("Client %d (ticket %d) completed\n", argp->client_id, ticket);

  return &result;
}

int *get_ticket_number_1_svc(int *argp, struct svc_req *rqstp) {
  (void)argp;
  (void)rqstp;

  static int result = 0;
  result++;
  return &result;
}

int *get_queue_position_1_svc(int *argp, struct svc_req *rqstp) {
  (void)argp;
  (void)rqstp;

  static int result = 0;
  return &result;
}

int bakery_prog_1_freeresult(SVCXPRT *transp, xdrproc_t xdr_result,
                             caddr_t result) {
  (void)transp;
  xdr_free(xdr_result, result);
  return 1;
}
