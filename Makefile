CXX = g++
CXXFLAGS = -std=c++11

SRCS = main.cpp items.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = POETrack

.PHONY: all clean generate

all: generate $(TARGET)

generate: items.h

items.h: items.json
	@xxd -i items.json > items.h

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET) items.h
