NAME         := nessemble
BIN_DIR      := /usr/local/bin
RM           := rm -f
CC           := gcc
CC_FLAGS     := -Wall -Wextra
CC_LIB_FLAGS := -ll -lpng
CC_INCLUDES  := /usr/local/include
CC_LIBRARIES := /usr/local/lib
LEX          := flex
LEX_OUT      := lex.yy
LEX_FLAGS    := --outfile=$(LEX_OUT).c
YACC         := bison
YACC_OUT     := y.tab
YACC_FLAGS   := --output=$(YACC_OUT).c --defines --yacc

OPCODES      := opcodes
TEST         := test
UNAME        := $(shell uname -s)

SRCS         := $(YACC_OUT).c $(LEX_OUT).c main.c assemble.c disassemble.c error.c instructions.c list.c macro.c math.c midi.c opcodes.c png.c $(shell ls pseudo/*.c) simulate.c $(shell ls simulate/*.c) usage.c utils.c wav.c
OBJS         := ${SRCS:c=o}

ifeq ($(ENV), debug)
	CC_FLAGS += -g
endif

ifeq ($(UNAME), Darwin)
	CC_FLAGS += -I$(CC_INCLUDES) -L$(CC_LIBRARIES) -Qunused-arguments
else
	CC_FLAGS += -lrt
endif

all: $(NAME)

$(LEX_OUT).c: $(NAME).l
	$(LEX) $(LEX_FLAGS) $<

$(YACC_OUT).c: $(NAME).y
	$(YACC) $(YACC_FLAGS) $<

$(OPCODES).c: $(OPCODES).csv
	./$(OPCODES).sh $< $@

%.o: %.c
	$(CC) -O -c $< $(CC_FLAGS) -o $@

$(NAME): $(OBJS) $(NAME).h
	$(CC) -o $@ $(OBJS) $(CC_FLAGS) $(CC_LIB_FLAGS)

$(TEST): all
	@./$(TEST).sh

check: all
	@./check.sh

install: all
	strip $(NAME)
	install -m 0755 $(NAME) $(BIN_DIR)

uninstall:
	rm -f $(BIN_DIR)/$(NAME)

.PHONY: clean
clean:
	$(RM) $(NAME) $(YACC_OUT).c $(YACC_OUT).h $(LEX_OUT).c $(OPCODES).c $(OBJS) check/suite_*
