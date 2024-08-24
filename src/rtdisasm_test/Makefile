NAME				=	rtdisasm_test

CFLAGS				=	-Wall -mavx -I$(INC_DIR)
ASFLAGS				=
LDFLAGS				=	-z noexecstack -lcap

SRC		=	rtdisasm_test.c rtdisasm_test_data.s
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
	$(CC) $(LDFLAGS) -o $(BIN_DIR)/$(NAME) $(OBJ) $(LIB_DIR)/librtdisasm.a

debug: CFLAGS += -DDEBUG -g
debug: LDFLAGS += -g
debug: ASFLAGS += -g
debug: all

clean:
	rm -f $(OBJ_DIR)/*