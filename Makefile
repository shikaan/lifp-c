include flags.mk

# build/%: %.c
# 	mkdir -p build
# 	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LDLIBS) -o $@

src/lexer.o: src/position.o
tests/lexer.test: src/lexer.o src/position.o

.PHONY: clean
clean:
	rm -rf **/*.o **/*.dSYM
	rm -f tests/lexer.test

.PHONY: test
test: tests/lexer.test
	./tests/lexer.test
