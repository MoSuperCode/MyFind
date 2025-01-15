#include <iostream>
#include <unistd.h> // fÃ¼r getopt
#include <vector>
#include <string>
#include <string.h>
#include <filesystem> // fÃ¼r das filesystem
#include <sys/types.h> // FÃ¼r fork()
#include <sys/wait.h>  // FÃ¼r wait()
#include <unistd.h>    // FÃ¼r fork und _exit

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    bool recursive = false; // -R Option
    bool caseInsensitive = false; // -i Option
    std::vector<std::string> files;
    std::string searchPath;

    int opt;
    //die Optionen z.B. -R -i 
    while((opt = getopt(argc,argv, "Ri")) != -1) {
        switch (opt)
        {
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
    // Suchpfad und Dateinamen
    if (optind < argc) {
    searchPath = argv[optind++];
    } else {
        std::cerr << "Error: Missing search path." << std::endl;
        return 1;
    }

    while (optind < argc)
    {
        files.push_back(argv[optind++]);
    }
     // Ausgabe zur ÃœberprÃ¼fung
    std::cout << "Recursive: " << (recursive ? "Yes" : "No") << std::endl;
    std::cout << "Case-Insensitive: " << (caseInsensitive ? "Yes" : "No") << std::endl;
    std::cout << "Search Path: " << searchPath << std::endl;
    
    //Dateisuche
    for (const auto& target : files) {
    pid_t pid = fork(); // Neuen Prozess erstellen
    if (pid == -1) {
        std::cerr << "Error: Failed to fork process." << std::endl;
        return 1;
    } else if (pid == 0) {
        // Kindprozess
        try {
            if (recursive) {
                for (const auto& entry : fs::recursive_directory_iterator(searchPath)) {
                    if (!entry.is_regular_file()) continue;

                    std::string filename = entry.path().filename().string();
                    if ((caseInsensitive && strcasecmp(filename.c_str(), target.c_str()) == 0) ||
                        (!caseInsensitive && filename == target)) {
                        std::cout << "PID " << getpid() << ": Found " << filename
                                  << " at " << entry.path() << std::endl;
                    }
                }
            } else {
                for (const auto& entry : fs::directory_iterator(searchPath)) {
                    if (!entry.is_regular_file()) continue;

                    std::string filename = entry.path().filename().string();
                    if ((caseInsensitive && strcasecmp(filename.c_str(), target.c_str()) == 0) ||
                        (!caseInsensitive && filename == target)) {
                        std::cout << "PID " << getpid() << ": Found " << filename
                                  << " at " << entry.path() << std::endl;
                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "PID " << getpid() << ": Error accessing " << searchPath << ": " << e.what() << std::endl;
            _exit(1); // Prozess mit Fehler beenden
        }
        _exit(0); // Kindprozess erfolgreich beenden
    }
}

// Warten auf alle Kindprozesse
int status;
while (wait(&status) > 0) {
    if (WIFEXITED(status)) {
        int exitCode = WEXITSTATUS(status);
        if (exitCode != 0) {
            std::cerr << "A child process exited with code " << exitCode << std::endl;
        }
    }
}

    return 0;
}