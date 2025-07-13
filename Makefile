include flags.mk

src/lexer.o: src/position.o src/list.o src/arena.o
src/parser.o: src/lexer.o src/position.o src/list.o src/arena.o src/node.o
src/list.o: src/arena.o
src/node.o: src/arena.o
src/evaluate.o: src/arena.o

tests/lexer.test: src/lexer.o src/position.o src/list.o src/arena.o
tests/parser.test: src/parser.o src/lexer.o src/position.o src/list.o src/node.o src/arena.o
tests/list.test: src/list.o src/arena.o
tests/arena.test: src/arena.o
tests/evaluate.test: src/evaluate.o src/node.o src/list.o src/arena.o src/position.o

LDFLAGS := -lreadline
main: src/lexer.o src/parser.o src/list.o src/evaluate.o src/node.o src/arena.o

.PHONY: clean
clean:
	rm -rf **/*.o **/*.dSYM
	rm -f tests/*.test

.PHONY: test
test: tests/lexer.test tests/parser.test tests/list.test tests/arena.test tests/evaluate.test
	tests/lexer.test
	tests/parser.test
	tests/list.test
	tests/arena.test
	tests/evaluate.test
