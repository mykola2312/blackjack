NAME				=	relf

CFLAGS				=	-Wall -I$(INC_DIR)
ASFLAGS				=
LDFLAGS				=	-z noexecstack

SRC		=	relf.c
OBJ		:=	$(addprefix $(OBJ_DIR)/,$(patsubst %.s,%.o,$(patsubst %.c,%.o,$(SRC))))
DEPS	=	relf.h

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: all clean debug

all: $(OBJ) $(DEPS)
	$(AR) -crs $(BIN_DIR)/lib$(NAME).a $(OBJ)

debug: CFLAGS += -DDEBUG -g
debug: LDFLAGS += -g
debug: ASFLAGS += -g
debug: all

clean:
	rm -f $(OBJ_DIR)/*