#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(void) {
    const int child_cnt = 2;
    pid_t childPIDS[child_cnt];

    for (int i = 0; i < child_cnt; i++) {
        childPIDS[i] = fork();
        if (childPIDS[i] == -1) {
            perror("Can't fork");
            exit(1);
        } else if (childPIDS[i] == 0) {
            printf("Child %d: PID = %d, PPID = %d, group = %d\n", i + 1, getpid(), getppid(), getpgrp());
            sleep(2);
            printf("Child %d: PID = %d, PPID = %d, group = %d\n", i + 1, getpid(), getppid(), getpgrp());
            // exit(0);
        }
    }

    printf("Parent: PID = %d, children = %d, %d,  group = %d \n", getpid(), childPIDS[0], childPIDS[1], getpgrp());
    
     exit(0);
}
