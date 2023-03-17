#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>
#include <iostream>
#include <fstream>
#include <string>
#include <signal.h>
#include "../include/memory.h"

using namespace std;

int memory[2000]; //memory array
int read_pipe, write_pipe; //to make writing to pipes easier

void initMem(char* fileName, int rp, int wp);
char parseRead_Write(); //used to decide whether we want to read or write
void read_Mem(int addr); //reads from a memory address & passes data to CPU
void write_mem(int addr, int value); //writes a value to the given memory address

//reads file into memory array
void initMem(char* fileName, int rp, int wp){
    int index = 0; //memory index
    ifstream file;
    file.open(fileName);
    string line;

    read_pipe = rp;
    write_pipe = wp;

    while(!file.eof()){
        getline(file, line);
        if (file.fail() || file.bad()) {
            break;
        }
        // skip empty lines
        if(line != ""){
            //storing digits to our string
            if (isdigit(line[0])){
                int val = stoi(line.substr(0, line.find(" ")));
                memory[index++] = val;
            }
            else if(line[0] == '.') { // for lines starting with '.', update index to that memory address
                index = stoi(line.substr(1, line.find(" ")));; //convert the substr between . and "\s" into an integer
            }
            if (file.fail() || file.bad()) {
                break;
            }
        }
    } //end while
    file.close();
}

char parseRead_Write(){
    char op;
    read(read_pipe, &op, sizeof(op));
    return op;
}

void runMem(){
    char operation; //determines whether we're reading, writing, or exiting
    int addr, value;
    bool running = true;
    operation = parseRead_Write(); //initial operation read
    while(running){
        switch(operation){
            case 'R': //read case
                read(read_pipe, &addr, sizeof(addr));
                write(write_pipe, &memory[addr], sizeof(memory[addr]));
                break;
            case 'W': //write case
                read(read_pipe, &addr, sizeof(addr));
                read(read_pipe, &value, sizeof(value));
                memory[addr] = value;
                break;
            case 'X': // end program
                running = false;
                exit(0);
            default: // invalid command
                perror("Invalid command\n");
                exit(-1);
                break;
        }
        operation = parseRead_Write(); //read next operation
    }
}