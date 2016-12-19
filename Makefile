NAME       := nessemble
CC         := gcc
CC_FLAGS   := -ll -lm -lpng
LEX        := flex
LEX_FLAGS  :=
LEX_OUT    := lex.yy.c
YACC       := bison
YACC_FLAGS := --defines --yacc
YACC_OUT   := y.tab
OPCODES    := opcodes
PNG        := png
TEST       := test
UNAME      := $(shell uname -s)

ifeq ($(UNAME), Darwin)
	CC_FLAGS += -I/usr/local/include -L/usr/local/lib
endif

all: $(NAME)

$(LEX_OUT): $(NAME).l
	$(LEX) $<

$(YACC_OUT).c: $(NAME).y
	$(YACC) $(YACC_FLAGS) $<

$(OPCODES).h: $(OPCODES).csv
	./$(OPCODES).sh $< $@

$(NAME): $(LEX_OUT) $(YACC_OUT).c $(PNG).c $(NAME).h $(OPCODES).h
	$(CC) -o $@ $(LEX_OUT) $(YACC_OUT).c $(PNG).c $(CC_FLAGS)

$(TEST): all
	@./$(TEST).sh

clean:
	rm -f $(NAME) $(YACC_OUT).c $(YACC_OUT).h $(LEX_OUT) $(OPCODES).h
