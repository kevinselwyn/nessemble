NAME         := nessemble
BIN_DIR      := /usr/local/bin
RM           := rm -f
CC           := gcc
CC_FLAGS     := -Wall -Wextra
CC_LIB_FLAGS := -lm -lpng -ljansson -lcurl -larchive
CC_INCLUDES  := /usr/local/include
CC_LIBRARIES := /usr/local/lib
LEX          := flex
LEX_OUT      := lex.yy
LEX_FLAGS    := --outfile=$(LEX_OUT).c
LEX_DEFINES  := -DYY_TYPEDEF_YY_SIZE_T -Dyy_size_t=ssize_t
YACC         := bison
YACC_OUT     := y.tab
YACC_FLAGS   := --output=$(YACC_OUT).c --defines --yacc

OPCODES      := opcodes
TEST         := test
UNAME        := $(shell uname -s)

SRCS         := $(YACC_OUT).c $(LEX_OUT).c main.c assemble.c disassemble.c download.c error.c init.c instructions.c json.c list.c macro.c math.c midi.c opcodes.c png.c $(shell ls pseudo/*.c) reference.c registry.c simulate.c $(shell ls simulate/*.c) usage.c utils.c wav.c zip.c
HDRS         := $(NAME).h init.h license.h
OBJS         := ${SRCS:c=o}

REFERENCE    := reference/registers/ppuctrl.h reference/registers/ppumask.h reference/registers/ppustatus.h reference/registers/oamaddr.h reference/registers/oamdata.h reference/registers/ppuscroll.h reference/registers/ppuaddr.h reference/registers/ppudata.h
REFERENCE    += reference/addressing/accumulator.h reference/addressing/implied.h reference/addressing/immediate.h reference/addressing/relative.h reference/addressing/zeropage.h reference/addressing/zeropage-x.h reference/addressing/zeropage-y.h reference/addressing/absolute.h reference/addressing/absolute-x.h reference/addressing/absolute-y.h reference/addressing/indirect.h reference/addressing/indirect-x.h reference/addressing/indirect-y.h

ifeq ($(ENV), debug)
	CC_FLAGS += -g
endif

ifeq ($(UNAME), Darwin)
	CC_FLAGS += -ll -I$(CC_INCLUDES) -L$(CC_LIBRARIES) -Qunused-arguments
else
	CC_FLAGS += -lfl -lrt
endif

all: $(NAME)

$(LEX_OUT).c: $(NAME).l
	$(LEX) $(LEX_FLAGS) $<

$(LEX_OUT).o: $(LEX_OUT).c
	$(CC) -O -c $< $(CC_FLAGS) $(LEX_DEFINES) -o $@

$(YACC_OUT).c: $(NAME).y
	$(YACC) $(YACC_FLAGS) $<

opcodes.c: opcodes.csv
	./opcodes.sh $< $@

%.o: %.c
	$(CC) -O -c $< $(CC_FLAGS) -o $@

%.h: %.txt
	$(eval STR := _$(shell echo "$@" | awk '{print toupper($$0)}' | sed "s/[^[:alpha:]]/_/g"))
	printf "#ifndef %s\n#define %s\n\n" $(STR) $(STR) > $@
	xxd -i $< >> $@
	printf "\n#endif /* %s */\n" $(STR) >> $@

reference.c: ${REFERENCE:txt=h} reference.h

init.c: init.h

init.h: ${init.txt:txt=h}

usage.c: license.h

license.h: ${licence.txt:txt=h}

$(NAME): $(OBJS) $(HDRS)
	$(CC) -o $@ $(OBJS) $(CC_FLAGS) $(CC_LIB_FLAGS)

$(TEST): all
	@./$(TEST).sh

check: all
	@./check.sh

registry: all
	python ./registry/server.py --debug

install: all
	strip $(NAME)
	install -m 0755 $(NAME) $(BIN_DIR)

uninstall:
	rm -f $(BIN_DIR)/$(NAME)

.PHONY: clean
clean:
	$(RM) $(NAME) $(YACC_OUT).c $(YACC_OUT).h $(LEX_OUT).c $(OPCODES).c $(OBJS) init.h license.h $(REFERENCE) check/suite_*
