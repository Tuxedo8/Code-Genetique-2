CC=clang
MEMCHECK_FLAGS=-fsanitize=address -fno-omit-frame-pointer
ifeq ($(shell [ -n "$$DEBUG" ] && [ "$$DEBUG" -ne 0 ] && echo HAS_DEBUG),HAS_DEBUG)
DEBUG_FLAGS=-DVERBOSE -DDEBUG
endif
BASE_CFLAGS=-g -Wall -Werror -Wno-unused -include my_memory.h
CFLAGS+=$(BASE_CFLAGS) $(MEMCHECK_FLAGS) -DTRACK_ALLOCS
LDFLAGS=-g $(MEMCHECK_FLAGS)

TEST_Q1_FLAGS=
TEST_Q2_FLAGS=-DDYNAMIC_ALLOCATIONS -DIN_PLACE -DCODES_PER_BYTE=3
TEST_HEADERS=parametrized_output.h my_memory.h code.h
PROGRAMS=test_Q1 test_Q2

.PHONY: clean

all:
	@echo "Tapez 'make program' où programme est parmi : $(PROGRAMS)"

$(PROGRAMS): test_%: test_%.o code.o parametrized_output.o my_memory.o

code.o: code.c $(TEST_HEADERS)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c -o $@ $<
test_Q%.o: test.c $(TEST_HEADERS)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(TEST_Q$*_FLAGS) -c -o $@ $<

clean:
	-rm *.o $(PROGRAMS)
