NAME				=	relf_test

CFLAGS				=	-Wall -I$(INC_DIR)
ASFLAGS				=
LDFLAGS				=	-z noexecstack

SRC		=	relf_test.c relf_dummy.c
OBJ		:=	$(addprefix $(OBJ_DIR)/,$(patsubst %.s,%.o,$(patsubst %.c,%.o,$(SRC))))
DEPS	=

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: %.s
	@mkdir -p $(OBJ_DIR)
	$(AS) $(ASFLAGS) -o $@ $<

.PHONY: all clean debug

all: $(OBJ) $(DEPS)
	$(CC) $(LDFLAGS) -shared -o $(BIN_DIR)/relf_dummy.so obj/relf_dummy.o
	$(CC) $(LDFLAGS) -o $(BIN_DIR)/relf_test obj/relf_test.o $(LIB_DIR)/librelf.a

debug: CFLAGS += -DDEBUG -g
debug: LDFLAGS += -g
debug: ASFLAGS += -g
debug: all

clean:
	rm -f $(OBJ_DIR)/*