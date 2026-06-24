cmake_minimum_required(VERSION 3.14)

project(ConstraintBasedTimetableScheduler
        VERSION 1.0.0
        DESCRIPTION "Constraint-based university timetable scheduler in C++17"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_library(scheduler_core
    src/Domain.cpp
    src/Schedule.cpp
    src/CsvReader.cpp
    src/ConflictGraph.cpp
    src/Validator.cpp
    src/QualityScorer.cpp
    src/Scheduler.cpp
    src/OutputWriter.cpp
)

target_include_directories(scheduler_core PUBLIC include)

if(MSVC)
    target_compile_options(scheduler_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(scheduler_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(scheduler src/main.cpp)
target_link_libraries(scheduler PRIVATE scheduler_core)

enable_testing()

add_executable(scheduler_tests tests/test_main.cpp)
target_link_libraries(scheduler_tests PRIVATE scheduler_core)
add_test(NAME scheduler_tests COMMAND scheduler_tests)
set_tests_properties(scheduler_tests PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
