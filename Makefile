NAME         := nessemble
VERSION      := 1.0.1
EXEC         := $(NAME)
BIN_DIR      := /usr/local/bin
RM           := rm -rf
CC           := gcc
CC_FLAGS     := -Wall -Wextra
CC_LIB_FLAGS := -lm
CC_INCLUDES  := /usr/local/include
CC_LIBRARIES := /usr/local/lib
AR           := ar rcu
RANLIB       := ranlib
LEX          := flex
LEX_OUT      := lex.yy
LEX_FLAGS    := --outfile=$(LEX_OUT).c
LEX_DEFINES  := -DYY_TYPEDEF_YY_SIZE_T -Dyy_size_t=ssize_t
YACC         := bison
YACC_OUT     := y.tab
YACC_FLAGS   := --output=$(YACC_OUT).c --defines --yacc

EMAIL        := "kevinselwyn@gmail.com"
MAINTAINER   := "Kevin Selwyn"
DESCRIPTION  := "A 6502 assembler for the Nintendo Entertainment System"
IDENTIFIER   := "kevinselwyn"

UNAME        := $(shell uname -s)

FILES        := main.c api.c assemble.c config.c coverage.c disassemble.c
FILES        += download.c error.c home.c i18n.c init.c instructions.c json.c
FILES        += list.c macro.c math.c midi.c opcodes.c pager.c png.c reference.c
FILES        += registry.c scripts.c simulate.c usage.c user.c utils.c wav.c
FILES        += zip.c
FILES        += $(shell ls pseudo/*.c) $(shell ls scripting/*.c)
FILES        += $(shell ls simulate/*.c)
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

debug: CC_FLAGS     += -g
debug: CC_LIB_FLAGS += -ldl

ifeq ($(UNAME), Darwin)
debug: CC_LIB_FLAGS += -llua -I/usr/include/lua5.1
else
debug: CC_LIB_FLAGS += -llua5.1 -I/usr/include/lua5.1
endif

js: EXEC         := $(NAME).js
js: CC           := emcc
js: CC_FLAGS     := -Wall -Wextra
js: CC_LIB_FLAGS :=

win32: EXEC     := $(NAME).exe
win32: CC       := i686-w64-mingw32-gcc
win32: CC_FLAGS := -lws2_32 -Ilua-5.1.5/src
win32: CC_FILES := lua-5.1.5/src/liblua.a
win32: AR       := i686-w64-mingw32-ar rcu
win32: RANLIB   := i686-w64-mingw32-ranlib

win64: EXEC     := $(NAME).exe
win64: CC       := x86_64-w64-mingw32-gcc
win64: CC_FLAGS := -lws2_32 -Ilua-5.1.5/src
win64: CC_FILES := lua-5.1.5/src/liblua.a
win64: AR       := x86_64-w64-mingw32-ar rcu
win64: RANLIB   := x86_64-w64-mingw32-ranlib

# TARGETS

all: $(EXEC)

debug: $(EXEC)

js: $(EXEC)

win32: liblua $(EXEC)

win64: liblua $(EXEC)

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

%.h: %.tar.gz
	./utils/xxd.py -b -i $< > $@

i18n.c: strings.h

strings.h: ${strings.json:json=h}

init.c: init.h

init.h: ${init.txt:txt=h}

usage.c: license.h

license.h: ${licence.txt:txt=h}

scripts.tar.gz:
	tar -czf $@ $(shell ls scripts/*)

scripts.h: ${scripts.tar.gz:tar.gz=h}

scripts.c: scripts.tar.gz scripts.h

$(EXEC): $(OBJS) $(HDRS)
	$(CC) -o $(EXEC) $(OBJS) $(CC_FLAGS) $(CC_FILES) $(CC_LIB_FLAGS)

# TESTING

test: all
	@python test.py

splint: all
	splint -I/usr/include -I/usr/include/x86_64-linux-gnu \
		   -warnposix $(FLAGS) $(FILES)

# REGISTRY

registry: all
	python ./registry/server.py --debug

# LIBLUA

liblua: lua-5.1.5/src/liblua.a

lua-5.1.5/src/liblua.a: lua-5.1.5.tar.gz
	tar -xzf $<
	make -C lua-5.1.5/src generic CC="$(CC)" AR="$(AR)" RANLIB="$(RANLIB)"

lua-5.1.5.tar.gz:
	curl -O "https://www.lua.org/ftp/$@"

# TRANSLATION

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

# INSTALL/UNINSTALL

install: all
	strip $(EXEC)
	install -m 0755 $(EXEC) $(BIN_DIR)

uninstall:
	rm -f $(BIN_DIR)/$(EXEC)

# PACKAGE

ifeq ($(UNAME), Linux)
ARCHITECTURE := $(shell dpkg --print-architecture)

package: all
	mkdir -p ./package/usr/local/bin
	cp $(EXEC) ./package/usr/local/bin
	mkdir -p ./package/DEBIAN
	printf "Package: %s\n" $(NAME)                         > ./package/DEBIAN/control
	printf "Version: %s\n" $(VERSION)                     >> ./package/DEBIAN/control
	printf "Section: base\n"                              >> ./package/DEBIAN/control
	printf "Priority: optional\n"                         >> ./package/DEBIAN/control
	printf "Architecture: %s\n" $(ARCHITECTURE)           >> ./package/DEBIAN/control
	printf "Depends:\n"                                   >> ./package/DEBIAN/control
	printf "Maintainer: %s <%s>\n" $(MAINTAINER) $(EMAIL) >> ./package/DEBIAN/control
	printf "Description: %s\n" $(DESCRIPTION)             >> ./package/DEBIAN/control
	dpkg-deb --build package $(NAME)-$(ARCHITECTURE).deb
endif

ifeq ($(UNAME), Darwin)
package: all
	mkdir -p ./package/usr/local/bin
	cp $(EXEC) ./package/usr/local/bin
	printf "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"                             > distribution.xml
	printf "<installer-gui-script minSpecVersion=\"1\">\n"                           >> distribution.xml
	printf "    <pkg-ref id=\"com.%s.%s\"/>\n" $(IDENTIFIER) $(NAME)                 >> distribution.xml
	printf "    <options customize=\"never\" require-scripts=\"false\"/>\n"          >> distribution.xml
	printf "    <readme file=\"README.md\" />\n"                                     >> distribution.xml
	printf "    <license file=\"COPYING\" />\n"                                      >> distribution.xml
	printf "    <choices-outline>\n"                                                 >> distribution.xml
	printf "        <line choice=\"default\">\n"                                     >> distribution.xml
	printf "            <line choice=\"com.%s.%s\"/>\n" $(IDENTIFIER) $(NAME)        >> distribution.xml
	printf "        </line>\n"                                                       >> distribution.xml
	printf "    </choices-outline>\n"                                                >> distribution.xml
	printf "    <choice id=\"default\"/>\n"                                          >> distribution.xml
	printf "    <choice id=\"com.%s.%s\" visible=\"false\">\n" $(IDENTIFIER) $(NAME) >> distribution.xml
	printf "        <pkg-ref id=\"com.%s.%s\"/>\n" $(IDENTIFIER) $(NAME)             >> distribution.xml
	printf "    </choice>\n"                                                         >> distribution.xml
	printf "    <pkg-ref id=\"com.%s.%s\" version=\"%s\" onConclusion=\"none\" auth=\"Root\">%s.pkg</pkg-ref>\n" $(IDENTIFIER) $(NAME) $(VERSION) $(NAME) >> distribution.xml
	printf "</installer-gui-script>\n"                                               >> distribution.xml
	pkgbuild --root ./package \
			 --identifier com.$(IDENTIFIER).$(NAME) \
			 --version $(VERSION) \
			 $(NAME).pkg
	productbuild --distribution distribution.xml \
				 --resources . \
				 --package-path $(NAME).pkg \
				 $(NAME)-$(VERSION).pkg
endif

# CLEAN

.PHONY: clean
clean:
	$(RM) $(EXEC) $(EXEC).exe $(EXEC).js
	$(RM) $(YACC_OUT).c $(YACC_OUT).h $(LEX_OUT).c
	$(RM) $(OBJS)
	$(RM) opcodes.c init.h license.h scripts.h scripts.tar.gz strings.h
	$(RM) lua-5.1.5 lua-5.1.5.tar.gz
	$(RM) *.deb *.pkg *.xml
