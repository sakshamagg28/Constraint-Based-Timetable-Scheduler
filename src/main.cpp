#include "CsvReader.h"
#include "OutputWriter.h"
#include "Scheduler.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void printHelp() {
    std::cout << "Constraint-Based Timetable Scheduler\n\n"
              << "Usage:\n"
              << "  scheduler --input ./data --output ./results\n"
              << "  scheduler --input ./data\n"
              << "  scheduler --help\n\n"
              << "Options:\n"
              << "  --input   Directory containing courses.csv, instructors.csv, rooms.csv,\n"
              << "            student_groups.csv, timeslots.csv, constraints.csv\n"
              << "  --output  Directory for generated CSV schedules (default: ./results)\n";
}

} // namespace

int main(int argc, char** argv) {
    std::filesystem::path input = "data";
    std::filesystem::path output = "results";

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printHelp();
            return 0;
        }
        if (arg == "--input" && i + 1 < argc) {
            input = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            output = argv[++i];
        } else {
            std::cerr << "Unknown or incomplete option: " << arg << "\n";
            printHelp();
            return 1;
        }
    }

    try {
        cbt::CsvReader reader;
        auto data = reader.loadProblem(input);
        cbt::Scheduler scheduler(data);
        std::cout << "Loaded " << data.courses.size() << " courses, "
                  << data.instructors.size() << " instructors, "
                  << data.rooms.size() << " rooms, "
                  << data.studentGroups.size() << " student groups, and "
                  << data.timeSlots.size() << " time slots.\n";
        std::cout << "Conflict graph: " << scheduler.conflictGraph().nodes().size()
                  << " nodes, " << scheduler.conflictGraph().edgeCount() << " edges.\n";

        auto result = scheduler.solve();
        if (!result.success) {
            std::cerr << "\nUnable to generate a complete timetable.\n";
            for (const auto& diagnostic : result.diagnostics) {
                std::cerr << " - " << diagnostic << "\n";
            }
            return 2;
        }

        std::cout << "\nTimetable generated successfully.\n";
        std::cout << "Quality score: " << result.quality.score
                  << " / 100 (penalty " << result.quality.rawPenalty << ")\n";
        for (const auto& note : result.quality.notes) {
            std::cout << " - " << note << "\n";
        }

        cbt::OutputWriter writer(data);
        writer.printAll(std::cout, result.schedule, result.metrics);
        writer.exportCsv(output, result.schedule);
        std::cout << "\nCSV schedules exported to " << output << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
