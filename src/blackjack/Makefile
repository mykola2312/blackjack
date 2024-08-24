NAME				=	blackjack

CFLAGS				=	-Wall -I$(INC_DIR)
ASFLAGS				=
LDFLAGS				=	-z noexecstack -lcap

SRC		=	main.c process.c
OBJ		:=	$(addprefix $(OBJ_DIR)/,$(patsubst %.s,%.o,$(patsubst %.c,%.o,$(SRC))))
DEPS	=	process.h process_debug.h

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: %.s
	@mkdir -p $(OBJ_DIR)
	$(AS) $(ASFLAGS) -o $@ $<

.PHONY: all clean debug

all: $(OBJ) $(DEPS)
	$(CC) $(LDFLAGS) -o $(BIN_DIR)/$(NAME) $(OBJ)

debug: CFLAGS += -DDEBUG -g
debug: LDFLAGS += -g
debug: ASFLAGS += -g
debug: all

clean:
	rm -f $(OBJ_DIR)/*