INC_DIR				=	include
SRC_DIR				=	src
OBJ_DIR				=	obj
BIN_DIR				=	bin

CC					=	gcc
LD					=	ld
CFLAGS				=	-Wall -I$(INC_DIR)
LDFLAGS				=	-lcap

BLACKJACK_SRC		=	main.c process.c
BLACKJACK_OBJ		:=	$(addprefix $(OBJ_DIR)/,$(patsubst %.c,%.o,$(BLACKJACK_SRC)))
BLACKJACK_SRC		:=	$(addprefix $(SRC_DIR)/,$(BLACKJACK_SRC))
BLACKJACK_DEPS		=	debug.h process.h
BLACKJACK_DEPS		:=	$(addprefix $(INC_DIR)/,$(BLACKJACK_DEPS))

DUMMY_TARGET_SRC	=	dummy_target.c
DUMMY_TARGET_OBJ	:=	$(addprefix $(OBJ_DIR)/,$(patsubst %.c,%.o,$(DUMMY_TARGET_SRC)))
DUMMY_TARGET_SRC	:=	$(addprefix $(SRC_DIR)/,$(DUMMY_TARGET_SRC))

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

blackjack: $(BLACKJACK_OBJ) $(BLACKJACK_DEPS)
	$(CC) $(LDFLAGS) -o $(BIN_DIR)/$@ $(BLACKJACK_OBJ)

dummy_target: $(DUMMY_TARGET_OBJ)
	$(CC) $(LDFLAGS) -o $(BIN_DIR)/$@ $(DUMMY_TARGET_OBJ)

.PHONY: all clean debug

TARGETS				=	blackjack dummy_target

all: $(TARGETS)

debug: CFLAGS += -DDEBUG -g
debug: LDFLAGS += -g
debug: $(TARGETS)

clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(BIN_DIR)/*