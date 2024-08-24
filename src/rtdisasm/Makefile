NAME				=	rtdisasm

CFLAGS				=	-Wall -I$(INC_DIR)
ASFLAGS				=
LDFLAGS				=	-z noexecstack -lcap

SRC		=	rtdisasm.c
OBJ		:=	$(addprefix $(OBJ_DIR)/,$(patsubst %.s,%.o,$(patsubst %.c,%.o,$(SRC))))
DEPS	=	rtdisasm.h rtdisasm_table.h

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: all clean debug

all: $(OBJ) $(DEPS)
	$(PYTHON) genc.py xml/raw/x86/Intel/AZ.xml | $(CC) -x c $(CFLAGS) -c -o $(OBJ_DIR)/rtdisasm_table.o -
	$(AR) -crs $(BIN_DIR)/librtdisasm.a $(OBJ) $(OBJ_DIR)/rtdisasm_table.o

debug: CFLAGS += -DDEBUG -g
debug: LDFLAGS += -g
debug: ASFLAGS += -g
debug: all

clean:
	rm -f $(OBJ_DIR)/*