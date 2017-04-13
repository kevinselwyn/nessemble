NAME     := quotient
CC       := gcc
CC_FLAGS := -shared -fPIC

all: scripts/$(NAME).so

scripts/$(NAME).so: scripts/$(NAME).c
	$(CC) -o $@ $< $(CC_FLAGS)

clean:
	rm -f scripts/$(NAME).so
