#include "calculator.h"
#include <pthread.h>
#include <rpc/rpc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define NUM_THREADS 5

typedef struct {
  int thread_id;
  char *host;
} client_thread_data;

void *client_thread_func(void *arg) {
  client_thread_data *data = (client_thread_data *)arg;
  int thread_id = data->thread_id;
  char *host = data->host;

  CLIENT *clnt;
  struct CALCULATOR request, *response;

  clnt = clnt_create(host, CALCULATOR_PROG, CALCULATOR_VER, "udp");
  if (clnt == NULL) {
    clnt_pcreateerror(host);
    pthread_exit(NULL);
  }

  srand(time(NULL) ^ thread_id);

  for (int i = 0; i < 3; i++) {
    request.op = rand() % 4;
    request.arg1 = (rand() % 1000) / 10.0;
    request.arg2 = (rand() % 1000) / 10.0 + 0.1;

    usleep(rand() % 500000);

    char *op_str;
    switch (request.op) {
    case ADD:
      op_str = "+";
      break;
    case SUB:
      op_str = "-";
      break;
    case MUL:
      op_str = "*";
      break;
    case DIV:
      op_str = "/";
      break;
    default:
      op_str = "?";
    }

    printf("Thread %d: Sending %.2f %s %.2f\n", thread_id, request.arg1, op_str,
           request.arg2);

    response = calculator_proc_1(&request, clnt);
    if (response == NULL) {
      clnt_perror(clnt, "RPC call failed");
      continue;
    }

    printf("Thread %d: Received result = %.2f\n", thread_id, response->result);
  }

  clnt_destroy(clnt);
  free(data);
  pthread_exit(NULL);
}

// Основная функция клиента (одиночный клиент)
void calculator_prog_1(char *host) {
  CLIENT *clnt;
  struct CALCULATOR request, *response;

#ifndef DEBUG
  clnt = clnt_create(host, CALCULATOR_PROG, CALCULATOR_VER, "udp");
  if (clnt == NULL) {
    clnt_pcreateerror(host);
    exit(1);
  }
#endif

  // Ввод данных от пользователя
  char c;
  printf("Choose the operation:\n\t0 - ADD\n\t1 - SUB\n\t2 - MUL\n\t3 - DIV\n");
  c = getchar();
  getchar(); // Считываем символ новой строки

  if (c > '3' || c < '0') {
    printf("Error: Invalid operation\n");
    exit(1);
  }

  request.op = c - '0';

  printf("Input the first number: ");
  scanf("%f", &request.arg1);

  printf("Input the second number: ");
  scanf("%f", &request.arg2);

  // Выполняем RPC-вызов
  response = calculator_proc_1(&request, clnt);
  if (response == (struct CALCULATOR *)NULL) {
    clnt_perror(clnt, "call failed");
  }

  // Выводим результат
  printf("The Result is %.3f\n", response->result);

#ifndef DEBUG
  clnt_destroy(clnt);
#endif
}

// Многопоточный клиент
void run_multithreaded_client(char *host) {
  pthread_t threads[NUM_THREADS];

  printf("Starting %d client threads...\n", NUM_THREADS);

  // Создаем потоки
  for (int i = 0; i < NUM_THREADS; i++) {
    client_thread_data *data = malloc(sizeof(client_thread_data));
    data->thread_id = i + 1;
    data->host = host;

    if (pthread_create(&threads[i], NULL, client_thread_func, data) != 0) {
      perror("Failed to create thread");
      exit(1);
    }
  }

  // Ожидаем завершения всех потоков
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  printf("All client threads completed.\n");
}

int main(int argc, char *argv[]) {
  char *host;
  host = argv[1];
  run_multithreaded_client(host);
  exit(0);
}
