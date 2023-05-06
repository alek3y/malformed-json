CXX = g++
CXXFLAGS = -g -std=c++14 -pedantic -Wall -Wextra -Wshadow -Wfatal-errors
CXXFLAGS += -I include

SRC = src/json.cpp tests/main.cpp
OBJ = $(SRC:%.cpp=%.o)

TEST = tests/main

.PHONY = all sanitized clean

sanitized: CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
all sanitized: $(OBJ) $(TEST)

# Here $^ means all the prerequisites and $@ the target
$(TEST): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Here $< means the first prerequisite
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-rm $(TEST) $(OBJ)
