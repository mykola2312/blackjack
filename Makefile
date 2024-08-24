ROOT_DIR			=	$(PWD)
export ROOT_DIR

INC_DIR				=	$(ROOT_DIR)/src
SRC_DIR				=	$(ROOT_DIR)/src
BIN_DIR				=	$(ROOT_DIR)/bin
LIB_DIR				=	$(ROOT_DIR)/bin
OBJ_DIR				=	obj

export INC_DIR
export SRC_DIR
export BIN_DIR
export LIB_DIR
export OBJ_DIR

CC					=	gcc
AS					=	as
AR					=	ar
LD					=	ld
MAKE				=	gmake
PYTHON				=	python

export CC
export AS
export AR
export LD
export MAKE
export PYTHON

# order matters here, build libraries first!
TARGETS				=	rtdisasm rtdisasm_test dummy_target blackjack

.PHONY: $(TARGETS) debug clean

all:
	for target in $(TARGETS); do $(MAKE) -C src/$$target; done

debug: CFLAGS += -DDEBUG -g
debug: LDFLAGS += -g
debug: ASFLAGS += -g
debug:
	for target in $(TARGETS); do $(MAKE) -C src/$$target debug; done

clean:
	for target in $(TARGETS); do $(MAKE) -C src/$$target clean; done
	rm -f $(BIN_DIR)/*