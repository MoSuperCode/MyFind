#include <iostream>
#include <unistd.h> // für getopt: zum Verarbeiten der Kommandozeilenoptionen
#include <vector>   // für die Speicherung der Dateinamen
#include <string>   // für die Arbeit mit Strings
#include <string.h> // für strcasecmp (case-insensitive Vergleich)
#include <filesystem> // für das Arbeiten mit Dateien und Verzeichnissen
#include <sys/types.h> // für fork(), um Prozesse zu erstellen
#include <sys/wait.h>  // für wait(), um Kindprozesse zu überwachen
#include <unistd.h>    // für fork() und _exit()

namespace fs = std::filesystem; // fs isz ein Alias für std::filesystem

int main(int argc, char* argv[]) {
    bool recursive = false;         //  für die rekursive Suche (-R)
    bool caseInsensitive = false;  //  für die Case-Insensitive-Suche (-i)
    std::vector<std::string> files; // Liste der zu suchenden Dateien
    std::string searchPath;         // Der Verzeichnispfad, in dem gesucht wird

    int opt;
    // Verarbeiten der Kommandozeilenoptionen (-R und -i)
    while ((opt = getopt(argc, argv, "Ri")) != -1) {
        switch (opt) {
        case 'R':
            recursive = true; // Rekursive Suche aktivieren
            break;
        case 'i':
            caseInsensitive = true; // Case-Insensitive-Suche aktivieren
            break;
        default:
            // Ausgabe der Usage bei unbekannten Optionen
            std::cerr << "Usage: " << argv[0] << " [-R] [-i] <searchpath> <filenames...>" << std::endl;
            return 1;
        }
    }

    // Überprüfen, ob der Suchpfad angegeben wurde
    if (optind < argc) {
        searchPath = argv[optind++]; // Nächstes Argument als Suchpfad übernehmen
    } else {
        std::cerr << "Error: Missing search path." << std::endl;
        return 1;
    }

    // Restliche Argumente als Dateinamen speichern
    while (optind < argc) {
        files.push_back(argv[optind++]);
    }

    // Ausgabe der Optionen und des Suchpfads zur Überprüfung
    std::cout << "Recursive: " << (recursive ? "Yes" : "No") << std::endl;
    std::cout << "Case-Insensitive: " << (caseInsensitive ? "Yes" : "No") << std::endl;
    std::cout << "Search Path: " << searchPath << std::endl;

    // Schleife für die parallele Suche: Ein Prozess pro Datei
    for (const auto& target : files) {
        pid_t pid = fork(); // Neuen Prozess für die Suche erstellen
        if (pid == -1) {
            std::cerr << "Error: Failed to fork process." << std::endl;
            return 1;
        } else if (pid == 0) {
            // Kindprozess führt die Dateisuche aus
            try {
                // Rekursive oder nicht-rekursive Iteration je nach Option -R
                if (recursive) {
                    // Durchläuft alle Dateien in Verzeichnissen und Unterverzeichnissen
                    for (const auto& entry : fs::recursive_directory_iterator(searchPath)) {
                        if (!entry.is_regular_file()) continue; // Nur reguläre Dateien berücksichtigen

                        // Dateiname des aktuellen Eintrags extrahieren
                        std::string filename = entry.path().filename().string();

                        // Prüfen, ob der Dateiname mit der Ziel-Datei übereinstimmt
                        if ((caseInsensitive && strcasecmp(filename.c_str(), target.c_str()) == 0) ||
                            (!caseInsensitive && filename == target)) {
                            // Ausgabe, wenn die Datei gefunden wurde
                            std::cout << "PID " << getpid() << ": Found " << filename
                                      << " at " << entry.path() << std::endl;
                        }
                    }
                } else {
                    // Durchläuft nur das angegebene Verzeichnis
                    for (const auto& entry : fs::directory_iterator(searchPath)) {
                        if (!entry.is_regular_file()) continue; // Nur reguläre Dateien berücksichtigen

                        // Dateiname des aktuellen Eintrags extrahieren
                        std::string filename = entry.path().filename().string();

                        // Prüfen, ob der Dateiname mit der Ziel-Datei übereinstimmt
                        if ((caseInsensitive && strcasecmp(filename.c_str(), target.c_str()) == 0) ||
                            (!caseInsensitive && filename == target)) {
                            // Ausgabe, wenn die Datei gefunden wurde
                            std::cout << "PID " << getpid() << ": Found " << filename
                                      << " at " << entry.path() << std::endl;
                        }
                    }
                }
            } catch (const fs::filesystem_error& e) {
                // Fehlerbehandlung bei Problemen mit dem Zugriff auf Verzeichnisse
                std::cerr << "PID " << getpid() << ": Error accessing " << searchPath << ": " << e.what() << std::endl;
                _exit(1); // Beende den Kindprozess mit Fehlerstatus
            }
            _exit(0); // Beende den Kindprozess nach erfolgreicher Suche
        }
    }

    // Hauptprozess: Warten auf alle Kindprozesse
    int status;
    while (wait(&status) > 0) {
        if (WIFEXITED(status)) {
            int exitCode = WEXITSTATUS(status);
            if (exitCode != 0) {
                std::cerr << "A child process exited with code " << exitCode << std::endl;
            }
        }
    }

    return 0; // Erfolgreicher Abschluss des Hauptprozesses
}
