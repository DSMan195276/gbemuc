
TESTS :=

CFLAGS += -I'./test' -I'./gbemuc'

# This defines the names of all the tests we should run
TESTS += cpu

# Definitions of the tests listed above
# Each test is defined by a list of files that make it up
#
# In general, there is a corespondence between one .c file in ./src and one .c
# file in ./test. The ./test file contains the definition of main() as well as
# the test functions to run (And this is also compiled in with ./test/test.c,
# which contains various functions for testing accessable via ./test/test.h)
#
# More complex tests may require the use of more then just one .c file from
# ./src, in which case all of them should be listed. Also possible is having
# more the one test program per ./src file.
cpu.OBJ := ./gbemuc/gb/cpu.o \
			./gbemuc/gb/mmu.o \
			./test/cpu.o

TEST_OBJS := $(foreach test,$(TESTS),$(filter ./test/%,$($(test).OBJ)))

# This template generates a list of the outputted test executables, as well as
# rules for compiling them.
define TEST_template
TEST_TESTS += ./test/bin/$(1)_test
./test/bin/$(1)_test: ./test/test.o $$($(1).OBJ) $$(SRC_OBJS) | ./test/bin
	@$$(call mecho," CCLD    test/bin/$(1)_test","$$(CC) $$(LDFLAGS) ./test/test.o -o $$@ $$($(1).OBJ) $$(SRC_OBJS)")
	$$(Q)$$(CC) $$(LDFLAGS) $$(GBEMUC_CFLAGS) ./test/test.o -o $$@ $$($(1).OBJ) $$(SRC_OBJS) $$(CBOY_LIBFLAGS)
endef

# Run the template over all of our tests
$(foreach test,$(TESTS),$(eval $(call TEST_template,$(test))))

test/bin:
	@$(call mecho," MKDIR   $@","mkdir ./test/bin")
	$(Q)mkdir ./test/bin

DEPS += $(TEST_OBJS)
CLEAN_LIST += $(TEST_OBJS)
CLEAN_LIST += $(TEST_TESTS) ./test/test.o ./test/bin

run_tests: ./test/test.o $(TEST_TESTS)
	$(Q)./test/run_tests.sh $(TESTS)

PHONY += check
check: run_tests

