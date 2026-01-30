#include <iostream>
#include <utility>
#include <thread>
#include <sstream>
#include <functional>
#include <tuple>
#include <cctype>
#include <limits>
#include "../libJournal/journal.hpp"
#include "../threadSafetyQueue/threadQueue.hpp"

ThreadSafetyQueue<std::pair<std::string, Importance>> messageQueue;

std::mutex coutMtx, jrnlMtx;

static inline void printErrorUsage() {
    std::cerr << "Incorrect usage of the app.\nUsage: ./app <filename> <importance by default: LOW/MIDDLE/HIGH>" << std::endl;
}

static inline void printMessageInvite() {
    std::cout << "\nPlease enter the message (or ctrl+d for exit) and importance level.\nMessage: ";    
}

static inline Importance getImportance(std::string_view importanceStr) {
    return importanceStr == "LOW" ? Importance::LOW : 
        (importanceStr == "MIDDLE" ? Importance::MIDDLE : Importance::HIGH);
}

static std::pair<std::string, Importance> parseArgv(const char *argv[]) {
    try {
        std::string filename = argv[1];
        if (filename.empty()) {
            filename.push_back('a');
        }
        if (filename.find(".txt") == std::string::npos) {
            filename.append(".txt");
        }

        std::string importanceStr = argv[2];
        std::transform(importanceStr.begin(), importanceStr.end(), importanceStr.begin(), toupper);
        if (importanceStr != "LOW" && importanceStr != "MIDDLE" && importanceStr != "HIGH") {
            return {};
        }
        
        Importance importance = importanceStr == "LOW" ? Importance::LOW : (importanceStr == "MIDDLE" ? Importance::MIDDLE : Importance::HIGH);

        return {filename, importance};
    }
    catch (const std::exception &err) {
        std::cerr << "Exception caught at parseArgv function: " << err.what() << '\n';
        return {};
    }
    catch (...) {
        std::cerr << "Unknown exception caught at parseArgv function: " << '\n';
        return {};
    }
}

static void threadHandler(Journal &journal) {
    std::pair<std::string, Importance> msg;
    while (true) {
        if (!messageQueue.pop(msg)) {
            break;
        }
        
        auto [message, importance] = msg;

        try {
            std::scoped_lock jrnlLock(jrnlMtx);
            journal.writeToJournal(message, importance);
        }
        catch (const std::exception &err) {
            std::scoped_lock coutLock(coutMtx);
            std::cout << "Caught an exception in threadHandler: " << err.what() << '\n';
        }
        catch (...) {
            std::scoped_lock coutLock(coutMtx);
            std::cout << "Caught an unknown exception in threadHandler: \n";
        }
    }
}

int main(const int argc, const char *argv[]) {
    if (argc != 3) {
        printErrorUsage();
        return -1;
    }

    auto [filename, importance] = parseArgv(argv);
    if (filename.empty()) {
        printErrorUsage();
        return -2;
    }

    std::cout << "Filename: " << filename << "\nImportance by default set to level: " 
        << (importance == Importance::LOW ? "LOW" : (importance == Importance::MIDDLE ? "MIDDLE" : "HIGH"))
        << "\nNOTE: To finish program send CTRL+D (EOF) command at any input.\n"
        << "NOTE2: To change importance level press \"Enter\" without typing anything at \"Importance\" input\n";

    Journal journal(filename, importance);
    std::thread writerThread(threadHandler, std::ref(journal));
    
    {
        std::scoped_lock coutLock(coutMtx);
        printMessageInvite();
    }

    std::string message, importanceStr;
    while (std::getline(std::cin, message)) {
        try {    
            {
                std::scoped_lock coutLock(coutMtx); 
                std::cout << "Importance: ";
            }
            std::getline(std::cin, importanceStr);
            std::transform(importanceStr.begin(), importanceStr.end(), importanceStr.begin(), toupper);
            if (std::cin.eof()) {
                break;
            }
            
            if (importanceStr.empty()) {
                {
                    std::scoped_lock coutLock(coutMtx); 
                    std::cout << "Which importance level do you want to set? Type: LOW, MIDDLE, HIGH or CANCEL to cancel changing\n";
                
                    while (importanceStr != "LOW" && importanceStr != "MIDDLE" && importanceStr != "HIGH" && importanceStr != "CANCEL") {
                        std::cout << "Set importance level: ";
                        std::getline(std::cin, importanceStr);
                        std::transform(importanceStr.begin(), importanceStr.end(), importanceStr.begin(), toupper);
                        if (std::cin.eof()) {
                            break;
                        }
                    }
                }
                if (std::cin.eof()) {
                    break;
                }
                if (importanceStr == "CANCEL") {
                    std::scoped_lock coutLock(coutMtx); 
                    printMessageInvite();
                }
                else {
                    Importance importanceInput = getImportance(importanceStr);
                    std::scoped_lock jrnlcoutLock(jrnlMtx, coutMtx);
                    journal.changeImportanceDefault(importanceInput);
                    std::cout << "\nChanged Importance level to " << importanceStr << '\n';
                    printMessageInvite();
                }
            }
            else if (importanceStr != "LOW" && importanceStr != "MIDDLE" && importanceStr != "HIGH") {
                std::scoped_lock coutLock(coutMtx);   
                std::cout << "\nIncorrect input of Importance. Try again.\n";
                printMessageInvite();
            }
            else {
                Importance importanceInput = getImportance(importanceStr);
                messageQueue.pushToQueue({message, importanceInput});
                std::scoped_lock coutLock(coutMtx);
                printMessageInvite();
            }
        }
        catch (const std::exception &err) {
            std::scoped_lock coutLock(coutMtx);
            std::cout << "Caught an exception: " << err.what() << '\n';
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        catch (...) {
            std::scoped_lock coutLock(coutMtx);
            std::cout << "Caught an unknown exception\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    messageQueue.close();

    if (writerThread.joinable()) {
        writerThread.join();
    }
    
    std::cout << "\nExiting..." << std::endl;

    return 0;
}