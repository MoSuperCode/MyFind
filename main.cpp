#include <iostream>
#include <unistd.h> // Für getopt
#include <vector>
#include <string>

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
    std::cout << "Files to search: ";
    for (const auto& file : files) {
        std::cout << file << " ";
    }
    std::cout << std::endl;

    return 0;
}
