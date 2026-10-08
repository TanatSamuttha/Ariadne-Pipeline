SRC := ./src
BENCH := ./benchmark
LANGGRAPHBENCH := $(BENCH)/langgraph

compiletest:
	g++ test/test.cpp -o test -O0 -I$(SRC)

compilelgbench:
	g++ $(LANGGRAPHBENCH)/benchmark.cpp -o benchmark -O3 -I$(SRC) -s