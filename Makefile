# Output binary name
bin=inspector

# Set the following to '0' to disable log messages:
DEBUG ?= 1

# Compiler/linker flags
CFLAGS += -g -Wall -lm -lncurses -fPIC -DDEBUG=$(DEBUG)
LDFLAGS +=

# Source C files
src=inspector.c procfs.c display.c util.c
obj=$(src:.c=.o)

# Makefile recipes --
all: $(bin) libinspector.so

$(bin): $(obj)
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -o $@

libinspector.so: $(obj)
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -shared -o $@

docs: Doxyfile
	doxygen

clean:
	rm -f $(bin) $(obj) libinspector.so
	rm -rf docs

# Individual dependencies --
inspector.o: inspector.c debug.h
procfs.o: procfs.c procfs.h
display.o: display.c display.h procfs.h util.h
util.o: util.c util.h


# Tests --
test: inspector  libinspector.so ./tests/run_tests
	./tests/run_tests $(run)

testupdate: testclean test

./tests/run_tests:
	rm -rf tests
	git clone https://github.com/USF-OS/P1-Tests.git tests

testclean:
	rm -rf tests
