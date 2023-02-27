void initCPU(int timeoutval, int rp, int wp);
int readFromMem(int addr); //read data from memory
void writeToMem(int addr, int value); //write to memory
void runCPU(); //runs CPU
void push(int value); //push value to stack
int pop(); //pop from stack & get value