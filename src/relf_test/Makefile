NAME				=	relf_test

CFLAGS				=	-Wall -I$(INC_DIR)
ASFLAGS				=
LDFLAGS				=	-z noexecstack

SRC		=	relf_test.c relf_dummy.c
OBJ		:=	$(addprefix $(OBJ_DIR)/,$(patsubst %.s,%.o,$(patsubst %.c,%.o,$(SRC)))) $(OBJ_DIR)/relf_dummy.o32
DEPS	=

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o32: %.c32
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -m32 -x c -c -o $@ $<

$(OBJ_DIR)/%.o: %.s
	@mkdir -p $(OBJ_DIR)
	$(AS) $(ASFLAGS) -o $@ $<

.PHONY: all clean debug

all: $(OBJ) $(DEPS)
	$(CC) $(LDFLAGS) -shared -o $(BIN_DIR)/relf_dummy.so obj/relf_dummy.o
	$(CC) $(LDFLAGS) -m32 -shared -o $(BIN_DIR)/relf_dummy32.so obj/relf_dummy.o32
	$(CC) $(LDFLAGS) -o $(BIN_DIR)/relf_test obj/relf_test.o $(LIB_DIR)/librelf.a -ldl

debug: CFLAGS += -DDEBUG -g
debug: LDFLAGS += -g
debug: ASFLAGS += -g
debug: all

clean:
	rm -f $(OBJ_DIR)/*