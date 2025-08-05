include flags.mk

.PHONY: all
all: clean bin/repl

linenoise.o: CFLAGS = -Wall -W -Os
linenoise.o: vendor/linenoise/linenoise.c
	$(CC) $(CFLAGS) -c $< -o $@

lib/list.o: lib/arena.o
lib/map.o: lib/arena.o

lifp/tokenize.o: lib/list.o lib/arena.o
lifp/parser.o: lifp/tokenize.o lib/list.o lib/arena.o lifp/node.o
lifp/node.o: lib/arena.o
lifp/environment.o: lib/arena.o lib/map.o
lifp/evaluate.o: lib/arena.o lifp/environment.o lib/map.o

tests/tokenize.test: lifp/tokenize.o lib/list.o lib/arena.o
tests/parser.test: lifp/parser.o lifp/tokenize.o lib/list.o lifp/node.o lib/arena.o
tests/list.test: lib/list.o lib/arena.o
tests/arena.test: lib/arena.o
tests/evaluate.test: lifp/evaluate.o lifp/node.o lib/list.o lib/arena.o lifp/environment.o lib/map.o
tests/map.test: lib/arena.o lib/map.o
tests/fmt.test: lifp/fmt.o lifp/node.o lib/arena.o lib/list.o

tests/integration.test: lifp/tokenize.o lifp/parser.o lib/arena.o lifp/evaluate.o lib/list.o lib/map.o lifp/node.o lifp/environment.o

bin/repl: lifp/tokenize.o lifp/parser.o lib/list.o lifp/evaluate.o lifp/node.o lib/arena.o lifp/environment.o lib/map.o lifp/fmt.o linenoise.o
bin/run: lifp/tokenize.o lifp/parser.o lib/list.o lifp/evaluate.o lifp/node.o lib/arena.o lifp/environment.o lib/map.o lifp/fmt.o


.PHONY: clean
clean:
	rm -rf **/*.o **/*.dSYM main *.dSYM *.plist
	rm -f tests/*.test

.PHONY: lifp-test
lifp-test: tests/tokenize.test tests/parser.test tests/evaluate.test tests/integration.test tests/fmt.test
	tests/tokenize.test
	tests/parser.test
	tests/evaluate.test
	tests/fmt.test
	tests/integration.test

.PHONY: lib-test
lib-test: tests/arena.test tests/list.test tests/map.test
	tests/arena.test
	tests/list.test
	tests/map.test

.PHONY: test
test: lifp-test lib-test
