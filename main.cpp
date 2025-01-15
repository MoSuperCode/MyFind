#include <iostream>
#include <unistd.h> // Für getopt
#include <vector>
#include <string>
#include <filesystem> // Für die Dateisuche
#include <string.h>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    bool recursive = false; // -R Option
    bool caseInsensitive = false; // -i Option
    std::vector<std::string> files; // Dateinamen
    std::string searchPath;

    int opt;
    while ((opt = getopt(argc, argv, "Ri")) != -1) {
        switch (opt) {
            case 'R':
                recursive = true;
                break;
            case 'i':
                caseInsensitive = true;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-R] [-i] <searchpath> <filenames...>" << std::endl;
                return 1;
        }
    }

    // Nach den Optionen: Suchpfad und Dateinamen
    if (optind < argc) {
        searchPath = argv[optind++]; // Nächstes Argument als Suchpfad
    } else {
        std::cerr << "Error: Missing search path." << std::endl;
        return 1;
    }

    while (optind < argc) {
        files.push_back(argv[optind++]); // Restliche Argumente als Dateien
    }

    // Ausgabe zur Überprüfung
    std::cout << "Recursive: " << (recursive ? "Yes" : "No") << std::endl;
    std::cout << "Case-Insensitive: " << (caseInsensitive ? "Yes" : "No") << std::endl;
    std::cout << "Search Path: " << searchPath << std::endl;

    // Dateisuche
    try {
    if (recursive) {
        for (const auto& entry : fs::recursive_directory_iterator(searchPath)) {
            if (!entry.is_regular_file()) continue;

            std::string filename = entry.path().filename().string();

            for (const auto& target : files) {
                if ((caseInsensitive && strcasecmp(filename.c_str(), target.c_str()) == 0) ||
                    (!caseInsensitive && filename == target)) {
                    std::cout << "Found: " << filename << " at " << entry.path() << std::endl;
                }
            }
        }
    } else {
        for (const auto& entry : fs::directory_iterator(searchPath)) {
            if (!entry.is_regular_file()) continue;

            std::string filename = entry.path().filename().string();

            for (const auto& target : files) {
                if ((caseInsensitive && strcasecmp(filename.c_str(), target.c_str()) == 0) ||
                    (!caseInsensitive && filename == target)) {
                    std::cout << "Found: " << filename << " at " << entry.path() << std::endl;
                }
            }
        }
    }
} catch (const fs::filesystem_error& e) {
    std::cerr << "Error accessing " << searchPath << ": " << e.what() << std::endl;
    return 1;
}


    return 0;
}
