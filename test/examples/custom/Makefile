NAME     := quotient
CC       := gcc
CC_FLAGS := -shared -fPIC

win32: CC := i686-w64-mingw32-gcc

# PLATFORMS

all: scripts/$(NAME).so

win32: scripts/$(NAME).dll

# RECIPES

scripts/$(NAME).so: scripts/$(NAME).c
	$(CC) -o $@ $< $(CC_FLAGS)

scripts/$(NAME).dll: scripts/$(NAME).c
	$(CC) -o $@ $< $(CC_FLAGS)

clean:
	rm -f scripts/$(NAME).so scripts/$(NAME).dll
