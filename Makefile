CC      = /opt/ohpc/pub/mpi/openmpi3-gnu7/3.1.0/bin/mpicc
CCLINK  = /opt/ohpc/pub/mpi/openmpi3-gnu7/3.1.0/bin/mpicc
SHELL   = /bin/sh
EXEC    = main  # Set your program name here

# List of source files
SRCS    = $(wildcard *.c)

# List of object files
OBJS    = $(SRCS:.c=.o)

# Main target
$(EXEC): $(OBJS)
	$(CC) -o $(EXEC) $(OBJS)

# Rule for compiling C source files
%.o: %.c
	$(CC) -c $< -o $@

# Clean target
clean:
	/bin/rm -f $(EXEC) $(OBJS) $(EXEC)*.s

