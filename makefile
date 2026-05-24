CC = gcc
LEX = flex
YACC = bison -d

OBJS = cpq.o parser.tab.o lex.yy.o codegen.o quad.o ast.o tables.o utils.o
TARGET = cpq.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS)

parser.tab.c parser.tab.h: parser.y
	$(YACC) parser.y

lex.yy.c: lexer.l
	$(LEX) lexer.l

%.o: %.c
	$(CC) -c $<

clean:
	rm -f *.o *.out lex.yy.c parser.tab.c parser.tab.h $(TARGET)

run: $(TARGET)
	./$(TARGET) test_input.ou