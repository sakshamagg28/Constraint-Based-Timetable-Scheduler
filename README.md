# Constraint-Based Timetable Scheduler

A production-quality university timetable scheduling engine written in modern C++17. The project models timetable generation as a Constraint Satisfaction Problem (CSP), builds a conflict graph, applies graph-coloring ideas, and uses recursive backtracking with practical heuristics to generate conflict-free weekly schedules.

## Project Overview

The scheduler assigns courses to instructors, rooms, student groups, and time slots while satisfying hard constraints and optimizing timetable quality with weighted soft constraints. It reads CSV input, validates every assignment, prints readable terminal tables, exports CSV schedules, and includes tests plus GitHub Actions CI.

This project is designed to demonstrate:

- Graph algorithms and conflict graph modeling
- Graph coloring concepts
- CSP search with recursive backtracking
- Forward checking and domain pruning
- MRV, degree, and least-constraining-value heuristics
- Schedule validation and quality scoring
- C++17 object-oriented engineering practices

## Problem Statement

Universities need weekly timetables that respect instructors, rooms, student groups, capacities, lab requirements, room types, and availability. Manual scheduling is slow and error-prone because every new class placement can create hidden conflicts. This project automates that process by turning each required class event into a CSP variable and searching for assignments that satisfy all hard constraints.

## Features

- CSV-driven input for courses, instructors, rooms, student groups, time slots, and constraints
- Strong domain models: `Course`, `Instructor`, `Room`, `StudentGroup`, `TimeSlot`, `Assignment`, `Schedule`, `Scheduler`
- Conflict graph construction for course conflicts
- CSP search with:
  - recursive backtracking
  - forward checking
  - dynamic legal-value filtering
  - MRV event selection
  - degree heuristic tie-breaking
  - least-constraining-value ordering
  - early failure detection
- Dedicated validation engine
- Weighted soft-constraint quality scoring
- Professional terminal tables
- CSV export into `results/`
- Unit-style tests without external dependencies
- CMake and Makefile build flows
- GitHub Actions CI

## System Architecture

```text
+------------------+       +------------------+       +------------------+
| CSV Input Files  | ----> | CsvReader        | ----> | ProblemData      |
+------------------+       +------------------+       +------------------+
                                                            |
                                                            v
+------------------+       +------------------+       +------------------+
| OutputWriter     | <---- | Scheduler        | <---- | ConflictGraph    |
+------------------+       +------------------+       +------------------+
        ^                         |
        |                         v
+------------------+       +------------------+
| CSV Exports      |       | Validator        |
+------------------+       +------------------+
                                  |
                                  v
                           +------------------+
                           | QualityScorer    |
                           +------------------+
```

Core responsibilities:

- `CsvReader`: loads and validates CSV structure.
- `ConflictGraph`: creates course-level graph edges.
- `Scheduler`: creates CSP variables and searches for a feasible assignment.
- `Validator`: enforces all hard constraints.
- `QualityScorer`: computes weighted soft penalties and a final score.
- `OutputWriter`: prints terminal tables and exports CSV files.

## Scheduling Workflow

```text
Load CSV data
    |
Build indexes and constraints
    |
Build conflict graph
    |
Expand each course into lecture/lab events
    |
Generate domains of (instructor, room, timeslot)
    |
Backtracking CSP search
    |
Validate final schedule
    |
Score timetable quality
    |
Print tables and export CSV files
```

## Conflict Graph Explanation

Every course is represented as a node. An undirected edge means two courses should not be scheduled at the same time.

Edges are created when:

- courses belong to the same student group
- courses share eligible instructors
- `constraints.csv` contains an `EXPLICIT_CONFLICT`

Example:

```text
CS101 ----- CS102
  |           |
AI401       SE301
```

The graph helps guide search. It is not the only source of truth. The validator still checks every concrete assignment against all room, instructor, group, capacity, room type, availability, blocked slot, and completeness rules.

## Graph Coloring Explanation

Graph coloring assigns colors to graph nodes so adjacent nodes do not share the same color. In this scheduler, time slots play the role of colors. If two courses have a conflict edge, their events should not be placed in incompatible simultaneous assignments.

The implementation goes beyond simple coloring because each event must also choose:

- one valid instructor
- one valid room
- one valid time slot

So the solver treats graph coloring as a guiding model inside a richer CSP.

## CSP Modeling Explanation

The CSP is modeled as:

- Variables: required course events, such as `CS101_LEC_1` or `CS101_LAB_1`
- Domains: valid triples of `(instructor, room, timeslot)`
- Hard constraints: rules that must never be violated
- Soft constraints: weighted preferences used to score schedule quality

Each course is expanded into exactly the required number of lecture and lab events, which guarantees weekly lecture and lab counts are satisfied exactly when every event is assigned.

## Hard Constraints

The validator enforces:

- instructor cannot teach multiple classes simultaneously
- room cannot host multiple classes simultaneously
- student group cannot attend multiple classes simultaneously
- room capacity must cover enrolled students
- room type must match lecture or lab requirement
- instructor availability must be respected
- weekly lecture count is exact
- weekly lab count is exact
- courses use only eligible instructors
- blocked time slots are forbidden
- every event has exactly one room
- every event has exactly one valid time slot

## Soft Constraints

The quality scorer applies weighted penalties for:

- instructor idle gaps
- student idle gaps
- late-evening slots
- repeated same-course placements on one day
- instructor room switching
- student room switching
- room utilization imbalance
- isolated single-class days
- missing instructor preferred time slots

