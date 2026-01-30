#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include "journal.hpp"

Journal::Journal(const std::string &filename, Importance importance) : file(filename), importanceDefault(importance) {
}

void Journal::writeToJournal(std::string_view message, Importance importance) {
    if (importance < importanceDefault) {
        return;
    }

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t tc = std::chrono::system_clock::to_time_t(now);
    std::tm *tmLocal = std::localtime(&tc);

    file << "TEXT: " << message << "\nIMPORTANCE: ";
    switch (importance) {
        case Importance::LOW:
            file << "LOW";
            break;
        case Importance::MIDDLE:
            file << "MIDDLE";
            break;
        case Importance::HIGH:
            file << "HIGH";
            break;
    }
    file << "\nTIME: " << std::put_time(tmLocal, "%d.%m.%Y at %H:%M:%S") << '\n' << std::endl;
}

void Journal::changeImportanceDefault(Importance newImportance) {
    importanceDefault = newImportance;
}