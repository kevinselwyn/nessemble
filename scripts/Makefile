NAME     := ease
CC       := gcc
CC_FLAGS := -shared -fPIC -lm

all: $(NAME).so

$(NAME).so: $(NAME).c
	$(CC) -o $@ $< $(CC_FLAGS)

clean:
	rm -f $(NAME).so