The final score is reported on a 0 to 100 scale.

## Backtracking Explanation

The scheduler recursively assigns one event at a time. At each step:

1. Choose the most constrained unscheduled event.
2. Compute legal values using the validator.
3. Order candidate values by least-constraining behavior and local preference penalty.
4. Place a candidate assignment.
5. Run forward checking.
6. Recurse.
7. If the branch fails, remove the assignment and try the next value.

This guarantees that any returned timetable satisfies hard constraints.

## MRV Heuristic Explanation

Minimum Remaining Values (MRV) selects the unscheduled event with the smallest number of legal assignments under the current partial schedule.

This catches difficult choices early. For example, a lab event with one available lab room and a limited instructor should be scheduled before a lecture with many room and instructor options.

## Degree Heuristic Explanation

When MRV produces a tie, the solver chooses the event whose course has the highest conflict graph degree. Higher-degree courses constrain more neighbors, so scheduling them earlier usually reduces wasted search.

## Least Constraining Value Explanation

For a selected event, candidate assignments are ordered by how much flexibility they leave for neighboring events. A candidate that preserves more future legal values is tried first.

The solver also considers local soft preferences, favoring:

- non-late slots
- instructor preferred slots
- spreading repeated meetings across days

## Constraint Validation Process

Validation runs during search and after search.

During search, `Validator::canPlace` checks each candidate assignment against the partial schedule. After search, `validateComplete` verifies that:

- every required event is scheduled
- no hard constraint has been violated
- lecture counts match exactly
- lab counts match exactly

## Search Engine Metrics

The scheduler reports:

- total assignments attempted
- total constraint checks
- backtracking count
- search nodes visited
- dead-end states
- runtime
- final quality score

These metrics make the search behavior visible and useful for debugging or benchmarking.

## Time Complexity Analysis

Let:

- `n` be the number of required events
- `d` be the maximum domain size per event
- `e` be the number of conflict graph edges

Worst-case backtracking complexity is exponential:

```text
O(d^n)
```

The practical search space is reduced by:

- precomputed domains
- hard-constraint validation
- MRV event ordering
- degree heuristic tie-breaking
- least-constraining-value ordering
- forward checking
- early failure detection

Conflict graph construction is:

```text
O(c^2 * i)
```

where `c` is the number of courses and `i` is the average eligible instructor list size.

## Folder Structure

```text
ConstraintBasedTimetableScheduler/
├── include/
├── src/
├── tests/
├── data/
├── results/
├── docs/
├── .github/workflows/
├── CMakeLists.txt
├── Makefile
├── README.md
├── LICENSE
└── .gitignore
```

## Build Instructions

Using CMake:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

Using Make:

```bash
make
```

## Run Instructions

From the project root:

```bash
./build/scheduler --input ./data --output ./results
```

Other supported commands:

```bash
./build/scheduler --input ./data
./build/scheduler --help
make run
```

## Test Instructions

```bash
make test
```

Or:

```bash
ctest --test-dir build --output-on-failure
```

## Sample Input

`data/courses.csv`:

```csv
id,name,room_type,student_group_id,enrolled_students,weekly_lectures,weekly_labs,eligible_instructors
CS101,Programming Fundamentals,Lecture,G1,38,2,1,I1|I2
CS102,Data Structures,Lecture,G1,38,2,1,I2|I3
```

`data/constraints.csv`:

```csv
type,primary,secondary,weight
EXPLICIT_CONFLICT,CS101,AI401,10
BLOCKED_TIMESLOT,CS101,MON_09,10
UNAVAILABLE_TIMESLOT,I1,FRI_15,10
```

## Sample Output

```text
Timetable generated successfully.
Quality score: 75.8 / 100

+-----------------------------+------------------+-----------------+-------------------+
| Course                      | Teacher          | Room            | Time Slot         |
+-----------------------------+------------------+-----------------+-------------------+
| Algorithms (Lecture)        | Dr. Asha Mehta   | Room 201        | Mon 10:00-11:00   |
| Database Systems (Lab)      | Dr. Leena Kapoor | Computing Lab 1 | Wed 09:00-10:00   |
+-----------------------------+------------------+-----------------+-------------------+
```

## Generated CSV Files

The scheduler writes:

- `results/instructor_schedule.csv`
- `results/room_schedule.csv`
- `results/student_schedule.csv`
- `results/full_schedule.csv`

## Example Generated Timetable

After running `make run`, inspect `results/full_schedule.csv` for the complete event-level timetable. Each row contains:

- event id
- course
- event type
- instructor
- student group
- room
- day
- start
- end
- time slot id

## Engineering Decisions

- C++17 standard library only for portability.
- No global mutable state.
- Small classes with focused responsibilities.
- RAII-based file handling through standard streams.
- Const-correct accessors and validation paths.
- Tests use a lightweight assertion harness to avoid external test dependencies.
- CSV format stays simple and human-editable.

## Technologies Used

- C++17
- STL containers and algorithms
- CMake
- Make
- GitHub Actions
- CSV file input/output

## Future Improvements

- Add JSON input support.
- Add configurable soft-constraint weights.
- Support multi-hour classes.
- Add a web UI for timetable visualization.
- Add metaheuristics such as simulated annealing for post-processing optimization.
- Add richer unschedulable explanations with minimal conflicting subsets.
