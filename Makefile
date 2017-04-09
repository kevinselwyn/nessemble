NAME         := nessemble
VERSION      := 1.0.1
EXEC         := $(NAME)
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

UNAME        := $(shell uname -s)

FILES        := main.c api.c assemble.c config.c coverage.c disassemble.c
FILES        += download.c error.c home.c i18n.c init.c instructions.c json.c
FILES        += list.c macro.c math.c midi.c opcodes.c pager.c png.c reference.c
FILES        += registry.c simulate.c usage.c user.c utils.c wav.c zip.c
FILES        += $(shell ls pseudo/*.c) $(shell ls simulate/*.c)
FILES        += third-party/jsmn/jsmn.c third-party/udeflate/deflate.c
FILES        += third-party/duktape/duktape.c

SRCS         := $(YACC_OUT).c $(LEX_OUT).c $(FILES)
HDRS         := $(NAME).h init.h license.h
OBJS         := ${SRCS:c=o}

# PLATFORM-SPECIFIC

ifeq ($(UNAME), Darwin)
	CC_FLAGS += -I$(CC_INCLUDES) -L$(CC_LIBRARIES) -Qunused-arguments -lintl
else
	CC_FLAGS += -lfl -lrt
endif

# TARGET-SPECIFIC

all: CC_LIB_FLAGS += -ldl

ifeq ($(UNAME), Darwin)
all: CC_LIB_FLAGS += -llua -I/usr/include/lua5.1
else
all: CC_LIB_FLAGS += -llua5.1 -I/usr/include/lua5.1
endif
all: CC_LIB_FLAGS += -lpython2.7 -I/usr/include/python2.7

debug: CC_FLAGS     += -g
debug: CC_LIB_FLAGS += -ldl
debug: CC_LIB_FLAGS += -llua5.1 -I/usr/include/lua5.1
debug: CC_LIB_FLAGS += -lpython2.7 -I/usr/include/python2.7

js: EXEC         := $(NAME).js
js: CC           := emcc
js: CC_FLAGS     := -Wall -Wextra
js: CC_LIB_FLAGS :=

win32: EXEC         := $(NAME).exe
win32: CC           := i686-w64-mingw32-gcc
win32: CC_FLAGS     := -lws2_32

win64: EXEC         := $(NAME).exe
win64: CC           := x86_64-w64-mingw32-gcc
win64: CC_FLAGS     := -lws2_32

# TARGETS

all: $(EXEC)

debug: $(EXEC)

js: $(EXEC)

win32: $(EXEC)

win64: $(EXEC)

# RECIPES

$(LEX_OUT).c: $(NAME).l
	$(LEX) $(LEX_FLAGS) $<

$(LEX_OUT).o: $(LEX_OUT).c
	$(CC) -O -c $< $(CC_FLAGS) $(LEX_DEFINES) -o $@

$(YACC_OUT).c: $(NAME).y
	$(YACC) $(YACC_FLAGS) $<

opcodes.c: opcodes.csv
	./utils/opcodes.py -i $< > $@

%.o: %.c
	$(CC) -O -c $< $(CC_FLAGS) $(CC_LIB_FLAGS) -o $@

%.h: %.txt
	./utils/xxd.py -i $< > $@

%.h: %.json
	./utils/xxd.py -i $< > $@

i18n.c: strings.h

strings.h: ${strings.json:json=h}

init.c: init.h

init.h: ${init.txt:txt=h}

usage.c: license.h

license.h: ${licence.txt:txt=h}

$(EXEC): $(OBJS) $(HDRS)
	$(CC) -o $(EXEC) $(OBJS) $(CC_FLAGS) $(CC_LIB_FLAGS)

test: all
	@python test.py

splint: all
	splint -I/usr/include -I/usr/include/x86_64-linux-gnu \
		   -warnposix $(FLAGS) $(FILES)

registry: all
	python ./registry/server.py --debug

translate/nessemble.pot:
	@mkdir -p translate
	@xgettext --keyword=_ --language=C --add-comments --sort-output \
	  		  --output=translate/nessemble.pot --package-name=$(NAME) \
			  --package-version=$(VERSION) *.c
	@printf "Language template created\n"

translate-template: translate/nessemble.pot

translate/$(LANG)/nessemble.po: translate/nessemble.pot
	@mkdir -p translate/$(LANG)
	@msginit --input=$< --locale=$(LANG) --output=$@
	@printf "Language created: %s\n" $(LANG)

translate-new: translate/$(LANG)/nessemble.po

translate/$(LANG)/nessemble.mo: translate/$(LANG)/nessemble.po
	@msgfmt --output-file=$@ $<
	@printf "Language compiled: %s\n" $(LANG)

translate-compile: translate/$(LANG)/nessemble.mo

translate-install: translate/$(LANG)/nessemble.mo
	@mkdir -p ~/.nessemble/locale/de/LC_MESSAGES
	@cp $< ~/.nessemble/locale/de/LC_MESSAGES/
	@printf "Language installed: %s\n" $(LANG)

install: all
	strip $(EXEC)
	install -m 0755 $(EXEC) $(BIN_DIR)

uninstall:
	rm -f $(BIN_DIR)/$(EXEC)

.PHONY: clean
clean:
	$(RM) $(EXEC) $(EXEC).exe $(EXEC).js $(YACC_OUT).c $(YACC_OUT).h
	$(RM) $(LEX_OUT).c opcodes.c $(OBJS) init.h license.h strings.h
