NAME         := nessemble
VERSION      := 1.0.1
EXEC         := $(NAME)
BIN_DIR      := /usr/local/bin
RM           := rm -rf
CC           := gcc
CC_FLAGS     := -Wall -Wextra -lm
CC_LIB_FLAGS := -Isrc/third-party/lua-5.1.5/src
CC_INCLUDES  := /usr/local/include
CC_LIBRARIES := /usr/local/lib
AR           := ar
RANLIB       := ranlib
LEX          := flex
LEX_OUT      := lex.yy
LEX_FLAGS    := --outfile=src/$(LEX_OUT).c
LEX_DEFINES  := -DYY_TYPEDEF_YY_SIZE_T -Dyy_size_t=ssize_t
YACC         := bison
YACC_OUT     := y.tab
YACC_FLAGS   := --output=src/$(YACC_OUT).c --defines --yacc
SCHEME_FLAGS :=

EMAIL        := kevinselwyn@gmail.com
MAINTAINER   := Kevin Selwyn
DESCRIPTION  := A 6502 assembler for the Nintendo Entertainment System
IDENTIFIER   := kevinselwyn
PACKAGE      := ./package
PAYLOAD      := $(PACKAGE)/payload
RELEASE      := ./release
TMP          := $(shell mktemp)
UNAME        := $(shell uname -s)

