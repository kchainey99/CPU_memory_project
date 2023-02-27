#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>
#include "../include/memory.h"

int memory[2000]; //memory array
int readpipe, writepipe; //to make writing to pipes easier

char parseRead_Write(); //used to decide whether we want to read or write
void read_Mem(int addr); //reads from a memory address & passes data to CPU
void write_mem(int addr, int value); //writes a value to the given memory address

//reads file into memory array
void initMem(char* fileName, int rp, int wp){
    printf("Initializing memory...\n");
    int index = 0; //memory index

    FILE *file = fopen(fileName, "r"); //opens file in read-only mode
    printf("opened file\n");
    char string[50]; //buffer to read lines
    char *line; //line pointer
    printf("created character vars\n");
    readpipe = rp;
    writepipe = wp;
    printf("assigned pipes\n");
    if (file == NULL){
        perror("Error: file not found\n");
        exit(1);
    }
    printf("Loading file into memory...\n");
    while(fgets(string, sizeof(string), file)){
        line = strtok(string, "\n"); //reading line the line with newline as delimeter
        puts(string);
        // check to see if we have reached end of file
        if(line != NULL){
            //storing digits to our string
            if (isdigit(string[0])){
                printf("Digit found!\n");
                printf(index);
                memory[index++] = atoi(string[0]); //atoi converts string to int
                printf("Adding instruction to memory: %d", atoi(string[0]));
            }
            else if(string[0] == '.') // for lines starting with '.', update index to that memory address
                index = atoi(string[1]);
        }
        printf("While done\n");
    } //end while
    fclose(file); //close the file
}

char parseRead_Write(){
    char op;
    read(readpipe, &op, sizeof(op));
    return op;
}

void runMem(){
    printf("running memory...");
    char op; //determines whether we're reading, writing, or exiting
    int addr, value;
    bool running = true;

    op = parseRead_Write(); //initial operation read
    while(running){
        switch(op){
            case 'R': //read case
                read(readpipe, &addr, sizeof(addr));
                write(writepipe, &memory[addr], sizeof(memory[addr]));
                break;
            case 'W': //write case
                read(readpipe, &addr, sizeof(addr));
                read(readpipe, &value, sizeof(value));
                memory[addr] = value;
                break;
            case 'X': // end program
                running = false;
                exit(0);
            default: // invalid command
                perror("Invalid command");
                exit(-1);
                break;
        }
        op = parseRead_Write(readpipe); //read next operation
    }
}