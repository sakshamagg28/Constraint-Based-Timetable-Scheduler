.PHONY: all build run test clean

CXX ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
BUILD_DIR := build
INCLUDES := -Iinclude
HEADERS := include/ConflictGraph.h include/CsvReader.h include/Domain.h include/OutputWriter.h include/QualityScorer.h include/Schedule.h include/Scheduler.h include/Validator.h
CORE_SRC := src/Domain.cpp src/Schedule.cpp src/CsvReader.cpp src/ConflictGraph.cpp src/Validator.cpp src/QualityScorer.cpp src/Scheduler.cpp src/OutputWriter.cpp

all: build

build: $(BUILD_DIR)/scheduler $(BUILD_DIR)/scheduler_tests

$(BUILD_DIR)/.dir:
	mkdir -p $(BUILD_DIR)
	touch $(BUILD_DIR)/.dir

$(BUILD_DIR)/scheduler: $(CORE_SRC) $(HEADERS) src/main.cpp | $(BUILD_DIR)/.dir
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(CORE_SRC) src/main.cpp -o $@

$(BUILD_DIR)/scheduler_tests: $(CORE_SRC) $(HEADERS) tests/test_main.cpp | $(BUILD_DIR)/.dir
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(CORE_SRC) tests/test_main.cpp -o $@

run: $(BUILD_DIR)/scheduler
	./$(BUILD_DIR)/scheduler --input ./data --output ./results

test: $(BUILD_DIR)/scheduler_tests
	./$(BUILD_DIR)/scheduler_tests

clean:
	rm -rf $(BUILD_DIR) results/*.csv
