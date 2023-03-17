#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/wait.h>
#include "../include/cpu.h"

using namespace std;

//global variables
int SP = 999, PC, IR, AC, X, Y;
bool done, userMode;
int interrupt_timer, timeout; //variables to track interrupt timeout
int readpipe, writepipe; //to make reading & writing pipes easier to use/more readable
int interruptaddr = 1500, timerintraddr = 1000, sysStack = 1999;

//CPU functions
void initCPU(int timeoutval, int rp, int wp);
int readFromMem(int addr); //read data from memory
void writeToMem(int addr, int value); //write to memory
void runCPU(); //runs CPU
void push(int value); //push value to stack
int pop(); //pop from stack & get value
int fetch(); //fetch the next instruction
void decode_exec(); //decode & execute the instruction

int readFromMem(int addr){
    if (userMode == true && addr >= timerintraddr){ //checking to see if we're accessing the wrong addr
        perror("Error: Accessing system address in user mode!\n");
        write(writepipe, "X", sizeof(char));
        exit(-1);
    }
    int val; //return value

    write(writepipe, "R", sizeof(char)); //read a value..
    write(writepipe, &addr, sizeof(addr)); //from this address..
    read(readpipe, &val, sizeof(val)); //get value
    return val;
}

void writeToMem(int addr, int val){
    if (userMode == true && addr >= timerintraddr){ //checking to see if we're accessing the wrong addr
        perror("Error: Accessing system addr in user mode!\n");
        write(writepipe, "X", sizeof(char));
        exit(-1);
    }

    write(writepipe, "W", sizeof(char)); //write a value...
    write(writepipe, &addr, sizeof(addr)); //to this address..
    write(writepipe, &val, sizeof(val)); //here is the value
}

void push(int value){
    writeToMem(SP--, value);
}

int pop(){
    int val = readFromMem(SP); //get value from stack
    writeToMem(SP++, 0); //replace with 0 and update stack pointer
    return val;
}

void initCPU(int timeoutval, int rp, int wp){
    userMode = true; //set mode

    //setting pipes
    readpipe = rp;
    writepipe = wp;

    //setting timeout value & timer
    timeout = timeoutval;
    interrupt_timer = 0;
}

void runCPU(){
    done = false;
    while(!done){
        //timer interrupt
        if (interrupt_timer >= timeout){
            printf("Timeout!\n");
            userMode = false; //swap to kernel mode
            interrupt_timer = 0; //reset interrupt timer
            int SP_temp = SP;
            SP = sysStack;
            push(SP_temp); //push SP to stack
            push(PC);
            PC = 1000;
        }

        IR = fetch(); //fetch instruction
        decode_exec(); //decode + execute instruction

        interrupt_timer++; //increment timer increases, regardless of mode
    }
}

int fetch(){
    int val = readFromMem(PC++); //read from memory & update PC
    return val;
}

void decode_exec(){
    int addr; //for fetching addresses
    switch(IR){ //switch = decode. execution happens when a case is met.
        case 1: // fetch value into AC
            AC = fetch();
            break;
        case 2: //read value from an address into AC
            AC = readFromMem(fetch());
            break;
        case 3: //read from address found in given address into AC
            AC = readFromMem(readFromMem(fetch()));
            break;
        case 4: //read value at addr + X into AC
            AC = readFromMem(fetch() + X);
            break;
        case 5: //read value at addr + Y into AC
            AC = readFromMem(fetch() + Y);
            break;
        case 6: //read from addr (SP + X) into AC
            AC = readFromMem(SP + X);
            break;
        case 7: ;//store AC into address
            int tempaddr;
            tempaddr = fetch();
            writeToMem(tempaddr, AC);
            break;
        case 8: //load random int (1-100) into AC
            AC = rand() % 100 + 1;
            break;
        case 9: ;// write AC as an int or char to screen, depending on port.
            int port;
            port = fetch();
            if (port == 1)
                printf("%d \n", AC);
            else if (port == 2)
                printf("%c \n", AC);
            break;
        case 10: //add X to AC
            AC += X;
            break;
        case 11: //add Y to AC
            AC += Y;
            break;
        case 12: //subtract X from AC
            AC -= X;
            break;
        case 13: //subtract Y from AC
            AC -= Y;
            break;
        case 14: //copy AC to X
            X = AC;
            break;
        case 15: //copy X to AC
            AC = X;
            break;
        case 16: //copy AC to Y
            Y = AC;
            break;
        case 17: //copy Y to AC
            AC = Y;
            break;
        case 18: //copy AC to SP
            SP = AC;
            break;
        case 19: // copy SP to AC
            AC = SP;
            break;
        case 20: //jump to address
            PC = fetch();
            break;
        case 21: //jump to addr if AC = 0
            addr = fetch(); //need to fetch regardless of AC value
            if (AC == 0)
                PC = addr;
            break;
        case 22: //jump to addr if AC != 0
            addr = fetch();
            if (AC != 0)
                PC = addr;
            break;
        case 23: //push PC to stack and jump to the address
            addr = fetch();
            push(PC);
            PC = addr;
            break;
        case 24: //pop old PC from stack & return
            PC = pop();
            break;
        case 25: //increment X
            X++;
            break;
        case 26: //decrement X
            X--;
            break;
        case 27: //push AC onto stack
            push(AC);
            break;
        case 28: //pop from stack into AC
            AC = readFromMem(SP++);
            break;
        case 29: //interrupt
            if (userMode){
                userMode = false; //now in Kernel mode
                int SP_temp = SP;
                SP = sysStack; //move SP to system stack
                push(SP_temp); //push program SP to stack
                push(PC); //push PC to stack
                PC = interruptaddr;
            }
            break;
        case 30: //interrupt return
            PC = readFromMem(SP++);
            SP = readFromMem(SP);
            userMode = true;
            break;
        case 50: //end program
            done = true;
            write(writepipe, "X", sizeof(char));
            break;
        default: //error case
            printf("Error: invalid instruction: %i", IR);
            write(writepipe, "X", sizeof(char));
            exit(-1);
    }
}

