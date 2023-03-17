#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "../include/memory.h"
#include "../include/cpu.h"

using namespace std;

int main(int argc, char *argv[]){
    if (argc != 3){
        perror("Expecting 2 arguments - filename and timeout value.");
        exit(1);
    }

    int timer = atoi(argv[2]); //get interrupt timer

    int cpu_mem[2], mem_cpu[2]; //child write, parent read pipe & parent write, child read pipe

    int result; //stores result from pipe creation, then PID if pipe was successfully created
    int status; //stores child's exit status for parent to read

    //creating pipes and checking for errors
    if ((result = pipe(cpu_mem)) == -1) {
        perror("CPU to Memory pipe creation failed.");
        exit(1);
        
    }
    if ((result = pipe(mem_cpu)) == -1) {
        perror("Memory to CPU pipe creation failed.");
        exit(1);
    }

    result = fork();
    switch(result){
        case -1: //pid = -1, meaning fork failed
            perror("The fork failed!\n");
            exit(-1);
        case 0: //Main memory
            printf("Starting memory...\n");
            initMem(argv[1], cpu_mem[0], mem_cpu[1]);
            runMem();
            _exit(0);
        default: //CPU
            sleep(1); //wait for memory to start
            initCPU(timer, mem_cpu[0], cpu_mem[1]);
            runCPU();
            waitpid(result, &status, 0);
    }

    //close our pipes
    close(cpu_mem[0]);
    close(cpu_mem[1]);
    close(mem_cpu[0]);
    close(mem_cpu[1]);

    exit(0);
}