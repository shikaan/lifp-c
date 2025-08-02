include flags.mk

.PHONY: all
all: clean bin/repl

lib/list.o: lib/arena.o
lib/map.o: lib/arena.o

lifp/lexer.o: lifp/position.o lib/list.o lib/arena.o
lifp/parser.o: lifp/lexer.o lifp/position.o lib/list.o lib/arena.o lifp/node.o
lifp/node.o: lib/arena.o
lifp/environment.o: lib/arena.o lib/map.o
lifp/evaluate.o: lib/arena.o lifp/environment.o lib/map.o

tests/lexer.test: lifp/lexer.o lifp/position.o lib/list.o lib/arena.o
tests/parser.test: lifp/parser.o lifp/lexer.o lifp/position.o lib/list.o lifp/node.o lib/arena.o
tests/list.test: lib/list.o lib/arena.o
tests/arena.test: lib/arena.o
tests/evaluate.test: lifp/evaluate.o lifp/node.o lib/list.o lib/arena.o lifp/position.o lifp/environment.o lib/map.o
tests/map.test: lib/arena.o lib/map.o
tests/fmt.test: lifp/fmt.o lifp/node.o lib/arena.o lib/list.o

tests/integration.test: lifp/lexer.o lifp/parser.o lib/arena.o lifp/evaluate.o lib/list.o lib/map.o lifp/node.o lifp/environment.o

LDFLAGS := -lreadline
bin/repl: lifp/lexer.o lifp/parser.o lib/list.o lifp/evaluate.o lifp/node.o lib/arena.o lifp/environment.o lib/map.o lifp/fmt.o

bin/test: lib/arena.o lifp/environment.o lib/map.o lib/list.o

.PHONY: run
run: bin/repl
	bin/repl

.PHONY: clean
clean:
	rm -rf **/*.o **/*.dSYM main *.dSYM *.plist
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
