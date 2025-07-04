include flags.mk

build/%: %.c
	mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LDLIBS) -o $@

.PHONY: clean
clean:
	rm -rf build/*