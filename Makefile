include flags.mk

src/lexer.o: src/position.o src/list.o
src/parser.o: src/lexer.o src/position.o src/list.o

tests/lexer.test: src/lexer.o src/position.o src/list.o
tests/parser.test: src/parser.o src/lexer.o src/position.o src/list.o
tests/stack.test: src/stack.o

LDFLAGS := -lreadline
main: src/lexer.o src/parser.o src/list.o

.PHONY: clean
clean:
	rm -rf **/*.o **/*.dSYM
	rm -f tests/*.test

.PHONY: test
test: tests/lexer.test tests/parser.test tests/stack.test
	tests/lexer.test
	tests/parser.test
	tests/stack.test
