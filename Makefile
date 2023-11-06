CFLAGS = -g -MMD -Wall -pedantic -Werror -std=c11 -I$(INC_DIR)
CXXFLAGS = -g -MMD -Wall -pedantic -Werror -std=c++2a -I$(INC_DIR) -I$(BOOST_DIR)
LIBS = -lm

CC = gcc
CXX = g++-13

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include
BOOST_DIR = /usr/local/boost_1_82_0/

HFILES = $(shell find $(INC_DIR) -type f -name '*.hpp') $(shell find $(INC_DIR) -type f -name '*.h')
CFILES = $(shell find $(SRC_DIR) -type f -name '*.cpp') $(shell find $(SRC_DIR) -type f -name '*.c')
OFILES = $(addprefix $(OBJ_DIR)/, $(notdir $(patsubst $(SRC_DIR)/%.cpp, %.o, $(CFILES))))
PROG = tabuwave

VPATH = $(SRC_DIR) $(shell find $(SRC_DIR) -type d)

ifdef PROFILE
FAST=1
undefine DEBUG
CFLAGS += -pg
CXXFLAGS += -pg
LIBS += -pg
endif

ifdef DEBUG
CFLAGS += -DDEBUG
CXXFLAGS += -DDEBUG
endif

ifdef FAST
CFLAGS += -O2
CXXFLAGS += -O2
endif

ifdef USE_OMP
CFLAGS += -DUSE_OMP
CXXFLAGS += -DUSE_OMP
CXXFLAGS += -fopenmp
LIBS += -L/usr/local/lib/ -lomp
endif

.PHONY: all clean

all: $(PROG)

$(PROG): $(OFILES)
	$(CXX) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: %.c $(HFILES) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp $(HFILES) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp $(HFILES) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(OBJ_DIR) $(PROG)

-include $(OFILES:.o=.d)
-include $(OFILES_TEST:.o=.d)