FILES        := src/main.c src/api.c src/assemble.c src/config.c src/coverage.c
FILES        += src/disassemble.c src/download.c src/error.c src/hash.c
FILES        += src/home.c src/i18n.c src/init.c src/instructions.c src/json.c
FILES        += src/list.c src/macro.c src/math.c src/midi.c src/opcodes.c
FILES        += src/pager.c src/png.c src/reference.c src/registry.c
FILES        += src/scripts.c src/simulate.c src/usage.c src/user.c src/utils.c
FILES        += src/wav.c src/zip.c
FILES        += $(shell ls src/pseudo/*.c) $(shell ls src/scripting/*.c)
FILES        += $(shell ls src/simulate/*.c)
FILES        += src/third-party/jsmn/jsmn.c src/third-party/udeflate/deflate.c
FILES        += src/third-party/duktape/duktape.c

SRCS         := src/$(YACC_OUT).c src/$(LEX_OUT).c $(FILES)
HDRS         := src/$(NAME).h src/font.h src/init.h src/license.h
HDRS         += src/third-party/tinyscheme-1.41/init.h
OBJS         := ${SRCS:c=o}

# PLATFORM-SPECIFIC

ifeq ($(UNAME), Darwin)
all: CC_FLAGS   += -I$(CC_INCLUDES) -L$(CC_LIBRARIES) -Qunused-arguments -lintl
debug: CC_FLAGS += -I$(CC_INCLUDES) -L$(CC_LIBRARIES) -Qunused-arguments -lintl
else
all: CC_FLAGS   += -lfl -lrt
debug: CC_FLAGS += -lfl -lrt
endif

# TARGET-SPECIFIC

all: CC_LIB_FLAGS += -ldl

debug: CC_FLAGS     += -g
debug: CC_LIB_FLAGS += -ldl

js: EXEC            := $(NAME).js
js: CC              := emcc
js: AR              := emar
js: RANLIB          := emranlib
js: SCHEME_FLAGS    := -DUSE_STRLWR=0

win32: EXEC         := $(NAME).exe
win32: CC           := i686-w64-mingw32-gcc
win32: CC_FLAGS     += -lws2_32
win32: AR           := i686-w64-mingw32-ar
win32: RANLIB       := i686-w64-mingw32-ranlib
win32: SCHEME_FLAGS := -DUSE_STRLWR=0

win64: EXEC         := $(NAME).exe
win64: CC           := x86_64-w64-mingw32-gcc
win64: CC_FLAGS     += -lws2_32
win64: AR           := x86_64-w64-mingw32-ar
win64: RANLIB       := x86_64-w64-mingw32-ranlib

# TARGETS

all: $(EXEC)

debug: $(EXEC)

js: $(EXEC)

win32: $(EXEC)

win64: $(EXEC)

# RECIPES

src/$(LEX_OUT).c: src/$(NAME).l
	$(LEX) $(LEX_FLAGS) $<

src/$(LEX_OUT).o: src/$(LEX_OUT).c
	$(CC) -O -c $< $(CC_FLAGS) $(LEX_DEFINES) -o $@

src/$(YACC_OUT).c: src/$(NAME).y
	$(YACC) $(YACC_FLAGS) $<

src/opcodes.c: src/opcodes.csv
	./utils/opcodes.py -i $< > $@

%.o: %.c
	$(CC) -O -c $< $(CC_FLAGS) $(CC_LIB_FLAGS) -o $@

%.h: %.txt
	./utils/xxd.py -i $< > $@

%.h: %.scm
	./utils/xxd.py -i $< > $@

%.h: %.json
	./utils/xxd.py -i $< > $@

%.h: %.chr
	./utils/xxd.py -b -i $< > $@

%.h: %.tar.gz
	./utils/xxd.py -b -i $< > $@

src/i18n.c: src/strings.h

src/strings.h: ${src/strings.json:json=h}

src/init.c: src/init.h

src/init.h: ${src/init.txt:txt=h}

src/pseudo/font.c: src/font.h

src/font.h: ${src/font.chr:chr=h}

src/font.chr: src/font.png
	./utils/img2chr.py -i $<

src/usage.c: src/license.h

src/license.h: ${src/licence.txt:txt=h}

src/scripts.tar.gz:
	tar -czf $@ $(shell ls src/scripts/*)

src/scripts.h: ${src/scripts.tar.gz:tar.gz=h}

src/scripts.c: src/scripts.tar.gz src/scripts.h

$(EXEC): $(OBJS) $(HDRS)
	$(CC) -o $(EXEC) $(OBJS) $(CC_FLAGS) $(CC_FILES) $(CC_LIB_FLAGS) \
		src/third-party/lua-5.1.5/src/liblua.a \
		src/third-party/tinyscheme-1.41/libtinyscheme.a

# TESTING

test: all
	@python test.py

splint: all
	splint -I/usr/include -I/usr/include/x86_64-linux-gnu \
		   -warnposix $(FLAGS) $(FILES)

# LIBLUA

src/scripting/lua.c: liblua

liblua: src/third-party/lua-5.1.5/src/liblua.a

src/third-party/lua-5.1.5/src/liblua.a:
	make -C src/third-party/lua-5.1.5/src/ generic \
		CC="$(CC)" AR="$(AR) rcu" RANLIB="$(RANLIB)"

# LIBTINYSCHEME

src/scripting/scm.c: libtinyscheme

libtinyscheme: src/third-party/tinyscheme-1.41/libtinyscheme.a

src/third-party/tinyscheme-1.41/libtinyscheme.a:
	make -C src/third-party/tinyscheme-1.41/ libtinyscheme.a \
		CC="$(CC) -fpic -pedantic" AR="$(AR) crs" PLATFORM_FEATURES="$(SCHEME_FLAGS)"

src/scripting/scm.c: src/third-party/tinyscheme-1.41/init.h

src/third-party/tinyscheme-1.41/init.h: ${src/third-party/tinyscheme-1.41/init.scm:scm=h}

# TRANSLATION

translate/nessemble.pot:
	@mkdir -p translate
	@xgettext --keyword=_ --language=C --add-comments --sort-output \
	  		  --output=translate/nessemble.pot --package-name=$(NAME) \
			  --package-version=$(VERSION) src/*.c
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

# SERVER

.PHONY: server
server:
	cd docs ; mkdocs build --clean
	python server.py --debug --port 8000

# WEBSITE

.PHONY: website
website:
	cd website ; python index.py --debug --port 9000

# REGISTRY

.PHONY: registry
registry:
	cd registry ; python index.py --debug --port 8000 --import registry.sql

# DOCUMENTATION

.PHONY: docs
docs:
	cd docs ; mkdocs build --clean ; python index.py --debug --port 9090

# INSTALL/UNINSTALL

install: all
	strip $(EXEC)
	install -m 0755 $(EXEC) $(BIN_DIR)

uninstall:
	rm -f $(BIN_DIR)/$(EXEC)

# PACKAGE

ifeq ($(UNAME), Linux)
ARCHITECTURE := $(shell dpkg --print-architecture)
YEAR         := $(shell date +"%Y")
endif

package: all
ifeq ($(UNAME), Linux)
	$(RM) $(PAYLOAD)
	mkdir -p $(RELEASE)
	mkdir -p $(PAYLOAD)/usr/local/bin
	mkdir -p $(PAYLOAD)/usr/share/doc/$(NAME)
	mkdir -p $(PAYLOAD)/usr/share/bash-completion/completions
	strip $(EXEC)
	cp $(EXEC) $(PAYLOAD)/usr/local/bin
	cp $(NAME)-completion.bash $(PAYLOAD)/usr/share/bash-completion/completions/$(NAME)
	mkdir -p $(PAYLOAD)/DEBIAN
	sed -e "s/\$${NAME}/$(NAME)/g" \
		-e "s/\$${VERSION}/$(VERSION)/g" \
		-e "s/\$${ARCHITECTURE}/$(ARCHITECTURE)/g" \
		-e "s/\$${MAINTAINER}/$(MAINTAINER)/g" \
		-e "s/\$${EMAIL}/$(EMAIL)/g" \
		-e "s/\$${SIZE}/$(shell wc -c < $(EXEC))/g" \
		-e "s/\$${DESCRIPTION}/$(DESCRIPTION)/g" \
	 	$(PACKAGE)/control > $(PAYLOAD)/DEBIAN/control
	sed -e "s/\$${NAME}/$(NAME)/g" \
		-e "s/\$${VERSION}/$(VERSION)/g" \
		-e "s/\$${YEAR}/$(YEAR)/g" \
		-e "s/\$${MAINTAINER}/$(MAINTAINER)/g" \
		-e "s/\$${LICENSE}/$(LICENSE)/g" \
		$(PACKAGE)/copyright > $(PAYLOAD)/usr/share/doc/$(NAME)/copyright
	cat src/license.txt >> $(PAYLOAD)/usr/share/doc/$(NAME)/copyright
	cd $(PAYLOAD) ; md5sum `find usr -type f` > DEBIAN/md5sums
	cp $(PACKAGE)/scripts/linux/* $(PAYLOAD)/DEBIAN
	dpkg-deb --build $(PAYLOAD) $(RELEASE)/$(NAME)_$(VERSION)_$(ARCHITECTURE).deb
	$(RM) $(PAYLOAD)
endif

ifeq ($(UNAME), Darwin)
	$(RM) $(PAYLOAD)
	mkdir -p $(RELEASE)
	mkdir -p $(PAYLOAD)/usr/local/bin
	mkdir -p $(PAYLOAD)/usr/local/etc/bash_completion.d
	strip $(EXEC)
	cp $(EXEC) $(PAYLOAD)/usr/local/bin
	cp $(NAME)-completion.bash $(PAYLOAD)/usr/local/etc/bash_completion.d
	sed -e "s/\$${NAME}/$(NAME)/g" \
		-e "s/\$${IDENTIFIER}/$(IDENTIFIER)/g" \
		-e "s/\$${VERSION}/$(VERSION)/g" \
	 	$(PACKAGE)/distribution.xml > $(TMP)
	pkgbuild --root $(PAYLOAD) \
			 --identifier com.$(IDENTIFIER).$(NAME) \
			 --scripts $(PACKAGE)/scripts/osx \
			 --version $(VERSION) \
			 $(NAME).pkg
	productbuild --distribution $(TMP) \
				 --resources . \
				 --package-path $(NAME).pkg \
				 $(RELEASE)/$(NAME)_$(VERSION).pkg
	$(RM) $(TMP) $(PAYLOAD) $(NAME).pkg
endif

win32_package: ARCHITECTURE := win32
win32_package: win32 win_package

win64_package: ARCHITECTURE := win64
win64_package: win64 win_package

win_package:
	$(RM) $(PAYLOAD)
	mkdir -p $(RELEASE)
	mkdir -p $(PAYLOAD)
	sed -e "s/\$${NAME}/$(NAME)/g" \
		-e "s/\$${VERSION}/$(VERSION)/g" \
		-e "s/\$${ID}/$(shell ./utils/guid.py --string $(VERSION))/g" \
		-e "s/\$${MAINTAINER}/$(MAINTAINER)/g" \
		-e "s/\$${DESCRIPTION}/$(DESCRIPTION)/g" \
		-e "s/\$${GUID}/$(shell ./utils/guid.py --input $(NAME).exe)/g" \
	 	$(PACKAGE)/msi.wxs > $(TMP)
	wixl $(TMP) --output $(RELEASE)/$(NAME)_$(VERSION)_$(ARCHITECTURE).msi
	$(RM) $(TMP) $(PAYLOAD)

# CLEAN

.PHONY: clean
clean:
	$(RM) $(EXEC) $(EXEC).exe $(EXEC).js
	$(RM) $(OBJS)
	$(RM) src/$(YACC_OUT).c src/$(YACC_OUT).h src/$(LEX_OUT).c
	$(RM) src/init.h src/license.h src/scripts.h src/strings.h
	$(RM) src/opcodes.c src/scripts.tar.gz
	$(RM) $(PAYLOAD)
	make -C src/third-party/tinyscheme-1.41/ clean
	make -C src/third-party/lua-5.1.5/src/ clean
