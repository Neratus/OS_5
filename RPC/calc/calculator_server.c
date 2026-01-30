#include "calculator.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  int op;
  float arg1;
  float arg2;
  float result;
  int is_done;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
} thread_data_t;

void *calculate_thread(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;

  switch (data->op) {
  case ADD:
    data->result = data->arg1 + data->arg2;
    break;
  case SUB:
    data->result = data->arg1 - data->arg2;
    break;
  case MUL:
    data->result = data->arg1 * data->arg2;
    break;
  case DIV:
    if (data->arg2 != 0.0) {
      data->result = data->arg1 / data->arg2;
    } else {
      data->result = 0.0;
    }
    break;
  default:
    data->result = 0.0;
    break;
  }

  pthread_mutex_lock(&data->mutex);
  data->is_done = 1;
  pthread_cond_signal(&data->cond);
  pthread_mutex_unlock(&data->mutex);

  return NULL;
}

float *calculate_1_svc(CALCULATOR_ARGS *argp, struct svc_req *rqstp) {
  static float result;
  pthread_t thread;
  pthread_attr_t attr;
  thread_data_t *data;

  data = malloc(sizeof(thread_data_t));
  if (!data) {
    result = 0.0;
    return &result;
  }

  memset(data, 0, sizeof(thread_data_t));
  data->op = argp->op;
  data->arg1 = argp->arg1;
  data->arg2 = argp->arg2;
  data->result = 0.0;
  data->is_done = 0;

  if (pthread_mutex_init(&data->mutex, NULL) != 0) {
    free(data);
    result = 0.0;
    return &result;
  }

  if (pthread_cond_init(&data->cond, NULL) != 0) {
    pthread_mutex_destroy(&data->mutex);
    free(data);
    result = 0.0;
    return &result;
  }

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  if (pthread_create(&thread, &attr, calculate_thread, data) != 0) {
    pthread_mutex_destroy(&data->mutex);
    pthread_cond_destroy(&data->cond);
    free(data);
    result = 0.0;
    return &result;
  }

  pthread_attr_destroy(&attr);

  pthread_mutex_lock(&data->mutex);
  while (!data->is_done) {
    pthread_cond_wait(&data->cond, &data->mutex);
  }
  pthread_mutex_unlock(&data->mutex);

  result = data->result;

  pthread_mutex_destroy(&data->mutex);
  pthread_cond_destroy(&data->cond);
  free(data);
}

return &result;
}