#include <iostream>
#include <windows.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <cstdio>
#include <cstdlib>

class GameEntry {
private:
    std::string id;
    std::string name;
    bool dlc;
    bool installed;

public:
    GameEntry() : dlc(false), installed(false) {}
    GameEntry(const std::string& id, const std::string& name, bool dlc, bool installed)
        : id(id), name(name), dlc(dlc), installed(installed) {}

    const std::string& getId() const { return id; }
    const std::string& getName() const { return name; }
    bool isDLC() const { return dlc; }
    bool isInstalled() const { return installed; }

    void setInstalled(bool val) { installed = val; }

    static GameEntry fromLine(const std::string& line) {
        std::istringstream iss(line);
        std::string id, name;
        bool dlc, installed;
        iss >> id >> name >> std::boolalpha >> dlc >> installed;
        return GameEntry(id, name, dlc, installed);
    }

    std::string toLine() const {
        std::ostringstream oss;
        oss << id << " " << name << " " << std::boolalpha << dlc << " " << installed;
        return oss.str();
    }
};

bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

enum class CommandType {
    HELP,
    VIEW,
    INSTALL,
    UNINSTALL,
    PLAY,
    UNKNOWN
};

CommandType parseCommand(const std::string& cmd) {
    if (cmd == "help") return CommandType::HELP;
    if (cmd == "view") return CommandType::VIEW;
    if (cmd == "install") return CommandType::INSTALL;
    if (cmd == "uninstall") return CommandType::UNINSTALL;
    if (cmd == "play") return CommandType::PLAY;
    return CommandType::UNKNOWN;
}

#pragma region Help
void help() {
    UINT oldCPOut = GetConsoleOutputCP();

    SetConsoleOutputCP(CP_UTF8);

    std::cout << "\x1B[93mLibrary \x1B[33mv1.0\n\n\x1B[37m";
    std::cout << "This app is handling the Library the \x1B[93m\"Mist\"\x1B[37m app.\nWith it you can install, uninstall and play the games you own..\n\n";
    
    std::cout << "Usage:\n";
    std::cout << "\x1B[93m   .\\Library.exe\x1B[37m help\n";
    std::cout << u8"     ↪ Shows this.\n\n\n";

    std::cout << "\x1B[93m   .\\Library.exe\x1B[37m view\n";
    std::cout << u8"     ↪ Shows what games are installed from your library.\n\n\n";

    std::cout << "\x1B[93m   .\\Library.exe\x1B[37m install <ID>\n";
    std::cout << u8"     ↪ Install the game with that specified ID if the game is in your library.\n\n\n";

    std::cout << "\x1B[93m   .\\Library.exe\x1B[37m uninstall <ID>\n";
    std::cout << u8"     ↪ Uninstalls the game with that specified ID if the game is installed.\n\n\n";

    std::cout << "\x1B[93m   .\\Library.exe\x1B[37m play <ID>\n";
    std::cout << u8"     ↪ Opens up the game if it's installed.\n\n\n";

    SetConsoleOutputCP(oldCPOut);
}
#pragma endregion

#pragma region View
void view() {
    std::ifstream file("library\\library.txt");
    if (!file.is_open()) {
        std::cerr << "\x1B[31mError\x1B[37m: Failed to open file.\nUse \x1B[93m.\\Engine.exe generate \x1B[37mto generate the prerequisite files. \n\n";
        return;
    }

    std::vector<GameEntry> entries;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        GameEntry entry = GameEntry::fromLine(line);
        if (entry.getId().length() == 9) {
            entries.push_back(entry);
        }
    }

    file.close();

    if (entries.empty()) {
        std::cout << "\x1B[37m[\x1B[93mInfo\x1B[37m] Your library is empty.\n\n";
        return;
    }

    std::sort(entries.begin(), entries.end(), [](const GameEntry& a, const GameEntry& b) {
        return a.getName() < b.getName();
        });

    for (const auto& entry : entries) {
        std::cout << "\x1B[93m" << entry.getId() << "\x1B[96m " << entry.getName() << " "
            << (entry.isInstalled() ? "\x1B[92mINSTALLED" : "\x1B[91mNOT INSTALLED") << "\x1B[37m\n";
    }
    std::cout << "\n";
}
#pragma endregion

