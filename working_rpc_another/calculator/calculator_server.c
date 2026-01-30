#include "calculator.h"
#include <math.h>
#include <rpc/rpc.h>
#include <stdio.h>
#include <stdlib.h>

struct CALCULATOR *calculator_proc_1_svc(struct CALCULATOR *argp,
                                         struct svc_req *rqstp) {
  static struct CALCULATOR result;

  printf("Server: Received operation %d: %.2f %s %.2f\n", argp->op, argp->arg1,
         (argp->op == ADD)   ? "+"
         : (argp->op == SUB) ? "-"
         : (argp->op == MUL) ? "*"
                             : "/",
         argp->arg2);

  switch (argp->op) {
  case ADD:
    result.result = argp->arg1 + argp->arg2;
    break;
  case SUB:
    result.result = argp->arg1 - argp->arg2;
    break;
  case MUL:
    result.result = argp->arg1 * argp->arg2;
    break;
  case DIV:
    if (fabs(argp->arg2) < 0.000001) {
      result.result = 0.0;
      printf("Warning: Division by zero!\n");
    } else {
      result.result = argp->arg1 / argp->arg2;
    }
    break;
  default:
    result.result = 0.0;
    printf("Error: Unknown operation\n");
  }

  result.op = argp->op;
  result.arg1 = argp->arg1;
  result.arg2 = argp->arg2;

  printf("Server: Result = %.2f\n", result.result);

  return &result;
}

int calculator_prog_1_freeresult(SVCXPRT *transp, xdrproc_t xdr_result,
                                 caddr_t result) {
  (void)transp;
  xdr_free(xdr_result, result);
  return 1;
}
