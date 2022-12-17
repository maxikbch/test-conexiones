# Libraries
LIBS=commons shared pthread

# Custom libraries' paths
SHARED_LIBPATHS=../shared
STATIC_LIBPATHS=

# Compiler flags
CDEBUG=-g -w -Wno-unused-function -Wno-switch -Wno-trigraphs -DDEBUG
CRELEASE=-O3 -w -Wno-unused-function -Wno-switch -Wno-trigraphs -DNDEBUG

# Arguments when executing with start, memcheck or helgrind
ARGS=configsMemory/tlb.config 0.0.0.0
#./bin/memory.out configsMemory/pmemoria.config 0.0.0.0


# Valgrind flags
MEMCHECK_FLAGS=--track-origins=yes --log-file="memcheck.log"
HELGRIND_FLAGS=--log-file="helgrind.log"
