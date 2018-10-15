CXXFLAGS= -std=c++11 -g -Wall

RUNNER_OBJS=src/runner.o src/bytestream.o src/value.o src/gamedata.o
RUNNER=./runner

all: $(RUNNER)

$(RUNNER): $(RUNNER_OBJS)
	$(CXX) $(RUNNER_OBJS) -o $(RUNNER)


clean:
	$(RM) src/*.o $(RUNNER)

.PHONY: all clean