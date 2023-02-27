CXX       := gcc
CXX_FLAGS := -std=c11 -ggdb

BIN     := bin
SRC     := src
INCLUDE := include

LIBRARIES   :=
EXECUTABLE  := CPU_Mem


all: $(BIN)/$(EXECUTABLE)

run:
	clean all
	clear
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/memory.c $(SRC)/cpu.c $(SRC)/main.c
	$(CXX) $(CXX_FLAGS) -I $(INCLUDE)/memory.h $(INCLUDE)/cpu.h $^ -o $@ $(LIBRARIES)

clean:
	-rm $(BIN)/*