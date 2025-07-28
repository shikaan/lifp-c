include flags.mk

.PHONY: all
all: clean bin/repl

src/lexer.o: src/position.o src/list.o src/arena.o
src/parser.o: src/lexer.o src/position.o src/list.o src/arena.o src/node.o
src/list.o: src/arena.o
src/node.o: src/arena.o
src/environment.o: src/arena.o src/map.o
src/evaluate.o: src/arena.o src/environment.o src/map.o
src/map.o: src/arena.o

tests/lexer.test: src/lexer.o src/position.o src/list.o src/arena.o
tests/parser.test: src/parser.o src/lexer.o src/position.o src/list.o src/node.o src/arena.o
tests/list.test: src/list.o src/arena.o
tests/arena.test: src/arena.o
tests/evaluate.test: src/evaluate.o src/node.o src/list.o src/arena.o src/position.o src/environment.o src/map.o
tests/map.test: src/arena.o src/map.o
tests/fmt.test: src/fmt.o src/node.o src/arena.o src/list.o

tests/integration.test: src/lexer.o src/parser.o src/arena.o src/evaluate.o src/list.o src/map.o src/node.o src/environment.o

LDFLAGS := -lreadline
bin/repl: src/lexer.o src/parser.o src/list.o src/evaluate.o src/node.o src/arena.o src/environment.o src/map.o src/fmt.o

.PHONY: run
run: bin/repl
	bin/repl

.PHONY: clean
clean:
	rm -rf **/*.o **/*.dSYM main *.dSYM
	rm -f tests/*.test

.PHONY: test
test: tests/lexer.test tests/parser.test tests/list.test tests/arena.test tests/evaluate.test tests/map.test tests/integration.test tests/fmt.test
	tests/lexer.test
	tests/parser.test
	tests/list.test
	tests/arena.test
	tests/evaluate.test
	tests/map.test
	tests/fmt.test
	tests/integration.test
