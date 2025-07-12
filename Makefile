include flags.mk

src/lexer.o: src/position.o src/list.o
src/parser.o: src/lexer.o src/position.o src/list.o

tests/lexer.test: src/lexer.o src/position.o src/list.o
tests/parser.test: src/parser.o src/lexer.o src/position.o src/list.o src/node.o
tests/list.test: src/list.o

LDFLAGS := -lreadline
main: src/lexer.o src/parser.o src/list.o src/evaluate.o src/node.o

.PHONY: clean
clean:
	rm -rf **/*.o **/*.dSYM
	rm -f tests/*.test

.PHONY: test
test: tests/lexer.test tests/parser.test tests/list.test
	tests/lexer.test
	tests/parser.test
	tests/list.test
