# Conflict Graph Construction

The scheduler builds a course-level conflict graph before CSP search starts.

Each course is a node. An undirected edge connects two courses when they should
not occupy the same time slot. The implementation adds an edge for:

- Same student group or section.
- Shared eligible instructor pool, because assigning both to the same instructor
  at the same time may become impossible.
- Explicit `EXPLICIT_CONFLICT` records in `constraints.csv`.

The graph is used in two places:

- Graph coloring intuition: assigning a time slot is analogous to assigning a
  color to an event while adjacent course nodes avoid incompatible colors.
- Degree heuristic: when two unscheduled events have the same number of
  remaining legal values, the scheduler picks the event whose course has higher
  conflict degree.

The actual CSP remains stricter than the graph. The validator still checks room,
instructor, student group, capacity, room type, availability, blocked slot, and
completeness constraints for every concrete assignment.
