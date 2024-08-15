.PHONY: all clean

TARGET = PhyloGraphUtil
CC = g++
CFLAGS += -O2 -Wall

# Define a recursive wildcard function
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

BIN_DIR = bin
SRC_DIR = src
OBJ_DIR = obj

SRCS = $(call rwildcard, $(SRC_DIR), *.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

all: $(BIN_DIR)/$(TARGET)

$(BIN_DIR)/$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $(OBJS) $(CFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -rf $(BIN_DIR)/* $(OBJ_DIR)/*
