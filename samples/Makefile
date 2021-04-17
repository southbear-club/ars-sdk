SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%,$(SRCS))
CXX := $(TOOL_PREFIX)g++
OUTPUT_DIR := output

ifneq ($(ARU_INC_DIR),)
ARU_INC = -I$(ARU_INC_DIR)
endif
ifneq ($(ARU_LIB_DIR),)
ARU_LIB = -L$(ARU_LIB_DIR)
endif

INC = $(ARU_INC) -I/usr/local/include
LIB = $(ARU_LIB) -L/usr/lib -L/usr/local/lib -laru -lssl -lcrypto -lglog -lpthread -lc
CFLAGS = -g -W -Wall -o2

all:
	@mkdir -p $(OUTPUT_DIR)
	@for item in $(OBJS); do \
		echo "$(CXX) $$item.cpp -o $$item"; \
		$(CXX) $$item.cpp $(INC) $(LIB) $(CFLAGS) -o $(OUTPUT_DIR)/$$item; \
	done
