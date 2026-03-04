TARGET := main

# Find all source files in the current directory
SRCS := $(wildcard *.c)

# Generate object file names with .o extension from source files
OBJS := $(SRCS:.c=.o)

# The default target is 'all', which builds the program
all: $(TARGET)

# Rule to link object files into the final executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LIBS)

# Rule to compile .c files into .o files
# Uses automatic variables:
# $<: the name of the prerequisite (the .c file)
# $@: the name of the target (the .o file)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# A phony target is a target that is not a file name
.PHONY: all clean

# Rule to clean up generated files
clean:
	rm -f $(TARGET) $(OBJS)

# Target to run the compiled program
run: $(TARGET)
	# Always start fresh by cleaning before running
	make clean
	# Build the program
	make all
	# Run the program
	./$(TARGET)