#pragma region Install
void Install(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "\x1B[31mError\x1B[37m: No ID provided for install command.\n\n";
        return;
    }

    std::string id = argv[2];
    std::ifstream inFile("library/library.txt");
    if (!inFile.is_open()) {
        std::cerr << "\x1B[31mError\x1B[37m: Could not open \x1B[93mlibrary/library.txt\x1B[37m.\n\n";
        return;
    }

    std::vector<std::string> lines;
    std::string line;
    bool found = false;
    std::string gameName;

    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        std::string gameID, name, dlc, installed;
        if (!(iss >> gameID >> name >> dlc >> installed)) {
            lines.push_back(line);
            continue;
        }

        if (gameID == id) {
            found = true;
            if (installed == "true") {
                std::cout << "\x1B[37m[\x1B[91mWarn\x1B[37m] Game with \x1B[93mID \x1B[33m" << id << "\x1B[37m is already installed.\n\n";
                return;
            }
            installed = "true";
            gameName = name;
            std::ostringstream updatedLine;
            updatedLine << gameID << " " << name << " " << dlc << " " << installed;
            lines.push_back(updatedLine.str());
        }
        else {
            lines.push_back(line);
        }
    }
    inFile.close();

    if (!found) {
        std::cerr << "\x1B[31mError\x1B[37m: Game with ID \x1B[33m" << id << "\x1B[37m not found.\n\n";
        return;
    }

    std::ofstream outFile("library/library.txt");
    if (!outFile.is_open()) {
        std::cerr << "\x1B[31mError\x1B[37m Could not write to \x1B[93mlibrary/library.txt\x1B[37m.\n\n";
        return;
    }

    for (const auto& l : lines) {
        outFile << l << "\n";
    }
    outFile.close();

    std::cout << "\x1B[37m[\x1B[93mInfo\x1B[37m] \x1B[96mlibrary/library.txt\x1B[37m has been updated.\n";

    std::string batFilename = "library/" + gameName + ".bat";
    std::ofstream batFile(batFilename);
    if (!batFile.is_open()) {
        std::cerr << "\x1B[31mError\x1B[37m: Could not create batch file \x1B[93m" << batFilename << "\x1B[37m.\n\n";
        return;
    }

    batFile << "@echo off\n";
    batFile << "mode 100\n";
    batFile << "echo This is the " << gameName << " game.\n";
    batFile << ":1\n";
    batFile << "goto 1\n";
    batFile.close();

    std::cout << "\x1B[37m[\x1B[93mInfo\x1B[37m]: Batch file \x1B[96m" << batFilename << "\x1B[37m has been created.\n";

    std::cout << "\x1B[37m[\x1B[93mInfo\x1B[37m]: Item with the \x1B[93mID\x1B[37m:\x1B[33m"
        << id << "\x1B[37m (Game: \x1B[96m" << gameName
        << "\x1B[37m) has been successfully \x1B[92minstalled\x1B[37m.\n\n";
}
#pragma endregion

