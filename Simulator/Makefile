# This target is to ensure accidental execution of Makefile as a bash script 
# will not execute commands like rm in unexpected directories and exit gracefully.
.prevent_execution:
	exit 0

CXX = g++

#remove @ for no make command prints
DEBUG = @

APP_NAME = simulator
APP_SRC_FILES = $(APP_NAME).cpp

MAIN_DIR = .

INCLUDE_DIRS = -I $(MAIN_DIR)/include

LIB_SRC_FILES = $(shell find $(MAIN_DIR)/src/ -name '*.cpp')

SRC_FILES += $(APP_SRC_FILES)
SRC_FILES += $(LIB_SRC_FILES)

# Essentially, the same as gcc main.c file1.c file 2.c -o main file1.h file2.h
MAKE_CMD = $(CXX) $(SRC_FILES) -o $(APP_NAME) $(INCLUDE_DIRS)

all:
	$(DEBUG)$(MAKE_CMD)
	./$(APP_NAME) ../Assembler/Output/Output.bin

# This command is issued before you recompile the project after making changes
clean:
	rm -f $(MAIN_DIR)/$(APP_NAME)
	rm -f ../logs/logfile.log
	rm -f ../logs/log_cache.log
