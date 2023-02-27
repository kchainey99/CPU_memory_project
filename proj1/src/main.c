#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../include/memory.h"
#include "../include/cpu.h"

int main(int argc, char *argv[]){
    if (argc != 3){
        perror("Expecting 2 arguments - filename and timeout value.");
        exit(1);
    }

    int timer = atoi(argv[2]); //get interrupt timer

    int cpu_mem[2], mem_cpu[2]; //child write, parent read pipe & parent write, child read pipe

    int result; //stores result from pipe creation, then PID if pipe was successfully created
    //creating pipes and checking for errors
    if ((result = pipe(cpu_mem)) == -1) {
        perror("CPU to Memory pipe creation failed.");
        exit(1);
    }
    if ((result = pipe(mem_cpu)) == -1) {
        perror("Memory to CPU pipe creation failed.");
        exit(1);
    }

    pid_t PID = fork();
    switch(PID){
        case -1: //pid = -1, meaning fork failed
            perror("The fork failed!");
            exit(-1);
        case 0: //Main memory
            printf("Starting memory...\n");
            printf(PID);
            printf("\n");
            initMem(argv[1], cpu_mem[0],mem_cpu[1]);
            printf("Called initMem");
            runMem();
            _exit(0);
        default: //CPU
            wait(NULL);
            printf(PID);
            initCPU(timer, mem_cpu[0], cpu_mem[1]);
            runCPU();
    }
    exit(0);
}
