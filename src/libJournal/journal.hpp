#pragma once

#include <string>
#include <fstream>
#include <string_view>
#include <chrono>
#include <ctime>

enum Importance {
    LOW, MIDDLE, HIGH
};

class Journal {
private:
    std::ofstream file;
    Importance importanceDefault;
    
public:
    Journal() = delete;
    Journal(const std::string &filename, Importance importance);
    
    Journal(const Journal &) = delete;
    Journal& operator=(const Journal &) = delete;
    
    Journal(Journal &&) noexcept = default;
    Journal& operator=(Journal &&) noexcept = default;
    
    void writeToJournal(std::string_view message, Importance importance);
    void changeImportanceDefault(Importance newImportance);
    
    ~Journal() = default;
};