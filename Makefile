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
CC_LIBLUA    := -DLUA_USE_POSIX -Wno-empty-body
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

DEPLOY_FILES := Procfile requirements.txt runtime.txt server.py settings.cfg
DEPLOY_FILES += docs/custom_theme docs/docs docs/site docs/__init__.py
DEPLOY_FILES += docs/index.py docs/mkdocs.yml docs/settings.cfg
DEPLOY_FILES += registry/registry registry/templates registry/__init__.py
DEPLOY_FILES += registry/index.py registry/registry.sql registry/settings.cfg
DEPLOY_FILES += website/static website/templates website/website
DEPLOY_FILES += website/__init__.py website/settings.cfg

MAINTENANCE  := 0

SCRIPTING     := -DSCRIPTING_JAVASCRIPT=1 -DSCRIPTING_LUA=1 -DSCRIPTING_SCHEME=1
SCRIPTING_LUA := src/third-party/lua-5.1.5/src/liblua.a
SCRIPTING_SCM := src/third-party/tinyscheme-1.41/libtinyscheme.a

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
FILES        += src/disassemble.c src/error.c src/hash.c src/home.c src/http.c
FILES        += src/i18n.c src/init.c src/instructions.c src/json.c src/list.c
FILES        += src/macro.c src/math.c src/static/opcodes.c src/pager.c
FILES        += src/png.c src/reference.c src/registry.c src/scripts.c
FILES        += src/simulate.c src/usage.c src/user.c src/utils.c src/wav.c
FILES        += src/zip.c
FILES        += $(shell ls src/pseudo/*.c)
FILES        += $(shell ls src/simulate/*.c)
FILES        += src/third-party/jsmn/jsmn.c src/third-party/udeflate/deflate.c

FILES        += src/scripting/cmd.c

ifneq (,$(findstring SCRIPTING_JAVASCRIPT=1,$(SCRIPTING)))
FILES        += src/scripting/js.c
FILES        += src/third-party/duktape/duktape.c
endif

ifneq (,$(findstring SCRIPTING_LUA=1,$(SCRIPTING)))
FILES        += src/scripting/lua.c
else
CC_LIB_FLAGS  :=
SCRIPTING_LUA :=
endif

ifneq (,$(findstring SCRIPTING_SCHEME=1,$(SCRIPTING)))
FILES        += src/scripting/scm.c
else
SCRIPTING_SCM :=
endif

FILES        += src/scripting/so.c

SRCS         := src/$(YACC_OUT).c src/$(LEX_OUT).c $(FILES)
HDRS         := src/$(NAME).h
HDRS         := src/static/font.h src/static/init.h src/static/license.h
HDRS         += src/third-party/tinyscheme-1.41/init.h
OBJS         := ${SRCS:c=o}

# PLATFORM-SPECIFIC

ifeq ($(UNAME), Darwin)
all:   CC_FLAGS += -I$(CC_INCLUDES) -L$(CC_LIBRARIES) -Qunused-arguments -lintl
debug: CC_FLAGS += -I$(CC_INCLUDES) -L$(CC_LIBRARIES) -Qunused-arguments -lintl
else
all:   CC_FLAGS += -lfl -lrt
debug: CC_FLAGS += -lfl -lrt
endif

# TARGET-SPECIFIC

all: CC_LIB_FLAGS += -ldl

debug: CC_FLAGS     += -g
debug: CC_LIB_FLAGS += -ldl

js: EXEC            := $(NAME).js
js: CC              := $(shell find $$HOME/emsdk-portable/emscripten/ -type f -name 'emcc' 2>/dev/null)
js: CC_FILES        := --embed-file src/static/scripts@/static/scripts
js: CC_FLAGS        += -s MODULARIZE=1
js: CC_LIBLUA       := -Wno-empty-body
js: AR              := $(shell find $$HOME/emsdk-portable/emscripten/ -type f -name 'emar' 2>/dev/null)
js: RANLIB          := $(shell find $$HOME/emsdk-portable/emscripten/ -type f -name 'emranlib' 2>/dev/null)
js: SCHEME_FLAGS    := -DUSE_STRLWR=0

win32: EXEC         := $(NAME).exe
win32: CC           := i686-w64-mingw32-gcc
win32: CC_FLAGS     += -lws2_32
win32: CC_LIBLUA    := -Wno-misleading-indentation
win32: AR           := i686-w64-mingw32-ar
win32: RANLIB       := i686-w64-mingw32-ranlib
win32: SCHEME_FLAGS := -DUSE_STRLWR=0

win64: EXEC         := $(NAME).exe
win64: CC           := x86_64-w64-mingw32-gcc
win64: CC_FLAGS     += -lws2_32
win64: CC_LIBLUA    := -Wno-misleading-indentation
win64: AR           := x86_64-w64-mingw32-ar
win64: RANLIB       := x86_64-w64-mingw32-ranlib
win64: SCHEME_FLAGS := -DUSE_STRLWR=0

# TARGETS

all: $(EXEC)

debug: $(EXEC)

js-pre:
	cd $$HOME/emsdk-portable && /bin/bash ./emsdk_env.sh

js: $(EXEC)

min.js:
	@printf "Building JS...\n"
	@$(MAKE) js >/dev/null 2>/dev/null || :
	@printf "Minifying JS...\n"
	@uglifyjs $(EXEC).js --output $(EXEC).min.js

wasm:
	$(MAKE) js CC="emcc -s WASM=1"

win32: $(EXEC)

win64: $(EXEC)

# RECIPES

src/$(LEX_OUT).c: src/$(NAME).l
	$(LEX) $(LEX_FLAGS) $<

src/$(LEX_OUT).o: src/$(LEX_OUT).c
	$(CC) -O -c $< $(CC_FLAGS) $(LEX_DEFINES) -o $@

src/$(YACC_OUT).c: src/$(NAME).y
	$(YACC) $(YACC_FLAGS) $<

src/static/opcodes.c: src/static/opcodes.csv
	./utils/opcodes.py -i $< > $@

%.o: %.c
	$(CC) -O -c $< $(CC_FLAGS) $(CC_LIB_FLAGS) $(SCRIPTING) -o $@

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

src/init.c: src/static/init.h

src/static/init.h: ${src/static/init.txt:txt=h}

src/pseudo/font.c: src/static/font.h

src/static/font.h: ${src/static/font.chr:chr=h}

src/static/font.chr: src/static/font.png
	./utils/img2chr.py -i $<

src/usage.c: src/static/license.h

src/static/license.h: ${src/static/licence.txt:txt=h}

src/static/scripts.tar.gz:
	cd src/static && \
		tar -czf scripts.tar.gz $(shell cd src/static && ls scripts/*)

src/static/scripts.h: ${src/static/scripts.tar.gz:tar.gz=h}

src/scripts.c: src/static/scripts.tar.gz src/static/scripts.h

$(EXEC): $(OBJS) $(HDRS)
	$(CC) -o $(EXEC) $(OBJS) $(CC_FLAGS) $(CC_FILES) $(CC_LIB_FLAGS) \
		$(SCRIPTING) \
		$(SCRIPTING_LUA) \
		$(SCRIPTING_SCM)

# TESTING

test: all
	@python test.py

test-js:
	@printf "Building JS...\n"
	@$(MAKE) js >/dev/null 2>/dev/null || :
	@$(MAKE) docs-js
	@printf "Building tests...\n"
	@python utils/jstest.py --input ./test/errors \
		--output ./test/js/errors.js
	@python utils/jstest.py --input ./test/examples \
		--output ./test/js/examples.js \
		--exclude custom,ease,incbin,include,incpal,incpng,incrle,incwav,macro,trainer
	@python utils/jstest.py --input ./test/opcodes \
		--output ./test/js/opcodes.js \
		--exclude undocumented
	@printf "Open: file://%s\n" $(shell pwd)/test/js/test.html

splint:
	splint -I/usr/include -I/usr/include/x86_64-linux-gnu \
		   -warnposix $(FLAGS) $(FILES)

# LIBLUA

src/scripting/lua.c: liblua

liblua: src/third-party/lua-5.1.5/src/liblua.a

src/third-party/lua-5.1.5/src/liblua.a:
	$(MAKE) -C src/third-party/lua-5.1.5/src/ all \
		CC="$(CC)" AR="$(AR) rcu" RANLIB="$(RANLIB)" \
		MYCFLAGS="$(CC_LIBLUA)"

# LIBTINYSCHEME

src/scripting/scm.c: libtinyscheme

libtinyscheme: src/third-party/tinyscheme-1.41/libtinyscheme.a

src/third-party/tinyscheme-1.41/libtinyscheme.a:
	$(MAKE) -C src/third-party/tinyscheme-1.41/ libtinyscheme.a \
		CC="$(CC) -fpic -pedantic -Wno-switch" AR="$(AR) crs" \
		PLATFORM_FEATURES="$(SCHEME_FLAGS)"

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
server: docs-js docs-css website-js website-css
	@sed -e "s/\$${DOCUMENTATION}/http:\/\/localhost:8000\/documentation/g" \
		-e "s/\$${REGISTRY}/http:\/\/localhost:8000\/registry/g" \
		-e "s/\$${WEBSITE}/http:\/\/localhost:8000/g" \
		-e "s/\$${ANALYTICS_ID}/UA-103019505-2/g" \
		-e "s/\$${ANALYTICS_DOMAIN}/none/g" \
	 	docs/mkdocs-template.yml > docs/mkdocs.yml
	@cd docs ; mkdocs build --clean
	@sed -e "s/\$${DOCUMENTATION}/http:\/\/localhost:8000\/documentation/g" \
		-e "s/\$${ANALYTICS_ID}/UA-103019505-2/g" \
		-e "s/\$${ANALYTICS_DOMAIN}/none/g" \
		website/settings-template.cfg > website/settings.cfg
	@printf "Starting server...\n"
	@python server.py --debug --port 8000

# WEBSITE

.PHONY: website-clean
website-clean:
	$(RM) website/static/js/website.js

.PHONY: website-css
website-css:
	@printf "Minifying CSS...\n"
	@uglifycss --output website/static/css/website.css \
		website/static/css/bootstrap.min.css \
		website/static/css/font-awesome.min.css \
		website/static/css/grayscale.css \
		website/static/css/asciinema-player.css \
		website/static/css/style.css

.PHONY: website-js
website-js:
	@printf "Minifying JS...\n"
	@uglifyjs --output website/static/js/website.js \
		website/static/js/jquery.min.js \
		website/static/js/jquery.easing.min.js \
		website/static/js/bootstrap.min.js \
		website/static/js/grayscale.js \
		website/static/js/asciinema-player.js \
		website/static/js/dynamicaudio-min.js \
		website/static/js/jsnes.js

.PHONY: website
website: website-js website-css
	@sed -e "s/\$${DOCUMENTATION}/http:\/\/localhost:9000\/documentation/g" \
		-e "s/\$${ANALYTICS_ID}/UA-103019505-2/g" \
		-e "s/\$${ANALYTICS_DOMAIN}/none/g" \
		website/settings-template.cfg > website/settings.cfg
	@cd website ; python index.py --debug --port 9000

# REGISTRY

.PHONY: registry
registry:
	@cd registry ; python index.py --debug --port 8000 --import registry.sql

# DOCUMENTATION

.PHONY: docs-clean
docs-clean:
	$(RM) docs/js/models/assembler.js
	$(RM) docs/pages/js/$(EXEC)*.js
	$(RM) docs/pages/js/assembler.js
	$(RM) docs/pages/js/docs.js
	$(RM) docs/pages/js/registry.js

.PHONY: docs-js
docs-js:
	@printf "Copying JS...\n"
	@cp $(EXEC).min.js docs/pages/js 2>/dev/null || \
		cp $(EXEC).js docs/pages/js/$(EXEC).min.js 2>/dev/null || \
		touch docs/pages/js/$(EXEC).min.js 2>/dev/null || :
	@printf "Transpiling JS...\n"
	@cd docs && yarn run build
	@printf "Minifying JS...\n"
	@uglifyjs --output docs/pages/js/docs.js \
		docs/pages/js/bundle.js

.PHONY: docs-css
docs-css:
	@printf "Minifying CSS...\n"
	@uglifycss --output docs/pages/css/docs.css \
		docs/pages/css/custom.css \
		docs/pages/css/assembler.css \
		docs/pages/css/registry.css \
		docs/pages/css/bootstrap-modal.css

.PHONY: docs
docs: docs-js docs-css
	@sed -e "s/\$${DOCUMENTATION}/http:\/\/localhost:9090/g" \
		-e "s/\$${REGISTRY}/http:\/\/localhost:9090\/registry/g" \
		-e "s/\$${WEBSITE}/http:\/\/localhost:9090\/website/g" \
		-e "s/\$${ANALYTICS_ID}/UA-103019505-2/g" \
		-e "s/\$${ANALYTICS_DOMAIN}/none/g" \
	 	docs/mkdocs-template.yml > docs/mkdocs.yml
	@cd docs ; mkdocs build --clean
	@printf "Starting server...\n"
	@cd docs; python index.py --debug --port 9090

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
	cp $(NAME)-completion.bash \
		$(PAYLOAD)/usr/share/bash-completion/completions/$(NAME)
	mkdir -p $(PAYLOAD)/DEBIAN
	sed -e "s/\$${NAME}/$(NAME)/g" \
		-e "s/\$${VERSION}/$(VERSION)/g" \
		-e "s/\$${ARCHITECTURE}/$(ARCHITECTURE)/g" \
		-e "s/\$${MAINTAINER}/$(MAINTAINER)/g" \
		-e "s/\$${EMAIL}/$(EMAIL)/g" \
		-e "s/\$${SIZE}/$(shell wc -c < $(EXEC))/g" \
		-e "s/\$${DESCRIPTION}/$(DESCRIPTION)/g" \
	 	$(PACKAGE)/data/linux/control > $(PAYLOAD)/DEBIAN/control
	sed -e "s/\$${NAME}/$(NAME)/g" \
		-e "s/\$${VERSION}/$(VERSION)/g" \
		-e "s/\$${YEAR}/$(YEAR)/g" \
		-e "s/\$${MAINTAINER}/$(MAINTAINER)/g" \
		$(PACKAGE)/data/linux/copyright > $(PAYLOAD)/usr/share/doc/$(NAME)/copyright
	cat src/static/license.txt >> $(PAYLOAD)/usr/share/doc/$(NAME)/copyright
	cd $(PAYLOAD) ; md5sum `find usr -type f` > DEBIAN/md5sums
	cp $(PACKAGE)/scripts/linux/* $(PAYLOAD)/DEBIAN
	dpkg-deb --build $(PAYLOAD) \
		$(RELEASE)/$(NAME)_$(VERSION)_$(ARCHITECTURE).deb
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
	 	$(PACKAGE)/data/osx/distribution.xml > $(TMP)
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
	 	$(PACKAGE)/data/win/msi.wxs > $(TMP)
	wixl $(TMP) --output $(RELEASE)/$(NAME)_$(VERSION)_$(ARCHITECTURE).msi
	$(RM) $(TMP) $(PAYLOAD)

.PHONY: js_package
js_package:
	$(MAKE) js
	$(RM) $(PAYLOAD)
	$(RM) $(RELEASE)/$(NAME)-js
	mkdir -p $(RELEASE)/$(NAME)-js
	mkdir -p $(PAYLOAD)/lib
	sed -e "s/\$${NAME}/$(NAME)/g" \
		-e "s/\$${VERSION}/$(VERSION)/g" \
		-e "s/\$${DESCRIPTION}/$(DESCRIPTION)/g" \
		-e "s/\$${IDENTIFIER}/$(IDENTIFIER)/g" \
		-e "s/\$${MAINTAINER}/$(MAINTAINER)/g" \
		-e "s/\$${EMAIL}/$(EMAIL)/g" \
	$(PACKAGE)/data/js/package.json > $(PAYLOAD)/package.json
	cp $(PACKAGE)/data/js/index.js $(PAYLOAD)
	cp COPYING $(PAYLOAD)/LICENSE.txt
	cp $(NAME).js $(PAYLOAD)/lib
	mv $(PAYLOAD)/* $(RELEASE)/$(NAME)-js

# DEPLOY
.PHONY: deploy
deploy: docs-js docs-css website-js website-css
	@printf "Prepping docs...\n"
	@sed -e "s/\$${DOCUMENTATION}/http:\/\/docs.nessemble.com/g" \
		-e "s/\$${REGISTRY}/http:\/\/registry.nessemble.com/g" \
		-e "s/\$${WEBSITE}/http:\/\/nessemble.com/g" \
		-e "s/\$${ANALYTICS_ID}/UA-103019505-1/g" \
		-e "s/\$${ANALYTICS_DOMAIN}/nessemble.com/g" \
	 	docs/mkdocs-template.yml > docs/mkdocs.yml
	@cd docs ; mkdocs build --clean

	@printf "Prepping website...\n"
	@sed -e "s/\$${DOCUMENTATION}/http:\/\/docs.nessemble.com/g" \
		-e "s/\$${ANALYTICS_ID}/UA-103019505-1/g" \
		-e "s/\$${ANALYTICS_DOMAIN}/nessemble.com/g" \
		website/settings-template.cfg > website/settings.cfg

	@printf "Gathering deploy files...\n"
	@$(RM) deploy-files.txt
	@touch deploy-files.txt
	@for file in $(DEPLOY_FILES); do \
		printf "./%s\n" $$file >> deploy-files.txt; \
	done

	@printf "Zipping files...\n"
	@tar czf deploy.tar.gz --files-from deploy-files.txt

	@python utils/deploy.py --tar deploy.tar.gz --maintenance $(MAINTENANCE)

# CLEAN

.PHONY: clean
clean:
	$(RM) $(EXEC) $(EXEC).exe $(EXEC).js $(EXEC).min.js $(EXEC).wasm
	$(RM) $(OBJS)
	$(RM) deploy-files.txt deploy.tar.gz
	$(RM) src/$(YACC_OUT).c src/$(YACC_OUT).h src/$(LEX_OUT).c
	$(RM) src/static/font.h src/static/font.chr
	$(RM) src/static/init.h
	$(RM) src/static/license.h
	$(RM) src/static/opcodes.c
	$(RM) src/static/scripts.h src/static/scripts.tar.gz
	$(RM) $(PAYLOAD)
	$(MAKE) -C src/third-party/tinyscheme-1.41/ clean
	$(MAKE) -C src/third-party/lua-5.1.5/src/ clean
	$(MAKE) docs-clean
	$(MAKE) website-clean