#pragma region Uninstall
void uninstall(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "\x1B[31mError\x1B[37m: Missing ID for uninstall command.\n\n";
        return;
    }

    std::string targetID = argv[2];
    std::ifstream infile("library/library.txt");
    if (!infile.is_open()) {
        std::cerr << "\x1B[31mError\x1B[37m: Could not open library\\library.txt\n\n";
        return;
    }

    std::vector<std::string> lines;
    std::string line;
    bool found = false;
    std::string gameName;

    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string id, name, dlcStr, installedStr;
        if (!(iss >> id >> name >> dlcStr >> installedStr)) {
            lines.push_back(line);
            continue;
        }

        if (id == targetID) {
            found = true;
            if (installedStr == "false") {
                std::cerr << "\x1B[37m[\x1B[91mWarn\x1B[37m] Game with \x1B[93mID \x1B[33m" << id << "\x1B[37m is already uninstalled.\n\n";
                return;
            }
            line = id + " " + name + " " + dlcStr + " false";
            gameName = name;
        }

        lines.push_back(line);
    }
    infile.close();

    if (!found) {
        std::cerr << "\x1B[31mError\x1B[37m: No game found with ID \x1B[93m" << targetID << "\x1B[37m.\n\n";
        return;
    }

    std::ofstream outfile("library/library.txt");
    if (!outfile.is_open()) {
        std::cerr << "\x1B[31mError\x1B[37m: Could not write to library\\library.txt\n\n";
        return;
    }

    for (const auto& l : lines) {
        outfile << l << "\n";
    }
    outfile.close();

    std::cout << "\x1B[37m[\x1B[93mInfo\x1B[37m]: Updated library.txt for the game '" << gameName << "\n";

    std::string batPath = "library/" + gameName + ".bat";
    if (fileExists(batPath)) {
        if (std::remove(batPath.c_str()) == 0) {
            std::cout << "\x1B[37m[\x1B[93mInfo\x1B[37m]: Deleted the game files of: " << gameName << ".\n\n";
        }
        else {
            std::cerr << "\x1B[31mError\x1B[37m: Failed to delete batch file " << batPath << ".\n\n";
        }
    }
    else {
        std::cout << "\x1B[37m[\x1B[91mWarn\x1B[37m] Batch file " << batPath << " not found. Nothing to delete.\n\n";
    }
}
#pragma endregion

#pragma region Play
void play(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "\x1B[31mError\x1B[37m: play command requires an ID argument.\n\n";
        return;
    }

    const char* idStr = argv[2];

    long long gameID = 0;
    try {
        gameID = std::stoll(idStr);
    }
    catch (const std::exception& e) {
        std::cerr << "\x1B[31mError\x1B[37m: Invalid ID: " << idStr << "\n\n";
        return;
    }

    const std::string libraryPath = "library\\library.txt";
    std::ifstream file(libraryPath);

    if (!file.is_open()) {
        std::cerr << "\x1B[31mError\x1B[37m: Failed to open " << libraryPath << "\n\n";
        return;
    }

    std::string line;
    bool found = false;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        long long id;
        std::string gameName;
        std::string dlcStr;
        std::string installedStr;

        if (!(iss >> id >> gameName >> dlcStr >> installedStr)) {
            std::cerr << "\x1B[91mWarn\x1B[37m: Malformed line: " << line << "\n\n";
            continue;
        }

        if (id == gameID) {
            found = true;
            if (installedStr == "true") {
                std::string batFilename = "library\\" + gameName + ".bat";
                if (fileExists(batFilename)) {
                    std::string command = "start \"\" \"" + batFilename + "\"";
                    std::system(command.c_str());
                    std::cout << "\x1B[37m[\x1B[93mInfo\x1B[37m]: Launched game \"" << gameName << "\".\n\n";
                }
                else {
                    std::cerr << "\x1B[91mWarn\x1B[37m: Batch file " << batFilename << " not found.\n\n";
                }
            }
            else {
                std::cerr << "[\x1B[91mWarn\x1B[37m] The Game \x1B[96m" << gameName << "\x1B[37m with ID " << gameID << " is not installed.\n\n";
            }
            break;
        }
    }

    if (!found) {
        std::cerr << "\x1B[31mError\x1B[37m: Game with ID " << gameID << " not found in library.\n\n";
    }
}
#pragma endregion

int main(int argc, char* argv[]) {
    if (argc < 2) {
        help();
        return 1;
    }

    std::string cmdStr = argv[1];
    CommandType cmd = parseCommand(cmdStr);

    switch (cmd) {

    case CommandType::HELP:
        help();
        break;

    case CommandType::VIEW:
        view();
        break;

    case CommandType::INSTALL:
        Install(argc, argv);
        break;

    case CommandType::PLAY:
        play(argc, argv);
        break;

    case CommandType::UNINSTALL:
        uninstall(argc, argv);
        break;

    default:
        std::cerr << "\x1B[31mError\x1B[37m: Unknown command: \x1B[93m" << cmdStr << "\x1B[37m\n";
        break;
    }
    return 0;
}