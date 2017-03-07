NAME         := nessemble
BIN_DIR      := /usr/local/bin
RM           := rm -f
CC           := gcc
CC_FLAGS     := -Wall -Wextra
CC_LIB_FLAGS := -lm
CC_INCLUDES  := /usr/local/include
CC_LIBRARIES := /usr/local/lib
LEX          := flex
LEX_OUT      := lex.yy
LEX_FLAGS    := --outfile=$(LEX_OUT).c
LEX_DEFINES  := -DYY_TYPEDEF_YY_SIZE_T -Dyy_size_t=ssize_t
YACC         := bison
YACC_OUT     := y.tab
YACC_FLAGS   := --output=$(YACC_OUT).c --defines --yacc

TEST         := test
UNAME        := $(shell uname -s)

FILES        := main.c assemble.c config.c coverage.c disassemble.c download.c
FILES        += error.c init.c instructions.c json.c list.c macro.c math.c
FILES        += midi.c opcodes.c pager.c png.c $(shell ls pseudo/*.c)
FILES        += reference.c registry.c simulate.c $(shell ls simulate/*.c)
FILES        += usage.c utils.c wav.c zip.c
FILES        += third-party/jsmn/jsmn.c third-party/udeflate/deflate.c

SRCS         := $(YACC_OUT).c $(LEX_OUT).c $(FILES)
HDRS         := $(NAME).h init.h license.h
OBJS         := ${SRCS:c=o}

REFERENCE    := $(shell ls reference/registers/*.txt | sed 's/.txt/.h/g')
REFERENCE    += $(shell ls reference/addressing/*.txt | sed 's/.txt/.h/g')
REFERENCE    += $(shell ls reference/mappers/*.txt | sed 's/.txt/.h/g')

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
	./utils/opcodes.py -i $< > $@

%.o: %.c
	$(CC) -O -c $< $(CC_FLAGS) -o $@

%.h: %.txt
	./utils/xxd.py -i $< > $@

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

splint: all
	splint -I/usr/include -I/usr/include/x86_64-linux-gnu -warnposix $(FLAGS) $(FILES)

registry: all
	python ./registry/server.py --debug

install: all
	strip $(NAME)
	install -m 0755 $(NAME) $(BIN_DIR)

uninstall:
	rm -f $(BIN_DIR)/$(NAME)

.PHONY: clean
clean:
	$(RM) $(NAME) $(YACC_OUT).c $(YACC_OUT).h $(LEX_OUT).c opcodes.c $(OBJS) init.h license.h $(REFERENCE) check/suite_*
