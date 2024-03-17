# Makefile for a C++ project with Makefile and include files in the src directory

# Compiler settings
CXX = g++
CXXFLAGS = -Wall -Wextra -O3 -march=native -flto -I.

# Define the executable file 
TARGET = ./Maello

# Source files
SRCS = $(wildcard *.cpp)

# Object files
OBJS = $(SRCS:.cpp=.o)

# Dependencies
DEPS = $(OBJS:.o=.d)

# Build the application
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	rm -f $(OBJS) $(DEPS)

# Include dependencies
-include $(DEPS)

# Rule to generate a dep file by using the C preprocessor
# (see man gcc for details on the -MM and -MT options)
%.d: %.cpp
	$(CXX) $(CXXFLAGS) -MM -MT $(@:.d=.o) $< -MF $@

# Rule to compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(TARGET) $(OBJS) $(DEPS)

# Phony targets
.PHONY: clean
