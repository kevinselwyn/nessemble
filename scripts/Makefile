CC       := gcc
CC_FLAGS := -shared -fPIC -lm

all: build

build: ease.so

ease.so: ease.c
	$(CC) -o $@ $< $(CC_FLAGS)
