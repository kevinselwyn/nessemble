CC       := gcc
CC_FLAGS := -shared -fPIC

all: build

build: scripts/quotient.so

scripts/quotient.so: scripts/quotient.c
	$(CC) -g -o $@ $< $(CC_FLAGS)
