# Define the compiler
CC = gcc

# Compiler flags:
# -Wall: Enable all warnings
# -std=c99: Use C99 standard
# -D_POSIX_C_SOURCE=199309L: Enable POSIX source features from 1993, including clock_gettime
CFLAGS = -Wall -std=c99

# Linker flags:
# Link against the real-time library for client2
LDFLAGS_client1 =
LDFLAGS_client2 = -lrt

# Define the names of the output binaries
BIN1 = client1
BIN2 = client2

# Default target
all: $(BIN1) $(BIN2)

# Build client1
$(BIN1): client1.c
	$(CC) $(CFLAGS) $(LDFLAGS_client1) -o $(BIN1) client1.c

# Build client2
$(BIN2): client2.c
	$(CC) $(CFLAGS) -D_POSIX_C_SOURCE=199309L $(LDFLAGS_client2) -o $(BIN2) client2.c

# Clean up binaries
clean:
	rm -f $(BIN1) $(BIN2)

# To handle file names same as Makefile targets
.PHONY: all clean
