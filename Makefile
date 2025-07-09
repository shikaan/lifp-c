include flags.mk

src/token.o: src/position.o
src/lexer.o: src/token.o src/position.o
src/parser.o: src/lexer.o src/position.o

tests/lexer.test: src/lexer.o src/token.o src/position.o
tests/token.test: src/token.o src/position.o
tests/parser.test: src/token.o src/parser.o src/lexer.o src/position.o

.PHONY: clean
clean:
	rm -rf **/*.o **/*.dSYM
	rm -f tests/lexer.test

.PHONY: test
test: tests/lexer.test tests/token.test tests/parser.test
	tests/lexer.test
	tests/token.test
	tests/parser.test
