INC_DIR				=	include
SRC_DIR				=	src
OBJ_DIR				=	obj
BIN_DIR				=	bin

CC					=	g++
LD					=	ld
CXXFLAGS			=	-Wall -I$(INC_DIR)
LDFLAGS				=

BLACKJACK_SRC		=	main.cpp process.cpp
BLACKJACK_OBJ		:=	$(addprefix $(OBJ_DIR)/,$(patsubst %.cpp,%.o,$(BLACKJACK_SRC)))
BLACKJACK_SRC		:=	$(addprefix $(SRC_DIR)/,$(BLACKJACK_SRC))
BLACKJACK_DEPS		=	process.h
BLACKJACK_DEPS		:=	$(addprefix $(INC_DIR)/,$(BLACKJACK_DEPS))

DUMMY_TARGET_SRC	=	dummy_target.cpp
DUMMY_TARGET_OBJ	:=	$(addprefix $(OBJ_DIR)/,$(patsubst %.cpp,%.o,$(DUMMY_TARGET_SRC)))
DUMMY_TARGET_SRC	:=	$(addprefix $(SRC_DIR)/,$(DUMMY_TARGET_SRC))

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

blackjack: $(BLACKJACK_OBJ) $(BLACKJACK_DEPS)
	$(CC) $(LDFLAGS) -o $(BIN_DIR)/$@ $(BLACKJACK_OBJ)

dummy_target: $(DUMMY_TARGET_OBJ)
	$(CC) $(LDFLAGS) -o $(BIN_DIR)/$@ $(DUMMY_TARGET_OBJ)

.PHONY: all clean debug

TARGETS				=	blackjack dummy_target

all: $(TARGETS)

debug: CXXFLAGS += -DDEBUG -g
debug: LDFLAGS += -g
debug: $(TARGETS)

clean:
	rm $(OBJ_DIR)/*.o
	rm $(BIN_DIR)/*