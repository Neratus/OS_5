#include "calculator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void calculator_prog_1(char *host) {
  CLIENT *clnt;
  float *result;
  CALCULATOR_ARGS args;
  int i;
  char operations[4] = {'+', '-', '*', '/'};
  char op_char;

  clnt = clnt_create(host, CALCULATOR_PROG, CALCULATOR_VER, "udp");
  if (clnt == NULL) {
    clnt_pcreateerror(host);
    exit(1);
  }

  srand(time(NULL));

  for (i = 0;; i++) {
    args.arg1 = (float)(rand() % 1000) / 10.0;
    args.arg2 = (float)(rand() % 1000) / 10.0 + 0.1;

    op_char = operations[rand() % 4];

    switch (op_char) {
    case '+':
      args.op = ADD;
      break;
    case '-':
      args.op = SUB;
      break;
    case '*':
      args.op = MUL;
      break;
    case '/':
      args.op = DIV;
      break;
    }

    result = calculate_1(&args, clnt);

    if (result == NULL) {
      clnt_perror(clnt, "call failed");
    } else {
      printf("Результат: %.2f %c %.2f = %.2f\n", args.arg1, op_char, args.arg2,
             *result);
    }

    usleep(10000);
  }

  clnt_destroy(clnt);
}

int main(int argc, char *argv[]) {
  char *host;

  if (argc < 2) {
    printf("usage: %s server_host\n", argv[0]);
    exit(1);
  }

  host = argv[1];
  calculator_prog_1(host);
  return 0;
}