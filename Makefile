INC_DIR			=	include
SRC_DIR			=	src
OBJ_DIR			=	obj
BIN_DIR			=	bin

CC				=	g++
LD				=	ld
CXXFLAGS		=	-Wall -I$(INC_DIR)
LDFLAGS			=

BLACKJACK_SRC	=	main.cpp
BLACKJACK_OBJ	:=	$(addprefix $(OBJ_DIR)/,$(patsubst %.cpp,%.o,$(BLACKJACK_SRC)))
BLACKJACK_SRC	:=	$(addprefix $(SRC_DIR)/,$(BLACKJACK_SRC))
BLACKJACK_DEPS	=

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

blackjack: $(BLACKJACK_OBJ) $(BLACKJACK_DEPS)
	$(CC) $(LDFLAGS) -o $(BIN_DIR)/$@ $(BLACKJACK_OBJ)

.PHONY: all clean debug

all: blackjack

debug: CXXFLAGS += -DDEBUG -g
debug: LDFLAGS += -g
debug: all

clean:
	rm $(OBJ_DIR)/*.o
	rm $(BIN_DIR)/*