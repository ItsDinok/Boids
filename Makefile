# Makefile for SDL2 C++ grid project on MSYS2/MinGW64

# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall

# SDL paths (adjust if necessary)
SDL_INCLUDE = -IC:/msys64/mingw64/include/SDL2
SDL_LIB = -LC:/msys64/mingw64/lib -lmingw32 -lSDL2main -lSDL2

# Target executable
TARGET = grid_sim

# Source files
SRC = main.cpp

# Build rule
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(SDL_INCLUDE) $(SDL_LIB)

# Clean rule
clean:
	rm -f $(TARGET).exe
