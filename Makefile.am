ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS=-fsanitize=address
AM_CFLAGS+=-D DEBUG
AM_CFLAGS+=-D WARNING
AM_CFLAGS+=-D BIG_ENDIAN_SIMULATOR
# Uncomment if performance when running with -DDEBUG is an issue
# Warning, if uncommented, issuing calls to debug functions during options
# parsing might result in debug flag incorrectly set to 0 for some files
#AM_CFLAGS+=-D CACHE_DEBUG_FLAG

LDADD=-lpthread

if HAVE_ARM_COMPILER
SUBDIRS=. Examples
endif

bin_PROGRAMS=arm_simulator send_irq memory_test registers_test test_load_store instruction_test test_shifts test_shifts_registers test_immediate

COMMON=csapp.h csapp.c scanner.h scanner.l debug.h debug.c \
       gdb_protocol.h gdb_protocol.c util.h util.c trace.h trace.c \
       memory.h memory.c trace_location.h no_trace_location.h \
       registers.h registers.c \
       arm.h arm.c \
       arm_constants.h arm_constants.c \
       arm_core.h arm_core.c \
       arm_exception.h arm_exception.c \
       arm_instruction.h arm_instruction.c \
       arm_data_processing.h arm_data_processing.c \
       arm_load_store.h arm_load_store.c \
       arm_branch_other.h arm_branch_other.c 

arm_simulator_SOURCES=$(COMMON) arm_simulator.c
test_branch_SOURCES=$(COMMON) test_branch.c

send_irq_SOURCES=send_irq.c csapp.h csapp.c arm_constants.h arm_constants.c

memory_test_SOURCES=memory_test.c memory.h memory.c util.h util.c
registers_test_SOURCES=registers_test.c registers.h registers.c util.h util.c arm_constants.h arm_constants.c
test_load_store_SOURCES=$(COMMON) test_load_store.c
instruction_test_SOURCES=instruction_test.c arm_instruction.c util.c util.c arm_constants.h arm_constants.c
test_shifts_SOURCES=$(COMMON) test_shifts.c
test_shifts_registers_SOURCES=$(COMMON) test_shifts_registers.c
test_immediate_SOURCES=$(COMMON) test_immediate.c

EXTRA_DIST=gdb_commands make_trace.sh License
