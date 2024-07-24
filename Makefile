INC_DIR				=	include
SRC_DIR				=	src
OBJ_DIR				=	obj
BIN_DIR				=	bin
LIB_DIR				=	bin

CC					=	gcc
AS					=	as
AR					=	ar
LD					=	ld
CFLAGS				=	-Wall -I$(INC_DIR)
ASFLAGS				=
LDFLAGS				=	-z noexecstack -lcap

RTDISASM_SRC		=	rtdisasm.c
RTDISASM_OBJ		:=	$(addprefix $(OBJ_DIR)/,$(patsubst %.s,%.o,$(patsubst %.c,%.o,$(RTDISASM_SRC))))
RTDISASM_SRC		:=	$(addprefix $(SRC_DIR)/,$(RTDISASM_SRC))
RTDISASM_DEPS		=	rtdisasm.h
RTDISASM_DEPS		:=	$(addprefix $(INC_DIR)/,$(RTDISASM_DEPS))

RTDISASM_TEST_SRC		=	rtdisasm_test.c
RTDISASM_TEST_OBJ		:=	$(addprefix $(OBJ_DIR)/,$(patsubst %.s,%.o,$(patsubst %.c,%.o,$(RTDISASM_TEST_SRC))))
RTDISASM_TEST_SRC		:=	$(addprefix $(SRC_DIR)/,$(RTDISASM_TEST_SRC))
RTDISASM_TEST_DEPS		=
RTDISASM_TEST_DEPS		:=	$(addprefix $(INC_DIR)/,$(RTDISASM_TEST_DEPS)) rtdisasm

BLACKJACK_SRC		=	main.c process.c
BLACKJACK_OBJ		:=	$(addprefix $(OBJ_DIR)/,$(patsubst %.s,%.o,$(patsubst %.c,%.o,$(BLACKJACK_SRC))))
BLACKJACK_SRC		:=	$(addprefix $(SRC_DIR)/,$(BLACKJACK_SRC))
BLACKJACK_DEPS		=	debug.h process.h
BLACKJACK_DEPS		:=	$(addprefix $(INC_DIR)/,$(BLACKJACK_DEPS)) rtdisasm

DUMMY_TARGET_SRC	=	dummy_target.c dummy_destination.s
DUMMY_TARGET_OBJ	:=	$(addprefix $(OBJ_DIR)/,$(patsubst %.s,%.o,$(patsubst %.c,%.o,$(DUMMY_TARGET_SRC))))
DUMMY_TARGET_SRC	:=	$(addprefix $(SRC_DIR)/,$(DUMMY_TARGET_SRC))

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.s
	$(AS) $(ASFLAGS) -o $@ $<

rtdisasm: $(RTDISASM_OBJ) $(RTDISASM_DEPS)
	$(AR) -crs $(BIN_DIR)/librtdisasm.a $(RTDISASM_OBJ)

rtdisasm_test: $(RTDISASM_TEST_OBJ) $(RTDISASM_TEST_DEPS)
	$(CC) $(LDFLAGS) $(LIB_DIR)/librtdisasm.a -o $(BIN_DIR)/$@ $(RTDISASM_TEST_OBJ)

blackjack: $(BLACKJACK_OBJ) $(BLACKJACK_DEPS)
	$(CC) $(LDFLAGS) $(LIB_DIR)/librtdisasm.a -o $(BIN_DIR)/$@ $(BLACKJACK_OBJ)

dummy_target: $(DUMMY_TARGET_OBJ)
	$(CC) $(LDFLAGS) -o $(BIN_DIR)/$@ $(DUMMY_TARGET_OBJ)

.PHONY: all clean debug

TARGETS				=	blackjack dummy_target rtdisasm_test

all: $(TARGETS)

debug: CFLAGS += -DDEBUG -g
debug: LDFLAGS += -g
debug: ASFLAGS += -g
debug: $(TARGETS)

clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(BIN_DIR)/*